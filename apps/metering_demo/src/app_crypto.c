/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_crypto.c

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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "app_crypto.h"
#include "stdint.h"
#include "stdlib.h"
#include "stdarg.h"
#include "definitions.h"
// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

//for Wolfssl v5.70
#include <wolfssl/wolfcrypt/asn.h>
#include <wolfssl/wolfcrypt/kdf.h>
#include <wolfssl/wolfcrypt/error-crypt.h>
#include <wolfssl/wolfcrypt/ecc.h>

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_CRYPTO_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_CRYPTO_DATA         app_cryptoData;

//below is to allocate heap for wolfssl processing
#define HEAP_MEMORY_SIZE 10*1024
uint8_t HeapMem[HEAP_MEMORY_SIZE];
WOLFSSL_HEAP_HINT* heapHint = NULL;

#define     APP_TRNG_LEN    16
uint8_t     app_rngData[APP_TRNG_LEN];
uint32_t    app_rngLen = APP_TRNG_LEN;

unsigned  char app_PublicKey[65];
unsigned  char app_PrivateKey[32];

uint8_t test_cert_private_key[32]={ //0x42,0xDF,0x1E,0xB2,0x78,0xF5,0x4F,0xC0,0x36,0xCC,0x54,0x9D,0x46,0xDC,0xD1,0x3E,
                                    //0x3F,0x2A,0x86,0x86,0x1A,0x78,0x46,0xD8,0x1D,0x28,0x3C,0x0C,0x46,0x8A,0xD7,0x49,
                                    0x63,0x3D,0x18,0x00,0x61,0xCC,0x5F,0xD6,0xED,0xD2,0x19,0x4E,0xBA,0xAE,0xC8,0x3F,
                                    0xBC,0x3E,0xA8,0x32,0xBE,0xF6,0xF6,0x8F,0xAD,0x91,0x61,0x45,0x90,0xAF,0x54,0x4D,
                                    };

uint8_t test_cert_public_key[65]={   
                                    //0x04,0x01,0xED,0x67,0xB4,0xAA,0x60,0xBB,0xE6,0x6B,0x72,0x34,0x80,0x46,0xCD,0x85,
                                    //0x31,0xC3,0x44,0xC8,0x33,0x89,0x20,0xB1,0x11,0x9C,0xED,0x9C,0x4C,0x04,0x3F,0xB5,
                                    //0x94,0x2B,0x2A,0x76,0x32,0x1D,0x90,0x2F,0xB8,0x2A,0x59,0xF5,0x53,0xDD,0xF5,0x67,
                                    //0x5B,0xD5,0xA4,0x92,0xFF,0x7D,0x40,0xD6,0x07,0xD9,0xC5,0xC8,0xDB,0x0B,0x9B,0xF4,
                                    //0x33
                                    0x04,0x35,0x08,0xA3,0xE4,0xCA,0x84,0xDB,0x48,0xFD,0x5D,0x1D,0x8D,0x48,0xA1,0x79,
                                    0x34,0x5B,0xAD,0x91,0x69,0xDC,0x55,0xE7,0x0F,0x4D,0x1E,0xD5,0x32,0xB6,0x67,0xBB,
                                    0xD2,0xE4,0x9A,0xF2,0x3F,0x10,0x20,0xAD,0x4E,0x53,0x1F,0xB8,0xBF,0x6D,0xAE,0x2B,
                                    0xC2,0xC9,0x82,0xB4,0x16,0xBF,0x1C,0xB4,0x90,0xA9,0xB8,0x63,0x79,0x48,0x6A,0x32,
                                    0xEA,
                                    };
uint8_t  test_cert_pem[] = 
//        "-----BEGIN CERTIFICATE----- \
//        MIICKTCCAc+gAwIBAgIQe+E0+wlKlDmeHRb4RAU/7TAKBggqhkjOPQQDAjBhMRMw \
//        EQYDVQQGEwpDQSBDb3VudHJ5MREwDwYDVQQIEwhDQSBzdGF0ZTEUMBIGA1UEBxML \
//        Q0EgTG9jYWxpdHkxDzANBgNVBAoTBkNBIE9yZzEQMA4GA1UEAxMHQ0EgUm9vdDAe \
//        Fw0yNjAzMTUwNjAwMDFaFw0yNzAzMTUwNjAwMDFaMIGDMSAwHgYDVQQGExdQSUMz \
//        MkNYTVR4IERlbW8gQ291bnRyeTEhMB8GA1UEBxMYUElDMzJDWE1UeCBEZW1vIExv \
//        Y2FsaXR5MRwwGgYDVQQKExNQSUMzMkNYTVR4IERlbW8gT3JnMR4wHAYDVQQDExVQ \
//        SUMzMkNYTVR4IERlbW8gTWV0ZXIwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAAQK \
//        U4VNG2379+v//rG98tR+bSdK5LiQBB0IjTuDj2WGM1OJZLxfdgUMvJvG+QCJ4bMI \
//        BruccBeLS9TMGL/tOw1ko0YwRDAdBgNVHQ4EFgQUmeggcSSpS+2p8kjgpTmsgKhW \
//        /kUwDgYDVR0PAQH/BAQDAgeAMBMGA1UdJQQMMAoGCCsGAQUFBwMBMAoGCCqGSM49 \
//        BAMCA0gAMEUCIB8H1ffbFrAsOTiOn3WJsJrECVKr5LmRXkThctuh0r5fAiEA3RdQ \
//        Z/df+xSw7PtD9qB7ZvoBFDf0gbn7l8Xaomx+gM8= \
//        -----END CERTIFICATE----- \
//        ";
            "-----BEGIN CERTIFICATE----- \
            MIICKTCCAc+gAwIBAgIQBHAnUu7iU4Kr9e0qumE7izAKBggqhkjOPQQDAjBhMRMw \
            EQYDVQQGEwpDQSBDb3VudHJ5MREwDwYDVQQIEwhDQSBzdGF0ZTEUMBIGA1UEBxML \
            Q0EgTG9jYWxpdHkxDzANBgNVBAoTBkNBIE9yZzEQMA4GA1UEAxMHQ0EgUm9vdDAe \
            Fw0yNjAzMTUwNjAwMDFaFw0yNzAzMTUwNjAwMDFaMIGDMSAwHgYDVQQGExdQSUMz \
            MkNYTVR4IERlbW8gQ291bnRyeTEhMB8GA1UEBxMYUElDMzJDWE1UeCBEZW1vIExv \
            Y2FsaXR5MRwwGgYDVQQKExNQSUMzMkNYTVR4IERlbW8gT3JnMR4wHAYDVQQDExVQ \
            SUMzMkNYTVR4IERlbW8gTWV0ZXIwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAASD \
            bEKK5DjlcA6H88r5SDS8joLOTyNaU4VBG0TcC/iOHVk83ByD74LoLuvYqij/5Sdj \
            9J8hXgcthreMYsOpkZsto0YwRDAdBgNVHQ4EFgQUGoTpnXvPhVx1mJGD+M4ow7Lk \
            7RMwDgYDVR0PAQH/BAQDAgeAMBMGA1UdJQQMMAoGCCsGAQUFBwMBMAoGCCqGSM49 \
            BAMCA0gAMEUCIQDbAQ5ZXTkrYtlr8KrsBWl5B0UGo8lPkHAF9DkjvshXpQIgaJEm \
            ZxL/a+7YQFhfWyGaSUHBpoJu8TxHOqPwhnttYek= \
            -----END CERTIFICATE----- \
            ";

uint8_t csr_priv[32]={  
//                        0xB3,0x22,0x33,0x66,0x0B,0x86,0x21,0x04,0xD2,0xD6,0x05,0x81,0x0A,0xF9,0xC5,0x74,
//                        0x7E,0x49,0x6A,0xFB,0xE8,0x6A,0x7B,0xA0,0x0F,0x62,0x44,0x6F,0x7F,0x51,0x95,0xDE,
                        0x54,0xCB,0x53,0x28,0x10,0x71,0xAC,0x5F,0x6E,0xC0,0xB4,0xAD,0x37,0x20,0x1B,0xEE,
                        0x75,0xCC,0xDD,0x8E,0x0E,0x46,0x51,0x02,0x19,0x88,0x4F,0x02,0xCF,0x14,0xBE,0x97,    
                        };

uint8_t csr_pub[65]={   
//                        0x04,0x83,0x6C,0x42,0x8A,0xE4,0x38,0xE5,0x70,0x0E,0x87,0xF3,0xCA,0xF9,0x48,0x34,
//                        0xBC,0x8E,0x82,0xCE,0x4F,0x23,0x5A,0x53,0x85,0x41,0x1B,0x44,0xDC,0x0B,0xF8,0x8E,
//                        0x1D,0x59,0x3C,0xDC,0x1C,0x83,0xEF,0x82,0xE8,0x2E,0xEB,0xD8,0xAA,0x28,0xFF,0xE5,
//                        0x27,0x63,0xF4,0x9F,0x21,0x5E,0x07,0x2D,0x86,0xB7,0x8C,0x62,0xC3,0xA9,0x91,0x9B,
//                        0x2D
                        0x04,0xFE,0x05,0x16,0x69,0xF4,0xCC,0x72,0xCD,0x26,0x6B,0x29,0x82,0xB4,0xEC,0xD5,
                        0xAD,0x7D,0x54,0x59,0x21,0x24,0x6A,0x08,0x7E,0xE7,0x68,0x3B,0x88,0x82,0x21,0x46,
                        0x2E,0xB5,0xB4,0x6D,0x1E,0x35,0x34,0x06,0xA9,0x90,0xE2,0x97,0x6C,0x5B,0xCC,0x3B,
                        0xA0,0xF2,0xFF,0x5A,0x6C,0x15,0x3C,0x11,0xA7,0xB4,0x06,0x5A,0x9E,0x9D,0x85,0x59,
                        0xD2,
                            };

