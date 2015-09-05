/* ADC10 - AbstractWiring msp430 value line support for the ADC10 basic peripheral found on the msp430x2xx devices */

#ifndef ADC10_H
#define ADC10_H

#include <AbstractWiring.h>

enum AdcVRef {
    DEFAULT = 0,
    INTERNAL,
    INTERNAL1V5,
    INTERNAL_EXPORT,  // Use internal vref but expose it to VeREF+/- pins
    INTERNAL1V5_EXPORT,
    INTERNAL2V5,
    INTERNAL2V5_EXPORT,
    EXTERNAL
};

class ADC10 {
    private:
        boolean use_calibration;
        enum AdcVRef voltage_reference;
        int16_t cal_adc_offset;
        uint16_t cal_adc_gain;
        uint16_t cal_adc_gain_v1_5;
        uint16_t cal_adc_gain_v2_5;

    public:
        ADC10() {
            voltage_reference = DEFAULT;
            // Upon init - Locate TLV data structure and populate adc calibration constants
            #if defined(TLV_ADC10_1_TAG_) && defined(TLV_ADC10_1_LEN_)
            cal_adc_offset = *( (const volatile uint16_t *)TLV_ADC10_1_TAG_ + 1 + CAL_ADC_OFFSET );
            cal_adc_gain = *( (const volatile uint16_t *)TLV_ADC10_1_TAG_ + 1 + CAL_ADC_GAIN_FACTOR );
            cal_adc_gain_v1_5 = *( (const volatile uint16_t *)TLV_ADC10_1_TAG_ + 1 + CAL_ADC_15VREF_FACTOR );
            cal_adc_gain_v2_5 = *( (const volatile uint16_t *)TLV_ADC10_1_TAG_ + 1 + CAL_ADC_25VREF_FACTOR );
            use_calibration = true;
            #else
            cal_adc_offset = 0;
            cal_adc_gain = 32768;  // default for gain = 1.0 (since it gets divided by 32768 after multiplying by this)
            cal_adc_gain_v1_5 = 32768;
            cal_adc_gain_v2_5 = 32768;
            use_calibration = false;
            #endif
            // TODO: Some Value-Line chips don't have the ADC10 tag constants defined, go looking for them in Info_A anyhow?
        };

        NEVER_INLINE
        void useCalibration(boolean yesno) {
            use_calibration = yesno;
        };

        NEVER_INLINE
        void setReference(enum AdcVRef vref) {
            switch (vref) {
                case DEFAULT:
                    ADC10CTL0 &= ~(SREF_7 | REFON | REFOUT | REFBURST | REF2_5V);
                    break;
                case INTERNAL:
                case INTERNAL1V5:
                    ADC10CTL0 = (ADC10CTL0 & ~(SREF_7 | REFBURST | REFOUT | REF2_5V)) | SREF_1 | REFON;
                    break;
                case INTERNAL_EXPORT:
                case INTERNAL1V5_EXPORT:
                    ADC10CTL0 = (ADC10CTL0 & ~(SREF_7 | REFBURST | REF2_5V)) | SREF_1 | REFON | REFOUT;
                    break;
                case INTERNAL2V5:
                    ADC10CTL0 = (ADC10CTL0 & ~(SREF_7 | REFBURST | REFOUT)) | SREF_1 | REFON | REF2_5V;
                    break;
                case INTERNAL2V5_EXPORT:
                    ADC10CTL0 = (ADC10CTL0 & ~(SREF_7 | REFBURST)) | SREF_1 | REFON | REFOUT | REF2_5V;
                    break;
                case EXTERNAL:
                    ADC10CTL0 = (ADC10CTL0 & ~(SREF_7 | REFBURST | REFOUT | REFON | REF2_5V)) | SREF_2;
            }
            voltage_reference = vref;
        };

