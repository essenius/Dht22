#ifndef ISENDER_H
#define ISENDER_H

class ISender {
public:
    virtual bool sendFloat(const char* item, float value) = 0;
};

#endif
