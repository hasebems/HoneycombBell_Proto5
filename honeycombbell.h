/* ========================================
 *
 *  TouchMIDI Common Platform for AVR
 *  honeycombbell.h
 *    description: HoneycombBell 
 *
 *  Copyright(c)2019- Masahiko Hasebe at Kigakudoh
 *  This software is released under the MIT License, see LICENSE.txt
 *
 * ========================================
 */
#ifndef HONEYCOMBBELL_H
#define HONEYCOMBBELL_H

#include <Arduino.h>

class HoneycombBell {

  enum LED_STATE {
    LIGHT_OFF,
    WHITE_ON,
    TOUCH_ON,
    FADE_OUT
  };

public:
  HoneycombBell( void ) : _swState(), _octave(0),
                          _tempo(20), _nextGlbTm(20), _beat(0),
                          _setNumber(0), _myNumber(0), _fadeCounter() {}

  void mainLoop( void );
  void periodic100msec( void ){}
  void rcvClock( uint8_t msg );
  void checkTwelveTouch( int device );

  void setOctave( int oct ){ _octave = oct;}
  void setSetNumber( int num ){ _setNumber = num;}
  void setMyNumber( int num ){ _myNumber = num;}

private:
  void nextBeat( void );
  void setNeoPixelFade( uint8_t locate, int fadeTime );
  void setNeoPixel( uint8_t locate, LED_STATE sw );


  static const int _MAX_BEAT = 4;
  static const int _MAX_LED_PER_DEVICE = 6;
  static const int _MAX_DEVICE_NUM = 2;
  static const int _MAX_LED = _MAX_LED_PER_DEVICE*_MAX_DEVICE_NUM;
  static const int _FADE_TIME = 20;

  uint16_t  _swState[2];
  int       _octave;
  int       _tempo;   //  times*10msec  50:120bpm, 20:300bpm
  uint32_t  _nextGlbTm;

  int       _beat;    //  0 - 3
  int       _setNumber; //  0 - 7
  int       _myNumber;  //  0 - 15

  int       _fadeCounter[_MAX_LED];   // 0:LIGHT_OFF, _FADE_TIME:TOUCH_ON

};
#endif
