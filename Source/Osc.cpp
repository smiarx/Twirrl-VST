#include "Osc.h"


Osc::Osc(double sampleRate, float freq){
    freqtophaseinc =  LUTSineSize / sampleRate *65536. *0.5;
    sR=sampleRate;
    phase = 0;//TODO random phase
    setFreq(freq);
};



void Osc::setFreq(float fr){
    freq=fr;
    phaseinc = fr*freqtophaseinc;
    int32_t N = (int32_t)(sR*0.5/fr);
    scale= 0.5/N;
    N2=2*N+1;


    y=-0.46f;
    leak=0.9999f;


}

void Osc::process(float* buf, int numSamples){
    float* tbl;
    float y1, num, denom, pfrac;
    int dphase, nphase;

    y1=y;

    for (int i=0; i< numSamples; ++i){

        //numerator Sine
        nphase = phase*N2;
        pfrac = PhaseFrac(nphase);
        tbl = LOOKUP(lutSine, nphase);
        num = LUTInterp(tbl,pfrac);
        

        //denominator (invSine)
        //dphase=LUTPhase(phase);
        pfrac = PhaseFrac(phase);
        tbl = LOOKUP(lutInvSine, phase);


        if(tbl[0]==invSineBad || tbl[1]==invSineBad){
            tbl = LOOKUP(lutSine, phase);
            denom = LUTInterp(tbl, pfrac);
            if(std::abs(denom) < 0.0005f){
                y1= 1.f +leak*y1;
            }
            else
                y1=(num/denom-1)*scale + leak*y1;

        }
        else
            denom = LUTInterp(tbl, pfrac);
            y1= (num*denom-1)*scale + leak*y1;


        *(buf++) = y1;
        phase+=phaseinc;

    }
    y=y1;


        
}
