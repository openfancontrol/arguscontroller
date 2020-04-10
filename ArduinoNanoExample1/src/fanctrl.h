
//---------------------------------------------------------
// Argus Controller (Open Hardware)
// fanctrl.h
// Copyright 2020 Argotronic UG (haftungsbeschraenkt)
//
// License: CC BY-SA 4.0
// https://creativecommons.org/licenses/by-sa/4.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, ShareAlike
//---------------------------------------------------------

#ifndef _FANCTRL_H_
#define _FANCTRL_H_

#define PIN_PWM_FAN1 9     // OC1A
#define PIN_PWM_FAN2 10    // OC1B
#define PIN_TACH_FAN1 2    // INT0
#define PIN_TACH_FAN2 3    // INT1
#define CYCLES_PER_REVOLUTION 2.0

class FANCTRL {

public:
    FANCTRL()
        : fanCount(0)
        , lastUpdateTime(0)
        , rpm({ 0 })
    {
    }

    void init(uint8_t fcount)
    {
        fanCount = fcount;
        if (fanCount >= 1) {
            pinMode(PIN_TACH_FAN1, INPUT);
            digitalWrite(PIN_TACH_FAN1, HIGH);
            attachInterrupt(digitalPinToInterrupt(PIN_TACH_FAN1), isr_fan1, FALLING);
            pinMode(PIN_PWM_FAN1, OUTPUT);

            TCCR1A = 1 << WGM11 | 1 << COM1A1 | 1 << COM1B1;    // PWM, PhaseCorrect, 0..ICR1 counting
            TCCR1B = 1 << CS10 | 1 << WGM13;                    // clk/1
            ICR1   = 320;    // fPwm = 25kHz (@16MHz Quarz) -> see Intel specification "4-Wire Pulse Width Modulation (PWM) Controlled Fans"
            OCR1A  = 160;    // fan0 50%

            if (fanCount >= 2) {
                pinMode(PIN_TACH_FAN2, INPUT);
                digitalWrite(PIN_TACH_FAN2, HIGH);
                attachInterrupt(digitalPinToInterrupt(PIN_TACH_FAN2), isr_fan2, FALLING);
                pinMode(PIN_PWM_FAN2, OUTPUT);

                OCR1B = 160;    // fan1 50%
            }
        }
    }

    void update()
    {
        if ((millis() - lastUpdateTime) >= 1000) {

            unsigned long t = millis() - lastUpdateTime;

            if (fanCount >= 1) {
                float frpm;

                // disable interrupt for atomic reading of counter
                detachInterrupt(digitalPinToInterrupt(PIN_TACH_FAN1));
                // calculate rpm
                frpm   = (float)rpmCnt1 * 60000.0 / (float)t / CYCLES_PER_REVOLUTION;
                rpm[0] = (uint16_t)frpm;
                // reset counter
                rpmCnt1 = 0;
                // enable interrupt again
                attachInterrupt(digitalPinToInterrupt(PIN_TACH_FAN1), isr_fan1, FALLING);

                if (fanCount >= 2) {
                    detachInterrupt(digitalPinToInterrupt(PIN_TACH_FAN2));
                    frpm    = (float)rpmCnt2 * 60000.0 / (float)t / CYCLES_PER_REVOLUTION;
                    rpm[1]  = (uint16_t)frpm;
                    rpmCnt2 = 0;
                    attachInterrupt(digitalPinToInterrupt(PIN_TACH_FAN2), isr_fan2, FALLING);
                }

                // set new time stamp
                lastUpdateTime = millis();
            }
        }
    }

    bool setPwm(uint8_t channel, uint8_t pwmPercent)
    {
        if (channel >= fanCount) {
            return false;
        }
        if (pwmPercent > 100) {
            return false;
        }
        uint16_t pwm = ((uint16_t)pwmPercent * 320) / 100;
        pwm          = 320 - pwm;    // on OCR1x pins, pwm signals are inverted with a transistor
        if (channel == 0) {
            OCR1A = pwm;
        } else if (channel == 1) {
            OCR1B = pwm;
        }
        return true;
    }

    uint8_t getPwm(uint8_t channel)
    {
        if (channel >= fanCount) {
            return 0;
        }
        uint16_t pwm;
        if (channel == 0) {
            pwm = OCR1A;
        } else if (channel == 1) {
            pwm = OCR1B;
        }
        pwm                = 320 - pwm;    // on OCR1x pins, pwm signals are inverted with a transistor
        uint8_t pwmPercent = pwm * 100 / 320;
        return (uint8_t)pwmPercent;
    }

    uint16_t getRpm(uint8_t channel)
    {
        if (channel < fanCount) {
            return rpm[channel];
        } else {
            return 0;
        }
    }

private:
    uint8_t       fanCount;
    unsigned long lastUpdateTime;
    uint16_t      rpm[2];
    static int    rpmCnt1, rpmCnt2;

    static void isr_fan1() { rpmCnt1++; }

    static void isr_fan2() { rpmCnt2++; }
};

int FANCTRL::rpmCnt1 = 0;
int FANCTRL::rpmCnt2 = 0;

#endif