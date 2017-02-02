#ifndef _LFO_H
#define _LFO_H

#include <cstdint>

class LFO{
    public:
        LFO();
        ~LFO() {delete buf;};
        void start(float sampleRate, int samplesPerBlock, float rate);
        void process();
        void updateRate(float rate){phaseinc = ratetophaseinc*rate;};
        int32_t* getBuf(){return buf;};

    private:
        int32_t* buf;
        int bufSize;
        int32_t ratetophaseinc;
        int32_t phaseinc;
        int32_t phase;
};

#endif
