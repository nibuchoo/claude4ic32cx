/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_multi_cal.c

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

#include "app_multi_cal.h"
#include "definitions.h"

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
    This structure should be initialized by the APP_MULTI_CAL_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_MULTI_CAL_DATA app_multi_calData;
APP_DATALOG_QUEUE_DATA appMultiCalDatalogQueueData;

extern APP_METROLOGY_DATA CACHE_ALIGN app_metrologyData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************
static void _APP_MULTI_CAL_LoadMultiCalParameterCallback(APP_DATALOG_RESULT result)
{
    if (result == APP_DATALOG_RESULT_SUCCESS)
    {
        app_multi_calData.dataIsRdy = true;
        //SYS_CONSOLE_Print(SYS_CONSOLE_INDEX_0, "m_ia_high = 0x%8X\r\n", app_multi_calData.multical_parameter.m_ia_high);
    }
    else
    {
        SYS_CONSOLE_Print(SYS_CONSOLE_INDEX_0, "Read muLti-cal parameter error\r\n");
    }
}
// *****************************************************************************
// *****************************************************************************
static void _APP_MULTI_CAL_StoreMultiCalParameterCallback(APP_DATALOG_RESULT result)
{
    if (result == APP_DATALOG_RESULT_SUCCESS)
    {
        app_multi_calData.dataIsStoreSuccess = true;
    }
    else
    {
        SYS_CONSOLE_Print(SYS_CONSOLE_INDEX_0, "Store muLti-cal parameter error\r\n");
    }
}


// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************
void APP_MULTI_CAL_LoadMultiCalParameterFromMemory(void)
{
    appMultiCalDatalogQueueData.userId = APP_DATALOG_USER_MULTICAL;
    appMultiCalDatalogQueueData.operation = APP_DATALOG_READ;
    appMultiCalDatalogQueueData.pData = (uint8_t *)&app_multi_calData.multical_parameter;
    appMultiCalDatalogQueueData.dataLen = sizeof(APP_MULTI_CAL_PARAMETERS);
    appMultiCalDatalogQueueData.endCallback = _APP_MULTI_CAL_LoadMultiCalParameterCallback;
    appMultiCalDatalogQueueData.date.month = APP_DATALOG_INVALID_MONTH;
    appMultiCalDatalogQueueData.date.year = APP_DATALOG_INVALID_YEAR;

    APP_DATALOG_SendDatalogData(&appMultiCalDatalogQueueData);
}
// *****************************************************************************
// *****************************************************************************
void APP_MULTI_CAL_StoreMultiCalParameterToMemory(void)
{
    appMultiCalDatalogQueueData.userId = APP_DATALOG_USER_MULTICAL;
    appMultiCalDatalogQueueData.operation = APP_DATALOG_WRITE;
    appMultiCalDatalogQueueData.pData = (uint8_t *)&app_multi_calData.multical_parameter;
    appMultiCalDatalogQueueData.dataLen = sizeof(APP_MULTI_CAL_PARAMETERS);
    appMultiCalDatalogQueueData.endCallback = _APP_MULTI_CAL_StoreMultiCalParameterCallback;
    appMultiCalDatalogQueueData.date.month = APP_DATALOG_INVALID_MONTH;
    appMultiCalDatalogQueueData.date.year = APP_DATALOG_INVALID_YEAR;

    APP_DATALOG_SendDatalogData(&appMultiCalDatalogQueueData);
}

