/**************************************************************************
  Crypto Framework Library Header

  Company:
    Microchip Technology Inc.

  File Name:
    drv_crypto_ecdh_hw_cpkcl.c
  
  Summary:
    Crypto Framework Library source for the CPKCC ECDH functions.

  Description:
    This source contains the function code for the CPKCC ECDH.
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
#include "crypto/drivers/driver/drv_crypto_ecdh_hw_cpkcl.h"
#include "crypto/drivers/cpkcl_lib/cryptolib_typedef_pb.h"
#include "crypto/drivers/cpkcl_lib/cryptolib_mapping_pb.h"
#include "crypto/drivers/cpkcl_lib/cryptolib_headers_pb.h"
#include "crypto/drivers/cpkcl_lib/cryptolib_jumptable_addr_pb.h"

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Variables
// *****************************************************************************
// *****************************************************************************

// All buffers maximum size + 4 header + 2 padding (P-521: u2ModuloPSize=68 > coordSize=66)
static u1 sharedKeyX[P521_PUBLIC_KEY_COORDINATE_SIZE + 6];    
static u1 sharedKeyY[P521_PUBLIC_KEY_COORDINATE_SIZE + 6];

// All buffers maximum size + 4 header + 2 padding
static u1 pubKeyX[P521_PUBLIC_KEY_COORDINATE_SIZE + 6];  
static u1 pubKeyY[P521_PUBLIC_KEY_COORDINATE_SIZE + 6]; 
static u1 privateKey[P521_PUBLIC_KEY_COORDINATE_SIZE + 6];

/* Actual coordinate byte-length (set in InitEccParams, used in GetSharedKey) */
static u2 s_coordSize;

// *****************************************************************************
// *****************************************************************************
// Section: CPKCL ECDH Common Interface Implementation
// *****************************************************************************
// *****************************************************************************

CRYPTO_ECDH_RESULT DRV_CRYPTO_ECDH_InitEccParams(CPKCL_ECC_DATA *pEccData, 
    pfu1 privKey, u4 privKeyLen, pfu1 pubKey, CRYPTO_CPKCL_CURVE eccCurveType)
{
    CRYPTO_CPKCL_RESULT result;
    
    /* Initialize CPKCL */
    result = DRV_CRYPTO_ECC_InitCpkcl();
    if (result != CRYPTO_CPKCL_RESULT_INIT_SUCCESS) 
    {
        return CRYPTO_ECDH_RESULT_INIT_FAIL;
    }
    
    /* Fill curve parameters */
    result = DRV_CRYPTO_ECC_InitCurveParams(pEccData, eccCurveType);
    if (result != CRYPTO_CPKCL_RESULT_CURVE_SUCCESS) 
    {
        return CRYPTO_ECDH_RESULT_ERROR_CURVE;
    }
    
    /* Get coordinates of public key */
    (void) memset(pubKeyX, 0, sizeof(pubKeyX));
    (void) memset(pubKeyY, 0, sizeof(pubKeyY));
    result = DRV_CRYPTO_ECC_SetPubKeyCoordinates(pEccData, pubKey, &pubKeyX[4], 
                                                 &pubKeyY[4], eccCurveType);
    if (result == CRYPTO_CPKCL_RESULT_CURVE_ERROR)
    {
        return CRYPTO_ECDH_RESULT_ERROR_CURVE;
    }
    else if (result == CRYPTO_CPKCL_RESULT_COORD_COMPRESS_ERROR) 
    {
        return CRYPTO_ECDH_ERROR_PUBKEYCOMPRESS;
    }
    else 
    {
        // Successful - continue
    }
    
    pEccData->pfu1PublicKeyX = (pfu1) pubKeyX;
    pEccData->pfu1PublicKeyY = (pfu1) pubKeyY;
    
    /* Remember the actual coordinate size for shared secret extraction */
    s_coordSize = (u2)privKeyLen;
    
    /* Store private key locally, right-justified within the u2OrderSize field.
     * For P-521: privKeyLen=66, u2OrderSize=68, so pad 2 zero bytes after header. */
	u2 padLen;
    (void) memset(privateKey, 0, sizeof(privateKey));
    padLen = pEccData->u2OrderSize - (u2)privKeyLen;
    (void) memcpy(&privateKey[4U + padLen], privKey, privKeyLen);
    pEccData->pfu1PrivateKey = (pfu1) privateKey;
    
    return CRYPTO_ECDH_RESULT_SUCCESS;
}