uint8_t csr_pem[] =
//        "-----BEGIN CERTIFICATE REQUEST----- \
//        MIIBgDCCASYCAQAwgYMxIDAeBgNVBAYTF1BJQzMyQ1hNVHggRGVtbyBDb3VudHJ5 \
//        MSEwHwYDVQQHExhQSUMzMkNYTVR4IERlbW8gTG9jYWxpdHkxHDAaBgNVBAoTE1BJ \
//        QzMyQ1hNVHggRGVtbyBPcmcxHjAcBgNVBAMTFVBJQzMyQ1hNVHggRGVtbyBNZXRl \
//        cjBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABINsQorkOOVwDofzyvlINLyOgs5P \
//        I1pThUEbRNwL+I4dWTzcHIPvgugu69iqKP/lJ2P0nyFeBy2Gt4xiw6mRmy2gQDA+ \
//        BgkqhkiG9w0BCQ4xMTAvMB0GA1UdDgQWBBRq4LJz3yDqDNOUv+rjuTJD/A/keDAO \
//        BgNVHQ8BAf8EBAMCB4AwCgYIKoZIzj0EAwIDSAAwRQIhAN2U4S8blRlILkwSD0F/ \
//        dEaVFUF/d/IdZanimxTAjHtKAiA1d5pIU0r09Bt1Z0dwfvy5CfnV1jdDAlBkWdaV \
//        ge+QYQ== \
//        -----END CERTIFICATE REQUEST----- \
//        ";
            "-----BEGIN CERTIFICATE REQUEST----- \
            MIIBgDCCASYCAQAwgYMxIDAeBgNVBAYTF1BJQzMyQ1hNVHggRGVtbyBDb3VudHJ5 \
            MSEwHwYDVQQHExhQSUMzMkNYTVR4IERlbW8gTG9jYWxpdHkxHDAaBgNVBAoTE1BJ \
            QzMyQ1hNVHggRGVtbyBPcmcxHjAcBgNVBAMTFVBJQzMyQ1hNVHggRGVtbyBNZXRl \
            cjBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABP4FFmn0zHLNJmspgrTs1a19VFkh \
            JGoIfudoO4iCIUYutbRtHjU0BqmQ4pdsW8w7oPL/WmwVPBGntAZanp2FWdKgQDA+ \
            BgkqhkiG9w0BCQ4xMTAvMB0GA1UdDgQWBBSdjWAaVCFGwi6VMDmynDiULKAkrjAO \
            BgNVHQ8BAf8EBAMCB4AwCgYIKoZIzj0EAwIDSAAwRQIgA/mQIk8P0dDe9eyqvap+ \
            QrlHBcAEyXuRI+53CUdRFy0CIQD+CN9hyAat1++Jx0gwMLReSW8r+B904xNd23Qb \
            yZgZhg== \
            -----END CERTIFICATE REQUEST----- \
            ";

uint8_t aes_key[16] = {
    0x4F, 0xA4, 0x57, 0x62, 0x17, 0x59, 0x56, 0xA8,
    0xC1, 0x9E, 0x0B, 0x0E, 0x7A, 0x31, 0xFC, 0xC7
};

uint8_t aes_iv[12] = {
    0x53, 0x7F, 0x55, 0x3F, 0x1A, 0x93,
    0xF2, 0x4D, 0x27, 0x25, 0x5C, 0x2D
};

uint8_t aes_aad[20] = {
    0xA2, 0x67, 0x89, 0xBC, 0x21, 0x47, 0x23, 0x10,
    0xD6, 0x4F, 0xED, 0xC9, 0xB9, 0x05, 0xCB, 0xD5,
    0x72, 0xC2, 0x83, 0xBE
};

uint8_t App_Plain[128] = 
{ 
    0x11,0x22,0x33,0x44,0x7B,0xAE,0xBD,0x4C,0xAD,0xE4,0x94,0xC5,0x4A,0x29,0xAE,0x70,
    0x12,0x34,0x56,0x78,0xC5,0xCD,0x91,0x8A,0xFC,0x1F,0x3C,0xBC,0xB3,0x92,0x91,0x9D,
    0xE0,0x7D,0xAC,0x04,0xDA,0x97,0x3E,0x76,0x8E,0x6D,0x0A,0x87,0x19,0xA3,0x43,0xB7,
    0xEC,0xF6,0xE8,0xFE,0x73,0x45,0x9B,0x9D,0xFB,0x21,0x7E,0xF8,0x3C,0xAD,0xBD,0x4D,
    0xDF,0x89,0x2B,0x7A,0xB1,0x16,0xBA,0xAF,0x35,0x52,0x0E,0xC0,0x78,0xB4,0xE8,0x43,
    0x9C,0xBE,0x65,0xFC,0xEE,0x48,0x24,0xCA,0x3E,0xA9,0x68,0x17,0x6F,0x58,0x04,0xF6,
    0x1D,0x04,0x82,0x4A,0x95,0xA0,0x09,0x92,0x5D,0x36,0x21,0x82,0xBB,0x78,0x37,0xFC,
    0x64,0xBE,0x03,0x58,0xCA,0xC8,0x62,0xF8,0xC0,0xA8,0xD6,0x1C,0xCE,0x14,0xCB,0xB1, 
};
uint8_t App_Cipher[128];
uint8_t App_output[128];
uint8_t App_Tag[16];
uint16_t Data_length = 20;

uint8_t SHA1_Output_Digest[20];
uint8_t SHA2_Output_Digest[32];
uint8_t SHA_Output_Digest[32];
uint8_t SHA_Input_Test_Data[1200] = 
{
    0xDF, 0xED, 0x61, 0x1C, 0xDF, 0xED, 0x61, 0x1C, 0x48, 0xCC, 0xD0, 0x17, 0x7E, 0xCF, 0x75, 0x79,
    0x5D, 0x5B, 0xCC, 0xED, 0x5D, 0x5B, 0xCC, 0xED, 0x5B, 0x2D, 0x78, 0xFC, 0xAC, 0x1F, 0x77, 0xEF,
    0xDF, 0x55, 0xF8, 0x9D, 0x6D, 0x06, 0x83, 0x21, 0x63, 0x9A, 0x83, 0x3F, 0xC9, 0xE1, 0xBA, 0x26,
    0x27, 0x33, 0x59, 0x9F, 0xB5, 0x4E, 0x66, 0xE0, 0x2F, 0xD3, 0x74, 0x46, 0x46, 0x73, 0x98, 0x3D,
    0x7B, 0xC5, 0x64, 0x35, 0x7B, 0xC5, 0x64, 0x35, 0x58, 0x5B, 0x6F, 0x4A, 0x6F, 0xF5, 0xF6, 0x1D,
    0x83, 0x26, 0x99, 0xEA, 0xB6, 0x52, 0xFB, 0x7D, 0x4A, 0x0E, 0x12, 0xC6, 0xB7, 0x65, 0x29, 0x55,
    0xA5, 0x37, 0x5F, 0x84, 0xCF, 0x60, 0x6F, 0xFA, 0xE2, 0x61, 0x01, 0x60, 0x46, 0x72, 0xB7, 0xD3,
    0x40, 0xE4, 0xC4, 0xF1, 0xC6, 0xF3, 0xAD, 0x68, 0x8D, 0x80, 0x12, 0xDB, 0xC8, 0x26, 0xF5, 0x1B,
    
    0x52, 0x15, 0xD9, 0x7A, 0x76, 0xA8, 0x9C, 0x7D, 0x0B, 0xA8, 0xE6, 0xA4, 0x45, 0x21, 0x53, 0x57,
    0x7E, 0x18, 0xC3, 0xF7, 0x62, 0x4E, 0x91, 0x86, 0x69, 0x12, 0x0B, 0x47, 0x68, 0x89, 0x11, 0x72,
    0x51, 0x90, 0x6C, 0xF2, 0x15, 0x78, 0x90, 0x1A, 0x93, 0x98, 0x1B, 0x8E, 0x78, 0xEF, 0xD3, 0x87,
    0x4A, 0x7A, 0x0B, 0x1F, 0x4A, 0x7A, 0x0B, 0x1F, 0x9B, 0x3B, 0xB1, 0x33, 0xAE, 0xB3, 0x9C, 0x12,
    0xCF, 0x56, 0x0D, 0x37, 0x5E, 0xEE, 0x5F, 0x25, 0x17, 0x09, 0x68, 0x2A, 0xB8, 0x3E, 0xBF, 0x2E,
    0x63, 0x12, 0x1B, 0xE4, 0xDB, 0xF1, 0x65, 0x39, 0xBB, 0x53, 0xA5, 0x5D, 0x43, 0x09, 0x33, 0xF1,
    0x71, 0xF7, 0x5F, 0x29, 0xA3, 0x33, 0xC2, 0x81, 0xE8, 0x04, 0x7E, 0x4F, 0xF6, 0xCA, 0x61, 0x71,
    0x6A, 0x35, 0xA2, 0x28, 0xDD, 0x7D, 0x0B, 0x51, 0x36, 0xE7, 0x3F, 0x67, 0x6A, 0x2F, 0x48, 0x67,
    
    0x03, 0x99, 0x99, 0x99, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xCF, 0x56, 0x0D, 0x37, 0x5E, 0xEE, 0x5F, 0x25, 0x17, 0x09, 0x68, 0x2A, 0xB8, 0x3E, 0xBF, 0x2E,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
    
    0x04, 0x99, 0x99, 0x99, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
    
    0x05, 0x99, 0x99, 0x99, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
    
    0x06, 0x99, 0x99, 0x99, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
    
    0x07, 0x99, 0x99, 0x99, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
    
    0x08, 0x99, 0x99, 0x99, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
};



// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************
void APP_CRYPTO_Print(const char *format, ...)
{
    char buffer[SYS_CONSOLE_PRINT_BUFFER_SIZE];
    va_list args;
    va_start(args, format);

    vsnprintf(buffer, sizeof(buffer), format, args);

    va_end(args);

    FLEXCOM0_USART_Write((uint8_t *)buffer, strlen(buffer));
    while(!(FLEXCOM0_USART_TransmitComplete()));
}
// *****************************************************************************
// *****************************************************************************
void APP_CRYPTO_Print_Hex(const uint8_t *buf, size_t len)
{
    size_t i;
    for (i = 0; i < len; i++)
    {
        APP_CRYPTO_Print("%02X ", buf[i]);

        if ((i + 1) % 32 == 0)
        {
            APP_CRYPTO_Print("\r\n");
        }
    }

    if (len % 32 != 0)
    {
        APP_CRYPTO_Print("\r\n");
    }
}
// *****************************************************************************
// *****************************************************************************
void APP_CRYPTO_Print_Char(const char *buf, size_t len)
{
    size_t i;
    for (i = 0; i < len; i++)
    {
        APP_CRYPTO_Print("%c", buf[i]);
    }
    APP_CRYPTO_Print("\r\n");
}

