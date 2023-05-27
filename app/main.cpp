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

int mainHelper(const char* configFile = "/home/pi/.config/dht.conf") {
   SensorData sensorData;
   Config config;
   config.begin(configFile);
   printf("Config began, device=%s\n", config.getEntry("device", "unknown"));
   Mqtt mqtt(&config);
   printf("MQTT defined\n");
   Homie homie(&mqtt, &config);
   Dht dht(&sensorData, &config);
   printf("Dht declared\n");
   printf("Declared objects\n");
   if (!homie.begin()) return -1;
   printf("Homie (and MQTT) started\n");
   // only fails if gpioInitialise fails
   if (!dht.begin()) return -2;
   // now gpioInitialise has succeeded. We need to ensure to shutdown before exiting
   // This happens in the destructor of dht (hence the signal handler for break and terminate).
   printf("Waiting to connect\n");
   if (!mqtt.waitForConnection(keepGoing)) return(keepGoing ? -3 : -4);
   printf("Connected to MQTT\n");
   if (!homie.sendMetadata()) return -5;
   printf("Sent metadata\n"); 
   ClimateMeasurement climateMeasurement(&homie);
   printf("Starting Main loop\n");
   while (keepGoing) {
      // ensure we don't reset the flag if break was pressed
      keepGoing &= mqtt.verifyConnection() && dht.waitForNextMeasurement();
      if (!keepGoing) break;
      auto temperature = dht.readTemperature();
      auto humidity = dht.readHumidity();
      climateMeasurement.processSample(temperature, humidity);
   }      
   printf("Shutting down\n");
   return 0;
}

int main(int argc, char** argv) {
   signal(SIGINT,signalHandler);
   signal(SIGTERM,signalHandler);
   if (argc > 1) return mainHelper(argv[1]);
   return mainHelper();
}
