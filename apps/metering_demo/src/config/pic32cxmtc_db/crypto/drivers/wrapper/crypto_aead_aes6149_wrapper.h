/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    crypto_aead_aes6149_wrapper..h

  Summary:
    Crypto Framework Library wrapper file for hardware AES.

  Description:
    This header file contains the wrapper interface to access the AEAD 
    algorithms in the AES hardware driver for Microchip microcontrollers.
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

#ifndef CRYPTO_AEAD_AES6149_WRAPPER_H
#define CRYPTO_AEAD_AES6149_WRAPPER_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include "crypto/common_crypto/crypto_common.h"
#include "crypto/common_crypto/crypto_aead_cipher.h"

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

typedef struct 
{
    uint32_t key[16];
    uint32_t calculatedIv[4];  
    uint32_t intermediateHash[4];
    uint32_t H[4];
    uint32_t invokeCtr[2];
    uint32_t totalDataLen;
    uint32_t totalAadLen;
} CRYPTO_GCM_HW_CONTEXT;

// *****************************************************************************
// *****************************************************************************
// Section: AEAD Algorithms Common Interface 
// *****************************************************************************
// *****************************************************************************

crypto_Aead_Status_E Crypto_Aead_Hw_AesGcm_Init(void *gcmInitCtx,
    crypto_CipherOper_E cipherOper_en, uint8_t *key, uint32_t keyLen);
    
crypto_Aead_Status_E Crypto_Aead_Hw_AesGcm_Cipher(void *gcmCipherCtx,  
    uint8_t *initVect, uint32_t initVectLen, uint8_t *inputData,uint32_t dataLen, 
    uint8_t *outData, uint8_t *aad, uint32_t aadLen, uint8_t *authTag, 
    uint32_t authTagLen);
 
crypto_Aead_Status_E Crypto_Aead_Hw_AesGcm_EncryptAuthDirect(uint8_t *inputData, 
    uint32_t dataLen, uint8_t *outData, uint8_t *key, uint32_t keyLen, 
    uint8_t *initVect, uint32_t initVectLen, uint8_t *aad, uint32_t aadLen, 
    uint8_t *authTag, uint32_t authTagLen);
 
crypto_Aead_Status_E Crypto_Aead_Hw_AesGcm_DecryptAuthDirect(uint8_t *inputData, 
    uint32_t dataLen, uint8_t *outData, uint8_t *key, uint32_t keyLen, 
    uint8_t *initVect, uint32_t initVectLen, uint8_t *aad, uint32_t aadLen, 
    uint8_t *authTag, uint32_t authTagLen);

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END

#endif /* CRYPTO_AEAD_AES6149_WRAPPER_H */
