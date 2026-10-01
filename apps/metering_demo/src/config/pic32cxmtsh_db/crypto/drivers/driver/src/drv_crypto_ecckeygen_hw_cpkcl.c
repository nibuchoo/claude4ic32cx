/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    drv_crypto_ecckeygen_hw_cpkcl.c
  
  Summary:
    Crypto Framework Library source for the CPKCC ECC key generation 
    function.

  Description:
    This source contains the function code for the CPKCC ECC key generation
    primitive. The flow is:
      1. Generate a random scalar k via the on-chip TRNG (CPKCL_Rng).
      2. Reduce k modulo the curve order n (CPKCL_Div).
      3. Compute the public point Q = k * G via scalar multiplication of the 
         curve base point G by k (CPKCL_ZpEccMulFast).
      4. Convert Q from projective to affine coordinates 
         (CPKCL_ZpEcConvProjToAffine).
      5. Extract the private key (k) and the public point coordinates (Qx, Qy) 
         from CryptoRAM, stripping the 4-byte CPKCC header.
**************************************************************************/

/*******************************************************************************
* Copyright (C) 2026 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <string.h>
#include "crypto/drivers/driver/drv_crypto_ecc_hw_cpkcl.h"
#include "crypto/drivers/driver/drv_crypto_ecckeygen_hw_cpkcl.h"
#include "crypto/drivers/cpkcl_lib/cryptolib_typedef_pb.h"
#include "crypto/drivers/cpkcl_lib/cryptolib_mapping_pb.h"
#include "crypto/drivers/cpkcl_lib/cryptolib_headers_pb.h"
#include "crypto/drivers/cpkcl_lib/cryptolib_jumptable_addr_pb.h"

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Variables
// *****************************************************************************
// *****************************************************************************

/* All buffers maximum size + 4 header + 2 padding (P-521: u2ModuloPSize=68 > coordSize=66) */
static u1 keygenPubKeyX[P521_PUBLIC_KEY_COORDINATE_SIZE + 6];
static u1 keygenPubKeyY[P521_PUBLIC_KEY_COORDINATE_SIZE + 6];
static u1 keygenPrivateKey[P521_PUBLIC_KEY_COORDINATE_SIZE + 6];

/* Actual coordinate byte-length */
static u2 s_coordSize;

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Functions
// *****************************************************************************
// *****************************************************************************

static u2 lDrvCryptoEccKeyGen_GetCoordSize(CRYPTO_CPKCL_CURVE eccCurveType)
{
    u2 coordSize;

    switch (eccCurveType)
    {
        case CRYPTO_CPKCL_CURVE_P192:
            coordSize = (u2) P192_PUBLIC_KEY_COORDINATE_SIZE;
            break;

        case CRYPTO_CPKCL_CURVE_P224:
            coordSize = (u2) P224_PUBLIC_KEY_COORDINATE_SIZE;
            break;

        case CRYPTO_CPKCL_CURVE_P256:
            coordSize = (u2) P256_PUBLIC_KEY_COORDINATE_SIZE;
            break;

        case CRYPTO_CPKCL_CURVE_P384:
            coordSize = (u2) P384_PUBLIC_KEY_COORDINATE_SIZE;
            break;

        case CRYPTO_CPKCL_CURVE_P521:
            coordSize = (u2) P521_PUBLIC_KEY_COORDINATE_SIZE;
            break;

        default:
            coordSize = 0U;
            break;
    }

    return coordSize;
}

// *****************************************************************************
// *****************************************************************************
// Section: CPKCL ECC Key Generation Common Interface Implementation
// *****************************************************************************
// *****************************************************************************

CRYPTO_ECCKEYGEN_RESULT DRV_CRYPTO_ECCKEYGEN_InitEccParams(
    CPKCL_ECC_DATA *pEccData, CRYPTO_CPKCL_CURVE eccCurveType)
{
    CRYPTO_CPKCL_RESULT result;
    u2 coordSize;

    coordSize = lDrvCryptoEccKeyGen_GetCoordSize(eccCurveType);
    if (coordSize == 0U)
    {
        return CRYPTO_ECCKEYGEN_RESULT_ERROR_CURVE;
    }

    /* Initialize CPKCL */
    result = DRV_CRYPTO_ECC_InitCpkcl();
    if (result != CRYPTO_CPKCL_RESULT_INIT_SUCCESS)
    {
        return CRYPTO_ECCKEYGEN_RESULT_INIT_FAIL;
    }

    /* Fill curve parameters */
    result = DRV_CRYPTO_ECC_InitCurveParams(pEccData, eccCurveType);
    if (result != CRYPTO_CPKCL_RESULT_CURVE_SUCCESS)
    {
        return CRYPTO_ECCKEYGEN_RESULT_ERROR_CURVE;
    }

    /* Remember the actual coordinate size for output extraction */
    s_coordSize = coordSize;

    return CRYPTO_ECCKEYGEN_RESULT_SUCCESS;
}

