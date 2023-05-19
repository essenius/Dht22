#include "ClimateMeasurement.h"
#include "Config.h"
#include "Dht.h"
#include "Mqtt.h"
#include <pigpio.h>
#include <stdio.h>
#include <signal.h>

bool keepGoing = true;

void signalHandler(sig_atomic_t s){
   keepGoing = false;
}

int main(int argc, char** argv) {
   if (gpioInitialise()<0) return -1;
   signal(SIGINT,signalHandler);
   SensorData sensorData;
   Config config;
   config.begin("/home/pi/.config/dht.conf");
   Dht dht(&sensorData, &config);
   Mqtt mqtt(&config);
   if (!mqtt.begin()) return -1;
   printf("Revision: %u\n", gpioHardwareRevision());
   dht.begin();
   printf("Waiting to connect\n");
   while(!mqtt.isConnected()) { 
      sleep(0.1); 
   }
   printf("Connected\n"); 

   ClimateMeasurement climateMeasurement(&mqtt);
   printf("Starting Main loop\n");
   while (keepGoing) {
      if (!mqtt.isConnected()) {
         printf("Connection lost. Reconnecting\n");
         mqtt.reconnect();
         while(!mqtt.isConnected()) { sleep(0.1); }
      }
      dht.waitForNextMeasurement();      
      auto temperature = dht.readTemperature();
      auto humidity = dht.readHumidity();
      climateMeasurement.processSample(temperature, humidity);
   }
   printf("Shutting down\n");
   dht.shutdown();
   mqtt.shutdown();
}
