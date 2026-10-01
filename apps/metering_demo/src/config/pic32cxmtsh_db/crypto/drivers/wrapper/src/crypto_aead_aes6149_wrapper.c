/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    crypto_aead_aes6149_wrapper.c

  Summary:
    Crypto Framework Library wrapper file for hardware AES.

  Description:
    This source file contains the wrapper interface to access the AEAD
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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <string.h>
#include "crypto/drivers/wrapper/crypto_aead_aes6149_wrapper.h"
#include "crypto/drivers/driver/drv_crypto_aes_hw_6149.h"

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Variables
// *****************************************************************************
// *****************************************************************************

static CRYPTO_AES_CONFIG aesGcmCfg;

// *****************************************************************************
// *****************************************************************************
// Section: File scope functions
// *****************************************************************************
// *****************************************************************************

static void lCrypto_Aead_Hw_Gcm_WriteKey(uint32_t *gcmKey)
{
    DRV_CRYPTO_AES_WriteKey(gcmKey);
   
    /* Wait for the GCMH to generate */
    while (!DRV_CRYPTO_AES_CipherIsReady())
    {
        ;
	} 
}    
    
static void lCrypto_Aead_Hw_Gcm_WriteGeneratedIv(CRYPTO_GCM_HW_CONTEXT *gcmCtx)
{
    uint32_t ivBuffer[4];
    uint8_t x;
    
    for (x = 0; x < 3UL; x++)
    {
        ivBuffer[x] = gcmCtx->calculatedIv[x];        
    }    

    ivBuffer[3] = gcmCtx->invokeCtr[0];
    
    DRV_CRYPTO_AES_WriteInitVector(ivBuffer);
}

