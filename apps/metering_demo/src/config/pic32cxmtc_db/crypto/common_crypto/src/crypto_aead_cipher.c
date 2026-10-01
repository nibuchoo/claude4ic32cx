/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    crypto_aead_cipher.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
*******************************************************************************/

//DOM-IGNORE-BEGIN
/*
Copyright (C) 2026 Microchip Technology Inc., and its subsidiaries. All rights reserved.

The software and documentation is provided by microchip and its contributors
"as is" and any express, implied or statutory warranties, including, but not
limited to, the implied warranties of merchantability, fitness for a particular
purpose and non-infringement of third party intellectual property rights are
disclaimed to the fullest extent permitted by law. In no event shall microchip
or its contributors be liable for any direct, indirect, incidental, special,
exemplary, or consequential damages (including, but not limited to, procurement
of substitute goods or services; loss of use, data, or profits; or business
interruption) however caused and on any theory of liability, whether in contract,
strict liability, or tort (including negligence or otherwise) arising in any way
out of the use of the software and documentation, even if advised of the
possibility of such damage.

Except as expressly permitted hereunder and subject to the applicable license terms
for any third-party software incorporated in the software and any applicable open
source software license terms, no license or other rights, whether express or
implied, are granted under any patent or other intellectual property rights of
Microchip or any third party.
*/
//DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "crypto/common_crypto/crypto_common.h"
#include "crypto/common_crypto/crypto_aead_cipher.h"
#include "crypto/drivers/wrapper/crypto_aead_aes6149_wrapper.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

#define CRYPTO_AEAD_SESSION_MAX (1)

// *****************************************************************************
// *****************************************************************************
// Section: Function Definitions
// *****************************************************************************
// *****************************************************************************



crypto_Aead_Status_E Crypto_Aead_AesGcm_Init(st_Crypto_Aead_AesGcm_ctx *ptr_aesGcmCtx_st, crypto_HandlerType_E handlerType_en, crypto_CipherOper_E cipherOper_en,
                                                              uint8_t *ptr_key, uint32_t keyLen, uint8_t *ptr_initVect, uint32_t initVectLen, uint32_t sessionID)
{
    crypto_Aead_Status_E ret_aesGcmStat_en = CRYPTO_AEAD_ERROR_CIPNOTSUPPTD;

    if(ptr_aesGcmCtx_st == NULL)
    {
        ret_aesGcmStat_en = CRYPTO_AEAD_ERROR_CTX;
    }
    else if( (ptr_key == NULL) ||
                    ( (keyLen != (uint32_t)CRYPTO_AESKEYSIZE_128)
                        && (keyLen != (uint32_t)CRYPTO_AESKEYSIZE_192)
                        && (keyLen != (uint32_t)CRYPTO_AESKEYSIZE_256) ) )
    {
       ret_aesGcmStat_en =  CRYPTO_AEAD_ERROR_KEY;
    }
    else if((ptr_initVect == NULL) || (initVectLen == 0u))
    {
        ret_aesGcmStat_en = CRYPTO_AEAD_ERROR_NONCE;
    }
    else if((cipherOper_en != CRYPTO_CIOP_ENCRYPT) && (cipherOper_en != CRYPTO_CIOP_DECRYPT))
    {
        ret_aesGcmStat_en = CRYPTO_AEAD_ERROR_CIPOPER;
    }
    else if( (sessionID == 0u) || (sessionID > (uint32_t)CRYPTO_AEAD_SESSION_MAX) )
    {
       ret_aesGcmStat_en =  CRYPTO_AEAD_ERROR_SID;
    }
    else
    {
        ptr_aesGcmCtx_st->cryptoSessionID =  sessionID;
        ptr_aesGcmCtx_st->aeadHandlerType_en = handlerType_en;
        ptr_aesGcmCtx_st->ptr_key = ptr_key;
        ptr_aesGcmCtx_st->aeadKeySize = keyLen;
        ptr_aesGcmCtx_st->aeadCipherOper_en = cipherOper_en;
        ptr_aesGcmCtx_st->ptr_initVect = ptr_initVect;
        ptr_aesGcmCtx_st->initVectLen = initVectLen;

        switch(ptr_aesGcmCtx_st->aeadHandlerType_en)
        {
            case CRYPTO_HANDLER_HW_INTERNAL:
                ret_aesGcmStat_en = Crypto_Aead_Hw_AesGcm_Init((void*)ptr_aesGcmCtx_st->arr_aeadDataCtx,
                    cipherOper_en, ptr_aesGcmCtx_st->ptr_key, ptr_aesGcmCtx_st->aeadKeySize);
                break;
            default:
                ret_aesGcmStat_en = CRYPTO_AEAD_ERROR_HDLR;
                break;
        }

    }
    return ret_aesGcmStat_en;
}

