/*
 * Movimientos.c
 *
 *  Created on: 5 nov. 2019
 *      Author: MIO
 */

#include "Movimientos.h"


void MovimientosBasicos(int accion){
    xSemaphoreTake(Mutexmotores,portMAX_DELAY);//Antes de usar PWMPulse... usamos el semáforo

    switch(accion){
        case PARADO:
            PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6,STOPCOUNT);//Der
            PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7,STOPCOUNT);//Izq
            break;

        case AVANZA:
            PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6,COUNT_1MS);
            PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7,COUNT_1MS);
            break;
        case AVANZAMEDIO:
                    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6,(STOPCOUNT-COUNT_1MS)/2+COUNT_1MS);
                    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7,(STOPCOUNT-COUNT_1MS)/2+COUNT_1MS);
                    break;

        case RETROCEDE:
             PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6,COUNT_2MS);
             PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7,COUNT_2MS);
            break;

        case IZQUIERDA:
             PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6,COUNT_1MS);
             PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7,COUNT_2MS);
             break;

        case DERECHA:
             PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6,COUNT_2MS);//Der
             PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7,COUNT_1MS);//Izq
             break;
        case ATRASDERECHA:
                     PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6,COUNT_2MS);//Der
                     PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7,STOPCOUNT);//Izq
                     break;
        case DELANTEDERECHA:
                             PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6,COUNT_1MS);//Der
                             PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7,STOPCOUNT);//Izq
                             break;
        case DELANTEIZQUIERDA:
                             PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6,COUNT_1MS);//Der
                             PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7,STOPCOUNT);//Izq
                             break;
        case ATRASIZQUIERDA:
                     PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6,STOPCOUNT);//Der
                     PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7,COUNT_2MS);//Izq
                     break;
        case GIRAR:
                     PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6,COUNT_1MS);//Der
                     PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7,COUNT_2MS);//Izq
                     vTaskDelay(configTICK_RATE_HZ);
                     break;
        default:
            PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6,STOPCOUNT);
            PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7,STOPCOUNT);
            break;
    }
   xSemaphoreGive(Mutexmotores);

}
void Flanqueo(int lado)
{
    if(lado==1)//derecha
    {
        MovimientosBasicos(PARADO);
        vTaskDelay(0.25*configTICK_RATE_HZ);
        MovimientosBasicos(DERECHA);
        vTaskDelay(0.25*configTICK_RATE_HZ);
        MovimientosBasicos(AVANZA);
        vTaskDelay(0.5*configTICK_RATE_HZ);
        MovimientosBasicos(PARADO);
        vTaskDelay(0.25*configTICK_RATE_HZ);
        MovimientosBasicos(IZQUIERDA);
        vTaskDelay(0.25*configTICK_RATE_HZ);
        MovimientosBasicos(AVANZA);
        vTaskDelay(0.5*configTICK_RATE_HZ);
    }
    else if(lado==0)//izquierda
    {
        MovimientosBasicos(PARADO);
        vTaskDelay(0.25*configTICK_RATE_HZ);
        MovimientosBasicos(IZQUIERDA);
        vTaskDelay(0.25*configTICK_RATE_HZ);
        MovimientosBasicos(AVANZA);
        vTaskDelay(0.5*configTICK_RATE_HZ);
        MovimientosBasicos(PARADO);
        vTaskDelay(0.25*configTICK_RATE_HZ);
        MovimientosBasicos(DERECHA);
        vTaskDelay(0.25*configTICK_RATE_HZ);
        MovimientosBasicos(AVANZA);
        vTaskDelay(0.5*configTICK_RATE_HZ);
    }
}