// *****************************************************************************
// *****************************************************************************
void APP_CRYPTO_Wolfssl_Allocate_Heap_memory(void)
{
    int ret;
   
    /* Load static memory into wolfSSL heap */
    ret = wc_LoadStaticMemory(&heapHint, HeapMem, HEAP_MEMORY_SIZE,    WOLFMEM_GENERAL, HEAP_MEMORY_SIZE);
    if (ret == 0) 
    {
        APP_CRYPTO_Print("wc_LoadStaticMemory() done\r\n");
    }
}

// *****************************************************************************
// *****************************************************************************
// Parse ASN.1 length field
uint32_t APP_CRYPTO_ASN1_Get_Length(const unsigned char* data, uint32_t* lenBytes) 
{
    if (data[0] < 0x80) 
    {
        *lenBytes = 1;
        return data[0];
    } 
    else 
    {
        int numLenBytes = data[0] & 0x7F;
        uint32_t length = 0;
        for (int i = 0; i < numLenBytes; i++) 
        {
            length = (length << 8) | data[1 + i];
        }
        *lenBytes = 1 + numLenBytes;
        return length;
    }
}
// *****************************************************************************
// *****************************************************************************
int APP_Crypto_HW_SHA1(uint8_t *input_data, uint32_t data_len, uint8_t *output_data)
{
    crypto_Hash_Status_E status;

    (void) memset(output_data, 0, 20);
    
    status = Crypto_Hash_Sha_Digest(
            CRYPTO_HANDLER_HW_INTERNAL,
            input_data,
            data_len,
            output_data,
            CRYPTO_HASH_SHA1,
            SESSION_ID
        );
    
    return (int) status;
}
// *****************************************************************************
// *****************************************************************************
int APP_Crypto_HW_SHA2_256(uint8_t *input_data, uint32_t data_len, uint8_t *output_data)
{
    crypto_Hash_Status_E status;

    (void) memset(output_data, 0, 32);
    
    status = Crypto_Hash_Sha_Digest(
            CRYPTO_HANDLER_HW_INTERNAL,
            input_data,
            data_len,
            output_data,
            CRYPTO_HASH_SHA2_256,
            SESSION_ID
        );
        
    return (int) status;
}
// *****************************************************************************
// *****************************************************************************
int APP_Crypto_HW_AES_GCM128_Encrypt(uint8_t *ptr_inputData, uint32_t dataLen, uint8_t *ptr_outData,
                                    uint8_t *ptr_key, uint32_t keyLen, 
                                    uint8_t *ptr_initVect,uint32_t initVectLen, 
                                    uint8_t *ptr_aad, uint32_t aadLen, 
                                    uint8_t *ptr_authTag, uint8_t authTagLen)
{
    crypto_Aead_Status_E status;
        
    (void) memset(ptr_outData, 0, dataLen);
    
    status = Crypto_Aead_AesGcm_EncryptAuthDirect(
        CRYPTO_HANDLER_HW_INTERNAL,
        ptr_inputData,
        dataLen,
        ptr_outData,
        ptr_key,
        keyLen,
        ptr_initVect,
        initVectLen,
        ptr_aad,
        aadLen,
        ptr_authTag,
        authTagLen,
        SESSION_ID
    );
    
    return (int) status;
}
// *****************************************************************************
// *****************************************************************************
int APP_Crypto_HW_AES_GCM128_Decrypt(uint8_t *ptr_inputData, uint32_t dataLen,uint8_t *ptr_outData, 
                                    uint8_t *ptr_key, uint32_t keyLen, 
                                    uint8_t *ptr_initVect, uint32_t initVectLen, 
                                    uint8_t *ptr_aad, uint32_t aadLen, 
                                    uint8_t *ptr_authTag, uint8_t authTagLen)
{
    crypto_Aead_Status_E status;
    
    (void) memset(ptr_outData, 0, dataLen);
    
    status = Crypto_Aead_AesGcm_DecryptAuthDirect(
        CRYPTO_HANDLER_HW_INTERNAL,
        ptr_inputData,
        dataLen,
        ptr_outData,
        ptr_key,
        keyLen,
        ptr_initVect,
        initVectLen,
        ptr_aad,
        aadLen,
        ptr_authTag,
        authTagLen,
        SESSION_ID
    );
    
    return (int) status;
}
// *****************************************************************************
// *****************************************************************************
/* Map our public curve enum to the wolfCrypt curve id used by wc_ecc_*. */
int APP_Crypto_lEccKeyGen_GetWolfCurveId(crypto_EccCurveType_E curve)
{
    switch (curve)
    {
        case CRYPTO_ECC_CURVE_P256:  return ECC_SECP256R1;
        case CRYPTO_ECC_CURVE_P384:  return ECC_SECP384R1;
        case CRYPTO_ECC_CURVE_P521:  return ECC_SECP521R1;
        default:                     return ECC_CURVE_INVALID;
    }
}

// *****************************************************************************
// *****************************************************************************
bool APP_Crypto_lEccKeyGen_SanityCheck(const uint8_t *priv, const uint8_t *pub, uint32_t coordSize)
{
    uint32_t i;
    bool privIsZero = true;
    bool xyEqual    = true;

    /* priv must not be all zeros. */
    for (i = 0U; i < coordSize; i++)
    {
        if (priv[i] != 0U)
        {
            privIsZero = false;
            break;
        }
    }
    if (privIsZero)
    {        
        APP_CRYPTO_Print("Key Sanity Check fail: private key is all zeros\r\n");
        return false;
    }

    /* X and Y must differ (collision astronomically unlikely). */
    for (i = 0U; i < coordSize; i++)
    {
        if (pub[i] != pub[coordSize + i])
        {
            xyEqual = false;
            break;
        }
    }
    if (xyEqual)
    {
        APP_CRYPTO_Print("Key Sanity Check fail: pubX == pubY (extraction bug suspected)\r\n");
        return false;
    }

    APP_CRYPTO_Print("Key Sanity Check PASS: invariants OK\r\n");
    return true;
}
// *****************************************************************************
// *****************************************************************************
bool APP_Crypto_lEccKeyGen_WolfCheck(const uint8_t *pub, uint32_t coordSize, crypto_EccCurveType_E curve)
{
    ecc_key key;
    int     wolfCurveId = APP_Crypto_lEccKeyGen_GetWolfCurveId(curve);
    int     ret;
    bool    ok = false;

    if (wolfCurveId == ECC_CURVE_INVALID)
    {
        APP_CRYPTO_Print("Key Wolfssl check FAIL: unsupported curve in wolfCrypt mapping\r\n");
        return false;
    }

    ret = wc_ecc_init(&key);
    if (ret != 0)
    {
        APP_CRYPTO_Print("Key Wolfssl check FAIL: wc_ecc_init -> %d\r\n", ret);
        return false;
    }

    /* Import the X || Y pair. */
    ret = wc_ecc_import_unsigned(&key, (byte *)pub, (byte *)&pub[coordSize],
                                 NULL, wolfCurveId);
    if (ret != 0)
    {
        APP_CRYPTO_Print("Key Wolfssl check FAIL: wc_ecc_import_unsigned -> %d\r\n", ret);
        goto done;
    }

    ret = wc_ecc_check_key(&key);
    if (ret == MP_OKAY)
    {
        APP_CRYPTO_Print("Key Wolfssl check PASS: wc_ecc_check_key (SP 800-56A 4-step)\r\n");
        ok = true;
    }
    else
    {
        APP_CRYPTO_Print("Key Wolfssl check FAIL: wc_ecc_check_key -> %d\r\n", ret);
    }

done:
    (void) wc_ecc_free(&key);
    return ok;
}
// *****************************************************************************
// *****************************************************************************
int APP_CRYPTO_HW_Gen_Key_P256(uint8_t *app_PubKey, uint8_t *app_PrivKey )
{
    uint8_t local_priv_SECP256R1[32];
    uint8_t local_publ_SECP256R1[64];        /* X || Y, no 0x04 prefix */
            
    crypto_EccKeyGen_Status_E status;

    (void) memset(local_priv_SECP256R1,    0, 32U);
    (void) memset(local_publ_SECP256R1,     0, 2U * 32U);
    
    /* keygen call must return SUCCESS. */
    status = Crypto_Ecc_Hw_KeyGen(local_priv_SECP256R1, 32U, local_publ_SECP256R1,  2U * 32U, CRYPTO_ECC_CURVE_P256);
    
    if (status != CRYPTO_ECCKEYGEN_SUCCESS)
    {
        APP_CRYPTO_Print("KeyGen FAIL: Crypto_Ecc_Hw_KeyGen -> %d\r\n", status);
        return (int) status;
    }
    
    /*sanity invariants. */
    if (!APP_Crypto_lEccKeyGen_SanityCheck(local_priv_SECP256R1, local_publ_SECP256R1, 32U))
    {
        return (int) CRYPTO_ECCKEYGEN_ERROR_FAIL;
    }
   
    /*wolfCrypt SP 800-56A full validation. */
    if (!APP_Crypto_lEccKeyGen_WolfCheck(local_publ_SECP256R1, 32U, CRYPTO_ECC_CURVE_P256))
    {
        return (int) CRYPTO_ECCKEYGEN_ERROR_FAIL;
    }
    
    (void) memcpy(app_PrivKey, local_priv_SECP256R1, 32U);
    app_PubKey[0] = 0x04U;
    (void) memcpy(&app_PubKey[1], local_publ_SECP256R1, 2U * 32U);

    return (int) CRYPTO_ECCKEYGEN_SUCCESS;
}
// *****************************************************************************
// *****************************************************************************
int APP_CRYPTO_HW_ECDSA_Sign(uint8_t * csr_der_hash, uint32_t csr_der_hash_len,
        uint8_t * csr_der_r_s_sig, uint32_t csr_der_r_s_sig_len,
        uint8_t * priv, uint32_t priv_len, crypto_EccCurveType_E ccc_curve_type) 
{
    crypto_DigiSign_Status_E status;
    
    status = Crypto_DigiSign_Ecdsa_Sign(
            CRYPTO_HANDLER_HW_INTERNAL,
            csr_der_hash, csr_der_hash_len,
            csr_der_r_s_sig, csr_der_r_s_sig_len,
            priv, priv_len,
            ccc_curve_type, 
            SESSION_ID);
    
    return (int)status;
           
}

