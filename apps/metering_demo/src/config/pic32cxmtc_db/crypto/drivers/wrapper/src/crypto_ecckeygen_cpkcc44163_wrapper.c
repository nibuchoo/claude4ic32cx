/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    crypto_ecc_cpkcc44163_wrapper.c

  Summary:
    Crypto Framework Library wrapper file for ECC key generation operations 
	in the hardware cryptographic library.

  Description:
    This source file contains the wrapper interface to access the hardware
    cryptographic library in Microchip microcontrollers for ECC key 
	generation primitives.
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

#include <stdint.h>
#include "crypto/drivers/wrapper/crypto_ecckeygen_cpkcc44163_wrapper.h"
#include "crypto/drivers/driver/drv_crypto_ecc_hw_cpkcl.h"
#include "crypto/drivers/driver/drv_crypto_ecckeygen_hw_cpkcl.h"

// *****************************************************************************
// *****************************************************************************
// Section: File scope functions
// *****************************************************************************
// *****************************************************************************

static crypto_EccKeyGen_Status_E lCrypto_EccKeyGen_Hw_GetCurve(
    crypto_EccCurveType_E eccCurveType, CRYPTO_CPKCL_CURVE *hwEccCurve, 
    uint32_t *coordSize)
{
    crypto_EccKeyGen_Status_E status = CRYPTO_ECCKEYGEN_SUCCESS;

    switch (eccCurveType)
    {
        case CRYPTO_ECC_CURVE_P192:
            *hwEccCurve = CRYPTO_CPKCL_CURVE_P192;
            *coordSize  = (uint32_t) P192_PUBLIC_KEY_COORDINATE_SIZE;
            break;

        case CRYPTO_ECC_CURVE_P224:
            *hwEccCurve = CRYPTO_CPKCL_CURVE_P224;
            *coordSize  = (uint32_t) P224_PUBLIC_KEY_COORDINATE_SIZE;
            break;

        case CRYPTO_ECC_CURVE_P256:
            *hwEccCurve = CRYPTO_CPKCL_CURVE_P256;
            *coordSize  = (uint32_t) P256_PUBLIC_KEY_COORDINATE_SIZE;
            break;

        case CRYPTO_ECC_CURVE_P384:
            *hwEccCurve = CRYPTO_CPKCL_CURVE_P384;
            *coordSize  = (uint32_t) P384_PUBLIC_KEY_COORDINATE_SIZE;
            break;

        case CRYPTO_ECC_CURVE_P521:
            *hwEccCurve = CRYPTO_CPKCL_CURVE_P521;
            *coordSize  = (uint32_t) P521_PUBLIC_KEY_COORDINATE_SIZE;
            break;

        default:
            status = CRYPTO_ECCKEYGEN_ERROR_CURVE;
            break;
    }

    return status;
}

static crypto_EccKeyGen_Status_E lCrypto_EccKeyGen_Hw_MapKeyGenResult(
    CRYPTO_ECCKEYGEN_RESULT result)
{
    crypto_EccKeyGen_Status_E status;

    switch (result)
    {
        case CRYPTO_ECCKEYGEN_RESULT_SUCCESS:
            status = CRYPTO_ECCKEYGEN_SUCCESS;
            break;

        case CRYPTO_ECCKEYGEN_RESULT_ERROR_CURVE:
            status = CRYPTO_ECCKEYGEN_ERROR_CURVE;
            break;

        case CRYPTO_ECCKEYGEN_RESULT_INIT_FAIL:
        case CRYPTO_ECCKEYGEN_RESULT_ERROR_FAIL:
        default:
            status = CRYPTO_ECCKEYGEN_ERROR_FAIL;
            break;
    }

    return status;
}

// *****************************************************************************
// *****************************************************************************
// Section: ECC Common Interface Implementation
// *****************************************************************************
// *****************************************************************************

crypto_EccKeyGen_Status_E Crypto_Ecc_Hw_KeyGen(uint8_t *privKey, 
    uint32_t privKeyLen, uint8_t *pubKey, uint32_t pubKeyLen, 
    crypto_EccCurveType_E eccCurveType_en)
{
    crypto_EccKeyGen_Status_E result;
    CRYPTO_ECCKEYGEN_RESULT hwResult;
    CPKCL_ECC_DATA eccData;
    CRYPTO_CPKCL_CURVE hwEccCurve;
    uint32_t coordSize;

    /* Parameter checks */
    if ((privKey == NULL) || (pubKey == NULL))
    {
        return CRYPTO_ECCKEYGEN_ERROR_PARAM;
    }

    /* Get curve and coord size */
    result = lCrypto_EccKeyGen_Hw_GetCurve(eccCurveType_en, &hwEccCurve, &coordSize);
    if (result != CRYPTO_ECCKEYGEN_SUCCESS)
    {
        return result;
    }

    /* Validate buffer sizes against the curve */
    if ((privKeyLen < coordSize) || (pubKeyLen < (2U * coordSize)))
    {
        return CRYPTO_ECCKEYGEN_ERROR_LEN;
    }

	/* Initialize the hardware library for key generation */
    hwResult = DRV_CRYPTO_ECCKEYGEN_InitEccParams(&eccData, hwEccCurve);
    if (hwResult != CRYPTO_ECCKEYGEN_RESULT_SUCCESS)
    {
        return lCrypto_EccKeyGen_Hw_MapKeyGenResult(hwResult);
    }

    /* Generate the keypair. Public key is laid out as X || Y in pubKey. */
    hwResult = DRV_CRYPTO_ECCKEYGEN_Generate(&eccData, (pfu1) privKey, 
                                             (pfu1) pubKey, 
                                             (pfu1) &pubKey[coordSize]);

    return lCrypto_EccKeyGen_Hw_MapKeyGenResult(hwResult);
}