CRYPTO_ECCKEYGEN_RESULT DRV_CRYPTO_ECCKEYGEN_Generate(CPKCL_ECC_DATA *pEccData,
    pfu1 privKey, pfu1 pubKeyX, pfu1 pubKeyY)
{
    /* Set sizes */
    u2 u2ModuloPSize = pEccData->u2ModuloPSize;
    u2 u2OrderSize = pEccData->u2OrderSize;
    u2 padLen;

    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 10.1, 10.4, 10.8, 20.7 deviated below. Deviation record ID - 
       H3_MISRAC_2012_R_10_1_DR_1 & H3_MISRAC_2012_R_10_4_DR_1 & H3_MISRAC_2012_R_10_8_DR_1 & H3_MISRAC_2012_R_20_7_DR_1 */
    /* Step 1 - generate a random scalar k < Modulo. */
    CPKCL_Rng(nu1RBase) = (nu1) BASE_SCA_MUL_SCALAR(u2ModuloPSize, u2OrderSize);
    CPKCL_Rng(u2RLength) = u2OrderSize;
    CPKCL(u2Option) = (u2) CPKCL_RNG_GET;
    /* MISRA C-2012 deviation block end */

    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 10.1, 11.1, 20.7 deviated below. Deviation record ID - 
       H3_MISRAC_2012_R_10_1_DR_1 & H3_MISRAC_2012_R_11_1_DR_1 & H3_MISRAC_2012_R_20_7_DR_1 */
    vCPKCL_Process(Rng, pvCPKCLParam);
    /* MISRA C-2012 deviation block end */
    if (CPKCL(u2Status) != (unsigned)CPKCL_OK)
    {
        return CRYPTO_ECCKEYGEN_RESULT_ERROR_FAIL;
    }

    /* Zero the 4-byte header above the scalar field. */
    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 10.1, 10.4, 10.8, 20.7 deviated below. Deviation record ID - 
       H3_MISRAC_2012_R_10_1_DR_1 & H3_MISRAC_2012_R_10_4_DR_1 & H3_MISRAC_2012_R_10_8_DR_1 & H3_MISRAC_2012_R_20_7_DR_1 */
    ((pu1) ((BASE_SCA_MUL_SCALAR(u2ModuloPSize, u2OrderSize))))[u2OrderSize]      = 0U;
    ((pu1) ((BASE_SCA_MUL_SCALAR(u2ModuloPSize, u2OrderSize))))[u2OrderSize + 1U] = 0U;
    ((pu1) ((BASE_SCA_MUL_SCALAR(u2ModuloPSize, u2OrderSize))))[u2OrderSize + 2U] = 0U;
    ((pu1) ((BASE_SCA_MUL_SCALAR(u2ModuloPSize, u2OrderSize))))[u2OrderSize + 3U] = 0U;

	/* Copies the curve order */
    DRV_CRYPTO_ECC_SecureCopy(
        (pu1) ((BASE_SCA_MUL_ORDER(u2ModuloPSize, u2OrderSize))),
        pEccData->pfu1APointOrder, u2OrderSize + 4U);

	/* Step 2 - reduce k modulo the curve order n so 0 <= k < n. */
	/* (reduce it to make it lower than the order of the points of the curve) */
    CPKCL_Div(nu1NumBase) = (nu1) BASE_SCA_MUL_SCALAR(u2ModuloPSize, u2OrderSize);
    CPKCL_Div(u2NumLength) = u2OrderSize;
    CPKCL_Div(nu1ModBase) = (nu1) BASE_SCA_MUL_ORDER(u2ModuloPSize, u2OrderSize);
    CPKCL_Div(u2ModLength) = u2OrderSize;
    CPKCL_Div(nu1WorkSpace) = (nu1) BASE_SCA_MUL_WORKSPACE(u2ModuloPSize, u2OrderSize);
    CPKCL_Div(nu1RBase) = (nu1) BASE_SCA_MUL_SCALAR(u2ModuloPSize, u2OrderSize);
    CPKCL_Div(nu1QuoBase) = (nu1) 0;
    /* MISRA C-2012 deviation block end */

    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 10.1, 11.1, 20.7 deviated below. Deviation record ID - 
       H3_MISRAC_2012_R_10_1_DR_1 & H3_MISRAC_2012_R_11_1_DR_1 & H3_MISRAC_2012_R_20_7_DR_1 */
    vCPKCL_Process(Div, pvCPKCLParam);
    /* MISRA C-2012 deviation block end */
    if (CPKCL(u2Status) != (unsigned)CPKCL_OK)
    {
        return CRYPTO_ECCKEYGEN_RESULT_ERROR_FAIL;
    }

    /* Step 3 - load curve parameters into CryptoRAM and compute Q = k * G. */
    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 10.1, 10.4, 10.8, 20.7 deviated below. Deviation record ID - 
       H3_MISRAC_2012_R_10_1_DR_1 & H3_MISRAC_2012_R_10_4_DR_1 & H3_MISRAC_2012_R_10_8_DR_1 & H3_MISRAC_2012_R_20_7_DR_1 */
    /* Save the private key (k) before the scalar mult overwrites the scalar field. */
    DRV_CRYPTO_ECC_SecureCopy(keygenPrivateKey,
        (pu1) ((BASE_SCA_MUL_SCALAR(u2ModuloPSize, u2OrderSize))),
        u2OrderSize + 4U);

    /* Copy modulus, reduction constant, base point, curve A, and order. */
    DRV_CRYPTO_ECC_SecureCopy(
        (pu1) ((BASE_SCA_MUL_MODULO(u2ModuloPSize, u2OrderSize))),
        pEccData->pfu1ModuloP, u2ModuloPSize + 4U);
    DRV_CRYPTO_ECC_SecureCopy(
        (pu1) ((BASE_SCA_MUL_CNS(u2ModuloPSize, u2OrderSize))),
        pEccData->pfu1Cns, u2ModuloPSize + 8U);
    DRV_CRYPTO_ECC_SecureCopy(
        (pu1) ((BASE_SCA_MUL_POINT_A_X(u2ModuloPSize, u2OrderSize))),
        pEccData->pfu1APointX, u2ModuloPSize + 4U);
    DRV_CRYPTO_ECC_SecureCopy(
        (pu1) ((BASE_SCA_MUL_POINT_A_Y(u2ModuloPSize, u2OrderSize))),
        pEccData->pfu1APointY, u2ModuloPSize + 4U);
    DRV_CRYPTO_ECC_SecureCopy(
        (pu1) ((BASE_SCA_MUL_POINT_A_Z(u2ModuloPSize, u2OrderSize))),
        pEccData->pfu1APointZ, u2ModuloPSize + 4U);
    DRV_CRYPTO_ECC_SecureCopy(
        (pu1) ((BASE_SCA_MUL_A(u2ModuloPSize, u2OrderSize))),
        pEccData->pfu1ACurve, u2ModuloPSize + 4U);

    /* Configure scalar multiplication: Q = k * G. */
    CPKCL_ZpEccMul(nu1ModBase) = (nu1) BASE_SCA_MUL_MODULO(u2ModuloPSize,
        u2OrderSize);
    CPKCL_ZpEccMul(nu1CnsBase) = (nu1) BASE_SCA_MUL_CNS(u2ModuloPSize,
        u2OrderSize);
    CPKCL_ZpEccMul(nu1PointBase) = (nu1) BASE_SCA_MUL_POINT_A(u2ModuloPSize,
        u2OrderSize);
    CPKCL_ZpEccMul(nu1ABase) = (nu1) BASE_SCA_MUL_A(u2ModuloPSize, u2OrderSize);
    CPKCL_ZpEccMul(nu1KBase) = (nu1) BASE_SCA_MUL_SCALAR(u2ModuloPSize,
        u2OrderSize);
    CPKCL_ZpEccMul(nu1Workspace) = (nu1) BASE_SCA_MUL_WORKSPACE(u2ModuloPSize,
        u2OrderSize);
    /* MISRA C-2012 deviation block end */
    CPKCL_ZpEccMul(u2ModLength) = u2ModuloPSize;
    CPKCL_ZpEccMul(u2KLength) = u2OrderSize;

    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 10.1, 11.1, 20.7 deviated below. Deviation record ID - 
       H3_MISRAC_2012_R_10_1_DR_1 & H3_MISRAC_2012_R_11_1_DR_1 & H3_MISRAC_2012_R_20_7_DR_1 */
    vCPKCL_Process(ZpEccMulFast, pvCPKCLParam);
    /* MISRA C-2012 deviation block end */
    if (CPKCL(u2Status) != (unsigned)CPKCL_OK)
    {
        return CRYPTO_ECCKEYGEN_RESULT_ERROR_FAIL;
    }

    /* Step 4 - convert the projective result to affine coordinates. */
    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 10.1, 10.4, 10.8, 20.7 deviated below. Deviation record ID - 
       H3_MISRAC_2012_R_10_1_DR_1 & H3_MISRAC_2012_R_10_4_DR_1 & H3_MISRAC_2012_R_10_8_DR_1 & H3_MISRAC_2012_R_20_7_DR_1 */
    CPKCL_ZpEcConvProjToAffine(nu1ModBase) = (nu1) BASE_SCA_MUL_MODULO(
        u2ModuloPSize, u2OrderSize);
    CPKCL_ZpEcConvProjToAffine(nu1CnsBase) = (nu1) BASE_SCA_MUL_CNS(
        u2ModuloPSize, u2OrderSize);
    CPKCL_ZpEcConvProjToAffine(nu1PointABase) = (nu1) BASE_SCA_MUL_POINT_A(
        u2ModuloPSize, u2OrderSize);
    CPKCL_ZpEcConvProjToAffine(u2ModLength) = u2ModuloPSize;
    CPKCL_ZpEcConvProjToAffine(nu1Workspace) = (nu1) BASE_SCA_MUL_WORKSPACE(
        u2ModuloPSize, u2OrderSize);
    /* MISRA C-2012 deviation block end */

    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 10.1, 11.1, 20.7 deviated below. Deviation record ID - 
       H3_MISRAC_2012_R_10_1_DR_1 & H3_MISRAC_2012_R_11_1_DR_1 & H3_MISRAC_2012_R_20_7_DR_1 */
    vCPKCL_Process(ZpEcConvProjToAffine, pvCPKCLParam);
    /* MISRA C-2012 deviation block end */
    if (CPKCL(u2Status) != (unsigned)CPKCL_OK)
    {
        return CRYPTO_ECCKEYGEN_RESULT_ERROR_FAIL;
    }

    /* Step 5 - extract the public point coordinates from CryptoRAM. */
    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 10.1, 10.4, 11.1, 20.7 deviated below. Deviation record ID - 
       H3_MISRAC_2012_R_10_1_DR_1 & H3_MISRAC_2012_R_10_4_DR_1 & H3_MISRAC_2012_R_20_7_DR_1 */
    DRV_CRYPTO_ECC_SecureCopy(keygenPubKeyX,
        (pu1) ((BASE_SCA_MUL_POINT_A(u2ModuloPSize, u2OrderSize))),
        u2ModuloPSize + 4U);
    DRV_CRYPTO_ECC_SecureCopy(keygenPubKeyY,
        (pu1) ((BASE_SCA_MUL_POINT_A(u2ModuloPSize, u2OrderSize)))
            + u2ModuloPSize + 4U, u2ModuloPSize + 4U);
    /* MISRA C-2012 deviation block end */

    /* SecureCopy reverses CryptoRAM little-endian into the local buffer as
     * big-endian within u2ModuloPSize bytes starting at offset 4. For P-521 the
     * actual coordinate is 66 bytes right-justified within the 68-byte field,
     * so skip the leading padding bytes (padLen = 68 - 66 = 2). For other 
     * curves padLen = 0. The same logic applies to the private key, which is
     * right-justified within u2OrderSize. */
    padLen = u2ModuloPSize - s_coordSize;
    (void) memcpy(pubKeyX, &keygenPubKeyX[4U + padLen], s_coordSize);
    (void) memcpy(pubKeyY, &keygenPubKeyY[4U + padLen], s_coordSize);

    padLen = u2OrderSize - s_coordSize;
    (void) memcpy(privKey, &keygenPrivateKey[4U + padLen], s_coordSize);

    return CRYPTO_ECCKEYGEN_RESULT_SUCCESS;
}