crypto_Aead_Status_E Crypto_Aead_AesGcm_AddAadData(st_Crypto_Aead_AesGcm_ctx *ptr_aesGcmCtx_st, uint8_t *ptr_aad, uint32_t aadLen)
{
    crypto_Aead_Status_E ret_aesGcmStat_en = CRYPTO_AEAD_ERROR_CIPNOTSUPPTD;
    if(ptr_aesGcmCtx_st == NULL)
    {
        ret_aesGcmStat_en = CRYPTO_AEAD_ERROR_CTX;
    }
    else if( ((ptr_aad == NULL) && (aadLen > 0u)) || ((ptr_aad != NULL) && (aadLen == 0u)) )
    {
        ret_aesGcmStat_en = CRYPTO_AEAD_ERROR_AAD;
    }
    else
    {
        switch(ptr_aesGcmCtx_st->aeadHandlerType_en)
        {
            case CRYPTO_HANDLER_HW_INTERNAL:
                ret_aesGcmStat_en = Crypto_Aead_Hw_AesGcm_Cipher((void*)ptr_aesGcmCtx_st->arr_aeadDataCtx,
                    ptr_aesGcmCtx_st->ptr_initVect, ptr_aesGcmCtx_st->initVectLen,
                    NULL, 0, NULL, ptr_aad, aadLen, NULL, 0);
                break;
            default:
                ret_aesGcmStat_en = CRYPTO_AEAD_ERROR_HDLR;
                break;
        }
    }
    return ret_aesGcmStat_en;
}

crypto_Aead_Status_E Crypto_Aead_AesGcm_Cipher(st_Crypto_Aead_AesGcm_ctx *ptr_aesGcmCtx_st, uint8_t *ptr_inputData, uint32_t dataLen,
                                                    uint8_t *ptr_outData)
{
    crypto_Aead_Status_E ret_aesGcmStat_en = CRYPTO_AEAD_ERROR_CIPNOTSUPPTD;
    if(ptr_aesGcmCtx_st == NULL)
    {
        ret_aesGcmStat_en = CRYPTO_AEAD_ERROR_CTX;
    }
    if( ((ptr_inputData == NULL) && (dataLen > 0u))
                || ((ptr_inputData != NULL) && (dataLen == 0u)) )
    {
        ret_aesGcmStat_en = CRYPTO_AEAD_ERROR_INPUTDATA;
    }
    else if( ((ptr_inputData != NULL) && (ptr_outData == NULL))
                || ((ptr_inputData == NULL) && (ptr_outData != NULL)) )
    {
        ret_aesGcmStat_en =  CRYPTO_AEAD_ERROR_OUTPUTDATA;
    }
    else
    {
        switch(ptr_aesGcmCtx_st->aeadHandlerType_en)
        {
            case CRYPTO_HANDLER_HW_INTERNAL:
                ret_aesGcmStat_en = Crypto_Aead_Hw_AesGcm_Cipher((void*)ptr_aesGcmCtx_st->arr_aeadDataCtx,
                    ptr_aesGcmCtx_st->ptr_initVect, ptr_aesGcmCtx_st->initVectLen, ptr_inputData, dataLen,
                    ptr_outData, NULL, 0, NULL, 0);
                break;
            default:
                ret_aesGcmStat_en = CRYPTO_AEAD_ERROR_HDLR;
                break;
        }
    }
    return ret_aesGcmStat_en;
}

