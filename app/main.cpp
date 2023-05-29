#include "OS.h"
#include "ClimateMeasurement.h"
#include "Config.h"
#include "Dht.h"
#include "Mqtt.h"
#include "Homie.h"
#include <cstdio>
#include <csignal>

bool keepGoing = true;
int signalCount = 0;

void signalHandler(sig_atomic_t s) {
   printf("Caught signal %d\n",s);
   signalCount++;
   keepGoing = false;
   if (signalCount > 10) {
      printf("Too many signals, exiting\n");
      exit(1);
   }
}

int mainHelper(const char* configFile = "/home/pi/.config/dht.conf") {
   OS os;
   Config config;
   printf("Config defined, hostname=%s\n", os.getHostName().c_str());
   config.begin(configFile, os.getHostName().c_str());
   printf("Config began, device=%s\n", config.getEntry("device", "unknown").c_str());
   SensorData sensorData;
   printf("SensorData defined\n");
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
      keepGoing &= mqtt.verifyConnection() && dht.waitForNextMeasurement(keepGoing);
      if (!keepGoing) break;
      auto temperature = dht.readTemperature();
      auto humidity = dht.readHumidity();
      climateMeasurement.processSample(temperature, humidity);
   }      
   printf("Shutting down\n");
   return 0;
}

int main(int argc, char** argv) {
   (void)signal(SIGINT,signalHandler);
   (void)signal(SIGTERM,signalHandler);
   (void)signal(SIGSEGV, signalHandler);
   if (argc > 1) return mainHelper(argv[1]);
   return mainHelper();
}
