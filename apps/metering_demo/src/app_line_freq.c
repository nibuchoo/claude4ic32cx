/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_line_freq.c

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

#include "app_line_freq.h"

APP_LINE_FREQ_DATA app_line_freqData;
extern APP_METROLOGY_DATA CACHE_ALIGN app_metrologyData;
// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************
/* _APP_LINE_FREQ_SIMULATE_Fullcycle_Callback()
 * This function will be called at each full cycle to count the number of full cycle 
 * and then determine the hour, time and second, which will be use to syn the RTC 
*/
static void _APP_LINE_FREQ_Fullcycle_Callback (void)
{
    if(app_line_freqData.line_sync_now == true)
    {
        app_line_freqData.line_full_cycle_cnt++;
        
        if(app_line_freqData.line_full_cycle_cnt >= app_line_freqData.line_cycle_cnt_per_second)//50 or 60 counter depend on 50Hz or 60Hz
        {
            app_line_freqData.line_full_cycle_cnt = 0;
            app_line_freqData.line_time.sec++;
            if(app_line_freqData.line_time.sec >= 60)
            {
                app_line_freqData.line_time.min++;
                app_line_freqData.line_time.sec = 0;
            }
            if(app_line_freqData.line_time.min >= 60)
            {
                app_line_freqData.line_time.min = 0;
                app_line_freqData.line_time.hour++;
            }
            if(app_line_freqData.line_time.hour >= 24)
            {
                app_line_freqData.line_time.hour = 0;
            }
        }     
    }
}

static void _APP_LINE_FREQ_TC1_CH0_Callback( TC_TIMER_STATUS status, uintptr_t context )
{
    float tmp_freq;
    
    APP_METROLOGY_GetMeasure(MEASURE_FREQ, &tmp_freq, 0);
    app_line_freqData.line_freq = (uint32_t)(tmp_freq*100.0);
    
    app_line_freqData.statusReg_state_flag = app_metrologyData.pMetStatus->STATE_FLAG;
    
    //check any voltage is active or not.
    if ( (app_line_freqData.statusReg_state_flag & STATUS_STATE_FLAG_FREQ_LOCKED_Msk ) && (app_line_freqData.statusReg_state_flag & (STATUS_STATE_FLAG_PH_A_ACTIVE_Msk | STATUS_STATE_FLAG_PH_B_ACTIVE_Msk | STATUS_STATE_FLAG_PH_C_ACTIVE_Msk))  )
    {
        if( ((app_line_freqData.line_freq >= 4749) && (app_line_freqData.line_freq  <= 5250)) || ((app_line_freqData.line_freq  >= 5699) && (app_line_freqData.line_freq <= 6300)) )
        {
            app_line_freqData.line_sync_now = true;
        }
        else
        {
            app_line_freqData.line_sync_now = false;
            app_line_freqData.line_full_cycle_cnt =0;
        }

        if(app_line_freqData.line_freq > 5500)
        {
            if(app_line_freqData.line_cycle_cnt_per_second == 50)
            {
                app_line_freqData.line_full_cycle_cnt = 0;
            }
            app_line_freqData.line_cycle_cnt_per_second = 60;
        }
        else
        {
            if( app_line_freqData.line_cycle_cnt_per_second == 60)
            {
                app_line_freqData.line_full_cycle_cnt =0;
            }
            app_line_freqData.line_cycle_cnt_per_second = 50;
        }

    }
    else
    {
        app_line_freqData.line_sync_now = false;
        app_line_freqData.line_full_cycle_cnt =0;
    }
     
    APP_LINE_FREQ_UpdateSyncTime();

}

static void _APP_LINE_FREQ_TC1_CH1_Callback( TC_TIMER_STATUS status, uintptr_t context )
{
    APP_LINE_FREQ_RTC_TimeSet(&app_line_freqData.rtc_time);
    app_line_freqData.line_adjust_time ^= true;
}
// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************
void APP_LINE_FREQ_RTC_Sec_Interrupt(void)
{
    app_line_freqData.rtc_sec_tick_event = true;
}

