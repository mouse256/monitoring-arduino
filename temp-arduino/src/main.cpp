#include "Arduino.h"
#include <math.h>

//Serial pc(USBTX, USBRX); // tx, rx

//DigitalOut led1(LED1);
#define NUMPINS 5
#define ON 1
#define OFF 0
int inputs[NUMPINS];
double temperatures[NUMPINS];
int pinHotWaterState = 22;
int pinPumpFloorState = 24;
int pinPumpWallState = 26;
int pinRelayHeatpump = 40;
int countRelay;
int relayState = OFF;
#define ID_TEMP_CV 3
#define TEMP_CV_OFF 30
#define TEMP_CV_ON 28

/*AnalogIn temp0(A0);
AnalogIn temp1(A1);
AnalogIn temp2(A2);
//AnalogIn temp3(A3);
AnalogIn temp4(A4);
AnalogIn temp5(A5);
AnalogIn* inputs[NUMPINS];
DigitalIn temphum(D2);
DigitalIn blueButton(BUTTON1);
DigitalOut d4(D4);
DHT dht1(D2, DHT22, &pc);

time_t curTime = 0;*/




// resistance at 25 degrees C
#define THERMISTORNOMINAL 9700
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 15
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950
// the value of the 'other' resistor
#define SERIESRESISTOR 10000


void convertTemp(int id, double average) {

  // convert the value to resistance
  average = 1023 / average - 1;
  average = SERIESRESISTOR / average;
  Serial.print("Thermistor ");
  Serial.print(id);
  Serial.print(" resistance: ");
  Serial.println(average, 2);

  double steinhart;
  steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to C

  Serial.print("Temperature ");
  Serial.print(id);
  Serial.print(": ");
  Serial.print(steinhart, 2);
  temperatures[id] = steinhart;
  Serial.println("*C");

}

void readAllAnalog() {
    int values[NUMPINS];

    for (int j=0; j< NUMPINS; j++) {
        values[j] = 0;
    }

    // take N samples in a row, with a slight delay
    for (int i=0; i< NUMSAMPLES; i++) {
        for (int j=0; j< NUMPINS; j++) {
            values[j] += analogRead(inputs[j]);
        }
        delay(500);
    }

    for (int j=0; j< NUMPINS; j++) {
        double raw = (values[j] / NUMSAMPLES);
        convertTemp(j, raw);
    }
}

void readAllDigital() {
  int sensorVal = digitalRead(pinHotWaterState);
  Serial.print("State HotWater: ");
  Serial.println((sensorVal == 0 ? 1 : 0));
  
  sensorVal = digitalRead(pinPumpFloorState);
  Serial.print("State PumpFloor: ");
  Serial.println((sensorVal == 0 ? 1 : 0));
  
  sensorVal = digitalRead(pinPumpWallState);
  Serial.print("State PumpWall: ");
  Serial.println((sensorVal == 0 ? 1 : 0));
}

void setRelays() {
  if (relayState == OFF) {
    if (temperatures[ID_TEMP_CV] < TEMP_CV_ON) {
      countRelay++;
    }
    if (countRelay > 5) {
      countRelay = 0;
      relayState = ON;
      digitalWrite(pinRelayHeatpump, LOW);
    }
    Serial.print("State HeatpumpCV: 0");
  }
  else {
    if (temperatures[ID_TEMP_CV] > TEMP_CV_OFF) {
      countRelay++;
    }
    if (countRelay > 5) {
      countRelay = 0;
      relayState = OFF;
      digitalWrite(pinRelayHeatpump, HIGH);
    }
    Serial.print("State HeatpumpCV: 1");
  }
}

void readAll() {
  Serial.println("READ BEGIN");
  readAllAnalog();
  readAllDigital();
  setRelays();
  Serial.println("READ END");
}

void setup() {
  Serial.begin(9600);

  // connect AREF to 3.3V and use that as VCC, less noisy!
  analogReference(EXTERNAL);

  pinMode(pinHotWaterState, INPUT_PULLUP);
  pinMode(pinPumpFloorState, INPUT_PULLUP);
  pinMode(pinPumpWallState, INPUT_PULLUP);
  pinMode(pinRelayHeatpump, OUTPUT);

}



void loop() {
  Serial.println("Starting 4 !!");
  delay(1000);
  inputs[0] = A0;
  inputs[1] = A1;
  inputs[2] = A2;
  inputs[3] = A3;
  inputs[4] = A4;
  countRelay = 0;

  digitalWrite(pinRelayHeatpump, HIGH); //relays have inverse logic

  while(true) {
    Serial.println("*********");
    readAll();
  }
}
