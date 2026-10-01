/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    drv_crypto_trng_hw_6334.c

  Summary:
    Crypto Framework Library interface file for hardware TRNG.

  Description:
    This source file contains the interface that make up the TRNG hardware 
    driver for the following families of Microchip microcontrollers:
    PIC32CXMTxx, SAMx70, SAMA5D2, SAM9X60, SAMA7D65.
**************************************************************************/

/*******************************************************************************
* Copyright (C) 2026 Microchip Technology Inc. and its subsidiaries.
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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <string.h>
#include "definitions.h"
#include "../drv_crypto_trng_hw_6334.h"

// *****************************************************************************
// *****************************************************************************
// Section: Macro definitions
// *****************************************************************************
// *****************************************************************************

#define RNG_BYTES_AT_A_TIME (uint32_t)4

// *****************************************************************************
// *****************************************************************************
// Section: RNG Common Interface Implementation
// *****************************************************************************
// *****************************************************************************

void DRV_CRYPTO_TRNG_Generate(uint8_t *rngData, uint32_t rngLen)
{
    /* rngLen is number of bytes */
    while (rngLen != 0U)
    {
        uint32_t result = TRNG_ReadData();

        /* Stuff the data into the output buffer. Watch for buffer overrun. */
        if (rngLen < RNG_BYTES_AT_A_TIME)
        {
            (void) memcpy((void *)rngData, (void*)&result, rngLen);
            rngLen = 0;
        }
        else
        {
            (void) memcpy((void *)rngData, (void*)&result, RNG_BYTES_AT_A_TIME);
            rngLen -= RNG_BYTES_AT_A_TIME;

            /* Bump the byte buffer by 4 since we are doing 32 bit randoms */
            rngData += RNG_BYTES_AT_A_TIME;
        }

        /* memory barrier */
        __DMB();
    }

    return;
}
