//*****************************************************************************
//
//Codigo creado por David Calabrés González Y Cristina López Jiménez
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h> 			 // rand()
#include "inc/hw_memmap.h"       // TIVA: Definiciones del mapa de memoria
#include "inc/hw_types.h"        // TIVA: Definiciones API
#include "inc/hw_ints.h"         // TIVA: Definiciones para configuracion de interrupciones
#include "driverlib/gpio.h"      // TIVA: Funciones API de GPIO
#include "driverlib/pin_map.h"   // TIVA: Mapa de pines del chip
#include "driverlib/rom.h"       // TIVA: Funciones API incluidas en ROM de micro (MAP_)
#include "driverlib/rom_map.h"   // TIVA: Mapeo automatico de funciones API incluidas en ROM de micro (MAP_)
#include "driverlib/sysctl.h"    // TIVA: Funciones API control del sistema
#include "driverlib/uart.h"      // TIVA: Funciones API manejo UART
#include "driverlib/interrupt.h" // TIVA: Funciones API manejo de interrupciones
#include "utils/uartstdio.h"     // TIVA: Funciones API UARTSTDIO (printf)
#include "drivers/buttons.h"     // TIVA: Funciones API manejo de botones
#include "FreeRTOS.h"            // FreeRTOS: definiciones generales
#include "task.h"                // FreeRTOS: definiciones relacionadas con tareas
#include "semphr.h"              // FreeRTOS: definiciones relacionadas con semaforos
#include "event_groups.h"        // FreeRTOS: definiciones relacionadas con grupos de eventos
#include "Movimientos.h"
#include "driverlib/pwm.h"
#include "config_adc.h"
#include "timers.h"
//ESTADOS
#define BUSCAR 0
#define ATACAR 1
#define ATAQUEM 2
#define TIMER_MUESTREO_ADC      TIMER2_BASE
//Globales
uint8_t compensacion=0;
EventGroupHandle_t FlagsEventos;
int ESTADO=BUSCAR;
int ESTADOANT=BUSCAR;
int prioridad=0;
int lugarflanqueo=0;
//EVENTBITS
#define ENCODERUP ( 1 << 3 )
#define ENCODERDOWN ( 1 << 4 )
#define ENCODERUP1 ( 1 << 5 )
#define ENCODERDOWN1 ( 1 << 6 )
#define ENSDD (1<< 7)
#define ENSDI (1<< 8)
#define ENSAD (1<< 9)
#define ENSAI (1<< 10)

//globales
TaskHandle_t ManejaADC=NULL,ManejaEncoder=NULL;

float giro[2];
int CurvaADC[]={3170,3165,1743,931,653,650};
int valores[]={3,4,8,16,24,25};
// Semaforo mutex para acceso a UART
xSemaphoreHandle g_pUARTSemaphore, Mutexmotores;
//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
}

#endif

//*****************************************************************************
//
// This hook is called by FreeRTOS when an stack overflow error is detected.
//
//*****************************************************************************
void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName)
{
	//
	// This function can not return, so loop forever.  Interrupts are disabled
	// on entry to this function, so no processor interrupts will interrupt
	// this loop.
	//
	while(1)
	{
	}
}


void vApplicationIdleHook(xTaskHandle *pxTask, signed char *pcTaskName)
{
	SysCtlSleep();
}

int binary_lookup(int *A, int key, int imin, int imax)
{
    int pos=imin;
  while (pos < imax)
    {
      if (A[pos] > key)
        pos++;
      else
      {
          return pos;
      }
    }
}

// Rutinas de interrupcion

void GPIOBIntHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken=pdFALSE;
    if(GPIOIntStatus(GPIO_PORTB_BASE,GPIO_PIN_4 |GPIO_PIN_5|GPIO_PIN_7| GPIO_PIN_3) & GPIO_PIN_3)//para ver que puerto es
    {
        if(GPIOPinRead(GPIO_PORTB_BASE,GPIO_PIN_3) & GPIO_PIN_3)
        {
            xEventGroupSetBitsFromISR(FlagsEventos,ENSAI,&xHigherPriorityTaskWoken);
        }
    }
    else if(GPIOIntStatus(GPIO_PORTB_BASE,GPIO_PIN_4 |GPIO_PIN_5|GPIO_PIN_7| GPIO_PIN_3) & GPIO_PIN_4)
    {
        if(GPIOPinRead(GPIO_PORTB_BASE,GPIO_PIN_4) & GPIO_PIN_4)
        {
            xEventGroupSetBitsFromISR(FlagsEventos,ENSDD,&xHigherPriorityTaskWoken);
        }
    }
    else if(GPIOIntStatus(GPIO_PORTB_BASE,GPIO_PIN_4 |GPIO_PIN_5|GPIO_PIN_7| GPIO_PIN_3) & GPIO_PIN_5)
    {
        if(GPIOPinRead(GPIO_PORTB_BASE,GPIO_PIN_5) & GPIO_PIN_5)
        {
            xEventGroupSetBitsFromISR(FlagsEventos,ENSAD,&xHigherPriorityTaskWoken);
        }
    }
    else if(GPIOIntStatus(GPIO_PORTB_BASE,GPIO_PIN_4 |GPIO_PIN_5|GPIO_PIN_7| GPIO_PIN_3) & GPIO_PIN_7)
    {
        if(GPIOPinRead(GPIO_PORTB_BASE,GPIO_PIN_7) & GPIO_PIN_7)
        {
            xEventGroupSetBitsFromISR(FlagsEventos,ENSDI,&xHigherPriorityTaskWoken);
        }
    }
          GPIOIntClear(GPIO_PORTB_BASE,GPIO_PIN_4 |GPIO_PIN_5|GPIO_PIN_7| GPIO_PIN_3);
}

static portTASK_FUNCTION(ENCODERSUELOTask,pvParameters){
    EventBits_t sensorsuelo;
    while(1)
    {
        sensorsuelo=xEventGroupWaitBits(FlagsEventos,ENSDD|ENSDI|ENSAD|ENSAI, pdTRUE,pdFALSE,portMAX_DELAY);
        prioridad=1;
        switch(sensorsuelo)
        {
        case ENSDD: //ENCODER SUELO DELANTE DERECHA
            MovimientosBasicos(PARADO);
            MovimientosBasicos(ATRASIZQUIERDA);//se para los motores y se mueve hacia atras la rueda izquierda

            break;
        case ENSDI: //ENCODER SUELO DELANTE IZQUIERDA
            MovimientosBasicos(PARADO);
            MovimientosBasicos(ATRASDERECHA);//se para los motores y se mueve hacia atras la rueda derecha

                    break;
        case ENSAD: //ENCODER SUELO ATRAS DERECHA
            MovimientosBasicos(PARADO);
            MovimientosBasicos(DELANTEIZQUIERDA);
            MovimientosBasicos(AVANZA);
            lugarflanqueo=0;//flanquear izquierda
            prioridad=0;
            //poner estado prioridad o delay
                    break;
        case ENSAI: //ENCODER SUELO ATRAS IZQUIERDA
            MovimientosBasicos(PARADO);
            MovimientosBasicos(DELANTEDERECHA);
            MovimientosBasicos(AVANZA);
            lugarflanqueo=1;//flanquear derecha
            prioridad=0;

                    break;
        }
    }
}

