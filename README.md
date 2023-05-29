# Dht22

A utility to read humidity and temperature from a DHT22 sensor on a Raspberry Pi and send it to an MQTT broker using the Homie convention. 
It uses PIGPIO callbacks rather than GPIO reads to ensure we don't miss pulses.