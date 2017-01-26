#ifndef _OSC_H
#define _OSC_H

#include "Lut.h"
#include "Constants.h"
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



class VCF{
    public:
        VCF(double sampleRate, float freq, float nk);

        void process(float* buf, int numSumples);
        void setFreq(float fr);

    private:
        float freq;
        float k;
        double b0, b0p4, a1;
        double s1,s2,s3,s4; //first order filter state

        double sR;
        double sD;//sampleDur
};

#endif