// *****************************************************************************
// *****************************************************************************
void APP_MULTI_CAL_SetDefaultMultiCalParameter(void)
{
    DRV_METROLOGY_REGS_CONTROL *pSrc;
    
    pSrc = DRV_METROLOGY_GetControlByDefault();
    
    (void) memcpy((void *)&app_multi_calData.multical_parameter.data[MCAL_M_IA_HIGH_ID],(void *)&pSrc->CAL_M_IA, 15*sizeof(int32_t) ); 
    (void) memcpy((void *)&app_multi_calData.multical_parameter.data[MCAL_M_IA_LOW_ID],(void *)&pSrc->CAL_M_IA, 15*sizeof(int32_t) ); 
    (void) memcpy((void *)&app_multi_calData.multical_parameter.data[MCAL_M_IA_ORG_ID],(void *)&pSrc->CAL_M_IA, 15*sizeof(int32_t) ); 
    
    app_multi_calData.multical_parameter.set_current_h = 0xFFFFFFFF;
    app_multi_calData.multical_parameter.set_current_l = 0;
}
// *****************************************************************************
// *****************************************************************************
void APP_MULTI_CAL_CopyCalParameter(int32_t * pDest)
{
    uint8_t i;
    char regName[18];
    uint32_t regValue32;
    
    for(i = CONTROL_CAL_M_IA; i<=CONTROL_CAL_PH_IN; i++)
    {
        APP_METROLOGY_GetControlRegister(i, &regValue32, regName);
        *pDest++ = regValue32;
    }  
}
// *****************************************************************************
// *****************************************************************************
void APP_MULTI_CAL_SaveCalParameter(void)
{
    const char deviceSeries[] = DEVICE_SERIES;
    char regName[18];
    uint32_t regValue32;
    
    if (strcmp((const char *)&deviceSeries, "PIC32CXMTSH")==0)
    {
        //setting for 1P3W or 1P2W
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_M_IB, &regValue32, regName);
        APP_METROLOGY_SetControlRegister(CONTROL_CAL_M_IC,regValue32 );
    
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_M_VB, &regValue32, regName);
        APP_METROLOGY_SetControlRegister(CONTROL_CAL_M_VC,regValue32 );
    
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_PH_IB, &regValue32, regName);
        APP_METROLOGY_SetControlRegister(CONTROL_CAL_PH_IC,regValue32 );
        
        APP_METROLOGY_GetControlRegister(CONTROL_FEATURE_CTRL, &regValue32, regName);
        APP_METROLOGY_SetControlRegister(CONTROL_FEATURE_CTRL,((regValue32 & (~0x10700))|0x10500) );
    
        APP_METROLOGY_GetControlRegister(CONTROL_METER_TYPE, &regValue32, regName);
        APP_METROLOGY_SetControlRegister(CONTROL_METER_TYPE,(regValue32 | 0x06000000) );
    
        
    }
}

