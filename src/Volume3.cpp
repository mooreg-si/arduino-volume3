/*
  Volume3.cpp - Library for tone() with 10-bit volume control.
  Created by Connor Nishijima, November 13th 2016.
  Released under the GPLv3 license.
*/

#include "Volume3.h"
#include "TimerOne.h"

volatile uint8_t pin1 = 0;
volatile uint8_t state1 = 0;
volatile uint16_t tick1 = -1;
volatile uint16_t period1 = 0;
volatile uint16_t volume1 = 0;

volatile bool toneRun1 = false;

const byte timerPin1 = 9;
const byte timerPin2 = 10;

volatile uint16_t beepCycles = -1;  //Total number of cycles if beep duration set
volatile uint16_t beepElapsed = -1; //Number of cycles elapsed from start of beep

void toneBegin(byte pin)
{
  if (pin != pin1)
  {
    if (pin == timerPin1 || pin == timerPin2)
    {
      pin1 = pin;
      Timer1.initialize(10);
      Timer1.pwm(pin1, 0);
      Timer1.setPwmDuty(pin1, 0);
      state1 = 0;
      tick1 = -1;
      period1 = 0;
      volume1 = 0;
      toneRun1 = false;
    }
    else
    {
      return;
    }
  }
}

void cycle()
{
  tick1++;
  if (tick1 >= period1)
  {
    tick1 = 0;
    state1 = !state1;
    uint16_t vol1 = state1 * volume1;
    Timer1.setPwmDuty(pin1, vol1);

    //if using a beep
    if (beepCycles >= 0)
    {
      beepElapsed += 1;
      //if past the beep time
      if (beepElapsed > beepCycles)
      {
        //reset the beep numbers
        beepCycles = -1;
        beepElapsed = -1;
        //stop the tone
        vol.noTone();
      }
    }
  }
}

void Volume::noTone()
{
  Timer1.stop();
  Timer1.setPeriod(2040);
  digitalWrite(pin1, LOW);
  pin1 = 0;
  tick1 = -1;
  period1 = 0;
  volume1 = 0;
  toneRun1 = false;
}

void Volume::tone(uint8_t pin, uint16_t frequency, uint16_t volume, uint16_t beep = -1)
{
  if (frequency <= 4186)
  {
    Timer1.detachInterrupt();
    toneBegin(pin);
    if (frequency < 2)
    {
      frequency = 2;
    }

    //if there is a beep defined
    if (beep > 0)
    {
      /* Calculate the number of cycles in the beep */
      //freq to second
      float freqTime = 1 / (float)frequency;
      //second to millisecond
      freqTime = freqTime * 1000;
      //to number of cycles
      beepCycles = beep / freqTime;
      beepElapsed = 0;
    }
    else
    {
      beepCycles = -1;
    }

    period1 = 100000 / (frequency * 2) + 1;
    volume1 = volume;
    if (toneRun1 == false)
    {
      toneRun1 = true;
      state1 = 0;
      tick1 = 0;
    }
    Timer1.attachInterrupt(cycle);
  }
}
