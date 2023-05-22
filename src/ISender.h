#ifndef ISENDER_H
#define ISENDER_H

class ISender {
public:
    virtual bool sendHumidity(float value) = 0;
    virtual bool sendTemperature(float value) = 0;
};

#endif
