/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_optouart.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_OPTOUART_Initialize" and "APP_OPTOUART_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_OPTOUART_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/
#ifndef _APP_OPTOUART_H
#define _APP_OPTOUART_H

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
#include "system/system.h"
#include "system/time/sys_time.h"

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
   
    APP_OPTOUART_STATE_INIT=0,
    APP_OPTOUART_STATE_SERVICE_TASKS,
    APP_OPTOUART_STATE_RX_FRAME_RECEIVE,
    APP_OPTOUART_STATE_IDLE,
   

} APP_OPTOUART_STATES;


// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    Application strings and buffers are be defined outside this structure.
 */
#define RX_BUF_SIZE         1024
#define TX_BUF_SIZE         1024

typedef struct
{
    /* The application's current state */
    APP_OPTOUART_STATES state;
    uint8_t         rx_buf[RX_BUF_SIZE];
    uint8_t         tx_buf[RX_BUF_SIZE];
    uint16_t       rx_buf_idx;
    uint16_t       tx_buf_idx;
    uint16_t       rx_buf_len;
    uint16_t       tx_buf_len;
    uint8_t         rx_byte;
    uint8_t         tx_byte;
    /* TODO: Define any additional data used by the application. */

} APP_OPTOUART_DATA;


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
    void APP_OPTOUART_Initialize ( void )

  Summary:
     MPLAB Harmony application initialization routine.

  Description:
    This function initializes the Harmony application.  It places the
    application in its initial state and prepares it to run so that its
    APP_OPTOUART_Tasks function can be called.

  Precondition:
    All other system initialization routines should be called before calling
    this routine (in "SYS_Initialize").

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_OPTOUART_Initialize();
    </code>

  Remarks:
    This routine must be called from the SYS_Initialize function.
*/

void APP_OPTOUART_Initialize ( void );


/*******************************************************************************
  Function:
    void APP_OPTOUART_Tasks ( void )

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
    APP_OPTOUART_Tasks();
    </code>

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */

void APP_OPTOUART_Tasks( void );

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _APP_OPTOUART_H */

/*******************************************************************************
 End of File
 */