// *****************************************************************************
// *****************************************************************************
int APP_CRYPTO_HW_ECDSA_Verify(uint8_t *inputHash, uint32_t hashLen,
        uint8_t *inputSig, uint32_t sigLen,
        uint8_t *pubKey, uint32_t pubKeyLen,
        int8_t *hashVerifyStatus, crypto_EccCurveType_E eccCurveType_En) 
{
    crypto_DigiSign_Status_E status;

    status = Crypto_DigiSign_Ecdsa_Verify(
            CRYPTO_HANDLER_HW_INTERNAL,
            inputHash, hashLen, 
            inputSig, sigLen, 
            pubKey, pubKeyLen,
            hashVerifyStatus, 
            eccCurveType_En,
            SESSION_ID);

    return (int) status;
}

// *****************************************************************************
// *****************************************************************************
int APP_CRYPTO_HW_ECDH(uint8_t *self_privKey, uint32_t self_privKey_len,
        uint8_t *other_pubKey, uint32_t other_pubKey_len,
        uint8_t *share_secret, uint32_t share_secret_len) 
{
    crypto_Kas_Status_E status;
    
    status = Crypto_Kas_Ecdh_SharedSecret(
            CRYPTO_HANDLER_HW_INTERNAL,
            self_privKey, self_privKey_len,
            other_pubKey, other_pubKey_len,
            share_secret, share_secret_len,
            CRYPTO_ECC_CURVE_SECP256R1,
            SESSION_ID);
            
    return (int) status;
}
// *****************************************************************************
// *****************************************************************************
int APP_CRYPTO_HW_Trng_Generate(uint8_t *rngData, uint32_t rngLen)
{
    crypto_Rng_Status_E status;
    
    status = Crypto_Rng_Generate(
            CRYPTO_HANDLER_HW_INTERNAL,
            rngData,
            rngLen,
            NULL,       // ptr_nonce
            0,          // sizeof(nonce)
            SESSION_ID
    );
    return (int) status;    
}
// *****************************************************************************
// *****************************************************************************
int APP_CRYPTO_HKDF_For_AES_GCM_Key(void)
{
    int ret;
    
    //First generate the common salt use in KDF.
    //In application, this salt should be shared between self and the other party before KDF process
    //Here simulate the common salt using TRNG to generate the salt.
    
    uint8_t hkdf_salt[32];   //since SHA256 is used, so the suggested length of slat will be 32
    uint8_t hkdf_info[10] = {0x50, 0x49, 0x43, 0x33, 0x32, 0x43, 0x58, 0x4d, 0x54, 0x78};      //"PIC32CXMTx"       //can be any length

    APP_CRYPTO_HW_Trng_Generate(hkdf_salt, sizeof(hkdf_salt));
    APP_CRYPTO_Print("Common HKDF salt:\r\n");
    APP_CRYPTO_Print_Hex(hkdf_salt, sizeof (hkdf_salt));
    APP_CRYPTO_Print("Common HKDF info:\r\n");
    APP_CRYPTO_Print_Hex(hkdf_info, sizeof (hkdf_info));
    
    //the input of HKDF API is the salt and the shared secret
    //Assume this is after obtaining the shared secret using ECDH
    uint8_t shared_secret[32]={ 
        0x42,0xE0,0xF4,0xAB,0x2C,0xCE,0xA0,0x11,0xC2,0xDA,0x8E,0x4F,0x53,0x8E,0x36,0x11,
        0xDF,0x5B,0xFD,0xC8,0xB6,0xA3,0x39,0x84,0x0C,0x00,0xBF,0xAF,0x71,0xAC,0xFA,0xE8};
    
    uint8_t kdf_aes_gcm_key[16];
    memset(kdf_aes_gcm_key, 0, sizeof(kdf_aes_gcm_key));
    
    ret = wc_HKDF(SHA256, shared_secret, sizeof(shared_secret),
                        hkdf_salt, sizeof(hkdf_salt), 
                        hkdf_info, sizeof(hkdf_info), 
                        kdf_aes_gcm_key, sizeof(kdf_aes_gcm_key));
    
    if(ret == 0)
    {
        APP_CRYPTO_Print("Success using HKDF generate AES-GCM-128 key:\r\n");
        APP_CRYPTO_Print_Hex(kdf_aes_gcm_key, sizeof(kdf_aes_gcm_key));
    }
    else
    {
        APP_CRYPTO_Print("Fail using HKDF generate AES-GCM-128 key\r\n");
    }
}
// *****************************************************************************
// *****************************************************************************
int APP_CRYPTO_NIST_HKDF_For_AES_GCM128_Key(void)
{
    //NIST Concatenation KDF (NIST SP 800 56A Rev. 2: 2013, 5.8.1.1) 
    //Function call: kdf(Z , OtherInput), KDF(Z, OtherInfo)= Hash(Z||OtherInfo)
    //Z: Shared Secret
    //OtherInfo: AlgorithmID (e.g., OID for AES?GCM?128), PartyUInfo (identifier for initiator), PartyVInfo (identifier for responder),KeyDataLen (e.g., key length)
    //Function: SHA256
    //1) Z : Shared Secret(a byte string), 32 bytes
    //2) keydatalen : AES-GCM-128: 128 bit, AES-GCM-256: 256 bit
    //3) OtherInfo : AlgorithmID || PartyUInfo || PartyVInfo
    //AlgorithmID /Object Identifier (OID): 7 bytes 2.16.756.5.8.3.0 (Encoded in DER hex :60 85 74 05 08 03 00)
    //PartyUinfo: 8 bytes originator-system-title
    //PartyVInfo: 8 bytes recipient-system-title
    
    //Need to have the share secret before calling KDF to derive the key
    //Assume this is after obtaining the shared secret using ECDH
    uint8_t shared_secret[32]={ 
        0x42,0xE0,0xF4,0xAB,0x2C,0xCE,0xA0,0x11,0xC2,0xDA,0x8E,0x4F,0x53,0x8E,0x36,0x11,
        0xDF,0x5B,0xFD,0xC8,0xB6,0xA3,0x39,0x84,0x0C,0x00,0xBF,0xAF,0x71,0xAC,0xFA,0xE8};
    
    
    uint8_t algorithmID[7] ={0x60, 0x85, 0x74, 0x05, 0x08, 0x03, 0x00};                 //OID depend on specific algorithm
    uint8_t partyUinfo[9] = {0x43, 0x6C, 0x69, 0x65, 0x6E, 0x74, 0x31, 0x32, 0x33};     //Example Party U info = "Client123", arbitrary length
    uint8_t partyVInfo[9] = {0x53, 0x65, 0x72, 0x76, 0x65, 0x72, 0x41, 0x42, 0x43};     //Example Party V info = "ServerABC", arbitrary length
    uint8_t keydatalen[1] = {128};    //128 = Example of AES_GCM128 //The length of the output key that KDF to produce
            
    //Hash input, size is 7+9+9+1+32 =  58      //32 is the length of the shared secret length       
    uint8_t hash_Input[58];
    uint8_t hash_Output_Digest[32];
    
    //Concatenate hex inputs follows NIST SP 800 56A Rev. 2: 2013, 5.8.1.1 
    //Z||algorithmID||PartyUInfo||PartyVInfo||KeyDataLen
    memcpy(hash_Input, shared_secret, sizeof(shared_secret));
    memcpy(hash_Input+sizeof(shared_secret), algorithmID, sizeof(algorithmID));
    memcpy(hash_Input+sizeof(shared_secret)+sizeof(algorithmID),partyUinfo, sizeof(partyUinfo));
    memcpy(hash_Input+sizeof(shared_secret)+sizeof(algorithmID)+sizeof(partyUinfo), partyVInfo, sizeof(partyVInfo));
    memcpy(hash_Input+sizeof(shared_secret)+sizeof(algorithmID)+sizeof(partyUinfo)+sizeof(partyVInfo), keydatalen, sizeof(keydatalen));

    //Compute SHA256;
    APP_Crypto_HW_SHA2_256(hash_Input, sizeof(hash_Input), hash_Output_Digest);     //output of hash256 is 32 bytes
    APP_CRYPTO_Print("NIST KDF output: \r\n");
    APP_CRYPTO_Print_Hex(hash_Output_Digest, sizeof (hash_Output_Digest));
    
    //AES_GCM128 key only 16 bytes, so use the first 16 bytes of the output of hash256 (32 bytes)
    uint8_t aes_gcm128_key[16];
    memcpy(aes_gcm128_key,hash_Output_Digest, 16);
    APP_CRYPTO_Print("AES GCM128 key from NIST KDF: \r\n");
    APP_CRYPTO_Print_Hex(aes_gcm128_key, sizeof (aes_gcm128_key));
   
    //Extra: Example: for DLMS, the GUK key and AK key can take from the hash output.
    uint8_t     dlms_nist_kdf_guk_key[16];
    uint8_t     dlms_nist_kdf_ak_key[16];
    memcpy(dlms_nist_kdf_guk_key,hash_Output_Digest, 16);
    memcpy(dlms_nist_kdf_ak_key,hash_Output_Digest+16, 16 );
    
    APP_CRYPTO_Print("GUK_Key (DLMS)\r\n:");
    APP_CRYPTO_Print_Hex(dlms_nist_kdf_guk_key, sizeof (dlms_nist_kdf_guk_key));
    APP_CRYPTO_Print("AK_Key (DLMS)\r\n:");
    APP_CRYPTO_Print_Hex(dlms_nist_kdf_ak_key, sizeof(dlms_nist_kdf_ak_key));
    
    return 0;
    
}
// *****************************************************************************
// *****************************************************************************
void APP_CRYPTO_Convert_Private_Key_To_PEM(uint8_t *priv_key, uint32_t key_len) 
{
    uint32_t i;
    int ret;
    uint8_t derBuf[1024];
    uint8_t pemBuf[1024];
        
    //init the key and add in ASN.1 format to the EC raw key data
    ecc_key key;
    wc_ecc_init(&key);      //remember to free the key
    
    // Import raw scalar into ECC key, input private key only
    ret = wc_ecc_import_private_key_ex(priv_key, key_len, NULL,0, &key, ECC_SECP256R1);
    if (ret != 0) 
    {
        APP_CRYPTO_Print("Import private key fail\r\n");
        wc_ecc_free(&key);
        return;
    }
   
    //Export key to DER 
    int derSize = wc_EccPrivateKeyToDer(&key, derBuf, sizeof(derBuf));
    if(derSize <0)
    {
        APP_CRYPTO_Print("Export private key to DER fail\r\n");
        wc_ecc_free(&key);
        return;
    }

    // Convert DER to PEM 
    int pemSize = wc_DerToPem(derBuf, derSize, pemBuf, sizeof(pemBuf), ECC_PRIVATEKEY_TYPE);
    if(pemSize<0)
    {
        APP_CRYPTO_Print("Convert DER private key to PEM fail\r\n");
        wc_ecc_free(&key);
        return;
    }

    APP_CRYPTO_Print("Private Key PEM: \r\n");
    APP_CRYPTO_Print_Char(pemBuf, pemSize);
      
    wc_ecc_free(&key);
    
}

