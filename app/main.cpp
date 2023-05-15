#include "Config.h"
#include "Dht.h"
#include "Mqtt.h"
#include <pigpio.h>
#include <stdio.h>

int main(int argc, char** argv) {
   if (gpioInitialise()<0) return -1;
   {
      SensorData sensorData;
      // using default power pin = 17, data pin = 4
      Dht dht(&sensorData);
      Config config;
      config.begin("/home/pi/.config/dht.conf");
      Mqtt mqtt(&config);
      if (!mqtt.begin()) return -1;
      printf("Revision: %u\n", gpioHardwareRevision());
      dht.begin();
      while(!mqtt.isConnected()) {}
      // impossible values so the readings will be different
      float previousHumidity = -10.0f;
      float previousTemperature = -300.0f;
      int measureCount = 0;
      printf("Starting loop\n");
      for (int i = 0; i < 30; i++) {
         measureCount++;
         auto humidity = dht.readHumidity();
         auto temperature = dht.readTemperature();
         if (abs(humidity - previousHumidity) > 0.1 || measureCount >= 10) {
            mqtt.sendFloat("humidity", humidity);
            previousHumidity = humidity;
         }
         if (abs(temperature - previousTemperature) > 0.1 || measureCount >= 10) {
            mqtt.sendFloat("temperature", temperature);
            previousTemperature = temperature;
         }
         if (measureCount >= 10) {
            measureCount = 0;
         }
         printf("[%2d] Humidity: %.2f  Temperature %.2f\n", measureCount, humidity, temperature);
         gpioDelay(2000000);
      }
      mqtt.disconnect();
   }
   gpioTerminate();
}
