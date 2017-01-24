#include "Osc.h"


Osc::Osc(double sampleRate, float freq){
    freqtophaseinc = LUTSineSize / sampleRate * (2 << 15);
    setFreq(freq);
    phase = 0;//TODO random phase
};



void Osc::setFreq(float fr){
    freq=fr;
    phaseinc = fr*freqtophaseinc;
}


void Osc::process(float* buf, int numSamples){
    float* tbl;
    float t0,t1, pfrac;
    int dphase;
    for (int i=0; i< numSamples; ++i){
        dphase=(phase >> 16) & LUTSineMask;
        pfrac = PhaseFrac(phase);
        tbl = (float*)(lutSine)+dphase;

        *(buf++) = LUTInterp(tbl,pfrac);
        
        phase+=phaseinc;
    }


        
}
