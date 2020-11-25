#ifndef PNG_ADC_H
#define PNG_ADC_H

#include "main.h"
#include <ADC.h>

extern IntervalTimer adcTimer;

void adc_isr();

void adc_init();

#define ADC_CHAN 10

extern ADC *adc;

const uint8_t adc_pins[] = {A0, A1, A2, A3, A4, A5, A6, A7, A8, A9};

#endif