static void lCrypto_Aead_Hw_Gcm_GenerateJ0(CRYPTO_GCM_HW_CONTEXT *gcmCtx, 
                                           uint8_t *iv, 
                                           uint32_t ivLen)
{
    uint8_t *ivSaved = (uint8_t*)gcmCtx->calculatedIv;
    
    /* Check if IV length is 96 bits */
    if (ivLen == 12UL)    
    {
        (void) memcpy(ivSaved, iv, ivLen);
        ivSaved[(sizeof(gcmCtx->calculatedIv) - 1UL)] = 0x1;
    }
    else 
    {
        /* Write the key */
        lCrypto_Aead_Hw_Gcm_WriteKey(gcmCtx->key);

        /* Configure AADLEN with: len(IV || 0s+64 || [len(IV)]64) */
        uint32_t numFullBlocks = ivLen / 16UL;
        if ((ivLen % 16UL) > 0UL)
        {
            // This is questionable. The formula says to use the bit size.
            // But the register description is byte size.
            DRV_CRYPTO_AES_WriteAuthDataLen((numFullBlocks + 2UL) * 128UL);
        }
        else
        {   
            DRV_CRYPTO_AES_WriteAuthDataLen((numFullBlocks + 1UL) * 128UL);        
        }

        /* Configure CLEN to 0. This will allow running a GHASHH only. */
        DRV_CRYPTO_AES_WritePCTextLen(0);

        /* MISRA C-2012 deviation block start */
        /* MISRA C-2012 Rule 11.3 deviated: 1. Deviation record ID - H3_MISRAC_2012_R_11_3_DR_1 */
        /* Write message to process (IV || 0s+64 || [len(IV)]64) */
        uint32_t *inPtr = (uint32_t *)iv;
        /* MISRA C-2012 deviation block end */
        uint32_t block;   /* 4 32bit block size */
        for (block = 0; block < numFullBlocks; block++)
        {
            /* Write the data to be ciphered to the input data registers */
            DRV_CRYPTO_AES_WriteInputData(&inPtr[block * 4U]);

            /* Wait for the cipher process to end */
            while (!DRV_CRYPTO_AES_CipherIsReady())
            {
                ;
            }        
        }    
    
        uint32_t numPartialBytes = ivLen % 16UL;
        if (numPartialBytes > 0UL)
        {
            uint32_t partialPlusPad[4] = {0};
            (void) memcpy(partialPlusPad, inPtr, numPartialBytes);

            /* Write the data to be ciphered to the input data registers */
            DRV_CRYPTO_AES_WriteInputData(partialPlusPad);

            /* Wait for the cipher process to end */
            while (!DRV_CRYPTO_AES_CipherIsReady())
            {
                ;
            }              
        }
    
        uint8_t finalBlock[16] = {0};
        uint32_t bits = ivLen * 8UL;
        // This may be wrong, but we have to change it to big endian format.
        // Per NIST AES GCM is big endian.
        finalBlock[15] = (uint8_t)(bits & (uint32_t)0xFFUL);
        finalBlock[14] = (uint8_t)((bits >> 8) & (uint32_t)0xFFUL);
        finalBlock[13] = (uint8_t)((bits >> 16)& (uint32_t)0xFFUL);
        finalBlock[12] = (uint8_t)((bits >> 24)& (uint32_t)0xFFUL);

        /* The lines below are subject to a type-punning warning because 
        * the (uint8_t*) is cast to a (uint32_t*) which might typically suffer 
        * from a misalignment problem. The conditional breakpoint will
        * trigger the debugger if the byte-pointer is misaligned, but will
        * be eliminated if the compiler can prove correct alignment.
        * Such a warning is thrown only at higher optimization levels.
        */
//#if defined(__GNUC__)
//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wstrict-aliasing"
//#include <assert.h> // prove we have 4-byte alignment
//      __conditional_software_breakpoint(0 == ((uint32_t)finalBlock) % 4);
//#endif
        /* MISRA C-2012 deviation block start */
        /* MISRA C-2012 Rule 11.3 deviated: 1. Deviation record ID - H3_MISRAC_2012_R_11_3_DR_1 */
        /* Write the data to be ciphered to the input data registers */
        DRV_CRYPTO_AES_WriteInputData((uint32_t *)finalBlock);
        /* MISRA C-2012 deviation block end */
#if defined(__GNUC__)
#pragma GCC diagnostic push
#endif

        /* Wait for the cipher process to end */
        while (!DRV_CRYPTO_AES_CipherIsReady())
        {
            ;
        }     
    
        /* Read hash to obtain the J0 value */
        DRV_CRYPTO_AES_ReadGcmHash(gcmCtx->intermediateHash);
        DRV_CRYPTO_AES_ReadGcmH(gcmCtx->H);
        (void) memcpy(ivSaved, (uint8_t *)gcmCtx->intermediateHash, 16);

        uint32_t tmp = (ivSaved[15] & 0xFFUL) |
                       ((ivSaved[14] & 0xFFUL) << 8U) |
                       ((ivSaved[13] & 0xFFUL) << 16U) |
                       ((ivSaved[12] & 0xFFUL) << 24U);
        tmp++;
        gcmCtx->invokeCtr[0] = (uint32_t)((tmp & 0x000000FFUL) << 24U) |
                               ((tmp & 0x0000FF00UL) << 8U) |
                               ((tmp & 0x00FF0000UL) >> 8U) |
                               ((tmp & 0xFF000000UL) >> 24U);
    }
}

static void lCrypto_Aead_Hw_Gcm_RunBlocks(uint32_t *in, uint32_t byteLen, 
                                          uint32_t* out)
{
    if (byteLen != 0UL)
    {
        uint32_t numFullBlocks = byteLen / 16UL;
        uint32_t remainingBytes = byteLen % 16UL;

        /* Process all full 16-byte blocks */
        uint32_t i;
        uint32_t *inPtr = in;   // Local pointer, safe to modify
        uint32_t *outPtr = out; // Local pointer, safe to modify
        for (i = 0; i < numFullBlocks; i++)
        {
            /* Write the data to be ciphered to the input data registers. */
            DRV_CRYPTO_AES_WriteInputData(&inPtr[i * 4U]);

            /* Wait for the cipher process to end */
            while (!DRV_CRYPTO_AES_CipherIsReady())
            {
                ;
            }  

            if (outPtr != NULL)
            {
                /* Cipher complete - read out the data */
                DRV_CRYPTO_AES_ReadOutputData(&outPtr[i * 4U]);
            }
        }

        /* Process the final partial block, if any */
        if (remainingBytes > 0UL)
        {
            uint32_t partialPlusPad[4] = {0};
            /* Copy the remaining bytes into the buffer, pad with zeros */
            (void) memcpy(partialPlusPad, &inPtr[numFullBlocks * 4U], remainingBytes);

            /* Write the data to be ciphered to the input data registers. */
            DRV_CRYPTO_AES_WriteInputData(partialPlusPad);

            /* Wait for the cipher process to end */
            while (!DRV_CRYPTO_AES_CipherIsReady())
            {
                ;
            }  

            if (outPtr != NULL)
            {
                uint32_t completeOut[4] = {0};

                /* Cipher complete - read out the data */
                DRV_CRYPTO_AES_ReadOutputData(completeOut);

                /* Copy only the valid output bytes to the partial-block
                 * position in the caller's buffer (after the last full
                 * block) */
                uint8_t *outBytes = (uint8_t *)&outPtr[numFullBlocks * 4U];
                uint8_t *srcBytes = (uint8_t *)completeOut;
                (void) memcpy(outBytes, srcBytes, remainingBytes);
            }
        }
    }
}

