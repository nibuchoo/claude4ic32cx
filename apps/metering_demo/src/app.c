/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app.c

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

#include "app.h"
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
    This structure should be initialized by the APP_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_DATA appData;
extern APP_METROLOGY_DATA CACHE_ALIGN app_metrologyData;
// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************
static void _APP_Pulse0Callback(void)
{
    appData.pulse0_flag = true;
}
static void _APP_Pulse1Callback(void)
{
    appData.pulse1_flag = true;
}
/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

void APP_Print_Str_Len(char *str, uint32_t len)
{
    while(len--)
    {
        FLEXCOM0_USART_Write((uint8_t *) str++,1); while(!(FLEXCOM0_USART_TransmitComplete()));
    }
}
// *****************************************************************************
// *****************************************************************************
void APP_Print_Str(char *str)
{
    while(*str)
    {
        FLEXCOM0_USART_Write((uint8_t *) str++,1); while(!(FLEXCOM0_USART_TransmitComplete()));
    }
}
// *****************************************************************************
// *****************************************************************************
void APP_Print_Byte(uint8_t *data, uint16_t len)
{
    uint16_t i;
        
    for(i = 0; i < len; i++)
    {
        if((i > 0) && (i%16 == 0))
        {
            appData.prtlen=sprintf((char *) appData.prtbuf, (const char *) "\r\n");
            FLEXCOM0_USART_Write(appData.prtbuf,appData.prtlen); while(!(FLEXCOM0_USART_TransmitComplete()));
        }
        //SYS_CONSOLE_Print(SYS_CONSOLE_INDEX_0, "%02X ", *(data + i));
        appData.prtlen=sprintf((char *) appData.prtbuf, (const char *) "%02X", *(data + i));
        FLEXCOM0_USART_Write(appData.prtbuf,appData.prtlen); while(!(FLEXCOM0_USART_TransmitComplete()));
        
        
    }
    //SYS_CONSOLE_Print(SYS_CONSOLE_INDEX_0, "\r\n");
    appData.prtlen=sprintf((char *) appData.prtbuf, (const char *) "\r\n");
    FLEXCOM0_USART_Write(appData.prtbuf,appData.prtlen); while(!(FLEXCOM0_USART_TransmitComplete()));
}
// *****************************************************************************
// *****************************************************************************
void APP_Print_Byte_No_NewLine(uint8_t *data, uint16_t len)
{
    uint16_t i;
        
    for(i = 0; i < len; i++)
    {
        appData.prtlen=sprintf((char *) appData.prtbuf, (const char *) "%02X", *(data + i));
        FLEXCOM0_USART_Write(appData.prtbuf,appData.prtlen); while(!(FLEXCOM0_USART_TransmitComplete()));
    }
    appData.prtlen=sprintf((char *) appData.prtbuf, (const char *) "\r\n");
    FLEXCOM0_USART_Write(appData.prtbuf,appData.prtlen); while(!(FLEXCOM0_USART_TransmitComplete()));
}
// *****************************************************************************
// *****************************************************************************
void APP_Print_Number(uint32_t data)
{
    appData.prtlen=sprintf((char *) appData.prtbuf, (const char *) "(dec) %ld", data);
    FLEXCOM0_USART_Write(appData.prtbuf,appData.prtlen); while(!(FLEXCOM0_USART_TransmitComplete()));
    
    appData.prtlen=sprintf((char *) appData.prtbuf, (const char *) "\r\n");
    FLEXCOM0_USART_Write(appData.prtbuf,appData.prtlen); while(!(FLEXCOM0_USART_TransmitComplete()));
}
// *****************************************************************************
// *****************************************************************************
void APP_Print_Number_No_NewLine(uint32_t data)
{
    appData.prtlen=sprintf((char *) appData.prtbuf, (const char *) "%08lX ", data);
    FLEXCOM0_USART_Write(appData.prtbuf,appData.prtlen); while(!(FLEXCOM0_USART_TransmitComplete()));
}
// *****************************************************************************
// *****************************************************************************
void APP_Print(uint8_t *data, uint16_t len)
{
    uint16_t i;
    
    for(i = 0; i < len; i++)
    {
        appData.prtlen=sprintf((char *) appData.prtbuf, (const char *) "%02X", *(data + i));
        FLEXCOM0_USART_Write(appData.prtbuf,appData.prtlen); while(!(FLEXCOM0_USART_TransmitComplete()));
        //SYS_CONSOLE_Print(SYS_CONSOLE_INDEX_0, "%02X", *(data + i));
    }
    //SYS_CONSOLE_Print(SYS_CONSOLE_INDEX_0, "\r\n");
    appData.prtlen=sprintf((char *) appData.prtbuf, (const char *) "\r\n");
    FLEXCOM0_USART_Write(appData.prtbuf,appData.prtlen); while(!(FLEXCOM0_USART_TransmitComplete()));
    
}
// *****************************************************************************
// *****************************************************************************
void APP_Print_Hex(uint8_t *data, uint16_t len)
{
    uint16_t i;
        
    for(i = 0; i < len; i++)
    {
        if((i > 0) && (i%16 == 0))
        {
            appData.prtlen=sprintf((char *) appData.prtbuf, (const char *) "\r\n");
            FLEXCOM0_USART_Write(appData.prtbuf,appData.prtlen); while(!(FLEXCOM0_USART_TransmitComplete()));
        }
        //SYS_CONSOLE_Print(SYS_CONSOLE_INDEX_0, "%02X ", *(data + i));
        appData.prtlen=sprintf((char *) appData.prtbuf, (const char *) "0x%02X,", *(data + i));
        FLEXCOM0_USART_Write(appData.prtbuf,appData.prtlen); while(!(FLEXCOM0_USART_TransmitComplete()));
        
        
    }
    //SYS_CONSOLE_Print(SYS_CONSOLE_INDEX_0, "\r\n");
    appData.prtlen=sprintf((char *) appData.prtbuf, (const char *) "\r\n");
    FLEXCOM0_USART_Write(appData.prtbuf,appData.prtlen); while(!(FLEXCOM0_USART_TransmitComplete()));
}


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;
    appData.pulse1_cnt = 0;
    appData.timer_10ms_start_flag = false;
    appData.timer_10ms_stop_flag = false;

    DRV_METROLOGY_Pulse0CallbackRegister(_APP_Pulse0Callback);
    DRV_METROLOGY_Pulse1CallbackRegister(_APP_Pulse1Callback);
    
    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}


