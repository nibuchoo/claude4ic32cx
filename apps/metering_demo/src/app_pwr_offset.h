/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_pwr_offset.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_PWR_OFFSET_Initialize" and "APP_PWR_OFFSET_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_PWR_OFFSET_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _APP_PWR_OFFSET_H
#define _APP_PWR_OFFSET_H

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
    APP_PWR_OFFSET_STATE_INIT=0,
    APP_PWR_OFFSET_STATE_CALIBRATION_INIT,
    APP_PWR_OFFSET_STATE_CALIBRATION_EXECUTE,
    APP_PWR_OFFSET_STATE_CALIBRATION_END, 
    APP_PWR_OFFSET_STATE_READ_PWROFFSET_DATA,      
    APP_PWR_OFFSET_STATE_SET_PWROFFSET_VOLTAGE,
    APP_PWR_OFFSET_STATE_SERVICE_TASKS,
    /* TODO: Define states used by the application state machine. */

} APP_PWR_OFFSET_STATES;

typedef enum {
    POWER_OS_P_A_ID_110V = 0,
    POWER_OS_P_B_ID_110V,
    POWER_OS_P_C_ID_110V,
    POWER_OS_Q_A_ID_110V,
    POWER_OS_Q_B_ID_110V,
    POWER_OS_Q_C_ID_110V,
    POWER_OS_P_A_ID_220V,
    POWER_OS_P_B_ID_220V,
    POWER_OS_P_C_ID_220V,
    POWER_OS_Q_A_ID_220V,
    POWER_OS_Q_B_ID_220V,
    POWER_OS_Q_C_ID_220V,
    POWER_OS_P_A_ID_480V,
    POWER_OS_P_B_ID_480V,
    POWER_OS_P_C_ID_480V,
    POWER_OS_Q_A_ID_480V,
    POWER_OS_Q_B_ID_480V,
    POWER_OS_Q_C_ID_480V,
    POWER_OS_DATA_END        
}POWER_OS_DATA_ID;

typedef enum {
    POWER_OS_CAL_110V = 0,
    POWER_OS_CAL_220V,
    POWER_OS_CAL_480V,
}POWER_OS_CAL_VOLTAGE;

typedef struct {
    int32_t         data[POWER_OS_DATA_END];
    uint32_t        set_voltage; 
} APP_PWR_OFFSET_PARAMETERS;

typedef struct {
    uint8_t                    voltage;
    uint32_t                   num_integration_period;
    uint32_t                   dsp_update_num;
    uint32_t                   dsp_acc_pa;
	uint32_t                   dsp_acc_pb;
	uint32_t                   dsp_acc_pc;
	uint32_t                   dsp_acc_qa;
	uint32_t                   dsp_acc_qb;
	uint32_t                   dsp_acc_qc;
    
	double                      tmp_dsp_acc_pa;
	double                      tmp_dsp_acc_pb;
	double                      tmp_dsp_acc_pc;
	double                      tmp_dsp_acc_qa;
	double                      tmp_dsp_acc_qb;
	double                      tmp_dsp_acc_qc;
    
    double                      dsp_acc_actual_pa;
    double                      dsp_acc_actual_pb;
    double                      dsp_acc_actual_pc;
    double                      dsp_acc_actual_qa;
    double                      dsp_acc_actual_qb;
    double                      dsp_acc_actual_qc;
    
} APP_PWR_OFFSET_CAL;
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
    APP_PWR_OFFSET_STATES state;

    bool                        dataIsRdy;
    bool                        dataIsStoreSuccess;
    bool                        poweroffset_enable;
    bool                        integrationFlag;
    bool                        show_offset_flag;
    APP_PWR_OFFSET_CAL          power_offset_cal;
    APP_PWR_OFFSET_PARAMETERS   power_offset_parameter;

} APP_PWR_OFFSET_DATA;

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
    void APP_PWR_OFFSET_Initialize ( void )

  Summary:
     MPLAB Harmony application initialization routine.

  Description:
    This function initializes the Harmony application.  It places the
    application in its initial state and prepares it to run so that its
    APP_PWR_OFFSET_Tasks function can be called.

  Precondition:
    All other system initialization routines should be called before calling
    this routine (in "SYS_Initialize").

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_PWR_OFFSET_Initialize();
    </code>

  Remarks:
    This routine must be called from the SYS_Initialize function.
*/

void APP_PWR_OFFSET_Initialize ( void );

void APP_PWR_OFFSET_Read_ACC_Power(void);
void APP_PWR_OFFSET_StorePowerOffsetParameterToMemory(void);
void APP_PWR_OFFSET_LoadPowerOffsetParameterFromMemory(void);
void APP_PWR_OFFSET_SetDefaultPowerOffsetParameter(void);
void APP_PWR_OFFSET_PwerOffsetCalibration(void);
/*******************************************************************************
  Function:
    void APP_PWR_OFFSET_Tasks ( void )

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
    APP_PWR_OFFSET_Tasks();
    </code>

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */

void APP_PWR_OFFSET_Tasks( void );

extern APP_PWR_OFFSET_DATA app_pwr_offsetData;

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _APP_PWR_OFFSET_H */

/*******************************************************************************
 End of File
 */