static void lCrypto_Aead_Hw_Gcm_CmpMsgWithTag(CRYPTO_GCM_HW_CONTEXT *gcmCtx,
    uint8_t *iv, uint32_t ivLen, uint8_t *inData, uint32_t dataLen, 
    uint8_t *outData, uint8_t *aad, uint32_t aadLen, uint8_t *tag, 
    uint32_t tagLen)
{
    (void)tagLen;
    
    /* Calculate the J0 value */
    gcmCtx->invokeCtr[0] = 0x02000000;
    lCrypto_Aead_Hw_Gcm_GenerateJ0(gcmCtx, iv, ivLen);
    
    /* Restart the driver */
    DRV_CRYPTO_AES_Init();
    
    /* Enable tag generation in driver */
    aesGcmCfg.gtagEn = 1;
    DRV_CRYPTO_AES_SetConfig(&aesGcmCfg);
    
    /* Write the key */
    lCrypto_Aead_Hw_Gcm_WriteKey(gcmCtx->key);
    
    /* Write IV with inc32(J0) (J0 + 1 on 32 bits) */
    lCrypto_Aead_Hw_Gcm_WriteGeneratedIv(gcmCtx);
    
    /* Write lengths */
    DRV_CRYPTO_AES_WriteAuthDataLen(aadLen);
    DRV_CRYPTO_AES_WritePCTextLen(dataLen);
    
    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 11.3 deviated: 3. Deviation record ID - H3_MISRAC_2012_R_11_3_DR_1 */
    lCrypto_Aead_Hw_Gcm_RunBlocks((uint32_t *)aad, aadLen, NULL);
    lCrypto_Aead_Hw_Gcm_RunBlocks((uint32_t *)inData, dataLen, 
                                  (uint32_t *)outData);
    /* MISRA C-2012 deviation block end */
    
    if ((aadLen != 0UL) || (dataLen != 0UL))
    {
        /* Wait for the tag to generate */
        while (!DRV_CRYPTO_AES_TagIsReady())
        {
            ;
        }   
    }  
    
    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 11.3 deviated: 1. Deviation record ID - H3_MISRAC_2012_R_11_3_DR_1 */
    /* Read the tag */
    DRV_CRYPTO_AES_ReadTag((uint32_t *)tag);
    /* MISRA C-2012 deviation block end */
    
    /* Read hash */
    DRV_CRYPTO_AES_ReadGcmHash(gcmCtx->intermediateHash);
    DRV_CRYPTO_AES_ReadGcmH(gcmCtx->H);
}