// *****************************************************************************
// *****************************************************************************
void APP_MULTI_CAL_MuiltiCalProcess(void)
{
    char regName[18];
    uint32_t regValue32;
    
    float ia, ib, ic;
    APP_METROLOGY_GetMeasure(MEASURE_IA_RMS, &ia, 0);
    APP_METROLOGY_GetMeasure(MEASURE_IB_RMS, &ib, 0);
    APP_METROLOGY_GetMeasure(MEASURE_IC_RMS, &ic, 0);
    
    //Phase A
    if(ia > (float)app_multi_calData.multical_parameter.set_current_h / 10000)
    {
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_M_IA, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_M_IA_HIGH_ID])     
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_M_IA,app_multi_calData.multical_parameter.data[MCAL_M_IA_HIGH_ID] );
        }
        
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_M_VA, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_M_VA_HIGH_ID]) 
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_M_VA,app_multi_calData.multical_parameter.data[MCAL_M_VA_HIGH_ID] );
        }
        
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_PH_IA, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_PH_IA_HIGH_ID]) 
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_PH_IA,app_multi_calData.multical_parameter.data[MCAL_PH_IA_HIGH_ID] );
        }
        
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_PH_VA, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_PH_VA_HIGH_ID]) 
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_PH_VA,app_multi_calData.multical_parameter.data[MCAL_PH_VA_HIGH_ID] );
        }
        
    }
    else if(ia <= (float)app_multi_calData.multical_parameter.set_current_l / 10000)
    {
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_M_IA, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_M_IA_LOW_ID])     
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_M_IA,app_multi_calData.multical_parameter.data[MCAL_M_IA_LOW_ID] );
        }
        
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_M_VA, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_M_VA_LOW_ID]) 
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_M_VA,app_multi_calData.multical_parameter.data[MCAL_M_VA_LOW_ID] );
        }
        
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_PH_IA, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_PH_IA_LOW_ID]) 
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_PH_IA,app_multi_calData.multical_parameter.data[MCAL_PH_IA_LOW_ID] );
        }
        
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_PH_VA, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_PH_VA_LOW_ID]) 
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_PH_VA,app_multi_calData.multical_parameter.data[MCAL_PH_VA_LOW_ID] );
        }
    }
    else
    {
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_M_IA, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_M_IA_ORG_ID])     
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_M_IA,app_multi_calData.multical_parameter.data[MCAL_M_IA_ORG_ID] );
        }
        
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_M_VA, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_M_VA_ORG_ID]) 
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_M_VA,app_multi_calData.multical_parameter.data[MCAL_M_VA_ORG_ID] );
        }
        
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_PH_IA, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_PH_IA_ORG_ID]) 
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_PH_IA,app_multi_calData.multical_parameter.data[MCAL_PH_IA_ORG_ID] );
        }
        
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_PH_VA, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_PH_VA_ORG_ID]) 
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_PH_VA,app_multi_calData.multical_parameter.data[MCAL_PH_VA_ORG_ID] );
        }
    }
    //Phase B
    if(ib > (float)app_multi_calData.multical_parameter.set_current_h / 10000)
    {
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_M_IB, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_M_IB_HIGH_ID])     
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_M_IB,app_multi_calData.multical_parameter.data[MCAL_M_IB_HIGH_ID] );
        }
        
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_M_VB, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_M_VB_HIGH_ID]) 
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_M_VB,app_multi_calData.multical_parameter.data[MCAL_M_VB_HIGH_ID] );
        }
        
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_PH_IB, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_PH_IB_HIGH_ID]) 
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_PH_IB,app_multi_calData.multical_parameter.data[MCAL_PH_IB_HIGH_ID] );
        }
        
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_PH_VB, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_PH_VB_HIGH_ID]) 
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_PH_VB,app_multi_calData.multical_parameter.data[MCAL_PH_VB_HIGH_ID] );
        }
        
    }
    else if(ib <= (float)app_multi_calData.multical_parameter.set_current_l / 10000)
    {
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_M_IB, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_M_IB_LOW_ID])     
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_M_IB,app_multi_calData.multical_parameter.data[MCAL_M_IB_LOW_ID] );
        }
        
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_M_VB, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_M_VB_LOW_ID]) 
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_M_VB,app_multi_calData.multical_parameter.data[MCAL_M_VB_LOW_ID] );
        }
        
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_PH_IB, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_PH_IB_LOW_ID]) 
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_PH_IB,app_multi_calData.multical_parameter.data[MCAL_PH_IB_LOW_ID] );
        }
        
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_PH_VB, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_PH_VB_LOW_ID]) 
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_PH_VB,app_multi_calData.multical_parameter.data[MCAL_PH_VB_LOW_ID] );
        }
    }
    else
    {
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_M_IB, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_M_IB_ORG_ID])     
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_M_IB,app_multi_calData.multical_parameter.data[MCAL_M_IB_ORG_ID] );
        }
        
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_M_VB, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_M_VB_ORG_ID]) 
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_M_VB,app_multi_calData.multical_parameter.data[MCAL_M_VB_ORG_ID] );
        }
        
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_PH_IB, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_PH_IB_ORG_ID]) 
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_PH_IB,app_multi_calData.multical_parameter.data[MCAL_PH_IB_ORG_ID] );
        }
        
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_PH_VB, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_PH_VB_ORG_ID]) 
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_PH_VB,app_multi_calData.multical_parameter.data[MCAL_PH_VB_ORG_ID] );
        }
    }
    //Phase C
    if(ic > (float)app_multi_calData.multical_parameter.set_current_h / 10000)
    {
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_M_IC, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_M_IC_HIGH_ID])     
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_M_IC,app_multi_calData.multical_parameter.data[MCAL_M_IC_HIGH_ID] );
        }
        
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_M_VC, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_M_VC_HIGH_ID]) 
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_M_VC,app_multi_calData.multical_parameter.data[MCAL_M_VC_HIGH_ID] );
        }
        
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_PH_IC, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_PH_IC_HIGH_ID]) 
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_PH_IC,app_multi_calData.multical_parameter.data[MCAL_PH_IC_HIGH_ID] );
        }
        
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_PH_VC, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_PH_VC_HIGH_ID]) 
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_PH_VC,app_multi_calData.multical_parameter.data[MCAL_PH_VC_HIGH_ID] );
        }
        
    }
    else if(ic <= (float)app_multi_calData.multical_parameter.set_current_l / 10000)
    {
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_M_IC, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_M_IC_LOW_ID])     
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_M_IC,app_multi_calData.multical_parameter.data[MCAL_M_IC_LOW_ID] );
        }
        
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_M_VC, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_M_VC_LOW_ID]) 
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_M_VC,app_multi_calData.multical_parameter.data[MCAL_M_VC_LOW_ID] );
        }
        
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_PH_IC, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_PH_IC_LOW_ID]) 
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_PH_IC,app_multi_calData.multical_parameter.data[MCAL_PH_IC_LOW_ID] );
        }
        
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_PH_VC, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_PH_VC_LOW_ID]) 
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_PH_VC,app_multi_calData.multical_parameter.data[MCAL_PH_VC_LOW_ID] );
        }
    }
    else
    {
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_M_IC, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_M_IC_ORG_ID])     
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_M_IC,app_multi_calData.multical_parameter.data[MCAL_M_IC_ORG_ID] );
        }
        
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_M_VC, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_M_VC_ORG_ID]) 
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_M_VC,app_multi_calData.multical_parameter.data[MCAL_M_VC_ORG_ID] );
        }
        
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_PH_IC, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_PH_IC_ORG_ID]) 
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_PH_IC,app_multi_calData.multical_parameter.data[MCAL_PH_IC_ORG_ID] );
        }
        
        APP_METROLOGY_GetControlRegister(CONTROL_CAL_PH_VC, &regValue32, regName);
        if(regValue32 != app_multi_calData.multical_parameter.data[MCAL_PH_VC_ORG_ID]) 
        {
            APP_METROLOGY_SetControlRegister(CONTROL_CAL_PH_VC,app_multi_calData.multical_parameter.data[MCAL_PH_VC_ORG_ID] );
        }
    }    
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_MULTI_CAL_Initialize ( void )

  Remarks:
    See prototype in app_multi_cal.h.
 */

