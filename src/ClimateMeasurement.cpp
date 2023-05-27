// Copyright 2021-2023 Rik Essenius
// 
//   Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
//   except in compliance with the License. You may obtain a copy of the License at
// 
//       http://www.apache.org/licenses/LICENSE-2.0
// 
//   Unless required by applicable law or agreed to in writing, software distributed under the License
//   is distributed on an "AS IS" BASIS WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and limitations under the License.

#include "ClimateMeasurement.h"
#include <cmath>
#include <iostream>

ClimateMeasurement::ClimateMeasurement(ISender* sender) {
    _sender = sender;
}

/// @brief Initialize the climate sensor
void ClimateMeasurement::begin() {
    _sampleCount = 0;
    _consecutiveNanCount = 0;
}

/// @brief Send the average of the last 5 measurements to the communicator
void ClimateMeasurement::processSample(float temperature, float humidity) {
    // Since a sample is taken every 2 seconds, we have a measurement to send every 10 seconds.
    _temperature[_sampleCount] = temperature;
    _humidity[_sampleCount] = humidity;
    _sampleCount++;
    if (_sampleCount >= SAMPLES_PER_MEASUREMENT) {
	    std::cout <<  "Temperatures: ";
        for (const float temperatureSample : _temperature) {
            std::cout << temperatureSample << ";";
        }
        std::cout << std::endl;
        const float averageTemperature = roundedAverage(_temperature, SAMPLES_PER_MEASUREMENT);
        _sender->sendTemperature(averageTemperature);

        std::cout << "Humidities: ";
        for (const float humiditySample : _humidity) {
            std::cout << humiditySample << ";";
        }
        std::cout << std::endl;

        const float averageHumidity = roundedAverage(_humidity, SAMPLES_PER_MEASUREMENT);
        _sender->sendHumidity(averageHumidity);
    	_sampleCount = 0;
    }
}

/// @brief Calculate the average of the last 5 measurements
/// We use an average over 5 samples for humidity and temperature to smoothen out the noise.
/// Also the highest and lowest values are discarded to eliminate outliers.
/// Finally, sometimes the sensor returns "NaN". If that is the case, ignore those samples if possible.
float ClimateMeasurement::average(float input[], const int sampleSize) {
    int firstNumberIndex = 0;
    while (firstNumberIndex < sampleSize && std::isnan(input[firstNumberIndex])) { firstNumberIndex++; }

    // if we only have NaN values, return NaN
    if (firstNumberIndex >= sampleSize) return NAN;

    int nanCount = firstNumberIndex;
    for (int i = firstNumberIndex + 1; i < sampleSize; i++) {
        if (std::isnan(input[i])) {
            nanCount++;
            _overallNanCount++;
            std::cout << "NaN count: " << _overallNanCount << std::endl;
        }
    }

    // if we have two or less measurements, return NaN. Outliers happen too often to ignore
    if (nanCount >= sampleSize - 2) return NAN;

    // Discard the highest and the lowest value and take the average of the rest.
    auto minValue = input[firstNumberIndex];
    auto maxValue = minValue;
    auto totalValue = minValue;
    for (int i = firstNumberIndex + 1; i < sampleSize; i++) {
        if (!std::isnan(input[i])) {
            totalValue += input[i];
            if (input[i] < minValue) minValue = input[i];
            if (input[i] > maxValue) maxValue = input[i];
        }
    }
    totalValue -= (minValue + maxValue);
    return totalValue / (sampleSize - nanCount - 2);
}

/// @brief Calculate the average of the measurements and round it to 1 decimal
/// @param input the measurements
/// @param length  the number of measurements
/// @return 
float ClimateMeasurement::roundedAverage(float input[], const int length) {
    // round is tricky. It converts NAN to zero, and it calls arguments twice.
    const auto result = average(input, length);
    if (std::isnan(result)) {
        return NAN;
    }
    return round(result * 10.0) / 10.0;
}