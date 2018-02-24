
#include "ADS1115.h"

ADS1115 adc0(ADS1115_ADDRESS_ADDR_GND);
ADS1115 adc1(ADS1115_ADDRESS_ADDR_VDD);

void initAdc(ADS1115& adc) {
  Serial.println("Testing ADC connection...");
  Serial.println(adc.testConnection() ? "ADS1115 connection successful" : "ADS1115 connection failed");

  adc.initialize(); // initialize ADS1115 16 bit A/D chip
  //continuous mode. Data will always be ready
  adc.setMode(ADS1115_MODE_CONTINUOUS);
  //set the gain
  adc.setGain(ADS1115_PGA_6P144);
  Serial.print("Rate before: "); Serial.println(adc.getRate());
  adc.setRate(ADS1115_RATE_860);
  Serial.print("Rate after: "); Serial.println(adc.getRate());
}

void setup() {
    //I2Cdev::begin();  // join I2C bus
    Wire.begin();
    Serial.begin(115200); // initialize serial communication
    initAdc(adc0);
    initAdc(adc1);
}

float getMv(ADS1115& adc, int output) {
  int16_t tmp;
  switch (output) {
    case 0: tmp = adc.getConversionP0GND(); break;
    case 1: tmp = adc.getConversionP1GND(); break;
    case 2: tmp = adc.getConversionP2GND(); break;
    case 3: tmp = adc.getConversionP3GND(); break;
  }
  return tmp * ADS1115_MV_6P144;
}


void loop() {

    Serial.print("0_A0: "); Serial.print(getMv(adc0, 0)); Serial.print("mV\t");
    //Serial.print("0_A1: "); Serial.print(getMv(adc0, 1)); Serial.print("mV\t");
    //Serial.print("0_A2: "); Serial.print(getMv(adc0, 2)); Serial.print("mV\t");
    //Serial.print("0_A3: "); Serial.print(getMv(adc0, 3)); Serial.print("mV\t");
    Serial.print("1_A2: "); Serial.print(getMv(adc1, 2)); Serial.print("mV\t");

    Serial.println("");
    delay(500);
}