// *****************************************************************************
// *****************************************************************************
void APP_CRYPTO_Convert_Public_Key_To_PEM(uint8_t *pub_key, uint32_t key_len) 
{
    uint32_t i;
    int ret;
    uint8_t derBuf[1024];
    uint8_t pemBuf[1024];
    
    //init the key and add in ASN.1 format to the EC raw key data
    ecc_key key;
    wc_ecc_init(&key);      //remember to free wc_ecc_free(&key);
    
    ret = wc_ecc_import_x963(pub_key, key_len, &key);
    if (ret != 0) 
    {
        APP_CRYPTO_Print("Import public key fail\r\n");
        wc_ecc_free(&key);
        return;
    }
   
    //Export key to DER 
    int derSize = wc_EccPublicKeyToDer(&key, derBuf, sizeof(derBuf),1);
    if(derSize <0)
    {
        APP_CRYPTO_Print("Export public key to DER fail\r\n");
        wc_ecc_free(&key);
        return;
    }

    // Convert DER to PEM 
    int pemSize = wc_DerToPem(derBuf, derSize, pemBuf, sizeof(pemBuf), ECC_PUBLICKEY_TYPE);
    if(pemSize<0)
    {
        APP_CRYPTO_Print("Convert DER public key to PEM fail\r\n");
        wc_ecc_free(&key);
        return;
    }

    APP_CRYPTO_Print("Public Key PEM: \r\n");
    APP_CRYPTO_Print_Char(pemBuf, pemSize);
     
    wc_ecc_free(&key);
    
}

// *****************************************************************************
// *****************************************************************************
int APP_CRYPTO_Get_Pub_Key(uint8_t *cert_pub_key, uint32_t cert_pub_key_len, uint8_t *pub_key, uint32_t *pub_key_len )
{
    int ret;
   
    uint32_t index;
    ecc_key key;
    
    ret = wc_ecc_init_ex(&key, heapHint, INVALID_DEVID);
    if(ret <0)
    {
        APP_CRYPTO_Print("Error in wc_ecc_init_ex()\r\n");
        wc_ecc_free(&key);
        return ret;
    }
    
    index = 0; //index MUST set to 0
    ret = wc_EccPublicKeyDecode((const byte*)cert_pub_key,(word32*) &index, &key, (word32) cert_pub_key_len); 
    if (ret < 0) 
    {
        APP_CRYPTO_Print("Error decoding public key\r\n");
        wc_ecc_free(&key);
        return ret;
    }
    
    ret = wc_ecc_export_x963(&key, (byte*)pub_key, (word32*)pub_key_len);
    if (ret < 0) {
        APP_CRYPTO_Print("Error wc_ecc_export_x963()\r\n");
        wc_ecc_free(&key);
        return ret;
    }
       
    wc_ecc_free(&key);
    return 0;
}

// *****************************************************************************
// *****************************************************************************
int APP_CRYPTO_Get_Signature(uint8_t *cert_sig, uint8_t *sig, uint32_t *sig_len )
{
    uint8_t *sig_seq =  cert_sig;
    uint8_t *sig_r, *sig_r_effective, *sig_s, *sig_s_effective;
    uint32_t sig_seq_len = 0, sig_r_len, sig_s_len;
    uint32_t sig_idx =0;
    
    // Check for Signature sequence tag (0x30)
    if (sig_seq[sig_idx] != 0x30) 
    {
        APP_CRYPTO_Print("Invalid signature sequence tag\n");
        return -1;
    }
    sig_seq_len = sig_seq[++sig_idx];
    
    //check the R component   
    if (sig_seq[++sig_idx] != ASN_INTEGER)      //check 0x02
    {
        APP_CRYPTO_Print("Invalid R ASN_INTEGER\n");
        return -1;
    }
    sig_r_len = sig_seq[++sig_idx];
    //get r value
    sig_r_effective = &sig_seq[++sig_idx];
    sig_idx = sig_idx + sig_r_len;
    
    //if have padding byte 0x00, then the byte after padding 0x00 is r value
    if ((sig_r_len == 0x21) && (*sig_r_effective == 0x00) && (*(sig_r_effective + 1) & 0x80)) 
    {
        sig_r_effective = sig_r_effective + 1 ;     //point to the next byte
        sig_r_len = sig_r_len - 1;
    }
            
    //check the S component   
    if (sig_seq[sig_idx] != ASN_INTEGER)      //check 0x02
    {
        APP_CRYPTO_Print("Invalid S ASN_INTEGER\n");
        return -1;
    }
    sig_s_len = sig_seq[++sig_idx];
    
    sig_s_effective = &sig_seq[++sig_idx];
    //if have padding byte 0x00, then the byte after padding 0x00 is s value
    if ((sig_s_len == 0x21) && (*sig_s_effective == 0x00) && (*(sig_s_effective + 1) & 0x80)) 
    {
        sig_s_effective = sig_s_effective + 1 ;     //point to the next byte
        sig_s_len = sig_s_len - 1;
    }
    
    memcpy(sig, sig_r_effective, sig_r_len);
    memcpy(sig + sig_r_len, sig_s_effective,sig_s_len);
    
    *sig_len = sig_r_len + sig_s_len;
    
    return 0;
}

// *****************************************************************************
// *****************************************************************************
int APP_CRYPTO_Decode_Parse_PEM_Cert(uint8_t *pemCert, uint32_t lenght, DecodedCert * cert, int cert_type)
{
    int ret;
    uint8_t     *ptr_pemCert = pemCert;
    uint32_t    pemCertLen = lenght;
    
    //Convert PEM to DER
    DerBuffer* der = NULL;
    ret = PemToDer(pemCert, pemCertLen, cert_type, &der, heapHint, NULL, NULL);
    if (ret < 0) 
    {
        APP_CRYPTO_Print("PemToDer failed: %d\n", ret);
        return ret;
    }

    APP_CRYPTO_Print("Cert DER: \r\n");
    APP_CRYPTO_Print_Hex(der->buffer, der->length);
    
    //Parse the DER certificate
    wc_InitDecodedCert(cert, der->buffer, der->length, heapHint);
    ret = wc_ParseCert(cert, cert_type, (int)NULL, NULL); 
    if (ret < 0) 
    {
        APP_CRYPTO_Print("Error parsing DER certificate\r\n");
    }
    
    FreeDer(&der);
}

// *****************************************************************************
// *****************************************************************************
int APP_CRYPTO_Parse_PEM_Cert(uint8_t *pemCert, uint32_t lenght, APP_CRYPTO_CERT_INFO *cert_info, int cert_type)
{
    int ret;
    DecodedCert cert;
    
    ret = APP_CRYPTO_Decode_Parse_PEM_Cert(pemCert, lenght, &cert, cert_type);
    if(ret < 0)
    {
        APP_CRYPTO_Print("Error decode and parse cert\r\n");
        wc_FreeDecodedCert(&cert);
        return ret;
    }
      
    //Get raw ECC public key from the cert.publicKey ASN.1 format
    uint8_t pub_key[133];
    uint32_t pub_key_len = sizeof(pub_key);
    
    ret = APP_CRYPTO_Get_Pub_Key((uint8_t *) cert.publicKey, ( uint32_t)cert.pubKeySize, (uint8_t *)pub_key, (uint32_t *) &pub_key_len );
    if(ret < 0)
    {
        APP_CRYPTO_Print("Error getting raw public key\r\n");
        wc_FreeDecodedCert(&cert);
        
        return ret;
    }
    //APP_Print_Str("Public Key: \r\n");
    //APP_Print_Byte(pub_key, pub_key_len);
    
    //Get cert signature
    uint8_t     signature[64];      
    uint32_t    signature_len;
    
    ret = APP_CRYPTO_Get_Signature((uint8_t *) cert.signature, (uint8_t *) signature, (uint32_t *) &signature_len);
    if (ret < 0) {
        APP_CRYPTO_Print("Error getting signature\r\n");
        wc_FreeDecodedCert(&cert);
        
        return ret;
    }
    //APP_Print_Str("Certificate signature:\r\n");
    //APP_Print_Byte(signature, signature_len);
    
    //Get the TBS data
    uint32_t    lenBytes;
    uint8_t     tbs[1024];  
    uint32_t    tbs_len;
    
    uint8_t *tmpDatabuf = (uint8_t *) cert.source + cert.certBegin;     //word32  certBegin; offset to start of cert 
    uint32_t contentLen = APP_CRYPTO_ASN1_Get_Length(tmpDatabuf+1, &lenBytes);
    tbs_len = 1 + lenBytes + contentLen;
    memcpy(tbs, tmpDatabuf, tbs_len);
    //APP_Print_Str("TBS data: \r\n");
    //APP_Print_Byte(tbs, tbs_len);  
    
    //compute the HASH of the TBS
    uint8_t     tbs_hash[32];  //hashed number is 32 bytes
    APP_Crypto_HW_SHA2_256(tbs, tbs_len, tbs_hash);
    //APP_Print_Str("TBS HASH data: \r\n");
    //APP_Print(tbs_hash, sizeof(tbs_hash));
    
    //copy data 
    memcpy(cert_info->tbs_data_hash,tbs_hash, sizeof(tbs_hash) );
    cert_info->tbs_data_hash_len = sizeof(tbs_hash);
    memcpy(cert_info->signature,signature, signature_len);
    cert_info->signature_len = signature_len;
    memcpy(cert_info->tbs_data,tbs, tbs_len);
    cert_info->tbs_data_len = tbs_len;
    memcpy(cert_info->pub_key,pub_key, pub_key_len);
    cert_info->pub_key_len = pub_key_len;
    
    wc_FreeDecodedCert(&cert);
      
    return 0;
}