void APP_MULTI_CAL_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    app_multi_calData.state = APP_MULTI_CAL_STATE_INIT;



    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}


/******************************************************************************
  Function:
    void APP_MULTI_CAL_Tasks ( void )

  Remarks:
    See prototype in app_multi_cal.h.
 */

void APP_MULTI_CAL_Tasks ( void )
{

    /* Check the application's current state. */
    switch ( app_multi_calData.state )
    {
        /* Application's initial state. */
        case APP_MULTI_CAL_STATE_INIT:
        {
            bool appInitialized = true;

            if (appInitialized)
            {
                if(app_multi_calData.dataIsRdy == false)
                {
                    if (APP_DATALOG_GetStatus() == APP_DATALOG_STATE_READY)
                    {
                        if (APP_DATALOG_DataIsValid(APP_DATALOG_USER_MULTICAL))
                        {
                            APP_MULTI_CAL_LoadMultiCalParameterFromMemory();
                        }
                        else
                        {
                            APP_MULTI_CAL_SetDefaultMultiCalParameter();
                            APP_MULTI_CAL_StoreMultiCalParameterToMemory();
                        }
                    }
                }
                else
                {
                    app_multi_calData.multical_enable = true;
                    app_multi_calData.state = APP_MULTI_CAL_STATE_SERVICE_TASKS;
                }
                    
            }
            break;
        }

        case APP_MULTI_CAL_STATE_SERVICE_TASKS:
        {
            if(app_multi_calData.integrationFlag == true) 
            {
                app_multi_calData.integrationFlag = false;
                if( app_multi_calData.multical_enable == true)
                {
                    if(app_metrologyData.state == APP_METROLOGY_STATE_RUNNING)
                        APP_MULTI_CAL_MuiltiCalProcess();    
                }
            }
            
            break;
        }
        
        case APP_MULTI_CAL_STATE_MULTICAL_PROCESS:
        {
            app_multi_calData.state = APP_MULTI_CAL_STATE_SERVICE_TASKS;
            break;
        }
        
        case APP_MULTI_CAL_STATE_READ_MCAL_ORG_PARAMETER:
        {
            APP_MULTI_CAL_LoadMultiCalParameterFromMemory();
            for(uint8_t i = MCAL_M_IA_ORG_ID; i <= MCAL_PH_IN_ORG_ID; i++)
                SYS_CONSOLE_Print(SYS_CONSOLE_INDEX_0, "i=%d, 0x%08X\r\n", i, app_multi_calData.multical_parameter.data[i]);
            
            app_multi_calData.state = APP_MULTI_CAL_STATE_SERVICE_TASKS;
            break;
        }
        case APP_MULTI_CAL_STATE_READ_MCAL_LOW_PARAMETER:
        {
            APP_MULTI_CAL_LoadMultiCalParameterFromMemory();
            for(uint8_t i = MCAL_M_IA_LOW_ID; i <= MCAL_PH_IN_LOW_ID; i++)
                SYS_CONSOLE_Print(SYS_CONSOLE_INDEX_0, "i=%d, 0x%08X\r\n", i, app_multi_calData.multical_parameter.data[i]);
            
            app_multi_calData.state = APP_MULTI_CAL_STATE_SERVICE_TASKS;
            break;
        }
        case APP_MULTI_CAL_STATE_READ_MCAL_HIGH_PARAMETER:
        {
            APP_MULTI_CAL_LoadMultiCalParameterFromMemory();
            for(uint8_t i = MCAL_M_IA_HIGH_ID; i <= MCAL_PH_IN_HIGH_ID; i++)
                SYS_CONSOLE_Print(SYS_CONSOLE_INDEX_0, "i=%d, 0x%08X\r\n", i, app_multi_calData.multical_parameter.data[i]);
            
            app_multi_calData.state = APP_MULTI_CAL_STATE_SERVICE_TASKS;
            break;
        }
        case APP_MULTI_CAL_STATE_SET_MCAL_LOW_PARAMETER:
        {
            APP_MULTI_CAL_CopyCalParameter(&app_multi_calData.multical_parameter.data[MCAL_M_IA_LOW_ID]);
            APP_MULTI_CAL_StoreMultiCalParameterToMemory();
            app_multi_calData.state = APP_MULTI_CAL_STATE_SERVICE_TASKS;
            break;
        }
        case APP_MULTI_CAL_STATE_SET_MCAL_HIGH_PARAMETER:
        {
            APP_MULTI_CAL_CopyCalParameter(&app_multi_calData.multical_parameter.data[MCAL_M_IA_HIGH_ID]);
            APP_MULTI_CAL_StoreMultiCalParameterToMemory();
            app_multi_calData.state = APP_MULTI_CAL_STATE_SERVICE_TASKS;
            break;
        }
        case APP_MULTI_CAL_STATE_SET_MCAL_ORG_PARAMETER:
        {
            APP_MULTI_CAL_CopyCalParameter(&app_multi_calData.multical_parameter.data[MCAL_M_IA_ORG_ID]);
            APP_MULTI_CAL_StoreMultiCalParameterToMemory();
            app_multi_calData.state = APP_MULTI_CAL_STATE_SERVICE_TASKS;
            break;
        }
        
        case APP_MULTI_CAL_STATE_SET_MCAL_LOW_CURRENT:
        {
            //SYS_CONSOLE_Print(SYS_CONSOLE_INDEX_0, "set_current_l = %u\r\n", app_multi_calData.multical_parameter.set_current_l);
            APP_MULTI_CAL_StoreMultiCalParameterToMemory();
            app_multi_calData.state = APP_MULTI_CAL_STATE_SERVICE_TASKS;
            break;
        }
        case APP_MULTI_CAL_STATE_SET_MCAL_HIGH_CURRENT:
        {
            //SYS_CONSOLE_Print(SYS_CONSOLE_INDEX_0, "set_current_h = %u\r\n", app_multi_calData.multical_parameter.set_current_h);
            APP_MULTI_CAL_StoreMultiCalParameterToMemory();
            app_multi_calData.state = APP_MULTI_CAL_STATE_SERVICE_TASKS;
            break;
        }
        
        case APP_MULTI_CAL_STATE_READ_MCAL_HIGH_CURRENT:
        {
            APP_MULTI_CAL_LoadMultiCalParameterFromMemory();
            SYS_CONSOLE_Print(SYS_CONSOLE_INDEX_0, "set_current_h = %.3fA\r\n", (float)app_multi_calData.multical_parameter.set_current_h / 10000);
            SYS_CONSOLE_Print(SYS_CONSOLE_INDEX_0, "set_current_l = %.3fA\r\n", (float)app_multi_calData.multical_parameter.set_current_l / 10000);
            app_multi_calData.state = APP_MULTI_CAL_STATE_SERVICE_TASKS;
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
