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
#include <avr/pgmspace.h>

/*----------------------------------------------------------------------------*/
extern GlobalTimer gt;

/*----------------------------------------------------------------------------*/
HoneycombBell::HoneycombBell( void ) : _swState(), _octave(0),
                          _tempo(20), _nextGlbTm(20), _beat(0),
                          _setNumber(0), _myConnectionNumber(0)
{
  for ( int i=0; i<_MAX_LED; ++i ){ _led[i].setLocate(i);}
}
/*----------------------------------------------------------------------------*/
void HoneycombBell::mainLoop( void )
{
  if ( gt.timer10msecEvent() ){
    //  LED Fade Out
    for ( int i=0; i<_MAX_LED; i++ ){
      _led[i].checkFade();
    }

    // only first board
    if (( _myConnectionNumber == 0) && ( _nextGlbTm < gt.timer10ms() )){
      displayNextBeat();
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
const uint8_t OctTable[8][16] PROGMEM = {
  { 5,4,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0},  
  { 5,4,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0},  //  Set Number:1
  { 4,5,5,4, 0,0,0,0, 0,0,0,0, 0,0,0,0},  //  :2
  { 3,4,5,5, 4,3,0,0, 0,0,0,0, 0,0,0,0},  //  :3
  { 5,4,3,3, 4,5,5,4, 3,0,0,0, 0,0,0,0},  //  :4
  { 6,5,4,3, 3,4,5,6, 6,5,4,3, 0,0,0,0},  //  :5
  { 5,4,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0},  //  :6
  { 5,4,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0}   //  
};
void HoneycombBell::decideOctave( void )
{
  _octave = pgm_read_byte_near(OctTable[_setNumber] + _myConnectionNumber);
}
/*----------------------------------------------------------------------------*/
//  This function is called when MIDI CC#10h comes to this board.
void HoneycombBell::rcvClock( uint8_t msg )
{
  int sentNum = msg & 0x0f;
  int beatNum = (msg & 0x70)>>4;

  if ( _myConnectionNumber != sentNum + 1 ){
    _myConnectionNumber = sentNum + 1;
    decideOctave();
  }
  _beat = beatNum;
  displayNextBeat();
  midiClock(_myConnectionNumber);
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
          _led[baseNum+i].setNeoPixel( TOUCH_ON );
        }
        else {
          setMidiNoteOff( baseNum+i+(12*_octave), 0x40 );
          _led[baseNum+i].setNeoPixel( FADE_OUT );
        }
      }
    }
    _swState[device] = sw;
    //setAda88_Number((swState[0]<<6) | (swState[1]&0x3f));
  }
}

/*----------------------------------------------------------------------------*/
const uint8_t EachLedPattern[2][_MAX_LED] PROGMEM = {
  //  Value means bit0:beat0, bit1:beat1 .... bit7:beat7
  { 0x80, 0x80, 0x80, 0x40, 0x40, 0x40, 0x20, 0x20, 0x20, 0x10, 0x10, 0x10 },
  { 0x08, 0x08, 0x08, 0x04, 0x04, 0x04, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01 }
};
const uint8_t LedWaitPtnNumber[8][16] PROGMEM = {
// Select Each Led Pattern above EachLedPattern[x]
// 1st2nd3rd...(block)
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //  not use
  { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //  setNumber = 1
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //  setNumber = 2
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //  setNumber = 3
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //  setNumber = 4
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //  setNumber = 5
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, //  setNumber = 6
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }  //  not use
};
/*----------------------------------------------------------------------------*/
void HoneycombBell::displayNextBeat( void )
{
  const uint8_t beatBit = 0x80 >> _beat;
  const uint8_t waitPtn = pgm_read_byte_near(LedWaitPtnNumber[_setNumber]+_myConnectionNumber);
  
  for ( int i=0; i<_MAX_LED; i++ ){
    const uint8_t ptn = pgm_read_byte_near(EachLedPattern[waitPtn]+i);
    if ( ptn & beatBit ){
      _led[i].setNeoPixel( WHITE_ON );
    }
    else {
      _led[i].setNeoPixel( LIGHT_OFF );      
    }
  }
}


/*----------------------------------------------------------------------------*/
void EachLed::checkFade( void )
{
  if (( _fadeCounter > 0 ) && ( _fadeCounter < _FADE_TIME )){
    _fadeCounter -= 1;

    const uint8_t red = static_cast<uint8_t>(((int)colorTbl(_myLocate%16,0)*_fadeCounter)/_FADE_TIME);
    const uint8_t blu = static_cast<uint8_t>(((int)colorTbl(_myLocate%16,1)*_fadeCounter)/_FADE_TIME);
    const uint8_t grn = static_cast<uint8_t>(((int)colorTbl(_myLocate%16,2)*_fadeCounter)/_FADE_TIME);
  
    setLed(_myLocate,red,blu,grn);
    lightLed();
  }   
}
/*----------------------------------------------------------------------------*/
void EachLed::setNeoPixel( LED_STATE sw )
{
  if ( sw == TOUCH_ON ){
    setLed(_myLocate,colorTbl(_myLocate%16,0),colorTbl(_myLocate%16,1),colorTbl(_myLocate%16,2));
    _fadeCounter = _FADE_TIME;
  }
  else if ( sw == FADE_OUT ){
    if ( _fadeCounter > 0 ){
      _fadeCounter -= 1;
    }
  }
  if ( _fadeCounter == 0 ){
    if ( sw == LIGHT_OFF ){
      setLed(_myLocate,0,0,0);
    }
    else if ( sw == WHITE_ON ){
      setLed(_myLocate,100,100,100);
    }
  }
  lightLed();
}

