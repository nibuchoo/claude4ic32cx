![Microchip logo](https://raw.githubusercontent.com/wiki/Microchip-MPLAB-Harmony/Microchip-MPLAB-Harmony.github.io/images/microchip_logo.png)
![Harmony logo small](https://raw.githubusercontent.com/wiki/Microchip-MPLAB-Harmony/Microchip-MPLAB-Harmony.github.io/images/microchip_mplab_harmony_logo_small.png)

# Microchip MPLAB® Harmony 3 Release Notes

## Smart Energy Metrology Apps Release v2.0.0

### New Features

- **Applications**

  - Provided Applications are the same as in previous release, but with enhanced capabilities.

- **Classic Metrology Projects General Improvements**
  - Reviewed callback handling after changes in context invocation (interrupt/process) from Driver.
    - Process Integration Period directly in callback, removing the need for intermediate flag setting.
  - Enhanced Event detection and handling for a better resource and memory management.
  - Updated memory offsets for Energy and Max Demand data; added logic to erase these data for all months.
  - Improved demand windowing and divisor accuracy.
  - Copy Energy and Max Demand data to a temporal buffer for storage integrity during month transitions.
  - Enhanced integration period processing by handling it directly in callbacks (taking advantage of invocation from main process context).
  - RTC changes no longer clear Energy/Max Demand.
  - Avoided printing Energy/Max Demand values when not requested.
  - Improved DataLog callbacks to only change state when appropriate.

- **Dependency Changes**
  - Projects updated to match newest driver, peripheral, and middleware repository changes.
  - Removed file system support from datalog apps to improve performance and simplify memory handling.

- **Metrology Library Versions**
  - This release uses the following versions of the Metrology Library binaries:
    - Traditional Metrology Library v3.04.00
    - Multi-Channel Metrology Library v4.00.00 (No change from previous release)
  - Refer to Metrology Library docuemntation to see changes introduced in the library (find links in Collaterals section on [Readme](./readme.md) file)

### Bug fixes

- Removed uncontrolled interrupt blocks by removing the usage of File System and using directly the Memory Driver in asynchronous mode.
- Improved safety of integration period handling within callback/task context.
- Fixed potential memory integrity issues by storing Energy and Max Demand data in temporary buffers before writing to memory.
- Corrected handling of memory offsets and deletion of monthly energy data.

### Known Issues

- None.

### Development Tools

- [MPLAB® X IDE v6.25](https://www.microchip.com/mplab/mplab-x-ide)
- [MPLAB® XC32 C/C++ Compiler v5.00](https://www.microchip.com/mplab/compilers)
- MPLAB® X IDE plug-ins:
  - MPLAB® Code Configurator 5.6.3 or higher
- PIC32CX-MT family (MCUs):
  - PIC32CX-MT DFP 1.4.152 or higher

### Notes

- None.

---

## Smart Energy Metrology Apps Release v1.0.0

### New Features

- **Applications**

  - Demo Meter application is provided in this release as an example of the most common functionalities included in an electricity meter. It allows the user to evaluate the features and to test the high accuracy that can be achieved with the Microchip Smart Metering solutions.
  - Demo Multi-channel Meter application is provided in this release. It allows the user to evaluate the multi channel features and to test the high accuracy that can be achieved with the Microchip Smart Metering solutions.
  - Examples of EMAFE peripheral usage. EMAFE consists of high resolution ADCs for Voltage and Current sensing, including Voltage reference and Temperature sensor. Examples for DMA and Polled versions are provided.
  - Examples of ICM peripheral usage. ICM is used to calculate Hash and monitor the integrity of a user defined memory region. Examples for Compare Mode and Monitor Mode are provided.

- **New Features and Enhancements**
  - EMAFE and ICM examples are new to this version.
  - Events App of Demo Meter Applications adapted to new Events reporting from Metrology Driver.

- **Metrology Library Versions**
  - This release uses the following versions of the Metrology Library binaries:
    - Traditional Metrology Library v3.03.00
    - Multi-Channel Metrology Library v4.00.00

- **Backup Mode Test**

  - When Backup mode is to be used with a battery, a line of code has to be modified in the *SUPC_BackupModeEnter* function, on the *Switch off voltage regulator* section, as seen on the following code block:
    ```c
    void SUPC_BackupModeEnter(void)
    {
        SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

        /* Switch off voltage regulator */
        //SUPC_REGS->SUPC_CR = SUPC_CR_KEY_PASSWD | SUPC_CR_VROFF_Msk;
        SUPC_REGS->SUPC_CR = SUPC_CR_KEY_PASSWD | SUPC_CR_SHDW_Msk;

        /* Enable CPU Interrupt */
        __DMB();
        __enable_irq();

        /* Enter Backup */
        __DSB();
        __WFI();
    }
    ```

### Bug fixes

- Corrected implementation of Backup Mode entering and associated wake-up.

### Known Issues

- None.

### Development Tools

- [MPLAB® X IDE v6.25](https://www.microchip.com/mplab/mplab-x-ide)
- [MPLAB® XC32 C/C++ Compiler v4.60](https://www.microchip.com/mplab/compilers)
- MPLAB® X IDE plug-ins:
  - MPLAB® Code Configurator 5.5.3 or higher
- PIC32CX-MT family (MCUs):
  - PIC32CX-MT DFP 1.3.132 or higher

### Notes

- This is the first Metrology Apps Release in this smartenergy_metrology_apps independent repository. Previous versions were released along with Metrology Drivers as part of smartenergy_metrology repository.