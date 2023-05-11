#include "Dht.h"
#include <pigpio.h>
#include <stdio.h>

constexpr uint8_t PORT = 4;

SensorData sensorData;
Dht dht(PORT, &sensorData);

int main(int argc, char** argv) {
   if (gpioInitialise()<0) return -1;
   printf("Revision: %u\n", gpioHardwareRevision());
   dht.begin();
   for (int i=0; i<10; i++) {
      printf("[%2d] Humidity: %.2f  Temperature %.2f\n", i, dht.readHumidity(), dht.readTemperature());
      gpioDelay(2000000);
}
   gpioTerminate();
}