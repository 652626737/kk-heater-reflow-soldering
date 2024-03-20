#ifndef _BUZZ_H_
#define _BUZZ_H_

#include <Arduino.h>

#define BUZZ_PIN 12
#define MAX_BUZZ_TIME 10
class Buzzer
{

public:
    Buzzer();

    void ring(int8_t times);
    void ringLong(int8_t times);
    void play(int8_t song);

private:
    bool state;
    void ringInterval(int8_t times, int interval);
    void tone(unsigned int frequency, unsigned long duration = 0);
    void noTone();
};

extern Buzzer buzzer;

#endif