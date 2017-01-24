#ifndef _OSC_H
#define _OSC_H

#include "Lut.h"


class Osc{
    public:
        Osc(double sampleRate, float freq);
        
        void process(float* buf, int numSamples);
        void setFreq(float fr);

    private:
        int32_t phase; // Q15 fixed float
        int32_t phaseinc;
        double freqtophaseinc;
        float freq;


};  

#endif
