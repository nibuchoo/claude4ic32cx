/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_crypto.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_CRYPTO_Initialize" and "APP_CRYPTO_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_CRYPTO_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _APP_CRYPTO_H
#define _APP_CRYPTO_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "configuration.h"

#include "crypto/drivers/wrapper/crypto_ecckeygen_cpkcc44163_wrapper.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************
#define SESSION_ID    1
// *****************************************************************************
/* Application states

  Summary:
    Application states enumeration

  Description:
    This enumeration defines the valid application states.  These states
    determine the behavior of the application at various times.
*/

typedef enum
{
    /* Application's state machine's initial state. */
    APP_CRYPTO_STATE_INIT=0,
    APP_CRYPTO_STATE_SERVICE_TASKS,
    APP_CRYPTO_STATE_TRNG,
    APP_CRYPTO_STATE_AESGCM_ENCRYPT_DECRYPT,   
    APP_CRYPTO_STATE_HASH,
    APP_CRYPTO_STATE_GEN_KEY,
    APP_CRYPTO_STATE_VERIFY_CERT,
    APP_CRYPTO_STATE_CSR,
    APP_CRYPTO_STATE_SIGN_CSR,        
    APP_CRYPTO_STATE_CREATE_SHARE_SECRET,        
    APP_CRYPTO_STATE_HKDF,
    APP_CRYPTO_STATE_NIST_CONCATENATION_KDF,        
            
    /* TODO: Define states used by the application state machine. */

} APP_CRYPTO_STATES;


// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    Application strings and buffers are be defined outside this structure.
 */

typedef struct
{
    /* The application's current state */
    APP_CRYPTO_STATES state;
    uint8_t     prtbuf[1024];
    uint16_t    prtlen;
    
    uint32_t    aes_num_of_byte_to_encdec;
    uint32_t    sha_num_of_byte_to_hash;
    /* TODO: Define any additional data used by the application. */

} APP_CRYPTO_DATA;


typedef struct
{
    uint8_t     der_data[2048];
    uint32_t    der_data_len;
    uint8_t     pub_key[65];           //include the first byte of 0x04 for uncompressed key
    uint32_t    pub_key_len;
    uint8_t     signature[64];
    uint32_t    signature_len;    
    uint8_t     tbs_data[2048];
    uint32_t    tbs_data_len;
    uint8_t     tbs_data_hash[32]; 
    uint32_t    tbs_data_hash_len;
    char        issuer[256];
} APP_CRYPTO_CERT_INFO;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Routines
// *****************************************************************************
// *****************************************************************************
/* These routines are called by drivers when certain events occur.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_CRYPTO_Initialize ( void )

  Summary:
     MPLAB Harmony application initialization routine.

  Description:
    This function initializes the Harmony application.  It places the
    application in its initial state and prepares it to run so that its
    APP_CRYPTO_Tasks function can be called.

  Precondition:
    All other system initialization routines should be called before calling
    this routine (in "SYS_Initialize").

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_CRYPTO_Initialize();
    </code>

  Remarks:
    This routine must be called from the SYS_Initialize function.
*/

void APP_CRYPTO_Initialize ( void );


/*******************************************************************************
  Function:
    void APP_CRYPTO_Tasks ( void )

  Summary:
    MPLAB Harmony Demo application tasks function

  Description:
    This routine is the Harmony Demo application's tasks function.  It
    defines the application's state machine and core logic.

  Precondition:
    The system and application initialization ("SYS_Initialize") should be
    called before calling this.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_CRYPTO_Tasks();
    </code>

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */

void APP_CRYPTO_Tasks( void );

extern APP_CRYPTO_DATA app_cryptoData;

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _APP_CRYPTO_H */

/*******************************************************************************
 End of File
 */

