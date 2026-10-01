/*******************************************************************************
  UART PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_uart.c

  Summary:
    UART PLIB Implementation File

  Description:
    None

*******************************************************************************/

/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/

#include "device.h"
#include "plib_uart.h"
#include "interrupts.h"

// *****************************************************************************
// *****************************************************************************
// Section: UART Implementation
// *****************************************************************************
// *****************************************************************************

static volatile UART_OBJECT uartObj;

void __attribute__((used)) UART_InterruptHandler( void )
{
    /* Error status */
    uint32_t errorStatusx = (UART_REGS->UART_SR & (UART_SR_OVRE_Msk | UART_SR_FRAME_Msk | UART_SR_PARE_Msk));

    if(errorStatusx != 0U)
    {
        /* Client must call UARTx_ErrorGet() function to clear the errors */

        /* Disable Read, Overrun, Parity and Framing error interrupts */
        UART_REGS->UART_PTCR = UART_PTCR_RXTDIS_Msk;
        UART_REGS->UART_IDR = (UART_IDR_ENDRX_Msk | UART_IDR_FRAME_Msk | UART_IDR_PARE_Msk | UART_IDR_OVRE_Msk);

        uartObj.rxBusyStatus = false;

        /* UART errors are normally associated with the receiver, hence calling
         * receiver callback */
        if( uartObj.rxCallback != NULL )
        {
            uintptr_t rxContext = uartObj.rxContext;

            uartObj.rxCallback(rxContext);
        }
    }

    if ((UART_REGS->UART_PTSR & UART_PTSR_RXTEN_Msk) && (UART_REGS->UART_SR & UART_SR_ENDRX_Msk))
    {
        if(uartObj.rxBusyStatus == true)
        {
            uartObj.rxBusyStatus = false;
            UART_REGS->UART_PTCR = UART_PTCR_RXTDIS_Msk;
            UART_REGS->UART_IDR = (UART_IDR_ENDRX_Msk | UART_IDR_FRAME_Msk | UART_IDR_PARE_Msk | UART_IDR_OVRE_Msk);

            if( uartObj.rxCallback != NULL )
            {
                uintptr_t rxContext = uartObj.rxContext;

                uartObj.rxCallback(rxContext);
            }
        }
    }

    if ((UART_REGS->UART_PTSR & UART_PTSR_TXTEN_Msk) && (UART_REGS->UART_SR & UART_SR_ENDTX_Msk))
    {
        if(uartObj.txBusyStatus == true)
        {
            uartObj.txBusyStatus = false;
            UART_REGS->UART_PTCR = UART_PTCR_TXTDIS_Msk;
            UART_REGS->UART_IDR = UART_IDR_ENDTX_Msk;

            if( uartObj.txCallback != NULL )
            {
                uintptr_t txContext = uartObj.txContext;

                uartObj.txCallback(txContext);
            }
        }
    }
}

static void UART_ErrorClear( void )
{
    uint8_t dummyData = 0u;

    UART_REGS->UART_CR = UART_CR_RSTSTA_Msk;

    /* Flush existing error bytes from the RX FIFO */
    while( UART_SR_RXRDY_Msk == (UART_REGS->UART_SR & UART_SR_RXRDY_Msk) )
    {
        dummyData = (uint8_t)(UART_REGS->UART_RHR & UART_RHR_RXCHR_Msk);
    }

    /* Ignore the warning */
    (void)dummyData;
}

void UART_Initialize( void )
{
    /* Reset UART */
    UART_REGS->UART_CR = (UART_CR_RSTRX_Msk | UART_CR_RSTTX_Msk | UART_CR_RSTSTA_Msk);

    /* Enable UART */
    UART_REGS->UART_CR = (UART_CR_TXEN_Msk | UART_CR_RXEN_Msk);

    /* Configure UART mode */
    UART_REGS->UART_MR = ((UART_MR_PAR_NO) | (0U << UART_MR_FILTER_Pos));

    /* Configure UART Baud Rate */
    UART_REGS->UART_BRGR = UART_BRGR_CD(64);

    /* Initialize instance object */
    uartObj.rxBuffer = NULL;
    uartObj.rxSize = 0;
    uartObj.rxProcessedSize = 0;
    uartObj.rxBusyStatus = false;
    uartObj.rxCallback = NULL;
    uartObj.txBuffer = NULL;
    uartObj.txSize = 0;
    uartObj.txProcessedSize = 0;
    uartObj.txBusyStatus = false;
    uartObj.txCallback = NULL;
}

UART_ERROR UART_ErrorGet( void )
{
    UART_ERROR errors = UART_ERROR_NONE;
    uint32_t status = UART_REGS->UART_SR;

    errors = (UART_ERROR)(status & (UART_SR_OVRE_Msk | UART_SR_PARE_Msk | UART_SR_FRAME_Msk));

    if(errors != UART_ERROR_NONE)
    {
        UART_ErrorClear();
    }

    /* All errors are cleared, but send the previous error state */
    return errors;
}

