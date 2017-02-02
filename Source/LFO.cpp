#include "LFO.h"


LFO::LFO() :
    buf(nullptr),
    bufSize(0),
    ratetophaseinc(0),
    phaseinc(0),
    phase(0)
{};

void LFO::start(float sampleRate, int samplesPerBlock, float rate){
    if(bufSize!=samplesPerBlock){
        bufSize = samplesPerBlock;
        buf = new int32_t[bufSize];
    }
    ratetophaseinc = 4.f/sampleRate*(1<<16);
    phaseinc = ratetophaseinc*rate;
    phase=0;
}



void LFO::process(){
    int32_t* bf=buf;;

    for(int i=0; i<bufSize; i++){
        *(bf++) = (phase > (1<<16)) ? (1<<17) - phase : phase;
        phase+=phaseinc;
        if(phase > 3*(1<<16))
            phase -= (1<<18);
    }
}