static void lCrypto_Aead_Hw_Gcm_1stMsgFrag(CRYPTO_GCM_HW_CONTEXT *gcmCtx,
    uint8_t *iv, uint32_t ivLen, uint8_t *inData, uint32_t dataLen, 
    uint8_t *outData, uint8_t *aad, uint32_t aadLen)
{
    /* Calculate the J0 value */
    gcmCtx->invokeCtr[0] = 0x02000000;
    lCrypto_Aead_Hw_Gcm_GenerateJ0(gcmCtx, iv, ivLen);
    
    /* Restart the driver */
    DRV_CRYPTO_AES_Init();
    
    /* Disable tag generation in driver */
    aesGcmCfg.gtagEn = 0;
    DRV_CRYPTO_AES_SetConfig(&aesGcmCfg);
    
    /* Write the key */
    lCrypto_Aead_Hw_Gcm_WriteKey(gcmCtx->key);
    
    /* Write IV with inc32(J0) (J0 + 1 on 32 bits) */
    lCrypto_Aead_Hw_Gcm_WriteGeneratedIv(gcmCtx);

    /* Write lengths */
    DRV_CRYPTO_AES_WriteAuthDataLen(aadLen);
    DRV_CRYPTO_AES_WritePCTextLen(dataLen);
    
    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 11.3 deviated: 1. Deviation record ID - H3_MISRAC_2012_R_11_3_DR_1 */
    lCrypto_Aead_Hw_Gcm_RunBlocks((uint32_t *)aad, aadLen, NULL);
    lCrypto_Aead_Hw_Gcm_RunBlocks((uint32_t *)inData, dataLen, 
                                  (uint32_t *)outData);
    /* MISRA C-2012 deviation block end */
   
    /* Read hash */
    DRV_CRYPTO_AES_ReadGcmHash(gcmCtx->intermediateHash);
    DRV_CRYPTO_AES_ReadGcmH(gcmCtx->H);
    
    /* Accumulate lengths */
    gcmCtx->totalDataLen += dataLen;
    gcmCtx->totalAadLen += aadLen;
}

static void lCrypto_Aead_Hw_Gcm_MoreMsgFrag(CRYPTO_GCM_HW_CONTEXT *gcmCtx,
    uint8_t *inData, uint32_t dataLen, uint8_t *outData)
{
    /* Disable tag generation in driver */
    aesGcmCfg.gtagEn = 0;
    DRV_CRYPTO_AES_SetConfig(&aesGcmCfg);
    
    /* Write the key */
    lCrypto_Aead_Hw_Gcm_WriteKey(gcmCtx->key);
    
    /* Write IV with inc32(J0) (J0 + 1 on 32 bits) */
    lCrypto_Aead_Hw_Gcm_WriteGeneratedIv(gcmCtx);
    
    /* Write lengths */
    DRV_CRYPTO_AES_WriteAuthDataLen(0);
    DRV_CRYPTO_AES_WritePCTextLen(dataLen);

    /* Load hash */
    DRV_CRYPTO_AES_WriteGcmHash(gcmCtx->intermediateHash);
    
    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 11.3 deviated: 2. Deviation record ID - H3_MISRAC_2012_R_11_3_DR_1 */
    lCrypto_Aead_Hw_Gcm_RunBlocks((uint32_t *)inData, dataLen, 
                                  (uint32_t *)outData);
    /* MISRA C-2012 deviation block end */
   
    /* Read hash */
    DRV_CRYPTO_AES_ReadGcmHash(gcmCtx->intermediateHash);
    DRV_CRYPTO_AES_ReadGcmH(gcmCtx->H);
    
    /* Accumulate length */
    gcmCtx->totalDataLen += dataLen;
}

static void lCrypto_Aead_Hw_Gcm_Write_BigEndian64(uint8_t *out, uint64_t val)
{
    out[0] = (uint8_t)(val >> 56);
    out[1] = (uint8_t)(val >> 48);
    out[2] = (uint8_t)(val >> 40);
    out[3] = (uint8_t)(val >> 32);
    out[4] = (uint8_t)(val >> 24);
    out[5] = (uint8_t)(val >> 16);
    out[6] = (uint8_t)(val >> 8);
    out[7] = (uint8_t)(val);
}

