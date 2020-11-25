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
    digitalToggleFast(SYNC_OUT_PIN);

    if (rBuf == wBuf)
    {
        debug("Num Writes: ", numWrites);
        error(PSTR("Buffer Overrun Error!"));
    }

    volatile int *off = &offset;
    if (rBuf == &pBOver)
    {
        off = &buf_overflow_offset; //change offset for overflow case
    }

    if (*off >= PRINT_BUF_MULT)
    {
        error(PSTR("offset too large"));
    }

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
            rBuf->data[*off][ADC_CHAN * j + i] = adc->analogRead(adc_pins[i]);
        }
    }

    rBuf->time[*off] = time;

    (*off)++;

    if (wBuf == nullptr) //wBuf is empty (waiting to write)
    {
        if (rBuf == &pBOver) //reset overflow
        {
            //reset wBuf and rBuf
            if (preOverflowBuffer == 1)
            {
                wBuf = &pB1;
                rBuf = &pB2;
            }
            else if (preOverflowBuffer == 2)
            {
                wBuf = &pB2;
                rBuf = &pB1;
            }
            else
            {
                error(PSTR("Invalid before set after overflow completion"));
            }

            offset = 0;              //wrap around buffer;
            print_ready_flag = 1;    //set print_ready_flag
            print_overflow_flag = 1; //set print_overflow_flag
        }
        else if (*off >= PRINT_BUF_MULT) //need to switch buffer
        {
            wBuf = rBuf; //set write buffer

            //swap read buffer
            if (rBuf == &pB1)
            {
                rBuf = &pB2;
            }
            else if (rBuf == &pB2)
            {
                rBuf = &pB1;
            }
            else
            {
                error(PSTR("Invalid Read Buffer"));
            }

            offset = 0;           //wrap around buffer;
            print_ready_flag = 1; //set print flag
        }
    }
    else //wBuf is full (write is busy)
    {
        if (*off >= PRINT_BUF_MULT) //need to switch buffer
        {
            if (rBuf == &pBOver) //already in overflow (lose data)
            {
                numErrors++;
                (*off)--; //discard last sample
            }
            else if (buf_overflow_offset > 0) //waiting to write overflow buf
            {
                numErrors++;
                (*off)--; //discard last sample
            }
            else //switch to overflow buffer
            {
                //track previous buffer before overflow
                if (rBuf == &pB1)
                {
                    preOverflowBuffer = 1;
                }
                else if (rBuf == &pB2)
                {
                    preOverflowBuffer = 2;
                }
                else
                {
                    error(PSTR("Invalid rBuf before overflow"));
                }

                rBuf = &pBOver; //set overflow buffer;
            }
        }
    }

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