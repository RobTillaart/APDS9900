//
//    FILE: APDS9900.cpp
//  AUTHOR: Rob Tillaart
//    DATE: 2024-12-09
// VERSION: 0.1.0
// PURPOSE: Arduino library for the APDS9900 environment sensor.
//     URL: https://github.com/RobTillaart/APDS9900



#include "APDS9900.h"


//
//  REGISTERS
//
#define APDS9900_ENABLE           0x00
#define APDS9900_ATIME            0x01
#define APDS9900_PTIME            0x02
#define APDS9900_WTIME            0x03
#define APDS9900_AILTL            0x04
#define APDS9900_AILTH            0x05
#define APDS9900_AIHTL            0x06
#define APDS9900_AIHTH            0x07
#define APDS9900_PILTL            0x08
#define APDS9900_PILTH            0x09
#define APDS9900_PIHTL            0x0A
#define APDS9900_PIHTH            0x0B
#define APDS9900_PERS             0x0C
#define APDS9900_CONFIG           0x0D
#define APDS9900_PPCOUNT          0x0E
#define APDS9900_CONTROL          0x0F
//                                0x10
#define APDS9900_REV              0x11
#define APDS9900_ID               0x12
#define APDS9900_STATUS           0x13
#define APDS9900_CDATAL           0x14
#define APDS9900_CDATAH           0x15
#define APDS9900_IRDATAL          0x16
#define APDS9900_IRDATAH          0x17
#define APDS9900_PDATAL           0x18
#define APDS9900_PDATAH           0x19



APDS9900::APDS9900(TwoWire *wire)
{
  _wire = wire;
  _error = 0;
}


bool APDS9900::begin()
{
  if (! isConnected())
  {
    return false;
  }
  //  COMMAND P18
  //  could be made more generic?
  uint8_t cmd = 0x80;  //  address the COMMAND register P18
  cmd |= 0x20;         //  auto increment protocol (for 16 bit values)
                       //  no interrupt clear flags
  writeRegister(cmd);
  return true;
}


bool APDS9900::isConnected()
{
  _wire->beginTransmission(_address);
  return (_wire->endTransmission() == 0);
}


uint8_t APDS9900::getAddress()
{
  return _address;
}


//  ENABLE REGISTER P18
void APDS9900::wakeUp()
{
  //  read back to keep all bits set
  uint8_t value = readRegister(APDS9900_ENABLE);
  //  only write when needed.
  if ((value & 0x01) == 0x00)
  {
    value |= 0x01;  //  set PON bit  PON = POWER ON
    writeRegister(APDS9900_ENABLE, value);
  }
}


void APDS9900::sleep()
{
  //  read back to keep all bits set
  uint8_t value = readRegister(APDS9900_ENABLE);
  value &= ~0x01;  //  clear PON bit  PON = POWER ON
  writeRegister(APDS9900_ENABLE, value);
}


//  ATIME P19
void APDS9900::setIntegrationTime(uint16_t milliseconds)
{
  if (milliseconds < 3) milliseconds = 3;
  uint16_t ATIME = round(milliseconds * 0.367647);  //  magic nr.  1/2.72 = 0,3676470588235294...
  if (ATIME > 255) ATIME = 255;
  ATIME = 256 - ATIME;
  writeRegister(APDS9900_ATIME, ATIME);
}


uint16_t APDS9900::getIntegrationTime()
{
  uint8_t value = readRegister(APDS9900_ATIME);
  uint16_t ms = round((256 - value) * 2.72);
  return ms;
}


//  PTIME P19
void APDS9900::setProximityTime(uint16_t milliseconds)
{
  if (milliseconds < 3) milliseconds = 3;
  uint16_t PTIME = round(milliseconds * 0.367647);  //  magic nr.  1/2.72 = 0,3676470588235294...
  if (PTIME > 255) PTIME = 255;
  PTIME = 256 - PTIME;
  writeRegister(APDS9900_PTIME, PTIME);
}


uint16_t APDS9900::getProximityTime()
{
  uint8_t value = readRegister(APDS9900_PTIME);
  uint16_t ms = round((256 - value) * 2.72);
  return ms;
}


//  WTIME P19, WLONG P21
void APDS9900::setWaitTime(uint16_t milliseconds)
{
  bool WLONG = (milliseconds > 696);
  //  first set WLONG register
  writeRegister(APDS9900_CONFIG, WLONG ? 0x02 : 0x00);

  uint16_t WTIME = 0;
  if (WLONG)
  {
    WTIME = round(milliseconds * 0.03125);  //  magic nr.  1/32 = 0,03125  (could be a >> 5.)
  }
  else
  {
    if (milliseconds < 3) milliseconds = 3;
    WTIME = round(milliseconds * 0.367647);  //  magic nr.  1/2.72 = 0,3676470588235294...
  }
  if (WTIME > 255) WTIME = 255;
  WTIME = 256 - WTIME;
  writeRegister(APDS9900_WTIME, WTIME);
}


uint16_t APDS9900::getWaitTime()
{
  bool WLONG = ((readRegister(APDS9900_CONFIG) & 0x02) == 0x02);
  uint8_t value = readRegister(APDS9900_WTIME);

  if (WLONG) return round((256 - value) * 32);
  return round((256 - value) * 2.72);
}


