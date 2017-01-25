#ifndef _OSC_H
#define _OSC_H

#include "Lut.h"
#include <cmath>


class Osc{
    public:
        Osc(double sampleRate, float freq);
        
        void process(float* buf, int numSamples);
        void setFreq(float fr);

    private:
        int32_t phase; // Q15 fixed float
        int32_t phaseinc;
        int32_t N2;//number of harmonics (odd)
        double freqtophaseinc;
        float freq;
        float scale;

        float y;
        float leak;
        double sR;//sampleRate


};  

#endif