void APP_LINE_FREQ_Set_Line_Freq_Time(void)
{
    //set the line frequency (time) same as RTC at initialization
    RTC_TimeGet(&app_line_freqData.rtc_time);
    app_line_freqData.line_time.hour= app_line_freqData.rtc_time.tm_hour;
    app_line_freqData.line_time.min = app_line_freqData.rtc_time.tm_min;
    app_line_freqData.line_time.sec = app_line_freqData.rtc_time.tm_sec;
}

static void APP_LINE_FREQ_RTC_Sec_Process(void)
{
    TC1_CH0_TimerStart();                   //fix 250ms timer
        
    RTC_TimeGet(&app_line_freqData.rtc_time);
    if(app_line_freqData.line_sync_flag == false)
    {
        app_line_freqData.line_time.hour = app_line_freqData.rtc_time.tm_hour;
        app_line_freqData.line_time.min = app_line_freqData.rtc_time.tm_min;
        app_line_freqData.line_time.sec = app_line_freqData.rtc_time.tm_sec;
                     
        app_line_freqData.line_sync_now = false;
        app_line_freqData.line_full_cycle_cnt = 0;
    }
    else
    {
        if(app_line_freqData.line_sync_now == false)
        {             
            app_line_freqData.line_time.hour = app_line_freqData.rtc_time.tm_hour;
            app_line_freqData.line_time.min = app_line_freqData.rtc_time.tm_min;
            app_line_freqData.line_time.sec = app_line_freqData.rtc_time.tm_sec;
        }
    }
        
    if(app_line_freqData.print_flag)
    {
        SYS_CMD_PRINT("Line time: %02u:%02u:%02u,    RTC time: %02u/%02u/%04u %02u:%02u:%02u,    :%u, :%u, :%d  \r\n", 
                        app_line_freqData.line_time.hour, app_line_freqData.line_time.min,app_line_freqData.line_time.sec, 
                        app_line_freqData.rtc_time.tm_mon,  app_line_freqData.rtc_time.tm_mday,  app_line_freqData.rtc_time.tm_year,  
                        app_line_freqData.rtc_time.tm_hour,  app_line_freqData.rtc_time.tm_min, app_line_freqData.rtc_time.tm_sec,
                        app_line_freqData.line_cycle_cnt_per_second, app_line_freqData.line_freq, app_line_freqData.line_adjust_time );
    }

}

void APP_LINE_FREQ_UpdateSyncTime(void)
{
    if(app_line_freqData.line_sync_now == true)
    {
        RTC_TimeGet(&app_line_freqData.rtc_time);
                
        if(app_line_freqData.line_time.sec < app_line_freqData.rtc_time.tm_sec)
        {
            //because line is slower, so need to delay the RTC by writing the same current time
            TC1_REGS->TC_CHANNEL[1].TC_RC = 3750000U;       //600ms timer
            TC1_CH1_TimerStart();
        }
        else if(app_line_freqData.line_time.sec > app_line_freqData.rtc_time.tm_sec)
        {
            //because line is faster, so we need to make RTC next second come earlier, write the same current time now
            TC1_REGS->TC_CHANNEL[1].TC_RC = 625U;       //100us timer
            TC1_CH1_TimerStart();
        }
        else
        {
            //both the same, so nothing
        }
    }
}

__STATIC_INLINE uint32_t app_linefreq_decimaltobcd( uint32_t aDecValue )
{
    uint32_t  decValueDiv10 = aDecValue / 10U;

    return( (decValueDiv10 << 4U) + ( aDecValue - (decValueDiv10 * 10U) ) );
}

