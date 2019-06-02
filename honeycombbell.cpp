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
#include "honeycombbell.h"
#include "TouchMIDI_AVR_if.h"
#include "configuration.h"
#include "i2cdevice.h"

/*----------------------------------------------------------------------------*/
extern GlobalTimer gt;

/*----------------------------------------------------------------------------*/
void HoneycombBell::mainLoop( void )
{
  if ( gt.timer10msecEvent() ){
    //  LED Fade Out
    for ( int i=0; i<_MAX_LED; i++ ){
      if (( _fadeCounter[i] > 0 ) && ( _fadeCounter[i] < _FADE_TIME )){
        setNeoPixelFade( i, _fadeCounter[i] );
        _fadeCounter[i] -= 1;
      }
    }

    // only first board
    if (( _myNumber == 0) && ( _nextGlbTm < gt.timer10ms() )){
      nextBeat();
      midiClock(_beat<<4);
      _beat += 1;
      if ( _beat == _MAX_BEAT ){ _beat = 0;}
      _nextGlbTm += _tempo;
    }
  }

  if ( gt.timer100msecEvent() ){
    
  }
}
/*----------------------------------------------------------------------------*/
void HoneycombBell::rcvClock( uint8_t msg )
{
  int sentNum = msg & 0x0f;
  int beatNum = (msg & 0x70)>>4;

  _myNumber = sentNum + 1;
  _beat = beatNum;
  nextBeat();
  midiClock(_myNumber);
}
/*----------------------------------------------------------------------------*/
void HoneycombBell::checkTwelveTouch( int device )
{
  uint8_t swb[2] = {0};
  int err = 1;
  const int baseNum = device*_MAX_LED_PER_DEVICE;

  if ( device >= _MAX_DEVICE_NUM ){ return; }

#ifdef USE_CY8CMBR3110
  err = MBR3110_readTouchSw(swb,device);
#endif

  if ( err == 0 ){
    uint16_t sw = ((uint16_t)swb[0]) | ((uint16_t)swb[1]<<8);
    for ( int i=0; i<_MAX_LED_PER_DEVICE; i++ ){
      uint16_t  bitPtn = 0x0001 << i;
      if ( (_swState[device]&bitPtn)^(sw&bitPtn) ){
        if ( sw & bitPtn ){
          setMidiNoteOn( baseNum+i+(12*_octave), 0x7f );
          setNeoPixel(baseNum+i,TOUCH_ON );
        }
        else {
          setMidiNoteOff( baseNum+i+(12*_octave), 0x40 );
          setNeoPixel( baseNum+i,FADE_OUT );
        }
      }
    }
    _swState[device] = sw;
    //setAda88_Number((swState[0]<<6) | (swState[1]&0x3f));
  }
}

/*----------------------------------------------------------------------------*/
void HoneycombBell::nextBeat( void )
{
  int block = _beat*3;
  //  light on
  setNeoPixel( block, WHITE_ON );
  setNeoPixel( block+1, WHITE_ON );
  setNeoPixel( block+2, WHITE_ON );
  if ( block == 0 ){ block = 9;}
  else { block-=3; }
  //  light off
  setNeoPixel( block, LIGHT_OFF );
  setNeoPixel( block+1, LIGHT_OFF );
  setNeoPixel( block+2, LIGHT_OFF ); 
}
/*----------------------------------------------------------------------------*/
void HoneycombBell::setNeoPixelFade( uint8_t locate, int fadeTime )
{
  uint8_t red = static_cast<uint8_t>(((int)colorTbl(locate%16,0)*fadeTime)/_FADE_TIME);
  uint8_t blu = static_cast<uint8_t>(((int)colorTbl(locate%16,1)*fadeTime)/_FADE_TIME);
  uint8_t grn = static_cast<uint8_t>(((int)colorTbl(locate%16,2)*fadeTime)/_FADE_TIME);
  
  setLed(locate,red,blu,grn);
  lightLed();
}
/*----------------------------------------------------------------------------*/
void HoneycombBell::setNeoPixel( uint8_t locate, LED_STATE sw )
{
  if ( sw == TOUCH_ON ){
    setLed(locate,colorTbl(locate%16,0),colorTbl(locate%16,1),colorTbl(locate%16,2));
    _fadeCounter[locate] = _FADE_TIME;
  }
  else if ( sw == FADE_OUT ){
    if ( _fadeCounter[locate] > 0 ){
      _fadeCounter[locate] -= 1;
    }
  }
  if ( _fadeCounter[locate] == 0 ){
    if ( sw == LIGHT_OFF ){
      setLed(locate,0,0,0);
    }
    else if ( sw == WHITE_ON ){
      setLed(locate,100,100,100);
    }
  }
  lightLed();
}