crypto_Aead_Status_E Crypto_Aead_AesGcm_Final(st_Crypto_Aead_AesGcm_ctx *ptr_aesGcmCtx_st, uint8_t *ptr_authTag, uint8_t authTagLen)
{
    crypto_Aead_Status_E ret_aesGcmStat_en = CRYPTO_AEAD_ERROR_CIPNOTSUPPTD;
    if(ptr_aesGcmCtx_st == NULL)
    {
        ret_aesGcmStat_en = CRYPTO_AEAD_ERROR_CTX;
    }
    else if((ptr_authTag == NULL) || (authTagLen > 16u) || (authTagLen < 4u))
    {
        ret_aesGcmStat_en = CRYPTO_AEAD_ERROR_AUTHTAG;
    }
    else
    {
        switch(ptr_aesGcmCtx_st->aeadHandlerType_en)
        {
            case CRYPTO_HANDLER_HW_INTERNAL:
                ret_aesGcmStat_en = Crypto_Aead_Hw_AesGcm_Cipher((void*)ptr_aesGcmCtx_st->arr_aeadDataCtx,
                    ptr_aesGcmCtx_st->ptr_initVect, ptr_aesGcmCtx_st->initVectLen,
                    NULL, 0, NULL, NULL, 0, ptr_authTag, authTagLen);
                break;
            default:
                ret_aesGcmStat_en = CRYPTO_AEAD_ERROR_HDLR;
                break;
        }
    }
    return ret_aesGcmStat_en;
}

crypto_Aead_Status_E Crypto_Aead_AesGcm_EncryptAuthDirect(crypto_HandlerType_E handlerType_en, uint8_t *ptr_inputData, uint32_t dataLen,
                                                            uint8_t *ptr_outData, uint8_t *ptr_key, uint32_t keyLen, uint8_t *ptr_initVect,
                                                            uint32_t initVectLen, uint8_t *ptr_aad, uint32_t aadLen, uint8_t *ptr_authTag, uint8_t authTagLen, uint32_t sessionID)
{
    crypto_Aead_Status_E ret_aesGcmStat_en = CRYPTO_AEAD_ERROR_CIPNOTSUPPTD;

    if ((ptr_inputData == NULL) == (dataLen > 0u))
    {
        ret_aesGcmStat_en = CRYPTO_AEAD_ERROR_INPUTDATA;
    }
    else if ((ptr_inputData == NULL) != (ptr_outData == NULL))
    {
        ret_aesGcmStat_en =  CRYPTO_AEAD_ERROR_OUTPUTDATA;
    }
    else if( (ptr_key == NULL) ||
                ( (keyLen != (uint32_t)CRYPTO_AESKEYSIZE_128)
                    && (keyLen != (uint32_t)CRYPTO_AESKEYSIZE_192)
                    && (keyLen != (uint32_t)CRYPTO_AESKEYSIZE_256) ) )
    {
        ret_aesGcmStat_en =  CRYPTO_AEAD_ERROR_KEY;
    }
    else if((ptr_initVect == NULL) || (initVectLen == 0u))
    {
        ret_aesGcmStat_en = CRYPTO_AEAD_ERROR_NONCE;
    }
    else if ((ptr_aad == NULL) == (aadLen > 0u))
    {
        ret_aesGcmStat_en = CRYPTO_AEAD_ERROR_AAD;
    }
    else if((ptr_authTag == NULL) || (authTagLen > 16u) || (authTagLen < 4u))
    {
        ret_aesGcmStat_en = CRYPTO_AEAD_ERROR_AUTHTAG;
    }
    else if( (sessionID == 0u) || (sessionID > (uint32_t)CRYPTO_AEAD_SESSION_MAX) )
    {
       ret_aesGcmStat_en =  CRYPTO_AEAD_ERROR_SID;
    }
    else
    {
        switch(handlerType_en)
        {
            case CRYPTO_HANDLER_HW_INTERNAL:
                ret_aesGcmStat_en = Crypto_Aead_Hw_AesGcm_EncryptAuthDirect(ptr_inputData, dataLen, ptr_outData, ptr_key, keyLen,
                                                            ptr_initVect, initVectLen, ptr_aad, aadLen, ptr_authTag, authTagLen);
                break;
            default:
                ret_aesGcmStat_en = CRYPTO_AEAD_ERROR_HDLR;
                break;
        }
    }

    return ret_aesGcmStat_en;
}

