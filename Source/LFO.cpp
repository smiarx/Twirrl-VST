#include "LFO.h"


LFO::LFO() :
    buf(nullptr),
    bufSize(0),
    ratetophaseinc(0.f),
    phaseinc(0.f),
    phase(0.f)
{};

void LFO::start(float sampleRate, int samplesPerBlock, float rate){
    if(bufSize!=samplesPerBlock){
        bufSize = samplesPerBlock;
        buf = new float[bufSize];
    }
    ratetophaseinc = 4.f/sampleRate;
    phaseinc = ratetophaseinc*rate;
    phase=0.f;
}



void LFO::process(){
    float* bf=buf;;

    for(int i=0; i<bufSize; i++){
        *(bf++) = (phase > 1.f) ? 2.f - phase : phase;
        phase+=phaseinc;
        if(phase > 3.f)
            phase -= 4.f;
    }
}