/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks ( void )
{

    /* Check the application's current state. */
    switch ( appData.state )
    {
        /* Application's initial state. */
        case APP_STATE_INIT:
        {
            bool appInitialized = true;


            if (appInitialized)
            {
                appData.app_metStatusData = DRV_METROLOGY_GetStatusData();
                
                appData.state = APP_STATE_SERVICE_TASKS;
                
                switch (RSTC_ResetCauseGet()) {
                    case RSTC_SR_RSTTYP_GENERAL_RST:
                        appData.prtlen = sprintf((char *) appData.prtbuf, (const char *) "\r\n0.RSTC_SR_RSTTYP_GENERAL_RST \r\n");
                        FLEXCOM0_USART_Write(appData.prtbuf, appData.prtlen);
                        while (!(FLEXCOM0_USART_TransmitComplete()));
                        break;
                    case RSTC_SR_RSTTYP_BACKUP_RST:
                        appData.prtlen = sprintf((char *) appData.prtbuf, (const char *) "\r\n1.RSTC_SR_RSTTYP_BACKUP_RST \r\n");
                        FLEXCOM0_USART_Write(appData.prtbuf, appData.prtlen);
                        while (!(FLEXCOM0_USART_TransmitComplete()));
                        break;
                    case RSTC_SR_RSTTYP_WDT0_RST:
                        appData.prtlen = sprintf((char *) appData.prtbuf, (const char *) "\r\n2.RSTC_SR_RSTTYP_WDT0_RST \r\n");
                        FLEXCOM0_USART_Write(appData.prtbuf, appData.prtlen);
                        while (!(FLEXCOM0_USART_TransmitComplete()));
                        break;
                    case RSTC_SR_RSTTYP_SOFT_RST:
                        appData.prtlen = sprintf((char *) appData.prtbuf, (const char *) "\r\n3.RSTC_SR_RSTTYP_SOFT_RST \r\n");
                        FLEXCOM0_USART_Write(appData.prtbuf, appData.prtlen);
                        while (!(FLEXCOM0_USART_TransmitComplete()));
                        break;
                    case RSTC_SR_RSTTYP_USER_RST:
                        appData.prtlen = sprintf((char *) appData.prtbuf, (const char *) "\r\n4.RSTC_SR_RSTTYP_USER_RST \r\n");
                        FLEXCOM0_USART_Write(appData.prtbuf, appData.prtlen);
                        while (!(FLEXCOM0_USART_TransmitComplete()));
                        break;
                    case RSTC_SR_RSTTYP_CORE_SM_RST:
                        appData.prtlen = sprintf((char *) appData.prtbuf, (const char *) "\r\n5.RSTC_SR_RSTTYP_CORE_SM_RST \r\n");
                        FLEXCOM0_USART_Write(appData.prtbuf, appData.prtlen);
                        while (!(FLEXCOM0_USART_TransmitComplete()));
                        break;
                    case RSTC_SR_RSTTYP_CPU_FAIL_RST:
                        appData.prtlen = sprintf((char *) appData.prtbuf, (const char *) "\r\n6.RSTC_SR_RSTTYP_CPU_FAIL_RST \r\n");
                        FLEXCOM0_USART_Write(appData.prtbuf, appData.prtlen);
                        while (!(FLEXCOM0_USART_TransmitComplete()));
                        break;
                    case RSTC_SR_RSTTYP_SLCK_XTAL_RST:
                        appData.prtlen = sprintf((char *) appData.prtbuf, (const char *) "\r\n7.RSTC_SR_RSTTYP_SLCK_XTAL_RST \r\n");
                        FLEXCOM0_USART_Write(appData.prtbuf, appData.prtlen);
                        while (!(FLEXCOM0_USART_TransmitComplete()));
                        break;
                    case RSTC_SR_RSTTYP_WDT1_RST:
                        appData.prtlen = sprintf((char *) appData.prtbuf, (const char *) "\r\n8.RSTC_SR_RSTTYP_WDT1_RST \r\n");
                        FLEXCOM0_USART_Write(appData.prtbuf, appData.prtlen);
                        while (!(FLEXCOM0_USART_TransmitComplete()));
                        break;
                    case RSTC_SR_RSTTYP_PORVDD3V3_RST:
                        appData.prtlen = sprintf((char *) appData.prtbuf, (const char *) "\r\n9.RSTC_SR_RSTTYP_PORVDD3V3_RST \r\n");
                        FLEXCOM0_USART_Write(appData.prtbuf, appData.prtlen);
                        while (!(FLEXCOM0_USART_TransmitComplete()));
                        break;
                 
                    }
                RTC_TimeGet(&appData.current_time);
                appData.prtlen=sprintf((char *)appData.prtbuf, (const char *) "Time: %02u:%02u:%02u \r\n", (unsigned int)appData.current_time.tm_hour, 
                        (unsigned int)appData.current_time.tm_min, (unsigned int)appData.current_time.tm_sec);
                FLEXCOM0_USART_Write(appData.prtbuf,appData.prtlen); while(!(FLEXCOM0_USART_TransmitComplete()));
                
                
                appData.prtlen=sprintf((char *)appData.prtbuf,"DRV_METROLOGY_REGS_CONTROL = %ld bytes\r\n",sizeof(DRV_METROLOGY_REGS_CONTROL));
                FLEXCOM0_USART_Write(appData.prtbuf,appData.prtlen); while(!(FLEXCOM0_USART_TransmitComplete()));
                appData.prtlen=sprintf((char *)appData.prtbuf,"APP_ENERGY_ACCUMULATORS = %ld bytes\r\n",sizeof(APP_ENERGY_ACCUMULATORS));
                FLEXCOM0_USART_Write(appData.prtbuf,appData.prtlen); while(!(FLEXCOM0_USART_TransmitComplete()));
                appData.prtlen=sprintf((char *)appData.prtbuf,"APP_ENERGY_MAX_DEMAND = %ld bytes\r\n",sizeof(APP_ENERGY_MAX_DEMAND));
                FLEXCOM0_USART_Write(appData.prtbuf,appData.prtlen); while(!(FLEXCOM0_USART_TransmitComplete()));
                appData.prtlen=sprintf((char *)appData.prtbuf,"APP_ENERGY_TOU = %ld bytes\r\n",sizeof(APP_ENERGY_TOU));
                FLEXCOM0_USART_Write(appData.prtbuf,appData.prtlen); while(!(FLEXCOM0_USART_TransmitComplete()));
                appData.prtlen=sprintf((char *)appData.prtbuf,"APP_DATALOG_USER_RTC = %ld bytes\r\n",sizeof(struct tm));
                FLEXCOM0_USART_Write(appData.prtbuf,appData.prtlen); while(!(FLEXCOM0_USART_TransmitComplete()));
                appData.prtlen=sprintf((char *)appData.prtbuf,"APP_DATALOG_USER_EVENTS = %ld bytes\r\n",sizeof(APP_EVENTS_EVENT_DATA));
                FLEXCOM0_USART_Write(appData.prtbuf,appData.prtlen); while(!(FLEXCOM0_USART_TransmitComplete()));
                appData.prtlen=sprintf((char *)appData.prtbuf,"APP_DATALOG_USER_MULTICAL = %ld bytes\r\n",sizeof(APP_MULTI_CAL_PARAMETERS));
                FLEXCOM0_USART_Write(appData.prtbuf,appData.prtlen); while(!(FLEXCOM0_USART_TransmitComplete()));
                appData.prtlen=sprintf((char *)appData.prtbuf,"APP_DATALOG_USER_POWEROFFSET = %ld bytes\r\n",sizeof(APP_PWR_OFFSET_PARAMETERS));
                FLEXCOM0_USART_Write(appData.prtbuf,appData.prtlen); while(!(FLEXCOM0_USART_TransmitComplete()));
                appData.prtlen=sprintf((char *)appData.prtbuf,"\r\n");
                FLEXCOM0_USART_Write(appData.prtbuf,appData.prtlen); while(!(FLEXCOM0_USART_TransmitComplete()));
                
                               
            }
            break;
        }

        case APP_STATE_SERVICE_TASKS:
        {

            if(appData.pulse0_flag)
            {
                appData.pulse0_flag = false;
                appData.lcd_energy_progress_bar = 0;
               
            }
            if(appData.pulse1_flag)
            {
                appData.pulse1_flag = false;
                appData.lcd_energy_progress_bar = (appData.app_metStatusData->PULSE1_COUNTER) % 10;
                
            }

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