// *****************************************************************************
// *****************************************************************************
int APP_CRYPTO_Verify_PEM_Cert(uint8_t *pemCert, uint32_t lenght, uint8_t *pubkey, uint32_t pubkeylen, int cert_type)
{
    int ret;
    APP_CRYPTO_CERT_INFO cert_info;
    
    ret = APP_CRYPTO_Parse_PEM_Cert(pemCert, lenght, &cert_info, cert_type);
    if(ret < 0)
    {
        APP_CRYPTO_Print("Error in parse PEM cert\r\n");
        return ret;
    }

    //now doing hardware ECDSA verify
    int8_t ecdsa_hw_verify_status;
    ret = APP_CRYPTO_HW_ECDSA_Verify(cert_info.tbs_data_hash, cert_info.tbs_data_hash_len,
                                    cert_info.signature, cert_info.signature_len,
                                    pubkey, sizeof(pubkeylen),
                                    &ecdsa_hw_verify_status, CRYPTO_ECC_CURVE_SECP256R1);
    
    if ((ret == CRYPTO_DIGISIGN_SUCCESS) && (ecdsa_hw_verify_status ==1))
    {
        return 0;
    }
    else
    {
        return -1;
    }
    
}

// *****************************************************************************
// *****************************************************************************
int APP_CRYPTO_Create_CSR(uint8_t * pub, uint32_t pub_len, uint8_t * priv, uint32_t priv_len, uint8_t * csr_pem, uint32_t * csr_pem_len)
{
    uint32_t i;
    int ret = 0;
    
    //Public is required in CSR, so import the public key
    ecc_key key;
    ret = wc_ecc_init_ex(&key, heapHint, INVALID_DEVID);
    if(ret < 0)
    {
        APP_CRYPTO_Print("Error in wc_ecc_init_ex()\r\n");
        return ret;
    }
    
    ret = wc_ecc_import_x963(pub, pub_len, &key);
    if (ret != 0) 
    {
        APP_CRYPTO_Print("Import public key fail\r\n");
        wc_ecc_free(&key);
        return ret;
    }
    
    //Init certificate
    Cert cert;
    wc_InitCert_ex(&cert, heapHint, INVALID_DEVID);
       
    //Fill in other parameters required in CSR
    strcpy(cert.subject.country, "PIC32CXMTx Demo Country");
    cert.subject.countryEnc = ASN_PRINTABLE_STRING;
    strcpy(cert.subject.org, "PIC32CXMTx Demo Org");
    cert.subject.orgEnc = ASN_PRINTABLE_STRING;
    strcpy(cert.subject.commonName, "PIC32CXMTx Demo Meter");
    cert.subject.commonNameEnc = ASN_PRINTABLE_STRING;
    strcpy(cert.subject.locality, "PIC32CXMTx Demo Locality");
    cert.subject.localityEnc = ASN_PRINTABLE_STRING;
    
    cert.daysValid = 365;
    cert.sigType = CTC_SHA256wECDSA;
    cert.version = 0;
    //Set key usage
    ret = wc_SetKeyUsage(&cert, "digitalSignature");
    if (ret != 0) 
    {
        APP_CRYPTO_Print("Set key usage fail\r\n");
        wc_ecc_free(&key);
        wc_CertFree(&cert);
        return ret;
    }
    //compute public key SHA1
    uint8_t pub_sha1[20];                   //sha1 is always 20 bytes
    APP_Crypto_HW_SHA1(pub, sizeof(pub), pub_sha1);
    memcpy(cert.skid, pub_sha1, sizeof(pub_sha1));
    cert.skidSz = sizeof(pub_sha1);
      
    //Create CSR DER cert
    uint8_t csr_der[1024];  
    int csr_der_size;
    memset(csr_der,0, sizeof(csr_der));     //clear the CSR buffer
    
    //make certificate request
    csr_der_size = wc_MakeCertReq_ex(&cert, csr_der, sizeof(csr_der), ECC_TYPE, &key);
    if (csr_der_size < 0) 
    {
        APP_CRYPTO_Print("Make Cert Req fail\r\n");
        wc_ecc_free(&key);
        wc_CertFree(&cert);
        return csr_der_size;
    }
    
    //compute the HASH of CSR DER
    uint8_t     csr_der_hash[32];           //hashed number is 32 bytes
    APP_Crypto_HW_SHA2_256(csr_der, csr_der_size, csr_der_hash);
    
    //Apply ECDSA sign to get signature, here needs private key
    uint8_t     csr_der_r_s_sig[64];      //signature is 64 bytes 
    
    ret = APP_CRYPTO_HW_ECDSA_Sign(csr_der_hash, sizeof(csr_der_hash), 
                    csr_der_r_s_sig, sizeof(csr_der_r_s_sig), priv, priv_len, CRYPTO_ECC_CURVE_SECP256R1);
    if(ret < 0)
    {
        APP_CRYPTO_Print("CSR DER signing fail\r\n");
        wc_ecc_free(&key);
        wc_CertFree(&cert);
        return -1;
    }
    
    //Encode the signature to ASN.1 format
    uint8_t     csr_der_sig[80];      //ASN.1 format
    uint32_t    csr_der_sig_len = sizeof(csr_der_sig);
    
    ret= StoreECC_DSA_Sig_Bin((byte*) csr_der_sig, (word32*) &csr_der_sig_len, (const byte*) &csr_der_r_s_sig[0], (word32) 32,(const byte*) &csr_der_r_s_sig[32], (word32) 32);
    if(ret < 0)
    {
        APP_CRYPTO_Print("CSR encode ASN.1 Sig fail\r\n");
        wc_ecc_free(&key);
        wc_CertFree(&cert);
        return ret;
    }
        
    //Add signature to CSR DER
    csr_der_size = AddSignature(csr_der, csr_der_size, csr_der_sig, csr_der_sig_len, CTC_SHA256wECDSA);
    if(csr_der_size <= 0)
    {
        APP_CRYPTO_Print("Add_signature error\r\n");
        wc_ecc_free(&key);
        wc_CertFree(&cert);
        return csr_der_size;
    }
        
    //Convert the complete CSR in DER to PEM
    int pemSz = wc_DerToPem(csr_der, csr_der_size, csr_pem, *csr_pem_len, CERTREQ_TYPE);
    if (pemSz < 0) {
        APP_CRYPTO_Print("Error to convert CSR DER to PEM\r\n");
        wc_ecc_free(&key);
        wc_CertFree(&cert);
        return pemSz;
    }
    *csr_pem_len = pemSz;
    
    wc_ecc_free(&key);
    wc_CertFree(&cert);
    
    return 0;
 }

