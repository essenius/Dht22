// Copyright 2023 Rik Essenius
// 
//   Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
//   except in compliance with the License. You may obtain a copy of the License at
// 
//       http://www.apache.org/licenses/LICENSE-2.0
// 
//   Unless required by applicable law or agreed to in writing, software distributed under the License
//   is distributed on an "AS IS" BASIS WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and limitations under the License.

#ifndef I_SENDER_H
#define I_SENDER_H

class ISender {
public:
    ISender() = default;
    virtual ~ISender() = default;
	ISender(const ISender&) = delete;
    ISender(ISender&&) = delete;
    ISender& operator=(const ISender&) = delete;
    ISender& operator=(ISender&&) = delete;
    virtual bool sendHumidity(float value) = 0;
    virtual bool sendTemperature(float value) = 0;
};

#endif
