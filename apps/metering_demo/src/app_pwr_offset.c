/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_pwr_offset.c

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

#include "app_pwr_offset.h"
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
    This structure should be initialized by the APP_PWR_OFFSET_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_PWR_OFFSET_DATA app_pwr_offsetData;
extern APP_METROLOGY_DATA CACHE_ALIGN app_metrologyData;
APP_DATALOG_QUEUE_DATA appPowerOffsetDatalogQueueData;
// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************
static void _APP_PWR_OFFSET_LoadPowerOffsetParameterCallback(APP_DATALOG_RESULT result)
{
    if (result == APP_DATALOG_RESULT_SUCCESS)
    {
        app_pwr_offsetData.dataIsRdy = true;
    }
    else
    {
        SYS_CONSOLE_Print(SYS_CONSOLE_INDEX_0, "Read power offset parameter error\r\n");
    }
}
static void _APP_PWR_OFFSET_StorePowerOffsetParameterCallback(APP_DATALOG_RESULT result)
{
    if (result == APP_DATALOG_RESULT_SUCCESS)
    {
        app_pwr_offsetData.dataIsStoreSuccess = true;
        app_pwr_offsetData.dataIsRdy = true;
    }
    else
    {
        SYS_CONSOLE_Print(SYS_CONSOLE_INDEX_0, "Save power offset parameter error\r\n");
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

void APP_PWR_OFFSET_Calibration_Init ( void )
{
    app_pwr_offsetData.power_offset_cal.dsp_update_num = app_pwr_offsetData.power_offset_cal.num_integration_period + 2;         //ignore the first 2 samples after disable the power offset control
    app_pwr_offsetData.power_offset_cal.dsp_acc_pa = 0;
    app_pwr_offsetData.power_offset_cal.dsp_acc_pb = 0;
    app_pwr_offsetData.power_offset_cal.dsp_acc_pc = 0;
    app_pwr_offsetData.power_offset_cal.dsp_acc_qa = 0;
    app_pwr_offsetData.power_offset_cal.dsp_acc_qb = 0;
    app_pwr_offsetData.power_offset_cal.dsp_acc_qc = 0;
    
    //Disable power offset
    APP_METROLOGY_SetControlRegister(CONTROL_PWR_OFFS_CTRL, 0x00000000);
    DRV_METROLOGY_SetControl(app_metrologyData.pMetControl);  

}
// *****************************************************************************
// *****************************************************************************
void APP_PWR_OFFSET_PowerOffsetProcess(void)
{
    char regName[18];
    uint32_t regValue32;
   
    float va, vb, vc;
    APP_METROLOGY_GetMeasure(MEASURE_UA_RMS, &va, 0);
    APP_METROLOGY_GetMeasure(MEASURE_UB_RMS, &vb, 0);
    APP_METROLOGY_GetMeasure(MEASURE_UC_RMS, &vc, 0);
 
    if(va < 160)
    {
        APP_METROLOGY_GetControlRegister(CONTROL_PWR_OFFS_P_A, &regValue32, regName);
        if(regValue32 != app_pwr_offsetData.power_offset_parameter.data[POWER_OS_P_A_ID_110V])
        {
            APP_METROLOGY_SetControlRegister(CONTROL_PWR_OFFS_P_A,app_pwr_offsetData.power_offset_parameter.data[POWER_OS_P_A_ID_110V] );
        }
        APP_METROLOGY_GetControlRegister(CONTROL_PWR_OFFS_Q_A, &regValue32, regName);
        if(regValue32 != app_pwr_offsetData.power_offset_parameter.data[POWER_OS_Q_A_ID_110V])
        {
            APP_METROLOGY_SetControlRegister(CONTROL_PWR_OFFS_Q_A,app_pwr_offsetData.power_offset_parameter.data[POWER_OS_Q_A_ID_110V] );
        }
    }
    else if(va < 300)
    {
        APP_METROLOGY_GetControlRegister(CONTROL_PWR_OFFS_P_A, &regValue32, regName);
        if(regValue32 != app_pwr_offsetData.power_offset_parameter.data[POWER_OS_P_A_ID_220V])
        {
            APP_METROLOGY_SetControlRegister(CONTROL_PWR_OFFS_P_A,app_pwr_offsetData.power_offset_parameter.data[POWER_OS_P_A_ID_220V] );
        }
        APP_METROLOGY_GetControlRegister(CONTROL_PWR_OFFS_Q_A, &regValue32, regName);
        if(regValue32 != app_pwr_offsetData.power_offset_parameter.data[POWER_OS_Q_A_ID_220V])
        {
            APP_METROLOGY_SetControlRegister(CONTROL_PWR_OFFS_Q_A,app_pwr_offsetData.power_offset_parameter.data[POWER_OS_Q_A_ID_220V] );
        }
    }
    else
    {
        APP_METROLOGY_GetControlRegister(CONTROL_PWR_OFFS_P_A, &regValue32, regName);
        if(regValue32 != app_pwr_offsetData.power_offset_parameter.data[POWER_OS_P_A_ID_480V])
        {
            APP_METROLOGY_SetControlRegister(CONTROL_PWR_OFFS_P_A,app_pwr_offsetData.power_offset_parameter.data[POWER_OS_P_A_ID_480V] );
        }
        APP_METROLOGY_GetControlRegister(CONTROL_PWR_OFFS_Q_A, &regValue32, regName);
        if(regValue32 != app_pwr_offsetData.power_offset_parameter.data[POWER_OS_Q_A_ID_480V])
        {
            APP_METROLOGY_SetControlRegister(CONTROL_PWR_OFFS_Q_A,app_pwr_offsetData.power_offset_parameter.data[POWER_OS_Q_A_ID_480V] );
        }
    }
    if(vb < 160)
    {
        APP_METROLOGY_GetControlRegister(CONTROL_PWR_OFFS_P_B, &regValue32, regName);
        if(regValue32 != app_pwr_offsetData.power_offset_parameter.data[POWER_OS_P_B_ID_110V])
        {
            APP_METROLOGY_SetControlRegister(CONTROL_PWR_OFFS_P_B,app_pwr_offsetData.power_offset_parameter.data[POWER_OS_P_B_ID_110V] );
        }
        APP_METROLOGY_GetControlRegister(CONTROL_PWR_OFFS_Q_B, &regValue32, regName);
        if(regValue32 != app_pwr_offsetData.power_offset_parameter.data[POWER_OS_Q_B_ID_110V])
        {
            APP_METROLOGY_SetControlRegister(CONTROL_PWR_OFFS_Q_B,app_pwr_offsetData.power_offset_parameter.data[POWER_OS_Q_B_ID_110V] );
        }
    }
    else if(vb < 300)
    {
        APP_METROLOGY_GetControlRegister(CONTROL_PWR_OFFS_P_B, &regValue32, regName);
        if(regValue32 != app_pwr_offsetData.power_offset_parameter.data[POWER_OS_P_B_ID_220V])
        {
            APP_METROLOGY_SetControlRegister(CONTROL_PWR_OFFS_P_B,app_pwr_offsetData.power_offset_parameter.data[POWER_OS_P_B_ID_220V] );
        }
        APP_METROLOGY_GetControlRegister(CONTROL_PWR_OFFS_Q_B, &regValue32, regName);
        if(regValue32 != app_pwr_offsetData.power_offset_parameter.data[POWER_OS_Q_B_ID_220V])
        {
            APP_METROLOGY_SetControlRegister(CONTROL_PWR_OFFS_Q_B,app_pwr_offsetData.power_offset_parameter.data[POWER_OS_Q_B_ID_220V] );
        }
    }
    else
    {
        APP_METROLOGY_GetControlRegister(CONTROL_PWR_OFFS_P_B, &regValue32, regName);
        if(regValue32 != app_pwr_offsetData.power_offset_parameter.data[POWER_OS_P_B_ID_480V])
        {
            APP_METROLOGY_SetControlRegister(CONTROL_PWR_OFFS_P_B,app_pwr_offsetData.power_offset_parameter.data[POWER_OS_P_B_ID_480V] );
        }
        APP_METROLOGY_GetControlRegister(CONTROL_PWR_OFFS_Q_B, &regValue32, regName);
        if(regValue32 != app_pwr_offsetData.power_offset_parameter.data[POWER_OS_Q_B_ID_480V])
        {
            APP_METROLOGY_SetControlRegister(CONTROL_PWR_OFFS_Q_B,app_pwr_offsetData.power_offset_parameter.data[POWER_OS_Q_B_ID_480V] );
        }
    }
    if(vc < 160)
    {
        APP_METROLOGY_GetControlRegister(CONTROL_PWR_OFFS_P_C, &regValue32, regName);
        if(regValue32 != app_pwr_offsetData.power_offset_parameter.data[POWER_OS_P_C_ID_110V])
        {
            APP_METROLOGY_SetControlRegister(CONTROL_PWR_OFFS_P_C,app_pwr_offsetData.power_offset_parameter.data[POWER_OS_P_C_ID_110V] );
        }
        APP_METROLOGY_GetControlRegister(CONTROL_PWR_OFFS_Q_C, &regValue32, regName);
        if(regValue32 != app_pwr_offsetData.power_offset_parameter.data[POWER_OS_Q_C_ID_110V])
        {
            APP_METROLOGY_SetControlRegister(CONTROL_PWR_OFFS_Q_C,app_pwr_offsetData.power_offset_parameter.data[POWER_OS_Q_C_ID_110V] );
        }
    }
    else if(vc < 300)
    {
        APP_METROLOGY_GetControlRegister(CONTROL_PWR_OFFS_P_C, &regValue32, regName);
        if(regValue32 != app_pwr_offsetData.power_offset_parameter.data[POWER_OS_P_C_ID_220V])
        {
            APP_METROLOGY_SetControlRegister(CONTROL_PWR_OFFS_P_C,app_pwr_offsetData.power_offset_parameter.data[POWER_OS_P_C_ID_220V] );
        }
        APP_METROLOGY_GetControlRegister(CONTROL_PWR_OFFS_Q_C, &regValue32, regName);
        if(regValue32 != app_pwr_offsetData.power_offset_parameter.data[POWER_OS_Q_C_ID_220V])
        {
            APP_METROLOGY_SetControlRegister(CONTROL_PWR_OFFS_Q_C,app_pwr_offsetData.power_offset_parameter.data[POWER_OS_Q_C_ID_220V] );
        }
    }
    else
    {
        APP_METROLOGY_GetControlRegister(CONTROL_PWR_OFFS_P_C, &regValue32, regName);
        if(regValue32 != app_pwr_offsetData.power_offset_parameter.data[POWER_OS_P_C_ID_480V])
        {
            APP_METROLOGY_SetControlRegister(CONTROL_PWR_OFFS_P_C,app_pwr_offsetData.power_offset_parameter.data[POWER_OS_P_C_ID_480V] );
        }
        APP_METROLOGY_GetControlRegister(CONTROL_PWR_OFFS_Q_C, &regValue32, regName);
        if(regValue32 != app_pwr_offsetData.power_offset_parameter.data[POWER_OS_Q_C_ID_480V])
        {
            APP_METROLOGY_SetControlRegister(CONTROL_PWR_OFFS_Q_C,app_pwr_offsetData.power_offset_parameter.data[POWER_OS_Q_C_ID_480V] );
        }
    }
}

// *****************************************************************************
// *****************************************************************************
void APP_PWR_OFFSET_LoadPowerOffsetParameterFromMemory(void)
{
    appPowerOffsetDatalogQueueData.userId = APP_DATALOG_USER_POWEROFFSET;
    appPowerOffsetDatalogQueueData.operation = APP_DATALOG_READ;
    appPowerOffsetDatalogQueueData.pData = (uint8_t *)&app_pwr_offsetData.power_offset_parameter;
    appPowerOffsetDatalogQueueData.dataLen = sizeof(APP_PWR_OFFSET_PARAMETERS);
    appPowerOffsetDatalogQueueData.endCallback = _APP_PWR_OFFSET_LoadPowerOffsetParameterCallback;
    appPowerOffsetDatalogQueueData.date.month = APP_DATALOG_INVALID_MONTH;
    appPowerOffsetDatalogQueueData.date.year = APP_DATALOG_INVALID_YEAR;

    APP_DATALOG_SendDatalogData(&appPowerOffsetDatalogQueueData);
}
// *****************************************************************************
// *****************************************************************************
void APP_PWR_OFFSET_StorePowerOffsetParameterToMemory(void)
{
    appPowerOffsetDatalogQueueData.userId = APP_DATALOG_USER_POWEROFFSET;
    appPowerOffsetDatalogQueueData.operation = APP_DATALOG_WRITE;
    appPowerOffsetDatalogQueueData.pData = (uint8_t *)&app_pwr_offsetData.power_offset_parameter;
    appPowerOffsetDatalogQueueData.dataLen = sizeof(APP_PWR_OFFSET_PARAMETERS);
    appPowerOffsetDatalogQueueData.endCallback = _APP_PWR_OFFSET_StorePowerOffsetParameterCallback;
    appPowerOffsetDatalogQueueData.date.month = APP_DATALOG_INVALID_MONTH;
    appPowerOffsetDatalogQueueData.date.year = APP_DATALOG_INVALID_YEAR;

    APP_DATALOG_SendDatalogData(&appPowerOffsetDatalogQueueData);
}
// *****************************************************************************
// *****************************************************************************
void APP_PWR_OFFSET_SetDefaultPowerOffsetParameter(void)
{
    uint8_t i;
    for(i = 0; i< POWER_OS_DATA_END; i++)
    {
        app_pwr_offsetData.power_offset_parameter.data[i] = 0x00000000;
    }
    app_pwr_offsetData.power_offset_parameter.set_voltage = 1100000;
}
// *****************************************************************************
// *****************************************************************************
static double APP_PWR_OFFSET_Metrology_GetPQ(int64_t val, uint32_t k_ix, uint32_t k_vx, double samples)
{
    double m, ki, kv;
    uint64_t divisor;

    /* k [uQ22.10] */
    divisor = (1ULL << FORMAT_CONST_uQ2210);
    ki = (double)k_ix / (double)divisor;
    kv = (double)k_vx / (double)divisor;

    /* value [sQ23.40] */
    divisor = (1ULL << FORMAT_CONST_sQ2340);
    m = (double)val / (double)divisor;
    m = m / samples;

    m *= (ki * kv);

    return m;
}
// *****************************************************************************
// *****************************************************************************
int32_t APP_PWR_OFFSET_Compute_Offset(double val, uint32_t k_ix, uint32_t k_vx)
{
    double double_tmp;
    int32_t tmp;
    
    double_tmp = val;
    double_tmp = double_tmp * 0x10000000000;    // accu PA 2^40
    double_tmp = double_tmp * 0x100000;         //Ki 2^10 and KV 2^10 
    double_tmp = double_tmp / k_ix;
    double_tmp = double_tmp / k_vx;
    
    tmp = (int32_t) double_tmp;
    
    //positive to negative and negative to positive
    tmp = tmp ^ 0xFFFFFFFF;
    tmp = tmp + 1;
    
    return tmp;
}

// *****************************************************************************
// *****************************************************************************

void APP_PWR_OFFSET_PwerOffsetCalibration(void)
{
    float pa, pb, pc, qa, qb, qc;
    DRV_METROLOGY_REGS_ACCUMULATORS *pwroffset_metAccData;
    DRV_METROLOGY_REGS_CONTROL      *pwroffset_metRegisters;
    DRV_METROLOGY_REGS_STATUS       *pwroffset_metStatusData;
    
    
    double samples;
    
    uint8_t prtbuf[0xff];
    uint8_t prtlen=0;
    
    if (app_pwr_offsetData.integrationFlag == true) 
    {
        pwroffset_metAccData = DRV_METROLOGY_GetAccData();
        pwroffset_metRegisters = DRV_METROLOGY_GetControlData();
        pwroffset_metStatusData = DRV_METROLOGY_GetStatusData();
        samples = (double)pwroffset_metStatusData->N;
                
        if (app_pwr_offsetData.power_offset_cal.dsp_update_num) 
        {
            if (app_pwr_offsetData.power_offset_cal.dsp_update_num <= app_pwr_offsetData.power_offset_cal.num_integration_period) 
            {
                pa  = (float)APP_PWR_OFFSET_Metrology_GetPQ(pwroffset_metAccData->P_A, pwroffset_metRegisters->K_IA, pwroffset_metRegisters->K_VA, samples);
                qa  = (float)APP_PWR_OFFSET_Metrology_GetPQ(pwroffset_metAccData->Q_A, pwroffset_metRegisters->K_IA, pwroffset_metRegisters->K_VA, samples);
                pb  = (float)APP_PWR_OFFSET_Metrology_GetPQ(pwroffset_metAccData->P_B, pwroffset_metRegisters->K_IB, pwroffset_metRegisters->K_VB, samples);
                qb  = (float)APP_PWR_OFFSET_Metrology_GetPQ(pwroffset_metAccData->Q_B, pwroffset_metRegisters->K_IB, pwroffset_metRegisters->K_VB, samples);
                pc  = (float)APP_PWR_OFFSET_Metrology_GetPQ(pwroffset_metAccData->P_C, pwroffset_metRegisters->K_IC, pwroffset_metRegisters->K_VC, samples);
                qc  = (float)APP_PWR_OFFSET_Metrology_GetPQ(pwroffset_metAccData->Q_C, pwroffset_metRegisters->K_IC, pwroffset_metRegisters->K_VC, samples);
        
                prtlen=sprintf((char *) prtbuf, (const char *) "Pa=%.6fW Pb=%.6fW Pc=%.6fW Qa=%.6fVAr Qb=%.6fVAr Qc=%.6fVAr \r\n",pa, pb, pc, qa, qb, qc);
                FLEXCOM0_USART_Write(prtbuf,prtlen); while(!(FLEXCOM0_USART_TransmitComplete()));
                 
                app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_pa += pa;
                app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_pb += pb;
                app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_pc += pc;
                app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_qa += qa;
                app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_qb += qb;
                app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_qc += qc;
                
            }
            app_pwr_offsetData.power_offset_cal.dsp_update_num--;
            
        }
        else 
        {
            app_pwr_offsetData.state = APP_PWR_OFFSET_STATE_SERVICE_TASKS;

            //get the average
            app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_pa = app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_pa / app_pwr_offsetData.power_offset_cal.num_integration_period;
            app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_pb = app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_pb / app_pwr_offsetData.power_offset_cal.num_integration_period;
            app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_pc = app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_pc / app_pwr_offsetData.power_offset_cal.num_integration_period;
            app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_qa = app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_qa / app_pwr_offsetData.power_offset_cal.num_integration_period;
            app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_qb = app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_qb / app_pwr_offsetData.power_offset_cal.num_integration_period;
            app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_qc = app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_qc / app_pwr_offsetData.power_offset_cal.num_integration_period;
                        
            prtlen=sprintf((char *)prtbuf, (const char *)"\r\nACC_Pa=%.6fW ACC_Pb=%.6fW ACC_Pc=%.6fW ACC_Qa=%.6fVAr ACC_Qb=%.6fVAr ACC_Qc=%.6fVAr \r\n\r\n",
                    app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_pa, app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_pb, app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_pc, 
                    app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_qa, app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_qb, app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_qc);
            FLEXCOM0_USART_Write(prtbuf,prtlen); while(!(FLEXCOM0_USART_TransmitComplete()));
            
            app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_pa = app_pwr_offsetData.power_offset_cal.dsp_acc_actual_pa - app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_pa; 
            app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_pb = app_pwr_offsetData.power_offset_cal.dsp_acc_actual_pb - app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_pb; 
            app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_pc = app_pwr_offsetData.power_offset_cal.dsp_acc_actual_pc - app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_pc; 
            app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_qa = app_pwr_offsetData.power_offset_cal.dsp_acc_actual_qa - app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_qa; 
            app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_qb = app_pwr_offsetData.power_offset_cal.dsp_acc_actual_qb - app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_qb; 
            app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_qc = app_pwr_offsetData.power_offset_cal.dsp_acc_actual_qc - app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_qc; 
            
            prtlen=sprintf((char *)prtbuf, (const char *)"\r\nOFF_Pa=%.6fW OFF_Pb=%.6fW OFF_Pc=%.6fW OFF_Qa=%.6fVAr OFF_Qb=%.6fVAr OFF_Qc=%.6fVAr \r\n\r\n",
                    app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_pa, app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_pb, app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_pc, 
                    app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_qa, app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_qb, app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_qc);
            FLEXCOM0_USART_Write(prtbuf,prtlen); while(!(FLEXCOM0_USART_TransmitComplete()));
            
            
            app_pwr_offsetData.power_offset_cal.dsp_acc_pa = APP_PWR_OFFSET_Compute_Offset(app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_pa, pwroffset_metRegisters->K_IA,pwroffset_metRegisters->K_VA);
            app_pwr_offsetData.power_offset_cal.dsp_acc_pb = APP_PWR_OFFSET_Compute_Offset(app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_pb, pwroffset_metRegisters->K_IB,pwroffset_metRegisters->K_VB);
            app_pwr_offsetData.power_offset_cal.dsp_acc_pc = APP_PWR_OFFSET_Compute_Offset(app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_pc, pwroffset_metRegisters->K_IC,pwroffset_metRegisters->K_VC);
            app_pwr_offsetData.power_offset_cal.dsp_acc_qa = APP_PWR_OFFSET_Compute_Offset(app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_qa, pwroffset_metRegisters->K_IA,pwroffset_metRegisters->K_VA);
            app_pwr_offsetData.power_offset_cal.dsp_acc_qb = APP_PWR_OFFSET_Compute_Offset(app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_qb, pwroffset_metRegisters->K_IB,pwroffset_metRegisters->K_VB);
            app_pwr_offsetData.power_offset_cal.dsp_acc_qc = APP_PWR_OFFSET_Compute_Offset(app_pwr_offsetData.power_offset_cal.tmp_dsp_acc_qc, pwroffset_metRegisters->K_IC,pwroffset_metRegisters->K_VC);
           

            APP_METROLOGY_SetControlRegister(CONTROL_PWR_OFFS_P_A, app_pwr_offsetData.power_offset_cal.dsp_acc_pa);
            APP_METROLOGY_SetControlRegister(CONTROL_PWR_OFFS_P_B, app_pwr_offsetData.power_offset_cal.dsp_acc_pb);
            APP_METROLOGY_SetControlRegister(CONTROL_PWR_OFFS_P_C, app_pwr_offsetData.power_offset_cal.dsp_acc_pc);
            APP_METROLOGY_SetControlRegister(CONTROL_PWR_OFFS_Q_A, app_pwr_offsetData.power_offset_cal.dsp_acc_qa);
            APP_METROLOGY_SetControlRegister(CONTROL_PWR_OFFS_Q_B, app_pwr_offsetData.power_offset_cal.dsp_acc_qb);
            APP_METROLOGY_SetControlRegister(CONTROL_PWR_OFFS_Q_C, app_pwr_offsetData.power_offset_cal.dsp_acc_qc);

            APP_METROLOGY_SetControlRegister(CONTROL_PWR_OFFS_CTRL, 0x00770000);
            DRV_METROLOGY_SetControl(app_metrologyData.pMetControl);

            APP_METROLOGY_StoreMetrologyData();
            
            if (app_pwr_offsetData.power_offset_cal.voltage == POWER_OS_CAL_110V) {

                app_pwr_offsetData.power_offset_parameter.data[POWER_OS_P_A_ID_110V] = app_pwr_offsetData.power_offset_cal.dsp_acc_pa;
                app_pwr_offsetData.power_offset_parameter.data[POWER_OS_P_B_ID_110V] = app_pwr_offsetData.power_offset_cal.dsp_acc_pb;
                app_pwr_offsetData.power_offset_parameter.data[POWER_OS_P_C_ID_110V] = app_pwr_offsetData.power_offset_cal.dsp_acc_pc;
                app_pwr_offsetData.power_offset_parameter.data[POWER_OS_Q_A_ID_110V] = app_pwr_offsetData.power_offset_cal.dsp_acc_qa;
                app_pwr_offsetData.power_offset_parameter.data[POWER_OS_Q_B_ID_110V] = app_pwr_offsetData.power_offset_cal.dsp_acc_qb;
                app_pwr_offsetData.power_offset_parameter.data[POWER_OS_Q_C_ID_110V] = app_pwr_offsetData.power_offset_cal.dsp_acc_qc;
            } 
            else if (app_pwr_offsetData.power_offset_cal.voltage == POWER_OS_CAL_220V) {

                app_pwr_offsetData.power_offset_parameter.data[POWER_OS_P_A_ID_220V] = app_pwr_offsetData.power_offset_cal.dsp_acc_pa;
                app_pwr_offsetData.power_offset_parameter.data[POWER_OS_P_B_ID_220V] = app_pwr_offsetData.power_offset_cal.dsp_acc_pb;
                app_pwr_offsetData.power_offset_parameter.data[POWER_OS_P_C_ID_220V] = app_pwr_offsetData.power_offset_cal.dsp_acc_pc;
                app_pwr_offsetData.power_offset_parameter.data[POWER_OS_Q_A_ID_220V] = app_pwr_offsetData.power_offset_cal.dsp_acc_qa;
                app_pwr_offsetData.power_offset_parameter.data[POWER_OS_Q_B_ID_220V] = app_pwr_offsetData.power_offset_cal.dsp_acc_qb;
                app_pwr_offsetData.power_offset_parameter.data[POWER_OS_Q_C_ID_220V] = app_pwr_offsetData.power_offset_cal.dsp_acc_qc;
            } 
            else if (app_pwr_offsetData.power_offset_cal.voltage == POWER_OS_CAL_480V) {

                app_pwr_offsetData.power_offset_parameter.data[POWER_OS_P_A_ID_480V] = app_pwr_offsetData.power_offset_cal.dsp_acc_pa;
                app_pwr_offsetData.power_offset_parameter.data[POWER_OS_P_B_ID_480V] = app_pwr_offsetData.power_offset_cal.dsp_acc_pb;
                app_pwr_offsetData.power_offset_parameter.data[POWER_OS_P_C_ID_480V] = app_pwr_offsetData.power_offset_cal.dsp_acc_pc;
                app_pwr_offsetData.power_offset_parameter.data[POWER_OS_Q_A_ID_480V] = app_pwr_offsetData.power_offset_cal.dsp_acc_qa;
                app_pwr_offsetData.power_offset_parameter.data[POWER_OS_Q_B_ID_480V] = app_pwr_offsetData.power_offset_cal.dsp_acc_qb;
                app_pwr_offsetData.power_offset_parameter.data[POWER_OS_Q_C_ID_480V] = app_pwr_offsetData.power_offset_cal.dsp_acc_qc;
            } 
            else {

            }
            
            for(uint8_t tmp = POWER_OS_P_A_ID_110V; tmp <= POWER_OS_Q_C_ID_480V; tmp++)
                SYS_CONSOLE_Print(SYS_CONSOLE_INDEX_0, "tmp=%d, 0x%08X\r\n", tmp, app_pwr_offsetData.power_offset_parameter.data[tmp]);

            
            APP_PWR_OFFSET_StorePowerOffsetParameterToMemory();
       
            SYS_CMD_MESSAGE("Power offset calibration done\r\n");
            APP_DISPLAY_SetSerialCommunication();
        }

        app_pwr_offsetData.integrationFlag = false;
    }
    
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_PWR_OFFSET_Initialize ( void )

  Remarks:
    See prototype in app_pwr_offset.h.
 */

void APP_PWR_OFFSET_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    app_pwr_offsetData.state = APP_PWR_OFFSET_STATE_INIT;

    app_pwr_offsetData.integrationFlag = false;
    app_pwr_offsetData.dataIsStoreSuccess = false;
    app_pwr_offsetData.dataIsRdy = false;
    app_pwr_offsetData.poweroffset_enable = true;
    app_pwr_offsetData.show_offset_flag = false;

    uint8_t i;
    for(i = 0; i< POWER_OS_DATA_END; i++)
    {
        app_pwr_offsetData.power_offset_parameter.data[i] = 0x00000000;
    }
    app_pwr_offsetData.power_offset_parameter.set_voltage = 1100000;
}


/******************************************************************************
  Function:
    void APP_PWR_OFFSET_Tasks ( void )

  Remarks:
    See prototype in app_pwr_offset.h.
 */

void APP_PWR_OFFSET_Tasks ( void )
{

    /* Check the application's current state. */
    switch ( app_pwr_offsetData.state )
    {
        /* Application's initial state. */
        case APP_PWR_OFFSET_STATE_INIT:
        {
            bool appInitialized = true;


            if (appInitialized)
            {
                if(app_pwr_offsetData.dataIsRdy == false)
                {
                    if (APP_DATALOG_GetStatus() == APP_DATALOG_STATE_READY)
                    {
                        if (APP_DATALOG_DataIsValid(APP_DATALOG_USER_POWEROFFSET))
                        {
                            APP_PWR_OFFSET_LoadPowerOffsetParameterFromMemory();
                        }
                        else
                        {
                            APP_PWR_OFFSET_SetDefaultPowerOffsetParameter();
                            APP_PWR_OFFSET_StorePowerOffsetParameterToMemory();
                        }
                        
                    }
                }
                else
                {
                    app_pwr_offsetData.state = APP_PWR_OFFSET_STATE_SERVICE_TASKS;
                }

            }
            break;
        }

        case APP_PWR_OFFSET_STATE_CALIBRATION_INIT:
        {
            APP_PWR_OFFSET_Calibration_Init();
            app_pwr_offsetData.state = APP_PWR_OFFSET_STATE_CALIBRATION_EXECUTE;
            SYS_CMD_MESSAGE("Power offset calibration start...\r\n");
            break;
        }
        
        case APP_PWR_OFFSET_STATE_CALIBRATION_EXECUTE:
        {
            APP_PWR_OFFSET_PwerOffsetCalibration();
            break;
        }

        case APP_PWR_OFFSET_STATE_SERVICE_TASKS:
        {
            if(app_pwr_offsetData.integrationFlag == true) 
            {
                app_pwr_offsetData.integrationFlag = false;
                
                if(app_pwr_offsetData.poweroffset_enable == true)
                {
                    if(app_metrologyData.state == APP_METROLOGY_STATE_RUNNING)
                        APP_PWR_OFFSET_PowerOffsetProcess();    
                }
                
                if( app_pwr_offsetData.show_offset_flag == true)
                {
                    SYS_CMD_PRINT("dsp_acc_pa = 0x%X, dsp_acc_qa = 0x%X\r\n", (app_metrologyData.pMetAccData->P_A / app_metrologyData.pMetStatus->N), (app_metrologyData.pMetAccData->Q_A / app_metrologyData.pMetStatus->N));
                }
            }
            break;
        }

        case APP_PWR_OFFSET_STATE_READ_PWROFFSET_DATA:
        {
            APP_PWR_OFFSET_LoadPowerOffsetParameterFromMemory();
            for(uint8_t i = POWER_OS_P_A_ID_110V; i <= POWER_OS_Q_C_ID_480V; i++)
                SYS_CONSOLE_Print(SYS_CONSOLE_INDEX_0, "i=%d, 0x%08X\r\n", i, app_pwr_offsetData.power_offset_parameter.data[i]);
            
             SYS_CONSOLE_Print(SYS_CONSOLE_INDEX_0, "set voltage = %uV\r\n", app_pwr_offsetData.power_offset_parameter.set_voltage / 10000);
            
            app_pwr_offsetData.state = APP_PWR_OFFSET_STATE_SERVICE_TASKS;
            break;
        }
        
        case APP_PWR_OFFSET_STATE_SET_PWROFFSET_VOLTAGE:
        {
            APP_PWR_OFFSET_StorePowerOffsetParameterToMemory();
            app_pwr_offsetData.state = APP_PWR_OFFSET_STATE_SERVICE_TASKS;
            break;
        }

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