// *****************************************************************************
// *****************************************************************************
int APP_CRYPTO_Sign_A_Cert_From_CSR(uint8_t *csr_pem, uint32_t csr_pem_len, int cert_type, uint8_t * ca_privKey, uint32_t ca_privKey_len, 
                                    uint8_t * cert_pem, uint32_t * cert_pem_len)
{
    //this an example to sign the CSR to get a issued certificate
    int ret;
        
    DecodedCert cert;
    ret = APP_CRYPTO_Decode_Parse_PEM_Cert(csr_pem, csr_pem_len, &cert, cert_type);
    if(ret < 0)
    {
        APP_CRYPTO_Print("Error decode and parse cert\r\n");
        wc_FreeDecodedCert(&cert);
        return ret;
    }
    
    //get the received csr public key and will embedded onto the new cert
    uint8_t pub_key[133];
    uint32_t pub_key_len = sizeof(pub_key);
    
    memset(pub_key,0,pub_key_len);
    //Get the ECC raw public key from ASN.1 format
    ret = APP_CRYPTO_Get_Pub_Key( (uint8_t *) cert.publicKey, cert.pubKeySize, pub_key, &pub_key_len );
    if(ret < 0)
    {
        APP_CRYPTO_Print("Error getting raw public key\r\n");
        wc_FreeDecodedCert(&cert);
        return ret;
    }
   
    //APP_Print_Str("CSR PubKey: \r\n");
    //APP_Print_Hex(pub_key, pub_key_len);
    
    uint8_t derCert[2048];
    int     derCertSize;
    memset(derCert,0, sizeof(derCert)); 
    derCertSize = sizeof(derCert);
    
    Cert  newCert;
    memset(&newCert,0, sizeof(Cert)); 
    wc_InitCert_ex(&newCert,heapHint, INVALID_DEVID );
    
    // Example: Set beforeDate to 2022-02-15 06:00:01 GMT
    newCert.beforeDate[0] = ASN_UTC_TIME; // ASN Tag for UTC Time
    newCert.beforeDate[1] = 13;           // Length of the date string
    memcpy(&newCert.beforeDate[2], "260315060001Z", 13);
    newCert.beforeDateSz = 15;            // Total bytes (1 tag + 1 len + 13 data)
    newCert.afterDate[0] = ASN_UTC_TIME; // ASN Tag for UTC Time
    newCert.afterDate[1] = 13;           // Length of the date string
    memcpy(&newCert.afterDate[2], "270315060001Z", 13);
    newCert.afterDateSz = 15;            // Total bytes (1 tag + 1 len + 13 data)
    newCert.daysValid = 365; // optional, but overridden by explicit dates
    // add extensions 
    newCert.version = 2;
    newCert.isCA = 0;  // End-entity certificate        , newCert.isCA = 1; //The certificate is a CA certificate
    newCert.keyUsage = KEYUSE_DIGITAL_SIG ;
    newCert.extKeyUsage = EXTKEYUSE_SERVER_AUTH;
    newCert.sigType = CTC_SHA256wECDSA;
    
    //compute public key SHA1
    uint8_t pub_sha1[20];                   //sha1 is always 20 bytes
    APP_Crypto_HW_SHA1(pub_key, (uint16_t) sizeof(pub_key), pub_sha1);
    memcpy(newCert.skid, pub_sha1, sizeof(pub_sha1));
    newCert.skidSz = sizeof(pub_sha1);

    wc_SetIssuerBuffer(&newCert, derCert, derCertSize);
    wc_SetSubjectBuffer(&newCert, derCert, derCertSize);
    
    /* Subject DN */
    memset(newCert.subject.country, (int) NULL, CTC_NAME_SIZE);
    strncpy(newCert.subject.country,cert.subjectC,cert.subjectCLen);
    newCert.subject.countryEnc = cert.subjectCEnc;
    
    memset(newCert.subject.commonName, (int) NULL, CTC_NAME_SIZE);
    strncpy(newCert.subject.commonName,cert.subjectCN,cert.subjectCNLen);
    newCert.subject.commonNameEnc = cert.subjectCNEnc;
       
    memset(newCert.subject.org, (int) NULL, CTC_NAME_SIZE);
    strncpy(newCert.subject.org,cert.subjectO,cert.subjectOLen);
    newCert.subject.orgEnc = cert.subjectOEnc;
    
    memset(newCert.subject.locality, (int) NULL, CTC_NAME_SIZE);
    strncpy(newCert.subject.locality,cert.subjectL,cert.subjectLLen);
    newCert.subject.localityEnc = cert.subjectLEnc;
       
    strcpy(newCert.issuer.country, "CA Country");
    newCert.issuer.countryEnc = ASN_PRINTABLE_STRING;

    strcpy(newCert.issuer.org, "CA Org");
    newCert.issuer.orgEnc = ASN_PRINTABLE_STRING;

    strcpy(newCert.issuer.state, "CA state");
    newCert.issuer.stateEnc = ASN_PRINTABLE_STRING;

    strcpy(newCert.issuer.locality, "CA Locality");
    newCert.issuer.localityEnc = ASN_PRINTABLE_STRING;

    strcpy(newCert.issuer.commonName, "CA Root");
    newCert.issuer.commonNameEnc = ASN_PRINTABLE_STRING;
    
    //encode the public key to ASN.1
    ecc_key rev_csr_pub_key;
    ret = wc_ecc_init_ex(&rev_csr_pub_key, heapHint, INVALID_DEVID);
    if(ret < 0)
    {
        APP_CRYPTO_Print("Error on wc_ecc_init_ex\r\n");
        wc_CertFree(&newCert);
        wc_FreeDecodedCert(&cert);
        return ret;
    }
    ret = wc_ecc_import_x963(pub_key, pub_key_len, &rev_csr_pub_key);
    if (ret != 0) 
    {
        APP_CRYPTO_Print("Failed wc_ecc_import_x963\r\n");
        wc_CertFree(&newCert);
        wc_FreeDecodedCert(&cert);
        wc_ecc_key_free(&rev_csr_pub_key);
        return ret;
    }
    
    WC_RNG  rng;
    wc_InitRng_ex(&rng, heapHint, INVALID_DEVID);
    
    //create a new certificate DER
    derCertSize = wc_MakeCert_ex(&newCert, derCert, derCertSize, ECC_TYPE, &rev_csr_pub_key, &rng);
    if (derCertSize < 0) 
    {
        APP_CRYPTO_Print("Error making certificate body %d\r\n", derCertSize);
                
        wc_ecc_key_free(&rev_csr_pub_key);
        wc_CertFree(&newCert);
        wc_rng_free(&rng);
        wc_FreeDecodedCert(&cert);
        return derCertSize;
    }
    //APP_Print_Str("\r\nCertificate DER before signature: ");
    //APP_Print_Number(derCertSize);
    //APP_Print_Str("\r\n");
    //APP_Print(derCert, derCertSize);
    
    //compute the HASH of CSR DER
    uint8_t derCertHash[32];           //hashed number is 32 bytes
    APP_Crypto_HW_SHA2_256(derCert, derCertSize, derCertHash);
        
    //Apply ECDSA sign to sign the certificate, the result is the signature of certificate
    uint8_t derCertSignature[64];      //signature is 64 bytes 
    
    ret = APP_CRYPTO_HW_ECDSA_Sign(derCertHash, sizeof(derCertHash), 
                    derCertSignature, sizeof(derCertSignature), ca_privKey, ca_privKey_len, CRYPTO_ECC_CURVE_SECP256R1);
    if(ret < 0)
    {
        APP_CRYPTO_Print("Certificate DER signing fail\r\n");
        wc_ecc_key_free(&rev_csr_pub_key);
        wc_CertFree(&newCert);
        wc_rng_free(&rng);
        wc_FreeDecodedCert(&cert);
        return -1;
    }
    //APP_Print_Str("Certificate DER_signature:\r\n");
    //APP_Print(derCertSignature, sizeof(derCertSignature));
    
    //Encode the signature to ASN.1 format
    uint8_t     csr_der_sig[80];      //ASN.1 format
    uint32_t    csr_der_sig_len = sizeof(csr_der_sig);
    
    ret= StoreECC_DSA_Sig_Bin(csr_der_sig, (word32*) &csr_der_sig_len, &derCertSignature[0], 32, &derCertSignature[32], 32);
    if(ret < 0)
    {
        APP_CRYPTO_Print("CSR encode ASN.1 Sig fail\r\n");
        wc_ecc_key_free(&rev_csr_pub_key);
        wc_CertFree(&newCert);
        wc_rng_free(&rng);
        wc_FreeDecodedCert(&cert);
        return ret;
    }
        
    //Add signature to DER
    derCertSize = AddSignature(derCert, derCertSize, csr_der_sig, csr_der_sig_len, CTC_SHA256wECDSA);
    if(derCertSize <= 0)
    {
        APP_CRYPTO_Print("Add_signature error\r\n");
        wc_ecc_key_free(&rev_csr_pub_key);
        wc_CertFree(&newCert);
        wc_rng_free(&rng);
        wc_FreeDecodedCert(&cert);
        return derCertSize;
    }
    
    //APP_Print_Str("DER with Sig: \r\n");
    //APP_Print(derCert, derCertSize);
      
    //Convert the complete CSR in DER to PEM
    int pemSz = wc_DerToPem(derCert, derCertSize, cert_pem, *cert_pem_len, CERT_TYPE);
    if (pemSz < 0) {
        APP_CRYPTO_Print("Error to convert CSR DER to PEM\r\n");
        wc_ecc_key_free(&rev_csr_pub_key);
        wc_CertFree(&newCert);
        wc_rng_free(&rng);
        wc_FreeDecodedCert(&cert);
        return pemSz;
    }
    *cert_pem_len = (uint32_t) pemSz;
    
    //APP_Print_Str("Complete Certificate PEM:\r\n");
    //APP_Print_Str_Len(pemBuf,pemSz);
       
    wc_ecc_key_free(&rev_csr_pub_key);
    wc_CertFree(&newCert);
    wc_rng_free(&rng);
    wc_FreeDecodedCert(&cert);
        
    return 0;
}
// *****************************************************************************
// *****************************************************************************
void APP_CRYPTO_AES_GCM_test(void)
{   
    int status;
        
    (void) memset(App_Cipher, 0, app_cryptoData.aes_num_of_byte_to_encdec);
        
    APP_CRYPTO_Print("**AES_GCM128 Encryption**\r\n");
    
    status = APP_Crypto_HW_AES_GCM128_Encrypt(
        App_Plain,
        app_cryptoData.aes_num_of_byte_to_encdec,
        App_Cipher,
        aes_key,
        sizeof(aes_key),
        aes_iv,
        sizeof(aes_iv),
        aes_aad,
        sizeof(aes_aad),
        App_Tag,
        sizeof(App_Tag)
    );
    if (status != CRYPTO_AEAD_CIPHER_SUCCESS)
    {
        APP_CRYPTO_Print("Failed to create cipher text, status: %d\r\n", status);
    }
    else 
    {
        APP_CRYPTO_Print("**Plain Text**\r\n");
        APP_CRYPTO_Print_Hex(App_Plain, app_cryptoData.aes_num_of_byte_to_encdec);
        APP_CRYPTO_Print("**Cipher Text**\r\n");
        APP_CRYPTO_Print_Hex(App_Cipher, app_cryptoData.aes_num_of_byte_to_encdec);
        APP_CRYPTO_Print("**TAG Text**\r\n");
        APP_CRYPTO_Print_Hex(App_Tag,16);
    }
    
    APP_CRYPTO_Print("**AES_GCM128 Encryption**\r\n");
    
    status = APP_Crypto_HW_AES_GCM128_Decrypt(
        App_Cipher,
        app_cryptoData.aes_num_of_byte_to_encdec,
        App_output,
        aes_key,
        sizeof(aes_key),
        aes_iv,
        sizeof(aes_iv),
        aes_aad,
        sizeof(aes_aad),
        App_Tag,
        sizeof(App_Tag)
    );

    if (status != CRYPTO_AEAD_CIPHER_SUCCESS)
    {
        APP_CRYPTO_Print("Failed to decipher text, status: %d\r\n", status);
    }
    else
    {
        APP_CRYPTO_Print("**Decrypted Text**\r\n");
        APP_CRYPTO_Print_Hex(App_output, app_cryptoData.aes_num_of_byte_to_encdec);
        APP_CRYPTO_Print("**TAG Text**\r\n");
        APP_CRYPTO_Print_Hex(App_Tag,16);
    }
   
}
// *****************************************************************************
// *****************************************************************************
void APP_CRYPTO_HASH_test(void)
{   
    int status;
    
    APP_CRYPTO_Print("\r\nHardware HASH SHA1\r\n");
    status = APP_Crypto_HW_SHA1(SHA_Input_Test_Data, app_cryptoData.sha_num_of_byte_to_hash, SHA1_Output_Digest);
    if(status != CRYPTO_HASH_SUCCESS)
    {
        APP_CRYPTO_Print("\r\nHardware HASH SHA1 FAIL\r\n");
    }
    else
    {
        APP_CRYPTO_Print("SHA1 input data, bytes: %d\r\n", app_cryptoData.sha_num_of_byte_to_hash);
        APP_CRYPTO_Print_Hex(SHA_Input_Test_Data, app_cryptoData.sha_num_of_byte_to_hash);
        APP_CRYPTO_Print("Digest (SHA1 is always 20 bytes):\r\n");
        APP_CRYPTO_Print_Hex(SHA1_Output_Digest, sizeof(SHA1_Output_Digest));
    }                  
            
    APP_CRYPTO_Print("\r\nHardware HASH SHA256\r\n");
    status = APP_Crypto_HW_SHA2_256(SHA_Input_Test_Data,app_cryptoData.sha_num_of_byte_to_hash,SHA2_Output_Digest);
    if(status != CRYPTO_HASH_SUCCESS)
    {
        APP_CRYPTO_Print("\r\nHardware HASH SHA256 FAIL\r\n");
    }
    else
    {
        APP_CRYPTO_Print("SHA256 input data, bytes: %d\r\n", app_cryptoData.sha_num_of_byte_to_hash);
        APP_CRYPTO_Print_Hex(SHA_Input_Test_Data, app_cryptoData.sha_num_of_byte_to_hash);
        APP_CRYPTO_Print("Digest (32 bytes):\r\n");
        APP_CRYPTO_Print_Hex(SHA2_Output_Digest, sizeof(SHA2_Output_Digest));
    }
    
}
// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_CRYPTO_Initialize ( void )

  Remarks:
    See prototype in app_crypto.h.
 */

