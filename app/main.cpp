#include "ClimateMeasurement.h"
#include "Config.h"
#include "Dht.h"
#include "Mqtt.h"
#include "Homie.h"
#include <pigpio.h>
#include <stdio.h>
#include <signal.h>

bool keepGoing = true;

void signalHandler(sig_atomic_t s){
   keepGoing = false;
}

int main(int argc, char** argv) {
   //gpioCfgSetInternals(1<<10); // turn off signal handling
   if (gpioInitialise()<0) return -1;
   printf("Initialized pigpio\n");
   signal(SIGINT,signalHandler);
   SensorData sensorData;
   Config config;
   config.begin("/home/pi/.config/dht.conf");
   printf("Config began, device=%s\n", config.getEntry("device", "unknown"));
   Dht dht(&sensorData, &config);
   printf("Dht declared\n");
   Mqtt mqtt(&config);
   printf("MQTT defined\n");
   Homie homie(&mqtt, &config);
   printf("Declared objects\n");
   if (!homie.begin()) return -1;
   printf("Revision: %u\n", gpioHardwareRevision());
   dht.begin();
   printf("Waiting to connect\n");
   if (!mqtt.waitForConnection()) return -2;
   if (!homie.sendMetadata()) return -3;
   printf("Connected\n"); 
   ClimateMeasurement climateMeasurement(&homie);
   printf("Starting Main loop\n");
   while (keepGoing) {
      // ensure we don't reset it if break was pressed
      keepGoing &= mqtt.verifyConnection();
      dht.waitForNextMeasurement();      
      auto temperature = dht.readTemperature();
      auto humidity = dht.readHumidity();
      climateMeasurement.processSample(temperature, humidity);
   }
   printf("Shutting down\n");
   dht.shutdown();
   homie.shutdown();
}
