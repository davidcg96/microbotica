/*
 * config_adc.h
 *
 *  Created on: 14 nov. 2019
 *      Author: MIO
 */

#ifndef CONFIG_ADC_H_
#define CONFIG_ADC_H_


typedef struct
{
    uint16_t chan1;
    //uint16_t chan2;
} MuestrasADC;

typedef struct
{
    uint32_t chan1;
   // uint32_t chan2;
} MuestrasLeidasADC;


void configADC_ISR(void);
void configADC_DisparaADC(void);
void configADC_LeeADC(MuestrasADC *datos);
void configADC_IniciaADC(void);


#endif /* CONFIG_ADC_H_ */
