#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "driverlib/pwm.h"
#include "inc/hw_gpio.h"
#include "driverlib\adc.h"
#include <stddef.h>
#include <stdio.h>                     /* This ert_main.c example uses printf/fflush */
#include "Subsystem.h"                 /* Model's header file */
#include "rtwtypes.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
uint8_t ssn = 0, sample = 0;
volatile uint32_t ui32Load;
volatile uint32_t ui32PWMClock;
volatile uint8_t ui8Adjust;
#define PWM_FREQUENCY 55
void rt_OneStep(void);
void rt_OneStep(void)
{
  static boolean_T OverrunFlag = false;
  if (OverrunFlag)
  {
    rtmSetErrorStatus(Subsystem_M, "Overrun");
    return;
  }
  OverrunFlag = true;
  Subsystem_step();
  OverrunFlag = false;
}
/*The error routine that is called if the driver library encounters an error.*/
#ifdef DEBUG
void
error(char *pcFilename, uint32_t ui32Line)
{
}
#endif
/*Configure the UART and its pins.  This must be called before UARTprintf().*/
void
ConfigureUART(void)
{
    /* Enable the GPIO Peripheral used by the UART.*/
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    /* Enable UART0*/
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    /* Configure GPIO Pins for UART mode.*/
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    /* Use the internal 16MHz oscillator as the UART clock source.*/
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    /* Initialize the UART for console I/O.*/
    UARTStdioConfig(0, 115200, 16000000);
}
void
ConfigurePWM(void)
{
    ROM_SysCtlPWMClockSet(SYSCTL_PWMDIV_64);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    ROM_GPIOPinTypePWM(GPIO_PORTD_BASE, GPIO_PIN_0);
    ROM_GPIOPinConfigure(GPIO_PD0_M1PWM0);
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;
    ROM_GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_DIR_MODE_IN);
    ROM_GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    ui32PWMClock = SysCtlClockGet() / 64;
    ui32Load = (ui32PWMClock / PWM_FREQUENCY) - 1;
    PWMGenConfigure(PWM1_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN);
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_0, ui32Load);
    ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, ui8Adjust * ui32Load / 1000);
    ROM_PWMOutputState(PWM1_BASE, PWM_OUT_0_BIT, true);
    ROM_PWMGenEnable(PWM1_BASE, PWM_GEN_0);
}

void
ConfigureADC(void)
{
    uint32_t ssn = 0, sample = 0;
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);
    ADCSequenceDisable(ADC0_BASE, ssn);
    ADCSequenceConfigure(ADC0_BASE, ssn, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, ssn, sample, ADC_CTL_IE|ADC_CTL_END|ADC_CTL_CH1);
    ADCSequenceEnable(ADC0_BASE, ssn);
}
    static void vTask1( void *pvParameters );
    static void vTask2( void *pvParameters );
    xTaskHandle xTask1Handle;
    xTaskHandle xTask2Handle;
void
main(void)
{
    /* Create the first task at priority 2.  This time the task parameter is
    not used and is set to NULL.  The task handle is also not used so likewise
    is also set to NULL. */
    xTaskCreate( vTask1, "Task 1", 240, NULL, 2, NULL );
                                     /* ^^^^void *pvParameters  */
         /* The task is created at priority 2 ^. */

    /* Create the second task at priority 1 - which is lower than the priority
    given to Task1.  Again the task parameter is not used so is set to NULL -
    BUT this time we want to obtain a handle to the task so pass in the address
    of the xTask2Handle variable. */
    xTaskCreate( vTask2, "Task 2", 240, NULL, 1, &xTask2Handle );
        /* The task handle is the last parameter ^^^^^^^^^^^^^ */


    ui8Adjust = 83;
    static boolean_T OverrunFlag = false;
    uint32_t value;
    /*Enable lazy stacking for interrupt handlers.  This allows floating-point
     instructions to be used within interrupt handlers, but at the expense of
     extra stack usage.*/
    ROM_FPULazyStackingEnable();
    /* Set the clocking to run directly from the crystal.*/
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
    /* Enable the GPIO port that is used for the on-board LED.*/
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    /* Enable the GPIO pins for the LED (PF2 & PF3).*/
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);
    /* Initialize the UART.*/
    ConfigureUART();
    /* Initialize PWM */
    ConfigurePWM();
    /* Initialize ADC */
    ConfigureADC();
    /* Initialize model */
    Subsystem_initialize();
    /* Attach rt_OneStep to a timer or interrupt service routine with
       period 1.0 seconds (the model's base sample time) here.  The
       call syntax for rt_OneStep is

       rt_OneStep();

       printf("Warning: The simulation will run forever. "
       "Generated ERT main won't simulate model step behavior. "
       To change this behavior select the 'MAT-file logging' option.\n");
       fflush((NULL));
       while (rtmGetErrorStatus(Subsystem_M) == (NULL)) {
       Perform other application tasks here
       }
       Disable rt_OneStep() here
       Terminate model
       Subsystem_terminate();*/

       vTaskStartScheduler();
       /* Start the scheduler so our tasks start executing. */
while(1)
    {
    /* Turn on the BLUE LED.*/
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
    /* Delay for a bit.*/
    /*SysCtlDelay(SysCtlClockGet() / 10 / 3);*/
    ADCProcessorTrigger(ADC0_BASE, ssn);
    while(!ADCIntStatus(ADC0_BASE, ssn, false)){}
    ADCSequenceDataGet(ADC0_BASE, ssn, &value);
    ADCIntClear(ADC0_BASE, ssn);
    /*SysCtlDelay(SysCtlClockGet() / 10 / 3);*/
    /* step el control*/
    if (OverrunFlag)
    {
    rtmSetErrorStatus(Subsystem_M, "Overrun");
    return;
    }
   OverrunFlag = true;
   /* Set model inputs here */
   Subsystem_U.In1 = 360;
   /*setpoint*/
   Subsystem_Y.Out1 =(value*360)/3000;
   /*actual value*/
   Subsystem_step();
   /* Get model outputs here */
   ui8Adjust = 83+Subsystem_DW.DiscreteTransferFcn_states;
if (ui8Adjust < 56)
    {
    ui8Adjust = 56;
    }
if (ui8Adjust > 111)
    {
    ui8Adjust = 111 ;
    }
   ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, ui8Adjust * ui32Load / 1000);
   /* Indicate task complete */
   OverrunFlag = false;
   UARTprintf("%d\n",ui8Adjust);
   /* Turn off the BLUE LED.*/
   GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
   ROM_SysCtlDelay(1000);
}
}
static void vTask1( void *pvParameters )
{

    for( ;; )
    {

        vTaskPrioritySet(NULL, 1);
    }
}static void vTask2( void *pvParameters )
{

    for( ;; )
    {

        vTaskPrioritySet(NULL, 1);
    }
}