bool UART_SerialSetup( UART_SERIAL_SETUP *setup, uint32_t srcClkFreq )
{
    bool status = false;
    uint32_t baud;
    uint32_t brgVal = 0;
    uint32_t uartMode;

    if(uartObj.rxBusyStatus == true)
    {
        /* Transaction is in progress, so return without updating settings */
        return false;
    }
    if(uartObj.txBusyStatus == true)
    {
        /* Transaction is in progress, so return without updating settings */
        return false;
    }
    if (setup != NULL)
    {
        baud = setup->baudRate;
        if(srcClkFreq == 0U)
        {
            srcClkFreq = UART_FrequencyGet();
        }

        /* Calculate BRG value */
        brgVal = srcClkFreq / (16U * baud);

        /* If the target baud rate is acheivable using this clock */
        if (brgVal <= 65535U)
        {
            /* Configure UART mode */
            uartMode = UART_REGS->UART_MR;
            uartMode &= ~UART_MR_PAR_Msk;
            UART_REGS->UART_MR = uartMode | setup->parity ;

            /* Configure UART Baud Rate */
            UART_REGS->UART_BRGR = UART_BRGR_CD(brgVal);

            status = true;
        }
    }

    return status;
}

bool UART_Read( void *buffer, const size_t size )
{
    bool status = false;
    UART_ERROR errorinfo;

    uint8_t * lBuffer = (uint8_t *)buffer;

    if(NULL != lBuffer)
    {
        /* Clear errors before submitting the request.
         * ErrorGet clears errors internally. */
         errorinfo = UART_ErrorGet();

         if(errorinfo != 0U)
         {
             /* Nothing to do */
         }

        /* Check if receive request is in progress */
        if(uartObj.rxBusyStatus == false)
        {
            uartObj.rxBuffer = lBuffer;
            uartObj.rxSize = size;
            uartObj.rxProcessedSize = 0;
            uartObj.rxBusyStatus = true;
            status = true;

            UART_REGS->UART_RPR = (uint32_t)buffer;
            UART_REGS->UART_RCR = (uint32_t)size;
            UART_REGS->UART_PTCR = UART_PTCR_RXTEN_Msk;
            UART_REGS->UART_IER = (UART_IER_ENDRX_Msk | UART_IER_FRAME_Msk | UART_IER_PARE_Msk | UART_IER_OVRE_Msk);
        }
    }

    return status;
}

bool UART_Write( void *buffer, const size_t size )
{
    bool status = false;
    uint8_t * lBuffer = (uint8_t *)buffer;

    if(NULL != lBuffer)
    {
        /* Check if transmit request is in progress */
        if(uartObj.txBusyStatus == false)
        {
            uartObj.txBuffer = lBuffer;
            uartObj.txSize = size;
            uartObj.txProcessedSize = 0;
            uartObj.txBusyStatus = true;
            status = true;

            UART_REGS->UART_TPR = (uint32_t)buffer;
            UART_REGS->UART_TCR = (uint32_t)size;
            UART_REGS->UART_PTCR = UART_PTCR_TXTEN_Msk;
            UART_REGS->UART_IER = UART_IER_ENDTX_Msk;
        }
    }

    return status;
}

void UART_WriteCallbackRegister( UART_CALLBACK callback, uintptr_t context )
{
    uartObj.txCallback = callback;

    uartObj.txContext = context;
}

void UART_ReadCallbackRegister( UART_CALLBACK callback, uintptr_t context )
{
    uartObj.rxCallback = callback;

    uartObj.rxContext = context;
}

bool UART_WriteIsBusy( void )
{
    return uartObj.txBusyStatus;
}

bool UART_ReadIsBusy( void )
{
    return uartObj.rxBusyStatus;
}

size_t UART_WriteCountGet( void )
{
    return (uartObj.txSize - UART_REGS->UART_TCR);
}

size_t UART_ReadCountGet( void )
{
    return (uartObj.rxSize - UART_REGS->UART_RCR);
}

bool UART_ReadAbort(void)
{
    if (uartObj.rxBusyStatus == true)
    {
        UART_REGS->UART_RCR = (uint32_t)0;
        UART_REGS->UART_PTCR = UART_PTCR_RXTDIS_Msk;
        UART_REGS->UART_IDR = (UART_IDR_ENDRX_Msk | UART_IDR_FRAME_Msk | UART_IDR_PARE_Msk | UART_IDR_OVRE_Msk);

        uartObj.rxBusyStatus = false;

        /* If required application should read the num bytes processed prior to calling the read abort API */
        uartObj.rxSize = 0;
        uartObj.rxProcessedSize = 0;
    }

    return true;
}


bool UART_TransmitComplete( void )
{
    bool status = false;

    if(UART_SR_TXEMPTY_Msk == (UART_REGS->UART_SR & UART_SR_TXEMPTY_Msk))
    {
        status = true;
    }

    return status;
}