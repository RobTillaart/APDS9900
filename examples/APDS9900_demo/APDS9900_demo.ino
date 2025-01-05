//
//    FILE: APDS9900_demo.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: test basic behaviour and performance
//     URL: https://github.com/RobTillaart/APDS9900


#include "APDS9900.h"
#include "Wire.h"

APDS9900 mySensor;


void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println(__FILE__);
  Serial.print("APDS9900_LIB_VERSION: ");
  Serial.println(APDS9900_LIB_VERSION);

  Wire.begin();
  mySensor.begin();
  Serial.println(mySensor.getAddress());

}


void loop()
{
  //  mySensor.requestSensor();
  //  while (mySensor.requestReady() == false) delay(10);
  //  mySensor.readSensor();
  //  Serial.print(mySensor.getCO2Concentration());
  //  Serial.print("\t");
  //  Serial.print(mySensor.getTemperature());
  //  Serial.println();

  delay(2000);
}


//  -- END OF FILE --
