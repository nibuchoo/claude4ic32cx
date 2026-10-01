/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_line_freq.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_LINE_FREQ_Initialize" and "APP_LINE_FREQ_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_LINE_FREQ_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _APP_LINE_FREQ_H
#define _APP_LINE_FREQ_H

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
#include "definitions.h"

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
    APP_LINE_FREQ_STATE_INIT=0,
    APP_LINE_FREQ_STATE_SERVICE_TASKS,
    /* TODO: Define states used by the application state machine. */

} APP_LINE_FREQ_STATES;


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
    uint32_t    hour;
    uint32_t    min;
    uint32_t    sec;
} APP_LINE_FREQ_TIME;

typedef struct
{
    /* The application's current state */
    APP_LINE_FREQ_STATES            state;

    bool                            line_sync_flag;
    bool                            line_sync_now;
    bool                            rtc_sec_tick_event;
    bool                            print_flag;
    bool                            line_adjust_time;
    uint32_t                        line_full_cycle_cnt;
    uint32_t                        line_cycle_cnt_per_second; 
    APP_LINE_FREQ_TIME              line_time;
    struct tm                       rtc_time; 
    
    uint32_t                        line_freq;
    uint32_t                        statusReg_state_flag;

} APP_LINE_FREQ_DATA;

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
    void APP_LINE_FREQ_Initialize ( void )

  Summary:
     MPLAB Harmony application initialization routine.

  Description:
    This function initializes the Harmony application.  It places the
    application in its initial state and prepares it to run so that its
    APP_LINE_FREQ_Tasks function can be called.

  Precondition:
    All other system initialization routines should be called before calling
    this routine (in "SYS_Initialize").

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_LINE_FREQ_Initialize();
    </code>

  Remarks:
    This routine must be called from the SYS_Initialize function.
*/

void APP_LINE_FREQ_Initialize ( void );


/*******************************************************************************
  Function:
    void APP_LINE_FREQ_Tasks ( void )

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
    APP_LINE_FREQ_Tasks();
    </code>

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */

void APP_LINE_FREQ_Tasks( void );
void APP_LINE_FREQ_RTC_Sec_Interrupt(void);
void APP_LINE_FREQ_Set_Line_Freq_Time(void);
void APP_LINE_FREQ_UpdateSyncTime(void);
bool APP_LINE_FREQ_RTC_TimeSet( struct tm *  );

extern APP_LINE_FREQ_DATA app_line_freqData;

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _APP_LINE_FREQ_H */

/*******************************************************************************
 End of File
 */