CRYPTO_ECDH_RESULT DRV_CRYPTO_ECDH_GetSharedKey(CPKCL_ECC_DATA *pEccData, 
    pfu1 sharedKey)
{
    /* Set sizes */
    u2 u2ModuloPSize = pEccData->u2ModuloPSize;
    u2 u2OrderSize = pEccData->u2OrderSize;

    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 10.1, 10.4, 10.8, 20.7 deviated below. Deviation record ID - 
       H3_MISRAC_2012_R_10_1_DR_1 & H3_MISRAC_2012_R_10_4_DR_1 & H3_MISRAC_2012_R_10_8_DR_1 & H3_MISRAC_2012_R_20_7_DR_1 */
    /* Copy parameters for ECDH in memory areas */
    DRV_CRYPTO_ECC_SecureCopy(
        (pu1) ((BASE_SCA_MUL_MODULO(u2ModuloPSize, u2OrderSize))), 
        pEccData->pfu1ModuloP, u2ModuloPSize + 4U);
    DRV_CRYPTO_ECC_SecureCopy(
        (pu1) ((BASE_SCA_MUL_CNS(u2ModuloPSize, u2OrderSize))), 
        pEccData->pfu1Cns, u2ModuloPSize + 8U);
    DRV_CRYPTO_ECC_SecureCopy(
        (pu1) ((BASE_SCA_MUL_SCALAR(u2ModuloPSize, u2OrderSize))), 
        pEccData->pfu1PrivateKey, u2OrderSize + 4U);
    DRV_CRYPTO_ECC_SecureCopy(
        (pu1) ((BASE_SCA_MUL_POINT_A_X(u2ModuloPSize, u2OrderSize))), 
        pEccData->pfu1PublicKeyX, u2ModuloPSize + 4U);
    DRV_CRYPTO_ECC_SecureCopy(
        (pu1) ((BASE_SCA_MUL_POINT_A_Y(u2ModuloPSize, u2OrderSize))), 
        pEccData->pfu1PublicKeyY, u2ModuloPSize + 4U);
    DRV_CRYPTO_ECC_SecureCopy(
        (pu1) ((BASE_SCA_MUL_POINT_A_Z(u2ModuloPSize, u2OrderSize))), 
        pEccData->pfu1PublicKeyZ, u2ModuloPSize + 4U);
    DRV_CRYPTO_ECC_SecureCopy(
        (pu1) ((BASE_SCA_MUL_A(u2ModuloPSize, u2OrderSize))), 
        pEccData->pfu1ACurve, u2ModuloPSize + 4U);
    DRV_CRYPTO_ECC_SecureCopy(
        (pu1) ((BASE_SCA_MUL_ORDER(u2ModuloPSize, u2OrderSize))), 
        pEccData->pfu1APointOrder, u2OrderSize + 4U);

    /* Ask for a key generation */
    CPKCL_ZpEccMul(nu1ModBase) = (nu1) BASE_SCA_MUL_MODULO(u2ModuloPSize, 
        u2OrderSize);
    CPKCL_ZpEccMul(nu1CnsBase) = (nu1) BASE_SCA_MUL_CNS(u2ModuloPSize, 
        u2OrderSize);
    CPKCL_ZpEccMul(nu1PointBase) = (nu1) BASE_SCA_MUL_POINT_A(u2ModuloPSize, 
        u2OrderSize);
    CPKCL_ZpEccMul(nu1ABase) = (nu1) BASE_SCA_MUL_A(u2ModuloPSize, 
        u2OrderSize);
    CPKCL_ZpEccMul(nu1Workspace) = (nu1) BASE_SCA_MUL_WORKSPACE(u2ModuloPSize, 
        u2OrderSize);
    CPKCL_ZpEccMul(nu1KBase) = (nu1) BASE_SCA_MUL_SCALAR(u2ModuloPSize, 
        u2OrderSize);
    /* MISRA C-2012 deviation block end */
    CPKCL_ZpEccMul(u2ModLength) = u2ModuloPSize;
    CPKCL_ZpEccMul(u2KLength) = u2ModuloPSize;
	
    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 10.1, 11.1, 20.7 deviated below. Deviation record ID - 
       H3_MISRAC_2012_R_10_1_DR_1 & H3_MISRAC_2012_R_11_1_DR_1 & H3_MISRAC_2012_R_20_7_DR_1 */
    /* Launch the key generation */
    /* See CPKCL_Rc_pb.h for possible u2Status Values */
    vCPKCL_Process(ZpEccMulFast, pvCPKCLParam);
    /* MISRA C-2012 deviation block end */
    if (CPKCL(u2Status) != (unsigned)CPKCL_OK)
    {
        return CRYPTO_ECDH_RESULT_ERROR_FAIL;
    }
	
    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 10.1, 10.4, 10.8, 20.7 deviated below. Deviation record ID - 
       H3_MISRAC_2012_R_10_1_DR_1 & H3_MISRAC_2012_R_10_4_DR_1 & H3_MISRAC_2012_R_10_8_DR_1 & H3_MISRAC_2012_R_20_7_DR_1 */
    /* Ask to convert coordinates */
    CPKCL_ZpEcConvProjToAffine(nu1ModBase) = (nu1) BASE_ECDSAV_MODULO(
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
    /* Launch the coordinates conversion */
    /* See CPKCL_Rc_pb.h for possible u2Status Values */
    vCPKCL_Process(ZpEcConvProjToAffine, pvCPKCLParam);
    /* MISRA C-2012 deviation block end */
    if (CPKCL(u2Status) != (unsigned)CPKCL_OK)
    {
        return CRYPTO_ECDH_RESULT_ERROR_FAIL;
    }
	
    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 10.1, 10.4, 11.1, 20.7 deviated below. Deviation record ID - 
       H3_MISRAC_2012_R_10_1_DR_1 & H3_MISRAC_2012_R_10_4_DR_1 & H3_MISRAC_2012_R_20_7_DR_1 */
    /* Copy the result */
    DRV_CRYPTO_ECC_SecureCopy(sharedKeyX,
        (pu1) ((BASE_SCA_MUL_POINT_A(u2ModuloPSize, u2OrderSize))),
                u2ModuloPSize + 4U);
    DRV_CRYPTO_ECC_SecureCopy(sharedKeyY,
        (pu1) ((BASE_SCA_MUL_POINT_A(u2ModuloPSize, u2OrderSize))) 
                + u2ModuloPSize + 4U, u2ModuloPSize + 4u);  
    /* MISRA C-2012 deviation block end */

    /* Copy the shared secret (X coordinate of the result point).
     * SecureCopy reverses from CryptoRAM little-endian into sharedKeyX as
     * big-endian within u2ModuloPSize bytes starting at offset 4.
     * For P-521 the actual coordinate is 66 bytes right-justified within the
     * 68-byte field, so skip the leading padding bytes (padLen = 68 - 66 = 2). */
	u2 padLen;
    padLen = u2ModuloPSize - s_coordSize;
    (void) memcpy(sharedKey, &sharedKeyX[4U + padLen], s_coordSize);

    return CRYPTO_ECDH_RESULT_SUCCESS;
}
