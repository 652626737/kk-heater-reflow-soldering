#include "Buzzer.h"
#include "pitches.h"
#include <Arduino.h>

// notes in the melody:
int melody[] = {

  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};
#define TONE_CHANNEL 15

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {

  4, 8, 8, 4, 4, 4, 4, 4
};
bool attatched = false;

Buzzer buzzer;
Buzzer::Buzzer()
{
    pinMode(BUZZ_PIN, OUTPUT);
    digitalWrite(BUZZ_PIN, LOW);
    state = false;
}
void Buzzer::ringInterval(int8_t times, int interval) {
    if (times <= 0 || times > MAX_BUZZ_TIME)
    {
        return;
    }
    state = true;
    
    uint8_t i;
    for (i = 0; i < times; i++){
        digitalWrite(BUZZ_PIN, HIGH);
        delay(interval);
        digitalWrite(BUZZ_PIN, LOW);
        delay(interval);
    }
    state = false;
}

void Buzzer::ring(int8_t times)
{
    ringInterval(times, 500);
}
void Buzzer::ringLong(int8_t times)
{
    ringInterval(times, 2000);
}
void Buzzer::play(int8_t song) {
  // iterate over the notes of the melody:

  for (int thisNote = 0; thisNote < 8; thisNote++) {

    // to calculate the note duration, take one second divided by the note type.

    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.

    // int noteDuration = 1000 / noteDurations[thisNote];
    // ledcWriteTone(channel, 1000); 
    // tone(8, melody[thisNote], noteDuration);

    // // to distinguish the notes, set a minimum time between them.

    // // the note's duration + 30% seems to work well:

    // int pauseBetweenNotes = noteDuration * 1.30;

    // delay(pauseBetweenNotes);

    // // stop the tone playing:

    // noTone(8);

  }
}
/**
 * 
 * https://github.com/lbernstone/Tone32
*/
void Buzzer::tone(unsigned int frequency, unsigned long duration)
{
    if (ledcRead(BUZZ_PIN)) {
        log_e("Tone channel %d is already in use", TONE_CHANNEL);
        return;
    }
    ledcAttachPin(BUZZ_PIN, TONE_CHANNEL);
    ledcWriteTone(TONE_CHANNEL, frequency);
    if (duration) {
        delay(duration);
        noTone();
    }    
}

void Buzzer::noTone()
{
    ledcDetachPin(BUZZ_PIN);
    ledcWrite(TONE_CHANNEL, 0);
}

/**
 * octave八度（1-4）
*/
// void tone(note_t note, uint8_t octave, int delayMill) {
//     ledcAttachPin(BUZZ_PIN, 1);
//     ledcWriteNote(1, note, octave);
//     delay(delayMill);
// }
