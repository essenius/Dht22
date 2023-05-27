// Copyright 2021-2022 Rik Essenius
// 
//   Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
//   except in compliance with the License. You may obtain a copy of the License at
// 
//       http://www.apache.org/licenses/LICENSE-2.0
// 
//   Unless required by applicable law or agreed to in writing, software distributed under the License
//   is distributed on an "AS IS" BASIS WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and limitations under the License.

#ifndef CLIMATE_MEASUREMENT_H
#define CLIMATE_MEASUREMENT_H
#include "ISender.h"

/// @brief Class to take climate measurements and send them to the communicator
class ClimateMeasurement {
public:
	explicit ClimateMeasurement(ISender* sender);
    void begin();
    void processSample(float temperatureIn, float humidityIn);

private:
    constexpr static int SAMPLES_PER_MEASUREMENT = 5; // number of samples for aggregation. Must be at least 3 as we discard highest and lowest.
    constexpr static int MAX_CONSECUTIVE_NANS = 2;    // Failing to get values two times or more requires action (reset)
    float _temperature[SAMPLES_PER_MEASUREMENT] = { 0 };
    float _humidity[SAMPLES_PER_MEASUREMENT] = { 0 };
    ISender* _sender;
    int _sampleCount = 0;
    int _consecutiveNanCount = 0;
    int _overallNanCount = 0;

    float average(float input[], int sampleSize);
    float roundedAverage(float input[], int length);
};

#endif