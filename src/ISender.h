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
