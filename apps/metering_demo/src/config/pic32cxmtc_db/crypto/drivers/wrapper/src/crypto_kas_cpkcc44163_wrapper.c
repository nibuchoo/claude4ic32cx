/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    crypto_kas_cpkcc44163_wrapper.c

  Summary:
    Crypto Framework Library wrapper file for the key agreement in the 
    hardware cryptographic library.

  Description:
    This source file contains the wrapper interface to access the hardware 
    cryptographic library in Microchip microcontrollers for key agreement.
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
#include "crypto/drivers/wrapper/crypto_kas_cpkcc44163_wrapper.h"
#include "crypto/drivers/driver/drv_crypto_ecc_hw_cpkcl.h"
#include "crypto/drivers/driver/drv_crypto_ecdh_hw_cpkcl.h"

// *****************************************************************************
// *****************************************************************************
// Section: File scope functions
// *****************************************************************************
// *****************************************************************************

static crypto_Kas_Status_E lCrypto_Kas_Ecdh_Hw_GetCurve(
    crypto_EccCurveType_E eccCurveType, CRYPTO_CPKCL_CURVE *hwEccCurve)
{
    crypto_Kas_Status_E kasStatus = CRYPTO_KAS_SUCCESS;
    
    switch (eccCurveType)
    {     
        case CRYPTO_ECC_CURVE_P192:
            *hwEccCurve = CRYPTO_CPKCL_CURVE_P192;
            break;
        
        case CRYPTO_ECC_CURVE_P224:
            *hwEccCurve = CRYPTO_CPKCL_CURVE_P224;
            break;
        
        case CRYPTO_ECC_CURVE_P256:
            *hwEccCurve = CRYPTO_CPKCL_CURVE_P256;
            break;
        
        case CRYPTO_ECC_CURVE_P384:
            *hwEccCurve = CRYPTO_CPKCL_CURVE_P384;
            break;
        
        case CRYPTO_ECC_CURVE_P521:
            *hwEccCurve = CRYPTO_CPKCL_CURVE_P521;
            break;
        
        default:
            kasStatus = CRYPTO_KAS_ERROR_CURVE;
            break;
    }
    
    return kasStatus;
}    

static crypto_Kas_Status_E lCrypto_Kas_Ecdh_Hw_MapResult(
    CRYPTO_ECDH_RESULT result)
{
    crypto_Kas_Status_E kasStatus;
    
    switch (result) 
    {
        case CRYPTO_ECDH_RESULT_SUCCESS:
            kasStatus = CRYPTO_KAS_SUCCESS;
            break;
            
        case CRYPTO_ECDH_RESULT_ERROR_CURVE:
            kasStatus = CRYPTO_KAS_ERROR_CURVE;
            break;
        
        case CRYPTO_ECDH_RESULT_INIT_FAIL:
        case CRYPTO_ECDH_RESULT_ERROR_FAIL:
            kasStatus = CRYPTO_KAS_ERROR_FAIL;
            break;
            
        default:
            kasStatus = CRYPTO_KAS_ERROR_FAIL;
            break;
    }
    
    return kasStatus;
}

// *****************************************************************************
// *****************************************************************************
// Section: Kas Common Interface Implementation
// *****************************************************************************
// *****************************************************************************

crypto_Kas_Status_E Crypto_Kas_Ecdh_Hw_SharedSecret(uint8_t *privKey, 
    uint32_t privKeyLen, uint8_t *pubKey, uint32_t pubKeyLen, 
    uint8_t *secret, uint32_t secretLen, crypto_EccCurveType_E eccCurveType_en)
{
    crypto_Kas_Status_E result;
    CRYPTO_ECDH_RESULT hwResult;
    CPKCL_ECC_DATA eccData;
    CRYPTO_CPKCL_CURVE hwEccCurve;

    /* Get curve */
    result = lCrypto_Kas_Ecdh_Hw_GetCurve(eccCurveType_en, &hwEccCurve);
    if (result != CRYPTO_KAS_SUCCESS)
    {
        return result;
    }
     
    hwResult = DRV_CRYPTO_ECDH_InitEccParams(&eccData, privKey, (u4) privKeyLen,
                                             pubKey, hwEccCurve);
    if (hwResult != CRYPTO_ECDH_RESULT_SUCCESS) 
    {
        return lCrypto_Kas_Ecdh_Hw_MapResult(hwResult);
        
    }
    
    /* Get shared key */
    hwResult = DRV_CRYPTO_ECDH_GetSharedKey(&eccData, (pfu1)secret);

    return lCrypto_Kas_Ecdh_Hw_MapResult(hwResult);     
}
