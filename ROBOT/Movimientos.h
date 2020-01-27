/*
 * Movimientos.h
 *
 *  Created on: 5 nov. 2019
 *      Author: MIO
 */

#ifndef MOVIMIENTOS_H_
#define MOVIMIENTOS_H_



#include <stdbool.h>
#include <stdint.h>
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/pwm.h"
#include "driverlib/interrupt.h"
#include "inc/hw_ints.h"
#include "drivers/buttons.h"
#include "driverlib/interrupt.h"

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/interrupt.h"
#include "driverlib/adc.h"
#include "driverlib/timer.h"
#include "utils/uartstdio.h"
#include "drivers/buttons.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "utils/cpu_usage.h"
#include "event_groups.h"

#define PARADO 0
#define AVANZA 1
#define RETROCEDE 2
#define IZQUIERDA 3 //derecha
#define DERECHA 4 //izq
#define ATRASIZQUIERDA 5 //derecha
#define ATRASDERECHA 6 //izq
#define AVANZAMEDIO 7 //izq
#define DELANTEIZQUIERDA 8 //izq
#define DELANTEDERECHA 9 //izq
#define GIRAR 10

#define BUTTONR_FLAG        ( 1 )
#define BUTTONL_FLAG        ( 1 << 1 )
#define CHOQUE_FLAG      ( 1 << 2 )


#define PERIOD_PWM 50000   // TODO: Ciclos de reloj para conseguir una señal periódica de 50Hz (según reloj de periférico usado)
#define COUNT_1MS 2500 // TODO: Ciclos para amplitud de pulso de 1ms (max velocidad en un sentido)
#define STOPCOUNT 3875  // TODO: Ciclos para amplitud de pulso de parada (1.52ms) diferencia por debajo de STOPCOUNT 10 y por encima 5 3740 gira derecho, izquierdo no, 3750 no gira ninguno, 3760 gira izquierdo derecho no
#define COUNT_2MS 5000   // TODO: Ciclos para amplitud de pulso de 2ms (max velocidad en el otro sentido)
#define NUM_STEPS 50    // Pasos para cambiar entre el pulso de 2ms al de 1ms
#define CYCLE_INCREMENTS (abs(COUNT_1MS-COUNT_2MS))/NUM_STEPS  // Variacion de amplitud tras pulsacion
#define RADIO 3

extern EventGroupHandle_t FlagsEventos;
extern xSemaphoreHandle Mutexmotores;
void MovimientosBasicos(int direccion);
void Flanqueo(int lado);
/* Movemos el robot en direcciÃ³n proa, popa, estribor babor o echamos el ancla*/

#endif /* MOVIMIENTOS_H_ */
