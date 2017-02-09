#include "Effects.h"
#include "Lut.h"
#include "PluginProcessor.h"



Effects::Effects(TwirrlAudioProcessor& prt) :
    parent(prt)
{}

void Effects::start(double sR, int sPB){
    sampleRate=sR;
    samplesPerBlock=sPB;
    chorus.start(sR, sPB);
}


void Effects::process(float* outleft, float* outright, int numSamples){
    chorus.process(outleft, outright, numSamples);
}


Effects::Chorus::Chorus() : bufleft(nullptr), bufright(nullptr)
{}

#define RATE 0.3
void Effects::Chorus::start(double sR, int sPB){
    bufsize = nextpow2(sR*(CHORUS_DEPTH*2+CHORUS_PREDELAY));
    bufmask = bufsize-1;
    bufleft = new float[bufsize];
    bufright = new float[bufsize];

    float *bfl=bufleft, *bfr = bufright;
    for(int i=0; i<bufsize; ++i)
        *(bfl++) = *(bfr++) = 0.;

    depth = sR * CHORUS_DEPTH * (1<<16);
    predelay = sR * CHORUS_PREDELAY * (1<<16);
    lfophase = 0;
    ratetophaseinc = 4./sR*sPB*(1<<16);
    lfophaseinc = ratetophaseinc*RATE;
    wphase=0;


}


void Effects::Chorus::stop(){
    if(bufleft != nullptr){
        delete bufleft;
        delete bufright;
        bufleft = bufright = nullptr;
    }
}



void Effects::Chorus::process(float* outleft, float* outright, int numSamples){

    ////LFO
    int nlfo;
    lfophase += lfophaseinc;
    if(lfophase < (1<<16)){
        int z = lfophase;
        nlfo = (1<<16) - Q16MUL(z,z);
    } else if(lfophase < 3*(1<<16)){
        int z = lfophase - (1<<17);
        nlfo = Q16MUL(z,z) - (1<<16);
    } else {
        lfophase -= (1<<18);
        int z = lfophase;
        nlfo = (1<<16) - Q16MUL(z,z);
    }
    nlfo = Q16MUL(nlfo,depth);
    int lfo1=lfo;
    int lfo_slop = (nlfo - lfo1)/numSamples;


    int wphase1 = wphase;
    int dell = (predelay + lfo1);
    int delr = (predelay - lfo1);
    float *outl=outleft, *outr=outright;
    for(int i=0; i<numSamples; ++i){

        //left
        bufleft[wphase1 & bufmask] = *outl;
        int irphasel = wphase1 - (dell >> 16);
        int fracl = PhaseFrac(dell);
        *(outl++) += cubicinterp(bufleft, irphasel, bufmask, fracl);
        dell += lfo_slop;

        //right
        bufright[wphase1 & bufmask] = *outr;
        int irphaser = wphase1 - (delr >> 16);
        int fracr = PhaseFrac(delr);
        *(outr++) += cubicinterp(bufright, irphaser, bufmask, fracr);
        delr -= lfo_slop;

        wphase1++;

        //std::cout << (depth>>16) << " - " << (dell >> 16) << "\n";
        //std::cout << (wphase1 & bufmask) << " - " << (irphase & bufmask) << " - " << (predelay>>16) << "\n";
    }


    lfo = nlfo;
    wphase = wphase1;

}