void APP_CRYPTO_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    app_cryptoData.state = APP_CRYPTO_STATE_INIT;

    APP_CRYPTO_Wolfssl_Allocate_Heap_memory();
    
}


/******************************************************************************
  Function:
    void APP_CRYPTO_Tasks ( void )

  Remarks:
    See prototype in app_crypto.h.
 */

void APP_CRYPTO_Tasks ( void )
{
    uint32_t i;
    int ret; 
    uint8_t     priv[32];
    uint8_t     pub[65];
    
    /* Check the application's current state. */
    switch ( app_cryptoData.state )
    {
        /* Application's initial state. */
        case APP_CRYPTO_STATE_INIT:
        {
            bool appInitialized = true;


            if (appInitialized)
            {

                app_cryptoData.state = APP_CRYPTO_STATE_SERVICE_TASKS;
            }
            break;
        }

        case APP_CRYPTO_STATE_SERVICE_TASKS:
        {

            break;
        }
        
        case APP_CRYPTO_STATE_TRNG:
        {
            APP_CRYPTO_HW_Trng_Generate(app_rngData, app_rngLen);
            APP_CRYPTO_Print_Hex(app_rngData, app_rngLen);
            
            app_cryptoData.state = APP_CRYPTO_STATE_SERVICE_TASKS;
            break;
        }
        
        case APP_CRYPTO_STATE_GEN_KEY:
        {
            APP_CRYPTO_HW_Gen_Key_P256(app_PublicKey, app_PrivateKey);
            APP_CRYPTO_Print("PrivateKey: \r\n");
            APP_CRYPTO_Print_Hex(app_PrivateKey, sizeof(app_PrivateKey));

            APP_CRYPTO_Print("PublicKey: \r\n");
            APP_CRYPTO_Print_Hex(app_PublicKey, sizeof(app_PublicKey));
                        
            APP_CRYPTO_Convert_Private_Key_To_PEM(app_PrivateKey, sizeof(app_PrivateKey));
            APP_CRYPTO_Convert_Public_Key_To_PEM(app_PublicKey, sizeof(app_PublicKey));
            
            app_cryptoData.state = APP_CRYPTO_STATE_SERVICE_TASKS;
            break;
        }
        
        case APP_CRYPTO_STATE_AESGCM_ENCRYPT_DECRYPT:
        {
            APP_CRYPTO_AES_GCM_test();
            app_cryptoData.state = APP_CRYPTO_STATE_SERVICE_TASKS;
            break;
        }
        
        case APP_CRYPTO_STATE_HASH:
        {
            APP_CRYPTO_HASH_test();
            app_cryptoData.state = APP_CRYPTO_STATE_SERVICE_TASKS;
            break;
        }  
                
        case APP_CRYPTO_STATE_VERIFY_CERT:
        {
            
            ret = APP_CRYPTO_Verify_PEM_Cert(test_cert_pem, sizeof(test_cert_pem),test_cert_public_key, sizeof(test_cert_public_key), CERT_TYPE);
            if(ret < 0)
            {
                APP_CRYPTO_Print("Verify Cert fail\r\n");
            }
            else
            {
                APP_CRYPTO_Print("Verify Cert success\r\n");
            }
            
            app_cryptoData.state = APP_CRYPTO_STATE_SERVICE_TASKS;
            break;
        }
        case APP_CRYPTO_STATE_CSR:
        {
            uint8_t     csr_pem[2048];
            uint32_t    csr_pem_len = sizeof(csr_pem);
            
            APP_CRYPTO_HW_Gen_Key_P256(pub, priv);
            APP_CRYPTO_Print("CSR PrivKey: \r\n");
            APP_CRYPTO_Print_Hex(priv, sizeof (priv));
            APP_CRYPTO_Print("CSR PubKey: \r\n");
            APP_CRYPTO_Print_Hex(pub, sizeof (pub));
            
            ret = APP_CRYPTO_Create_CSR(pub, sizeof(pub), priv, sizeof(priv), csr_pem, &csr_pem_len);
            if(ret < 0)
            {    
                APP_CRYPTO_Print("Create CSR PEM fail\r\n");
            }
            else
            {
                APP_CRYPTO_Print("Complete CSR PEM:\r\n");
                APP_CRYPTO_Print_Char(csr_pem,csr_pem_len);
            }
            app_cryptoData.state = APP_CRYPTO_STATE_SERVICE_TASKS;
            break;
        }
        case APP_CRYPTO_STATE_SIGN_CSR:
        {
            uint8_t     cert_pem[2048];
            uint32_t    cert_pem_len = sizeof(cert_pem);
            
            //verify the CSR 
            ret = APP_CRYPTO_Verify_PEM_Cert(csr_pem, sizeof(csr_pem), csr_pub, sizeof(csr_pub),CERTREQ_TYPE);   
            if(ret < 0)
            {
                APP_CRYPTO_Print("Verify PEM CSR fail\r\n");
                app_cryptoData.state = APP_CRYPTO_STATE_SERVICE_TASKS;
                break;
            }
            else
                APP_CRYPTO_Print("Verify PEM CSR pass\r\n");
            
            //generate CA key pair
            APP_CRYPTO_HW_Gen_Key_P256(pub, priv);
            APP_CRYPTO_Print("CA PrivKey: \r\n");
            APP_CRYPTO_Print_Hex(priv, sizeof (priv));
            APP_CRYPTO_Print("CA PubKey: \r\n");
            APP_CRYPTO_Print_Hex(pub, sizeof (pub));
            
            //sign CSR to get a certificate in PEM format
            ret = APP_CRYPTO_Sign_A_Cert_From_CSR(csr_pem,sizeof(csr_pem), CERTREQ_TYPE, priv, (uint32_t) sizeof(priv), cert_pem, &cert_pem_len);
            if(ret < 0)
            {
                APP_CRYPTO_Print("Sign CSR fail\r\n");
                app_cryptoData.state = APP_CRYPTO_STATE_SERVICE_TASKS;
                break;
            }
            else
            {
                APP_CRYPTO_Print("Complete Certificate in PEM:\r\n");
                APP_CRYPTO_Print_Char(cert_pem,cert_pem_len);
            }
            
            //check the generated PEM Cert
            ret = APP_CRYPTO_Verify_PEM_Cert(cert_pem, cert_pem_len, pub, sizeof(pub), CERT_TYPE);   
            if(ret < 0)
            {
                APP_CRYPTO_Print("Verify generated PEM cert fail\r\n");
                app_cryptoData.state = APP_CRYPTO_STATE_SERVICE_TASKS;
                break;
            }
            else
                APP_CRYPTO_Print("Verify generated PEM cert pass\r\n");
            
            app_cryptoData.state = APP_CRYPTO_STATE_SERVICE_TASKS;
            
            break;
        }
        
        
        case APP_CRYPTO_STATE_CREATE_SHARE_SECRET:
        {
            uint8_t     self_priv[32], other_priv[32];
            uint8_t     self_pub[65], other_pub[65];
            uint8_t     self_share_secret[32], other_share_secret[32];
            
            //this is to generate
            APP_CRYPTO_HW_Gen_Key_P256(self_pub, self_priv);
            APP_CRYPTO_Print("Self PrivKey: \r\n");
            APP_CRYPTO_Print_Hex(self_priv, sizeof (self_priv));
            APP_CRYPTO_Print("Self PubKey: \r\n");
            APP_CRYPTO_Print_Hex(self_pub, sizeof (self_pub));
            
            APP_CRYPTO_HW_Gen_Key_P256(other_pub, other_priv);
            APP_CRYPTO_Print("Other party PrivKey: \r\n");
            APP_CRYPTO_Print_Hex(other_priv, sizeof (other_priv));
            APP_CRYPTO_Print("Other party PubKey: \r\n");
            APP_CRYPTO_Print_Hex(other_pub, sizeof (other_pub));
            
            ret = APP_CRYPTO_HW_ECDH(self_priv, sizeof (self_priv),other_pub, sizeof (other_pub),self_share_secret, sizeof(self_share_secret));
            if(ret < 0)
            {
                APP_CRYPTO_Print("Self create share secret fail\r\n");
            }
            else
            {
                APP_CRYPTO_Print("Self create share secret:\r\n");
                APP_CRYPTO_Print_Hex(self_share_secret,sizeof(self_share_secret));
            }
            
            ret = APP_CRYPTO_HW_ECDH(other_priv, sizeof (other_priv), self_pub, sizeof (self_pub), other_share_secret, sizeof(other_share_secret));
            if(ret < 0)
            {
                APP_CRYPTO_Print("Other create share secret fail\r\n");
            }
            else
            {
                APP_CRYPTO_Print("Other create share secret:\r\n");
                APP_CRYPTO_Print_Hex(other_share_secret,sizeof(other_share_secret));
            }
            
            app_cryptoData.state = APP_CRYPTO_STATE_SERVICE_TASKS;
            break;
        }
        
        case APP_CRYPTO_STATE_HKDF:
        {
            APP_CRYPTO_HKDF_For_AES_GCM_Key();
            app_cryptoData.state = APP_CRYPTO_STATE_SERVICE_TASKS;
            break;
        }
        
        case APP_CRYPTO_STATE_NIST_CONCATENATION_KDF:
        {
            APP_CRYPTO_NIST_HKDF_For_AES_GCM128_Key();
            app_cryptoData.state = APP_CRYPTO_STATE_SERVICE_TASKS;
            break;
        }
       
        /* TODO: implement your application state machine.*/


        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}


/*******************************************************************************
 End of File
 */
