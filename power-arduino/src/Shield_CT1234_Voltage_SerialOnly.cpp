
#include "EmonLib.h"
#include "ADS1115.h"

//tussen 400 en 700
#define ADS0_A0 1
#define ADS0_A1 2
#define ADS0_A2 3
#define ADS0_A3 4
#define ADS1_A0 5

// Create  instances for each CT channel
EnergyMonitor ct1,ct2,ct3, ct4;

// On-board emonTx LED
const int LEDpin = 9;

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
  //set to max rate
  adc.setRate(ADS1115_RATE_860);
}

// Make a callback method for reading the pin value from the ADS instance
int ads1115PinReader(int addr){
  int16_t tmp;
  switch (addr) {
    case ADS0_A0: tmp = adc0.getConversionP0GND(); break;
    case ADS0_A1: tmp = adc0.getConversionP1GND(); break;
    case ADS0_A2: tmp = adc0.getConversionP2GND(); break;
    case ADS0_A3: tmp = adc0.getConversionP3GND(); break;
    case ADS1_A0: tmp = adc1.getConversionP0GND(); break;
  }
  //tmp = analogRead(addr);
  //Serial.print(tmp); Serial.print(" -- "); Serial.println(tmp>>6);
  //Serial.print(addr); Serial.print(": "); Serial.println(tmp*ADS1115_MV_6P144 / 5);

  //calculation:
  // tmp * ADS1115_MV_6P144 returns a voltage measurement between 0 and 5v
  // emonlib expect a value between 0 an 1024, so convert
  // TODO: we're loosing precision here as it's converted to a 10 bit value. fix this.
  return (tmp * ADS1115_MV_6P144 * 1024) / 5000;
  //return tmp*ADS1115_MV_6P144;
  //return tmp*ADS1115_MV_1P024;

}

void setup()
{
  Wire.begin();
  //Serial.begin(115200); // initialize serial communication
  Serial.begin(9600); // initialize serial communication
  initAdc(adc0);
  initAdc(adc1);
  // while (!Serial) {}
  // wait for serial port to connect. Needed for Leonardo only

  Serial.println("emonTX Shield reading");

  ct1.inputPinReader = ads1115PinReader; // Replace the default pin reader with the customized ads pin reader
  ct2.inputPinReader = ads1115PinReader;
  ct3.inputPinReader = ads1115PinReader;

  // Calibration factor = CT ratio / burden resistance = (100A / 0.05A) / 33 Ohms = 60.606
  ct1.current(ADS0_A0, 59.606);
  ct2.current(ADS0_A1, 59.606);
  ct3.current(ADS0_A2, 59.606);
  //ct4.current(4, 60.606);

  // (ADC input, calibration, phase_shift)
  ct1.voltage(ADS1_A0, 240, 1.2);
  ct2.voltage(ADS1_A0, 240, 1.2);
  ct3.voltage(ADS1_A0, 240, 1.2);
  //ct1.voltage(0, 300.6, 1.7);
  //ct2.voltage(0, 300.6, 1.7);
  //ct3.voltage(0, 300.6, 1.7);
  //ct4.voltage(0, 300.6, 1.7);

  // Setup indicator LED
  pinMode(LEDpin, OUTPUT);
  digitalWrite(LEDpin, HIGH);
}

void loop()
{
  // Calculate all. No.of crossings, time-out
  //ct4.calcVI(20,2000);

  // Print power
  Serial.println("READ BEGIN");
  unsigned long start = millis();

  ct1.calcVI(50,2000);
  Serial.print("ct1: ");
  Serial.println(ct1.realPower);
//  Serial.print(ct1.Irms);
//  Serial.print(" ");
//  Serial.print(ct1.apparentPower);
//  Serial.print(" ");

  ct2.calcVI(50,2000);
  Serial.print("ct2: ");
  Serial.println(ct2.realPower);
//  Serial.print(ct2.Irms);
//  Serial.print(" ");
//  Serial.print(ct2.apparentPower);
//  Serial.print(" ");

  ct3.calcVI(50,2000);
  Serial.print("ct3: ");
  Serial.println(ct3.realPower);

//  Serial.print(ct3.Irms);
//  Serial.print(" ");
//  Serial.print(ct3.apparentPower);
//  Serial.print(" ");

  Serial.print("Vrms");
  Serial.println(ct1.Vrms);

  Serial.print("took ");Serial.print(millis() - start);Serial.println("ms");
  Serial.println("READ END");

  // Available properties: ct1.realPower, ct1.apparentPower, ct1.powerFactor, ct1.Irms and ct1.Vrms

  delay(2000);
}