crypto_Aead_Status_E Crypto_Aead_AesGcm_DecryptAuthDirect(crypto_HandlerType_E handlerType_en, uint8_t *ptr_inputData, uint32_t dataLen,
                                                            uint8_t *ptr_outData, uint8_t *ptr_key, uint32_t keyLen, uint8_t *ptr_initVect,
                                                            uint32_t initVectLen, uint8_t *ptr_aad, uint32_t aadLen, uint8_t *ptr_authTag, uint8_t authTagLen, uint32_t sessionID)
{
    crypto_Aead_Status_E ret_aesGcmStat_en = CRYPTO_AEAD_ERROR_CIPNOTSUPPTD;

    if ((ptr_inputData == NULL) == (dataLen > 0u))
    {
        ret_aesGcmStat_en = CRYPTO_AEAD_ERROR_INPUTDATA;
    }
    else if ((ptr_inputData == NULL) != (ptr_outData == NULL))
    {
        ret_aesGcmStat_en =  CRYPTO_AEAD_ERROR_OUTPUTDATA;
    }
    else if( (ptr_key == NULL) ||
                ( (keyLen != (uint32_t)CRYPTO_AESKEYSIZE_128)
                    && (keyLen != (uint32_t)CRYPTO_AESKEYSIZE_192)
                    && (keyLen != (uint32_t)CRYPTO_AESKEYSIZE_256) ) )
    {
        ret_aesGcmStat_en =  CRYPTO_AEAD_ERROR_KEY;
    }
    else if((ptr_initVect == NULL) || (initVectLen == 0u))
    {
        ret_aesGcmStat_en = CRYPTO_AEAD_ERROR_NONCE;
    }
    else if ((ptr_aad == NULL) == (aadLen > 0u))
    {
        ret_aesGcmStat_en = CRYPTO_AEAD_ERROR_AAD;
    }
    else if((ptr_authTag == NULL) || (authTagLen > 16u) || (authTagLen < 4u))
    {
        ret_aesGcmStat_en = CRYPTO_AEAD_ERROR_AUTHTAG;
    }
    else if( (sessionID == 0u) || (sessionID > (uint32_t)CRYPTO_AEAD_SESSION_MAX) )
    {
       ret_aesGcmStat_en =  CRYPTO_AEAD_ERROR_SID;
    }
    else
    {
        switch(handlerType_en)
        {
            case CRYPTO_HANDLER_HW_INTERNAL:
                ret_aesGcmStat_en = Crypto_Aead_Hw_AesGcm_DecryptAuthDirect(ptr_inputData, dataLen, ptr_outData, ptr_key, keyLen,
                                                            ptr_initVect, initVectLen, ptr_aad, aadLen, ptr_authTag, authTagLen);
                break;
            default:
                ret_aesGcmStat_en = CRYPTO_AEAD_ERROR_HDLR;
                break;
        }
    }

    return ret_aesGcmStat_en;
}
// *****************************************************************************