        NEVER_INLINE
        uint16_t performCalibrationCorrection(uint16_t val) {
            uint32_t intermediate = val;

            switch (voltage_reference) {
                case DEFAULT:
                    // Vcc reference - use ADC_GAIN_OFFSET and ADC_GAIN_FACTOR
                    // Nothing to do here; gain_offset and gain_factor are added after the switch ()
                    break;
                case INTERNAL:
                case INTERNAL1V5:
                case INTERNAL_EXPORT:
                case INTERNAL1V5_EXPORT:
                    intermediate *= cal_adc_gain_v1_5;
                    intermediate /= 32768;
                    break;
                case INTERNAL2V5:
                case INTERNAL2V5_EXPORT:
                    intermediate *= cal_adc_gain_v2_5;
                    intermediate /= 32768;
                    break;
                default:
                    return val;
            }
            #ifdef CAL_ADC_OFFSET
            intermediate += (int32_t)cal_adc_offset;
            #endif
            #ifdef CAL_ADC_GAIN_FACTOR
            intermediate *= cal_adc_gain;
            intermediate /= 32768;
            #endif
            return (uint16_t) intermediate;
        };

        NEVER_INLINE
        uint16_t sample(uint16_t channel) {
            if (channel == 10)
                return (uint16_t)sample_tempsensor();  /* This will be a 2's-complement integer (in degrees C) returned and should be cast back
                                                        * to int16_t by the caller...
                                                        */
            if (channel > 11)
                return 0;  // not available on MSP430 value-line chips

            // Configure channel
            ADC10CTL1 = channel * INCH0 | ADC10DIV_1;
            if (channel < 8)
                ADC10AE0 = _bitvect[channel];
            // Start conversion (SHT=01, 8xADC10CLK's sample & hold)
            ADC10CTL0 = (ADC10CTL0 & (SREF_7 | REFOUT | REFBURST | REFON | REF2_5V)) | ADC10SHT_2 | ADC10ON;  // Enable REF if used
            if (ADC10CTL0 & REFON)
                __delay_cycles(64);  // VRef settling time

            // Begin conversion
            ADC10CTL0 |= ENC | ADC10SC;
            while (ADC10CTL1 & ADC10BUSY)
                LPM0;
            uint16_t res = ADC10MEM;
            ADC10AE0 = 0x00;

            if (use_calibration)
                res = performCalibrationCorrection(res);

            ADC10CTL0 &= ~(ADC10ON | ENC);
            return res;
        };

        NEVER_INLINE
        int16_t sample_tempsensor() {
            ADC10CTL1 = (uint16_t)10 * INCH0 | ADC10DIV_4;
            ADC10CTL0 = SREF_1 | ADC10SHT_3 | REF2_5V | REFON | ADC10ON | ADC10IE;
            __delay_cycles(64);  // wait for VRef to settle

            uint16_t res = 0, i = 0;
            for (i = 0; i < 8; i++) {
                ADC10CTL0 |= ENC | ADC10SC;
                while (ADC10CTL1 & ADC10BUSY)
                    LPM0;
                res += ADC10MEM;
                __delay_cycles(16);
            }
            ADC10CTL0 &= ~(REFON | ADC10ON | ENC);
            res >>= 3;

            // Perform temperature-compensation
            #if defined(TLV_ADC10_1_TAG_) && defined(CAL_ADC_25T30) && defined(CAL_ADC_25T85)
            uint16_t cal_t30 = *( (const volatile uint16_t *)TLV_ADC10_1_TAG_ + 1 + CAL_ADC_25T30 );
            uint16_t cal_t85 = *( (const volatile uint16_t *)TLV_ADC10_1_TAG_ + 1 + CAL_ADC_25T85 );

            int32_t intermediate = res;
            intermediate -= cal_t30;
            intermediate *= 85 - 30;
            intermediate /= cal_t85 - cal_t30;
            intermediate += 30;
            int16_t resC = (int16_t) intermediate;
            #else
            int16_t resC = res;
            #endif

            setReference(voltage_reference);  // restore original voltage reference configuration

            return resC;
        };
};


#endif /* ADC10_H */
