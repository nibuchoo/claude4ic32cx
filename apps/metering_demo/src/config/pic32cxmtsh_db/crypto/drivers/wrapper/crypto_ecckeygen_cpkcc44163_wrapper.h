/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    crypto_ecckeygen_cpkcc44163_wrapper.h

  Summary:
    Crypto Framework Library wrapper file for ECC key generation in the
    hardware cryptographic library.

  Description:
    This header file contains the wrapper interface to access the hardware
    cryptographic library in Microchip microcontrollers for ECC key 
	generation primitives (random scalar k, public point Q = k * G).
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

#ifndef CRYPTO_ECCKEYGEN_CPKCC44163_WRAPPER_H
#define CRYPTO_ECCKEYGEN_CPKCC44163_WRAPPER_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include "crypto/common_crypto/crypto_common.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

typedef enum
{
    CRYPTO_ECCKEYGEN_SUCCESS = 0,
    CRYPTO_ECCKEYGEN_ERROR_PARAM,
    CRYPTO_ECCKEYGEN_ERROR_LEN,
    CRYPTO_ECCKEYGEN_ERROR_CURVE,
    CRYPTO_ECCKEYGEN_ERROR_FAIL
} crypto_EccKeyGen_Status_E;

// *****************************************************************************
// *****************************************************************************
// Section: ECC Key Generation Common Interface 
// *****************************************************************************
// *****************************************************************************

/* Generate an ECC keypair on the selected NIST P-curve.
 *
 * privKey    [out] - Buffer to receive the private key. Must be at least 
 *                    privKeyLen bytes (one curve coordinate, big-endian).
 * privKeyLen [in]  - Size of the private key buffer. Must equal the curve 
 *                    coordinate size (24 for P-192, 28 for P-224, 32 for 
 *                    P-256, 48 for P-384, 66 for P-521).
 * pubKey     [out] - Buffer to receive the public key as the concatenation 
 *                    X || Y (no 0x04 prefix). Must be at least pubKeyLen
 *                    bytes (two curve coordinates).
 * pubKeyLen  [in]  - Size of the public key buffer. Must equal twice the 
 *                    curve coordinate size.
 * eccCurveType_en  - Curve to use. Supported: CRYPTO_ECC_CURVE_P192, 
 *                    P224, P256, P384, P521.
 *
 * Returns CRYPTO_ECCKEYGEN_SUCCESS on success, or an error code on failure.
 */
crypto_EccKeyGen_Status_E Crypto_Ecc_Hw_KeyGen(uint8_t *privKey, 
    uint32_t privKeyLen, uint8_t *pubKey, uint32_t pubKeyLen, 
    crypto_EccCurveType_E eccCurveType_en);

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END

#endif /* CRYPTO_ECCKEYGEN_CPKCC44163_WRAPPER_H */
