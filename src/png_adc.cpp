#include "main.h"
#include "png_adc.h"
#include "png_sync.h"
#include "png_buf.h"
#include "png_mux.h"
#include "png_serial.h"

IntervalTimer adcTimer;

ADC *adc = new ADC();

void adc_isr()
{
    digitalToggleFast(LED_BUILTIN);

    sync_count++;
    if (sync_stage_count < sync_stage_count_switch) {
        if (sync_count >= sync_pulse_period) {
            digitalToggleFast(SYNC_OUT_PIN);
            sync_count = 0;
            sync_stage_count++;
        }
    } else {
        if (sync_count >= sync_wait_period) {
            sync_stage_count = 0;
            sync_count = 0;
        }
    }

    // if (cBufWriteIdx == cBufReadIdx)  
    // {
    //     numErrors++;
    //     cBufReadIdx--;
    //     // debug("Num Writes: ", numWrites);   // right now overrun causes terminal error
    //     // error(PSTR("Buffer Overrun Error!"));
    // }

    // if (cBufReadIdx >= PRINT_BUF_MULT)
    // {
    //     error(PSTR("offset too large"));
    // }

    for (int j = 0; j < MUXED_CHAN; j++)
    {
        /**
			 * MUX LOGIC (CD4052)
			 * 	31	30     <----MCU output pins
			 * 	B	A	OUT	
			 * 	0	0	0
			 * 	0	1	1
			 * 	1	0	2
			 * 	1	1	3
			 */
        switch (mux_state)
        {
        case 0:
        {
            digitalWriteFast(mux_pins[SEL_A], LOW);
            digitalWriteFast(mux_pins[SEL_B], LOW);
            break;
        }
        case 1:
        {
            digitalWriteFast(mux_pins[SEL_A], HIGH);
            digitalWriteFast(mux_pins[SEL_B], LOW);
            break;
        }
        case 2:
        {
            digitalWriteFast(mux_pins[SEL_A], LOW);
            digitalWriteFast(mux_pins[SEL_B], HIGH);
            break;
        }
            // case 3:
            // {
            // 	digitalWriteFast(mux_pins[SEL_A], HIGH);
            // 	digitalWriteFast(mux_pins[SEL_B], HIGH);
            // 	mux_state = 0; //wrap around
            // 	break;
            // }
        }
        mux_state++;
        if (mux_state > 2)
        {
            mux_state = 0; //wrap around
        }

        //read in adc channels
        for (int i = 0; i < ADC_CHAN; i++)
        {
            fill_data(ADC_CHAN * j + i, adc->analogRead(adc_pins[i]) );
        }
        //read in sync adc channel
        fill_sync(adc->analogRead(SYNC_IN_PIN));
    }

    fill_time(time);

    next_line();

#ifdef SERIAL_DEBUG
    debugAll("end of adc_isr");
#endif
}

void adc_init()
{
    for (int i = 0; i < ADC_CHAN; i++)
    {
        pinMode(adc_pins[i], INPUT);
    }

    ///// ADC0 ////
    adc->adc0->setAveraging(1);                                           // set number of averages
    adc->adc0->setResolution(12);                                         // set bits of resolution
    adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED); // change the conversion speed
    adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED);     // change the sampling speed

////// ADC1 /////
#ifdef ADC_DUAL_ADCS
    adc->adc1->setAveraging(1);                                           // set number of averages
    adc->adc1->setResolution(12);                                         // set bits of resolution
    adc->adc1->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED); // change the conversion speed
    adc->adc1->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED);     // change the sampling speed
#endif
    Serial.print(PSTR("ADC Initialized with ["));
    Serial.print(ADC_CHAN);
    Serial.println(PSTR("] ADC channels"));
    Serial.print(PSTR("Sampling at ["));
    Serial.print(1e6 / SAMPLING_PERIOD);
    Serial.println("] Hz");
}