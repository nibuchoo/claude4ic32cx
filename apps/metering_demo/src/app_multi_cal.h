/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_multi_cal.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_MULTI_CAL_Initialize" and "APP_MULTI_CAL_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_MULTI_CAL_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _APP_MULTI_CAL_H
#define _APP_MULTI_CAL_H

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
    APP_MULTI_CAL_STATE_INIT=0,
    APP_MULTI_CAL_STATE_SERVICE_TASKS,
    APP_MULTI_CAL_STATE_MULTICAL_PROCESS,
    APP_MULTI_CAL_STATE_READ_MCAL_LOW_PARAMETER, 
    APP_MULTI_CAL_STATE_READ_MCAL_HIGH_PARAMETER, 
    APP_MULTI_CAL_STATE_READ_MCAL_ORG_PARAMETER,         
    APP_MULTI_CAL_STATE_SET_MCAL_LOW_PARAMETER,
    APP_MULTI_CAL_STATE_SET_MCAL_HIGH_PARAMETER,
    APP_MULTI_CAL_STATE_SET_MCAL_ORG_PARAMETER,
    APP_MULTI_CAL_STATE_SET_MCAL_LOW_CURRENT,
    APP_MULTI_CAL_STATE_SET_MCAL_HIGH_CURRENT, 
    APP_MULTI_CAL_STATE_READ_MCAL_HIGH_CURRENT,
    /* TODO: Define states used by the application state machine. */

} APP_MULTI_CAL_STATES;

typedef enum {
    MCAL_M_IA_HIGH_ID =0,
    MCAL_M_VA_HIGH_ID,
    MCAL_M_IB_HIGH_ID,
    MCAL_M_VB_HIGH_ID,
    MCAL_M_IC_HIGH_ID,
    MCAL_M_VC_HIGH_ID,
    MCAL_M_IN_HIGH_ID,
    MCAL_M_VD_HIGH_ID,
    MCAL_PH_IA_HIGH_ID,
    MCAL_PH_VA_HIGH_ID,
    MCAL_PH_IB_HIGH_ID,
    MCAL_PH_VB_HIGH_ID,
    MCAL_PH_IC_HIGH_ID,
    MCAL_PH_VC_HIGH_ID,
    MCAL_PH_IN_HIGH_ID,
    MCAL_M_IA_LOW_ID,
    MCAL_M_VA_LOW_ID,
    MCAL_M_IB_LOW_ID,
    MCAL_M_VB_LOW_ID,
    MCAL_M_IC_LOW_ID,
    MCAL_M_VC_LOW_ID,
    MCAL_M_IN_LOW_ID,
    MCAL_M_VD_LOW_ID,
    MCAL_PH_IA_LOW_ID,
    MCAL_PH_VA_LOW_ID,
    MCAL_PH_IB_LOW_ID,
    MCAL_PH_VB_LOW_ID,
    MCAL_PH_IC_LOW_ID,
    MCAL_PH_VC_LOW_ID,
    MCAL_PH_IN_LOW_ID,
    MCAL_M_IA_ORG_ID,
    MCAL_M_VA_ORG_ID,
    MCAL_M_IB_ORG_ID,
    MCAL_M_VB_ORG_ID,
    MCAL_M_IC_ORG_ID,
    MCAL_M_VC_ORG_ID,
    MCAL_M_IN_ORG_ID,
    MCAL_M_VD_ORG_ID,
    MCAL_PH_IA_ORG_ID,
    MCAL_PH_VA_ORG_ID,
    MCAL_PH_IB_ORG_ID,
    MCAL_PH_VB_ORG_ID,
    MCAL_PH_IC_ORG_ID,
    MCAL_PH_VC_ORG_ID,
    MCAL_PH_IN_ORG_ID,
    MCAL_DATA_END,        
             
}MCAL_DATA_ID;

typedef enum {
    MCAL_LEVEL_HIGH = 0,
    MCAL_LEVEL_LOW,
    MCAL_LEVEL_ORG,
    MCAL_LEVEL_INVALID,
} MCAL_LEVEL_ID;


typedef struct {
    int32_t    data[MCAL_DATA_END];
    uint32_t   set_current_h;
    uint32_t   set_current_l;
    
} APP_MULTI_CAL_PARAMETERS;


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
    APP_MULTI_CAL_STATES state;
    
    bool                        integrationFlag;
    bool                        dataIsRdy;
    bool                        dataIsStoreSuccess;
    bool                        multical_enable;
    APP_MULTI_CAL_PARAMETERS    multical_parameter;

    /* TODO: Define any additional data used by the application. */

} APP_MULTI_CAL_DATA;

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
    void APP_MULTI_CAL_Initialize ( void )

  Summary:
     MPLAB Harmony application initialization routine.

  Description:
    This function initializes the Harmony application.  It places the
    application in its initial state and prepares it to run so that its
    APP_MULTI_CAL_Tasks function can be called.

  Precondition:
    All other system initialization routines should be called before calling
    this routine (in "SYS_Initialize").

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_MULTI_CAL_Initialize();
    </code>

  Remarks:
    This routine must be called from the SYS_Initialize function.
*/

void APP_MULTI_CAL_Initialize ( void );
void APP_MULTI_CAL_SetDefaultMultiCalParameter(void);
void APP_MULTI_CAL_StoreMultiCalParameterToMemory(void);
void APP_MULTI_CAL_SaveCalParameter(void);

extern APP_MULTI_CAL_DATA app_multi_calData;

/*******************************************************************************
  Function:
    void APP_MULTI_CAL_Tasks ( void )

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
    APP_MULTI_CAL_Tasks();
    </code>

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */

void APP_MULTI_CAL_Tasks( void );

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _APP_MULTI_CAL_H */

/*******************************************************************************
 End of File
 */