bool APP_LINE_FREQ_RTC_TimeSet( struct tm * sysTime )
{
    bool retval = true;
    uint32_t year = (uint32_t)sysTime->tm_year + 1900U;
    uint32_t data_cal = (app_linefreq_decimaltobcd((uint32_t)sysTime->tm_mday ) << RTC_CALR_DATE_Pos)
                        | (app_linefreq_decimaltobcd(((uint32_t)sysTime->tm_wday + 1U )) << RTC_CALR_DAY_Pos)
                        | (app_linefreq_decimaltobcd(((uint32_t)sysTime->tm_mon + 1U )) << RTC_CALR_MONTH_Pos)
                        | (app_linefreq_decimaltobcd(year - ((year/100U) * 100U )) << RTC_CALR_YEAR_Pos)
                        | (app_linefreq_decimaltobcd((year/100U)) << RTC_CALR_CENT_Pos);

    uint32_t data_time = (app_linefreq_decimaltobcd((uint32_t)sysTime->tm_hour) << RTC_TIMR_HOUR_Pos )
                        | (app_linefreq_decimaltobcd((uint32_t)sysTime->tm_min) << RTC_TIMR_MIN_Pos)
                        | (app_linefreq_decimaltobcd((uint32_t)sysTime->tm_sec) << RTC_TIMR_SEC_Pos);

    RTC_REGS->RTC_CR &= ~(RTC_CR_UPDCAL_Msk | RTC_CR_UPDTIM_Msk);

    RTC_REGS->RTC_SCCR = RTC_SCCR_CALCLR_Msk;

    //while( (RTC_REGS->RTC_SR & RTC_SR_SEC_Msk) != RTC_SR_SEC_Msk )
    //{
    //    ;   // spin lock
    //}

    // request RTC Configuration
    RTC_REGS->RTC_CR |= RTC_CR_UPDCAL_Msk | RTC_CR_UPDTIM_Msk;

    // Wait for ack
    while( (RTC_REGS->RTC_SR & RTC_SR_ACKUPD_Msk) == 0U )
    {
        ;   // spin lock
    }

    // clear ack flag
    RTC_REGS->RTC_SCCR |= RTC_SCCR_ACKCLR_Msk | RTC_SCCR_ALRCLR_Msk | RTC_SCCR_TIMCLR_Msk;
    RTC_REGS->RTC_CALR = data_cal;
    RTC_REGS->RTC_TIMR = data_time;
    RTC_REGS->RTC_CR &= ~(RTC_CR_UPDCAL_Msk | RTC_CR_UPDTIM_Msk);

    if( (RTC_REGS->RTC_VER & (RTC_VER_NVTIM_Msk | RTC_VER_NVCAL_Msk)) != 0U )
    {
        retval = false;     // valid entry register indicates a problem
    }

    return retval;
}
// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_LINE_FREQ_Initialize ( void )

  Remarks:
    See prototype in app_line_freq.h.
 */

void APP_LINE_FREQ_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    app_line_freqData.state = APP_LINE_FREQ_STATE_INIT;

    app_line_freqData.line_sync_flag = true;
    app_line_freqData.line_sync_now = true;
    app_line_freqData.rtc_sec_tick_event = false;
    app_line_freqData.line_full_cycle_cnt = 0;
    app_line_freqData.line_cycle_cnt_per_second = 50;           /* Default set to 50 cycle count for 50Hz */
    app_line_freqData.print_flag = false;
    app_line_freqData.line_adjust_time = false;
    
    //enable RTC second interrupt
    RTC_InterruptEnable(RTC_REGS->RTC_IER | RTC_INT_SECOND);
    
    TC1_CH1_TimerStart();
    
    TC1_CH0_TimerCallbackRegister(_APP_LINE_FREQ_TC1_CH0_Callback, (uintptr_t)NULL);
    TC1_CH1_TimerCallbackRegister(_APP_LINE_FREQ_TC1_CH1_Callback, (uintptr_t)NULL);
        
    
}


/******************************************************************************
  Function:
    void APP_LINE_FREQ_Tasks ( void )

  Remarks:
    See prototype in app_line_freq.h.
 */

void APP_LINE_FREQ_Tasks ( void )
{

    /* Check the application's current state. */
    switch ( app_line_freqData.state )
    {
        /* Application's initial state. */
        case APP_LINE_FREQ_STATE_INIT:
        {
            bool appInitialized = true;
            
            DRV_METROLOGY_FullCycleCallbackRegister(_APP_LINE_FREQ_Fullcycle_Callback);
             
            if (appInitialized)
            {

                app_line_freqData.state = APP_LINE_FREQ_STATE_SERVICE_TASKS;
            }
            break;
        }

        case APP_LINE_FREQ_STATE_SERVICE_TASKS:
        {
            if(app_line_freqData.rtc_sec_tick_event)
            {
                APP_LINE_FREQ_RTC_Sec_Process();
                app_line_freqData.rtc_sec_tick_event = false;
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