static portTASK_FUNCTION(ADCTask,pvParameters){
    MuestrasADC muestras;
int i=0;
        while(1){
            configADC_LeeADC(&muestras);
            int dist=binary_lookup(CurvaADC,muestras.chan1,0,5);

            if(prioridad==0)//sino estoy saliedome del tatami
            {
                if(valores[dist]<=8)//atacar
                {
                    ESTADO=ATACAR;
                    if(ESTADOANT==BUSCAR)//miro el estado aterior
                    {
                        MovimientosBasicos(AVANZA);
                    }
                    else if(ESTADOANT==ATAQUEM)//si lo he detectado a distancia
                    {
                        if(lugarflanqueo==0)//flanqueo por la iz
                        {
                            Flanqueo(0);
                        }
                        else if(lugarflanqueo==1)//flanqueo derecha
                        {
                            Flanqueo(1);
                        }
                        else
                        {
                            MovimientosBasicos(AVANZA);
                        }
                    }
                    else if(ESTADOANT==ATACAR)
                    {
                        if(i==0)//solo lo eecuto cuado entro al estado
                        {
                            MovimientosBasicos(PARADO);
                            i=1;
                        }
                        MovimientosBasicos(AVANZA);
                    }
                    else
                    {
                        MovimientosBasicos(AVANZA);
                    }
                    ESTADOANT=ATACAR;
                }
                else if(valores[dist]>8 && valores[dist]<=16)//atacar avanzado
                {
                    ESTADO=ATAQUEM;
                    if(ESTADOANT==BUSCAR)
                    {
                        MovimientosBasicos(AVANZA);
                    }
                    else if(ESTADOANT==ATACAR)
                    {

                        MovimientosBasicos(GIRAR);
                    }
                    else
                    {
                        MovimientosBasicos(AVANZA);
                    }
                    ESTADOANT=ATAQUEM;
                    i=0;
                }
                else if(valores[dist]>16)//buscar
                {
                    ESTADO=BUSCAR;
                    if(ESTADOANT==ATACAR)
                    {

                        MovimientosBasicos(GIRAR);
                    }
                    else
                    {
                        MovimientosBasicos(AVANZA);
                    }
                    ESTADOANT=BUSCAR;
                    i=0;
                }
            }
       }
}