//////////////////////////////////////////////////////////////////////////


bool APDS9900::setALSThresholds(uint16_t lowTH, uint16_t highTH)
{
  if (lowTH >= highTH) return false;
  writeRegister16(APDS9900_AILTL, lowTH);
  writeRegister16(APDS9900_AIHTL, highTH);
  return true;
}


bool APDS9900::setPROXThresholds(uint16_t lowTH, uint16_t highTH)
{
  if (lowTH >= highTH) return false;
  writeRegister16(APDS9900_PILTL, lowTH);
  writeRegister16(APDS9900_PIHTL, highTH);
  return true;
}


//  PPERS P20
bool APDS9900::setALSInterruptPersistence(uint8_t value)
{
  if (value > 0x0F) return false;
  uint8_t val = readRegister(APDS9900_PERS);
  if ((val & 0x0F) != value)
  {
    val &= 0xF0;
    val |= value;
    writeRegister(APDS9900_PERS, val);
  }
  return true;
}


bool APDS9900::setPROXInterruptPersistence(uint8_t value)
{
  if (value > 0x0F) return false;
  uint8_t val = readRegister(APDS9900_PERS);
  uint8_t tmp = value << 4;
  if ((val & 0xF0) != tmp)
  {
    val &= 0x0F;
    val |= tmp;
    writeRegister(APDS9900_PERS, val);
  }
  return true;
}


//  PPCOUNT P21
void APDS9900::setProximityPulseCount(uint8_t value)
{
  writeRegister(APDS9900_PPCOUNT, value);
}


//  PDRIVE  P22
bool APDS9900::setLedDriveStrength(uint8_t value)
{
  if (value > 3) return false;
  uint8_t val = readRegister(APDS9900_CONTROL);
  val &= 0x3F;
  val |= (value << 6);
  writeRegister(APDS9900_CONTROL, val);
  return true;
}


//  PDIODE  P22
bool APDS9900::setProximityDiodeSelect(uint8_t channel)
{
  if (channel > 1) return false;
  uint8_t val = readRegister(APDS9900_CONTROL);
  val &= 0x3F;
  val |= (channel << 6);
  writeRegister(APDS9900_CONTROL, val);
  return true;
}


//  PGAIN  P22
//  not implemented as it is a constant value 0 

//  AGAIN  P22
bool APDS9900::setALSGainControl(uint8_t value)
{
  if (value > 3) return false;
  uint8_t val = readRegister(APDS9900_CONTROL);
  val &= 0xFC;
  val |= value;
  writeRegister(APDS9900_CONTROL, val);
  return true;
}


//  REV  P22
uint8_t APDS9900::getRevision()
{
  return readRegister(APDS9900_REV);
}


//  ID  P23
uint8_t APDS9900::getDeviceID()
{
  return readRegister(APDS9900_ID);
}


uint8_t APDS9900::getStatus()
{
  return readRegister(APDS9900_STATUS);
}

uint16_t APDS9900::getALS_CDATA()
{
  return readRegister16(APDS9900_CDATAL);
}

uint16_t APDS9900::getALS_IRDATA()
{
  return readRegister16(APDS9900_IRDATAL);
}

uint16_t APDS9900::getPROX_DATA()
{
  return readRegister16(APDS9900_PDATAL);
}


//////////////////////////////////////////////////////////////////////////


int APDS9900::getLastError()
{
  int e = _error;
  _error = 0;
  return e;
}


//////////////////////////////////////////////////////////////////////////
//
//  PROTECTED
//

int APDS9900::writeRegister(uint8_t reg)
{
  _wire->beginTransmission(_address);
  _wire->write(reg);
  _error = _wire->endTransmission();
  return _error;
}


int APDS9900::writeRegister(uint8_t reg, uint8_t value)
{
  _wire->beginTransmission(_address);
  _wire->write(reg);
  _wire->write(value);
  _error = _wire->endTransmission();
  return _error;
}


int APDS9900::writeRegister16(uint8_t reg, uint16_t value)
{
  _wire->beginTransmission(_address);
  _wire->write(reg);
  _wire->write(value & 0xFF);
  _wire->write(value >> 8);
  _error = _wire->endTransmission();
  return _error;
}


uint8_t APDS9900::readRegister(uint8_t reg)
{
  _wire->beginTransmission(_address);
  _wire->write(reg);
  _error = _wire->endTransmission();
  uint8_t bytes = _wire->requestFrom(_address, (uint8_t)1);
  if (bytes == 1)
  {
    return _wire->read();
  }
  //  ERROR HANDLING
  return 0;
}


uint16_t APDS9900::readRegister16(uint8_t reg)
{
  _wire->beginTransmission(_address);
  _wire->write(reg);
  _error = _wire->endTransmission();
  uint8_t bytes = _wire->requestFrom(_address, (uint8_t)2);
  if (bytes == 2)
  {
    uint16_t value = _wire->read();
    value += (_wire->read() * 256);
    return value;
  }
  //  ERROR HANDLING
  return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//  DERIVED
//
APDS9901::APDS9901(TwoWire * wire) : APDS9900(wire)
{
  //  no specific code yet
}


//  -- END OF FILE --