static void lCrypto_Aead_Hw_Gcm_GenerateTag(CRYPTO_GCM_HW_CONTEXT *gcmCtx,
    uint32_t dataLen, uint32_t aadLen, uint8_t *tag, uint32_t tagLen)
{
    /* Disable tag generation in driver */
    aesGcmCfg.gtagEn = 0;
    DRV_CRYPTO_AES_SetConfig(&aesGcmCfg);
    
    /* Write the key */
    lCrypto_Aead_Hw_Gcm_WriteKey(gcmCtx->key);
    
    /* Configure authentication data length to 0x10 (16 bytes) */
    /* And plain text length to 0 */
    DRV_CRYPTO_AES_WriteAuthDataLen(0x10);
    DRV_CRYPTO_AES_WritePCTextLen(0);
   
    /* Load hash */
    DRV_CRYPTO_AES_WriteGcmHash(gcmCtx->intermediateHash);
    
    /* Fill input data with lengths in bits */
    uint8_t lenBlock[16];
    lCrypto_Aead_Hw_Gcm_Write_BigEndian64(lenBlock, ((uint64_t)aadLen) * 8UL);
    lCrypto_Aead_Hw_Gcm_Write_BigEndian64(&lenBlock[8U], ((uint64_t)dataLen) * 8UL);
    
    /* Write the data to be ciphered to the input data registers. */
    DRV_CRYPTO_AES_WriteInputData((uint32_t*)lenBlock);
        
    /* Wait for the cipher process to end */
    while (!DRV_CRYPTO_AES_CipherIsReady())
    { 
        ;
    }  
    
    /* Read hash */
    DRV_CRYPTO_AES_ReadGcmHash(gcmCtx->intermediateHash);
    DRV_CRYPTO_AES_ReadGcmH(gcmCtx->H);
    
    /* Reset the driver */
    DRV_CRYPTO_AES_Init();
    
    /* Processing T = GCTRK(J0, S) */
    
    /* Configure AES-CTR mode */
    aesGcmCfg.opMode = CRYPTO_AES_MODE_CTR;
    aesGcmCfg.encryptMode = CRYPTO_AES_ENCRYPTION;
    aesGcmCfg.gtagEn = 0;
    DRV_CRYPTO_AES_SetConfig(&aesGcmCfg);
    
    /* Write key */
    DRV_CRYPTO_AES_WriteKey(gcmCtx->key);
    
    /* Write initialization vector with J0 value */
    DRV_CRYPTO_AES_WriteInitVector(gcmCtx->calculatedIv);

    /* Write the data to be ciphered to the input data registers. */
    DRV_CRYPTO_AES_WriteInputData(gcmCtx->intermediateHash);

    /* Wait for the cipher process to end */
    while (!DRV_CRYPTO_AES_CipherIsReady())
    { 
        ;
    }  

    /* Cipher complete - read out the data */
    uint32_t gcmTag[4];
    DRV_CRYPTO_AES_ReadOutputData(gcmTag);
   
    (void) memcpy(tag, (uint8_t*)gcmTag, tagLen);
}

// *****************************************************************************
// *****************************************************************************
// Section: AEAD Algorithms Common Interface Implementation
// *****************************************************************************
// *****************************************************************************

crypto_Aead_Status_E Crypto_Aead_Hw_AesGcm_Init(void *gcmInitCtx,
    crypto_CipherOper_E cipherOper_en, uint8_t *key, uint32_t keyLen)
{
    CRYPTO_GCM_HW_CONTEXT *gcmCtx = (CRYPTO_GCM_HW_CONTEXT*)gcmInitCtx;
    
    /* Initialize the context */
    (void) memset(gcmCtx, 0, sizeof(CRYPTO_GCM_HW_CONTEXT));
    
    /* Get the default configuration from the driver */
    DRV_CRYPTO_AES_GetConfigDefault(&aesGcmCfg);
    
    /* Initialize the driver */
    DRV_CRYPTO_AES_Init();
    
    /* Set configuration in the driver */
    aesGcmCfg.keySize = DRV_CRYPTO_AES_GetKeySize(keyLen / 4UL);
    aesGcmCfg.startMode = CRYPTO_AES_AUTO_START;
    aesGcmCfg.opMode = CRYPTO_AES_MODE_GCM;
    aesGcmCfg.gtagEn = 0;
    if (cipherOper_en == CRYPTO_CIOP_ENCRYPT)
    {
        aesGcmCfg.encryptMode = CRYPTO_AES_ENCRYPTION;
    }
    else 
    {
        aesGcmCfg.encryptMode = CRYPTO_AES_DECRYPTION;
    }
    
    DRV_CRYPTO_AES_SetConfig(&aesGcmCfg);

    /* Store the key */
    uint32_t i;
    for (i = 0; i < (keyLen / 4UL); i++)
    {
        uint32_t idx = i * 4UL;
        gcmCtx->key[i]  = ((uint32_t) key[idx]);
        gcmCtx->key[i] += ((uint32_t) key[idx + 1UL]) << 8UL;
        gcmCtx->key[i] += ((uint32_t) key[idx + 2UL]) << 16UL;
        gcmCtx->key[i] += ((uint32_t) key[idx + 3UL]) << 24UL;
    }
    
    return CRYPTO_AEAD_CIPHER_SUCCESS;
}