//*****************************************************************************
//
// Initialize FreeRTOS and start the initial set of tasks.
//
//*****************************************************************************
int main(void)
{
	//
	// Set the clocking to run at 40 MHz from the PLL.
	//
	MAP_SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
			SYSCTL_OSC_MAIN);

	//
	// Initialize the UART and configure it for 115,200, 8-N-1 operation.
	//
	// se usa para mandar mensajes por el puerto serie
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
	MAP_GPIOPinConfigure(GPIO_PA1_U0TX);
	MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	UARTStdioConfig(0, 115200, SysCtlClockGet());

	//Inicializa el puerto F
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	     //PUERTO PARA ENCODERS DE SUELO
	         SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	         GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_4 |GPIO_PIN_5|GPIO_PIN_7| GPIO_PIN_3);
	         GPIOPadConfigSet(GPIO_PORTA_BASE,GPIO_PIN_4 |GPIO_PIN_5|GPIO_PIN_7| GPIO_PIN_3, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_OD);
	         GPIOIntTypeSet(GPIO_PORTB_BASE,GPIO_PIN_4 |GPIO_PIN_5|GPIO_PIN_7| GPIO_PIN_3 , GPIO_BOTH_EDGES);
	         GPIOIntClear(GPIO_PORTB_BASE,GPIO_PIN_4 |GPIO_PIN_5|GPIO_PIN_7| GPIO_PIN_3);
	         GPIOIntEnable(GPIO_PORTB_BASE,GPIO_PIN_4 |GPIO_PIN_5|GPIO_PIN_7| GPIO_PIN_3);
	         IntEnable(INT_GPIOB);

    //Inicializa los botones (tambien en el puerto F) y habilita sus interrupciones
    ButtonsInit();
    MAP_GPIOIntTypeSet(GPIO_PORTF_BASE, ALL_BUTTONS,GPIO_FALLING_EDGE);
    MAP_GPIOIntEnable(GPIO_PORTF_BASE,ALL_BUTTONS);
    MAP_IntEnable(INT_GPIOF);

    // The PWM peripheral must be enabled for use.
            //
            SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
            // For this example PWM0 is used with PortB Pin6.  The actual port and
            // pins used may be different on your part, consult the data sheet for
            // more information.
            // GPIO port B needs to be enabled so these pins can be used.

            //SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
              // Set the PWM clock to the system clock.
              //
              SysCtlPWMClockSet(SYSCTL_PWMDIV_16);

            // Configure the GPIO pin muxing to select PWM00 functions for these pins.
            // This step selects which alternate function is available for these pins.
            // This is necessary if your part supports GPIO pin function muxing.
            // Consult the data sheet to see which functions are allocated per pin.

            GPIOPinConfigure(GPIO_PF2_M1PWM6);
            GPIOPinConfigure(GPIO_PF3_M1PWM7);
            // Configure the PWM function for this pin.
            // Consult the data sheet to see which functions are allocated per pin.
            GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_2);
            GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_3);
            // Configure the PWM0 to count up/down without synchronization.

            PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
            //
            // Set the PWM period to 250Hz.  To calculate the appropriate parameter
            // use the following equation: N = (1 / f) * SysClk.  Where N is the
            // function parameter, f is the desired frequency, and SysClk is the
            // system clock frequency.
            // In this case you get: (1 / 50Hz) * 1MHz = 20000 cycles.  Note that
            // the maximum period you can set is 2^16.

            PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, PERIOD_PWM);
            //
            // Set PWM0 to a duty cycle of 25%.  You set the duty cycle as a function
            // of the period.  Since the period was set above, you can use the
            // PWMGenPeriodGet() function.  For this example the PWM will be high for
            // 25% of the time or 16000 clock ticks (20000 / 4).
            //
            PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, COUNT_1MS);//PWMGenPeriodGet(PWM0_BASE, PWM_GEN_0) / 4 //pf2
            PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, COUNT_1MS); //Pf3//motor derecho
            //
            // Enable the PWM0 Bit0 (PD0) output signal.
            //
            PWMOutputState(PWM1_BASE, PWM_OUT_6_BIT, true);
            PWMOutputState(PWM1_BASE, PWM_OUT_7_BIT, true);

            //timer
            MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
               MAP_SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_TIMER2);

            PWMGenEnable(PWM1_BASE, PWM_GEN_3);
            configADC_IniciaADC();
                uint32_t ui32Period;
                ADCSequenceDisable(ADC0_BASE,3);
                ADCSequenceConfigure(ADC0_BASE,3,ADC_TRIGGER_TIMER,0);
                TimerControlTrigger(TIMER_MUESTREO_ADC,TIMER_A,true);//TIMER2_BASE,TIMER_A,true);
                TimerConfigure(TIMER_MUESTREO_ADC, TIMER_CFG_PERIODIC);
                ui32Period = SysCtlClockGet();
                TimerLoadSet(TIMER_MUESTREO_ADC, TIMER_A, ui32Period -1);
                TimerEnable(TIMER_MUESTREO_ADC, TIMER_A);
                ADCSequenceEnable(ADC0_BASE,3);

	UARTprintf("\n\nWelcome to the TIVA EK-TM4C123GXL FreeRTOS Demo!\n");

	 Mutexmotores=xSemaphoreCreateMutex();
	 vTaskDelay(5000);

    if((xTaskCreate(ADCTask, (portCHAR *)"ADC", 2048,NULL,tskIDLE_PRIORITY +1, &ManejaADC) != pdTRUE))
    {
                while(1);
    }
	if((xTaskCreate(ENCODERSUELOTask, (portCHAR *)"ENCODERSUELO", 128,NULL,tskIDLE_PRIORITY +1, &ManejaEncoder) != pdTRUE))
	        {
	            while(1);
	        }

	//Crea el grupo de eventos
	FlagsEventos = xEventGroupCreate();
	if( FlagsEventos == NULL )
	{
		while(1);
	}

	//
	// Start the scheduler.  This should not return.
	//
	vTaskStartScheduler();	//el RTOS habilita las interrupciones al entrar aqui, asi que no hace falta habilitarlas

	//
	// In case the scheduler returns for some reason, print an error and loop
	// forever.
	//

	while(1)
	{
	}
}

