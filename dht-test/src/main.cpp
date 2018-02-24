#include "mbed.h"
#include <math.h>
#include "DHT.h"

Serial pc(USBTX, USBRX); // tx, rx

DigitalOut led1(LED1);
#define NUMPINS 5

AnalogIn temp0(A0);
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

time_t curTime = 0;




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
  pc.printf("Thermistor %d resistance %f\r\n", id, average);

  double steinhart;
  steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to C

  pc.printf("Temperature %d: %f*C\r\n",id, steinhart);

}

void readAll() {
    double values[NUMPINS];

    for (int j=0; j< NUMPINS; j++) {
        values[j] = 0;
    }

    // take N samples in a row, with a slight delay
    for (int i=0; i< NUMSAMPLES; i++) {
        for (int j=0; j< NUMPINS; j++) {
            values[j] += inputs[j]->read();
        }
        wait_ms(500);
     //delay(10);
    }

    for (int j=0; j< NUMPINS; j++) {
        convertTemp(j, (values[j] / NUMSAMPLES) * 1024);
    }
}

void loop() {
  pc.printf("****************\r\n");
  double raw = temp1;
  double temp;
  pc.printf("Raw = %f\r\n", raw);
  //convertTemp(temp0 * 1024);
  //convertTemp(temp1 * 1024);
  //convertTemp(temp2 * 1024);
  //convertTemp(temp3 * 1024);
  //convertTemp(temp4 * 1024);
  //convertTemp(temp5 * 1024);

  readAll();

//  pc.printf("read data result: %d\r\n", dht1.readData());
//  pc.printf("DHT temp: %f\r\n", dht1.ReadTemperature(CELCIUS));
//  pc.printf("DHT hum: %f\r\n", dht1.ReadHumidity());
}

time_t read_rtc(void) {
  curTime += 10;
    return curTime;
}

int d4State = 0;
void buttonStuff() {
    pc.printf("buttonStuff\r\n");
    d4.write(0);
    d4State = 0;
    while (true) {
        int blueButtonState = blueButton.read();
        if (blueButtonState != d4State) {
            d4State = blueButtonState;//(blueButtonState == 0 ? 1 : 0);
            pc.printf("Setting d4 to %d (%d)\r\n", d4State, blueButtonState);
            d4.write(d4State);
        }
    }
}


// main() runs in its own thread in the OS
// (note the calls to Thread::wait below for delays)
int main() {

  pc.printf("\r\nStarting 2!\r\n");
  attach_rtc(&read_rtc, NULL, NULL, NULL);

  //buttonStuff();
    //  set_time(1256729737);  // Set RTC time to Wed, 28 Oct 2009 11:35:37
    //set_time(1387188323); // Set RTC time to 16 December 2013 10:05:23 UTC

    // connect AREF to 3.3V and use that as VCC, less noisy!
  //    analogReference(EXTERNAL);



  //time_t currentTime = time(NULL);
  //pc.printf("Time ok!\r\n");
  inputs[0] = &temp0;
  inputs[1] = &temp1;
  inputs[2] = &temp2;
  inputs[3] = &temp4;
  inputs[4] = &temp5;

    while (true) {
      wait(3);
      led1 = !led1;
      loop();

    }
}
