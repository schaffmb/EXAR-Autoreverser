//*****************************************************************************
//*
//* EXAR_AutoReverser.cpp
//*   Copyright © 2024-2026 Michael Schaff
//*   All rights reserved.
//*
//* Brief:
//*   DCC Isolated Autoreversing Loop Circuit for Model railway.
//*
//* Notes:
//*   This is free software: you can redistribute it and/or modify it
//*   under the terms of the GNU General Public License as published by
//*   the Free Software Foundation, either version 3 of the License, or
//*   (at your option) any later version.
//*
//*   It is distributed in the hope that it will be useful, but WITHOUT
//*   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
//*   FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
//*   for more details.
//*
//*   You should have received a copy of the GNU General Public License
//*   along with the product. If not, see <https://www.gnu.org/licenses/>.
//*
//* Credits:
//*   This design is based upon a solution created by InvertLogic
//*   <https://www.youtube.com/c/InvertLogic>.
//*
//*****************************************************************************

#include <Arduino.h>

// DCC-EX AutoReverser definitions
///////////////////////////////////////////////////////////////////////////////
#define DCCEX_AR_ACS712       A0    // Analog input pin for sensor
#define DCCEX_AR_TRIMPOT      A1    // Analog input pin for sensitivity
#define DCCEX_AR_VCCREF       A2    // Analog input pin for Arduino voltage
#define DCCEX_AR_TRIGGER      12    // Pin connected to NPN transistor 

#define DCCEX_AR_SAMPLERATE   19    // Analog sampling rate

// Serial communication pin configuration
///////////////////////////////////////////////////////////////////////////////
#define SERIAL_BAUD           9600

// ...
///////////////////////////////////////////////////////////////////////////////
bool
  _autoRefresh;

double
  _vcc,
  _sensitivity;

int
  _ACS712  = DCCEX_AR_ACS712,
  _trimpot = DCCEX_AR_TRIMPOT,
  _vccref  = DCCEX_AR_VCCREF,
  _trigger = DCCEX_AR_TRIGGER;

uint32_t trigger(int state = -1);
double getVCCRef(void);
double getSensitivity(void);

//*****************************************************************************
//*****************************************************************************
void setup()
{
  Serial.begin(SERIAL_BAUD);

  _autoRefresh = true;

  pinMode(_ACS712, INPUT);
  pinMode(_trimpot, INPUT);
  pinMode(_vccref, INPUT);
  pinMode(_trigger, OUTPUT);

  //=============================================================================
  // ...
  //-----------------------------------------------------------------------------
  getVCCRef();        _vcc = 5.0;
  getSensitivity();   _sensitivity = 0.185;

  trigger(1);
  delay(750);
  trigger(0);

  return;
}

//*****************************************************************************
//*****************************************************************************
void loop()
{
  static uint32_t
    lastTrigger = millis(),
    lastRefresh = millis();

  // Use _autoRefresh judiciously since it implements sample averaging.
  if (_autoRefresh && millis() - lastRefresh > 1500)
    {
    getVCCRef();
    getSensitivity();

    lastRefresh = millis();
    }
          //         +---  Read analog signal from current sensor
          //         |
          //         |                              +--- Remove offset
          //         |                              |
          //         |                              |           +--- Divide by sensor sensitivity
          //         |                              |           |
  double  //         v                              v           v
    ampere = (((analogRead(_ACS712) + 5) * _vcc) - 2.5) / _sensitivity;
  //
  //                                +--- Wait to avoid false triggers
  //                                |
  //                                v
  if (ampere > 1.0 && millis() - lastTrigger > 500)
    lastTrigger = trigger();  // Toggle output

  return;
}

//*****************************************************************************
//*
//* Brief:
//*   ???.
//*
//* Parameters:
//*   state - Trigger state to set.
//*
//* Returns:
//*   Last trigger time.
//*
//*****************************************************************************
uint32_t trigger(int state)
{
  Serial.println("trigger(" + String(state) + ")");

  digitalWrite(_trigger, state == -1 ? !digitalRead(_trigger) : state);

  return (millis());
}

//*****************************************************************************
//*
//* Brief:
//*   Class accessors.
//*
//* Parameters:
//*   Varies.
//*
//* Returns:
//*   Varies.
//*
//*****************************************************************************
double getVCCRef(void)
{
  Serial.println("getVCCRef()");

  if (_autoRefresh)
    {
    uint8_t
      indx = 0;

    for (_vcc = 0.0; indx < DCCEX_AR_SAMPLERATE; indx++)
      _vcc += analogRead(_vccref);

    _vcc = _vcc * 0.0048875855327468 / indx / 1024;
    }

  return (_vcc);
}
///////////////////////////////////////////////////////////////////////////////
double getSensitivity(void)
{
  Serial.println("getSensitivity()");

  if (_autoRefresh)
    {
    uint8_t
      indx = 0;

    for (_sensitivity = 0.0; indx < DCCEX_AR_SAMPLERATE; indx++)
      _sensitivity += analogRead(_trimpot);

    _sensitivity = _sensitivity / 1000 / indx;
    }

  return (_sensitivity);
}