crypto_Aead_Status_E Crypto_Aead_Hw_AesGcm_Cipher(void *gcmCipherCtx,
    uint8_t *initVect, uint32_t initVectLen, uint8_t *inputData,
    uint32_t dataLen, uint8_t *outData, uint8_t *aad, uint32_t aadLen,
    uint8_t *authTag, uint32_t authTagLen)
{
    CRYPTO_GCM_HW_CONTEXT *gcmCtx = (CRYPTO_GCM_HW_CONTEXT*)gcmCipherCtx;
    bool cipherIsDone = false;
    
    if ((dataLen != 0U) || (aadLen != 0U))
    {
        if (gcmCtx->invokeCtr[0] == 0UL)
        {
            if (authTag != NULL)
            {
                lCrypto_Aead_Hw_Gcm_CmpMsgWithTag(gcmCtx, initVect, initVectLen, 
                    inputData, dataLen, outData, aad, aadLen, authTag, 
                    authTagLen);
                
                cipherIsDone = true;
            }
            else
            {
                lCrypto_Aead_Hw_Gcm_1stMsgFrag(gcmCtx, initVect, initVectLen,
                    inputData, dataLen, outData, aad, aadLen);
                
                cipherIsDone = true;
            }        
        }
        
        if (cipherIsDone != true)
        {
            lCrypto_Aead_Hw_Gcm_MoreMsgFrag(gcmCtx, inputData, dataLen, outData);
        }
    }
    
    if ((cipherIsDone != true) && (authTag != NULL))
    {
        if (gcmCtx->invokeCtr[0] == 0UL)
        {
            /* Calculate the J0 value */
            lCrypto_Aead_Hw_Gcm_GenerateJ0(gcmCtx, initVect, initVectLen);
        }
        
        lCrypto_Aead_Hw_Gcm_GenerateTag(gcmCtx, gcmCtx->totalDataLen, 
                            gcmCtx->totalAadLen, authTag, authTagLen);
    }
    
    return CRYPTO_AEAD_CIPHER_SUCCESS;
}
 
crypto_Aead_Status_E Crypto_Aead_Hw_AesGcm_EncryptAuthDirect(uint8_t *inputData, 
    uint32_t dataLen, uint8_t *outData, uint8_t *key, uint32_t keyLen, 
    uint8_t *initVect, uint32_t initVectLen, uint8_t *aad, uint32_t aadLen, 
    uint8_t *authTag, uint32_t authTagLen)
{
    CRYPTO_GCM_HW_CONTEXT gcmCtx;
    crypto_Aead_Status_E result;
    
    result = Crypto_Aead_Hw_AesGcm_Init(&gcmCtx, CRYPTO_CIOP_ENCRYPT, key, keyLen);
    if (result == CRYPTO_AEAD_CIPHER_SUCCESS)
    {
        result = Crypto_Aead_Hw_AesGcm_Cipher(&gcmCtx, initVect, initVectLen, 
                inputData, dataLen, outData, aad, aadLen, authTag, authTagLen);
    }
    
    return result;
}
 
crypto_Aead_Status_E Crypto_Aead_Hw_AesGcm_DecryptAuthDirect(uint8_t *inputData, 
    uint32_t dataLen, uint8_t *outData, uint8_t *key, uint32_t keyLen, 
    uint8_t *initVect, uint32_t initVectLen, uint8_t *aad, uint32_t aadLen, 
    uint8_t *authTag, uint32_t authTagLen)
{
    CRYPTO_GCM_HW_CONTEXT gcmCtx;
    crypto_Aead_Status_E result;
    
    result = Crypto_Aead_Hw_AesGcm_Init(&gcmCtx, CRYPTO_CIOP_DECRYPT, key, keyLen);
    if (result == CRYPTO_AEAD_CIPHER_SUCCESS)
    {
        result = Crypto_Aead_Hw_AesGcm_Cipher(&gcmCtx, initVect, initVectLen, 
                inputData, dataLen, outData, aad, aadLen, authTag, authTagLen);
    }
    
    return result;
}
