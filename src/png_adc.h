#ifndef PNG_ADC_H
#define PNG_ADC_H

#include <ADC.h>
#include <ADC_util.h>

void adc_isr();

#define ADC_CHAN 10

ADC *adc = new ADC();

const uint8_t adc_pins[] = {A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10};

#endif
