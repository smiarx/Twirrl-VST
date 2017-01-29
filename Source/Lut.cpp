//Lookup Tables


#include "Lut.h"
#include "Constants.h"




float lutSine[(LUTSineSize<<1)+1];
float lutInvSine[(LUTSineSize<<1)+1];


float lutMidi[(LUTMidiSize<<1)-1];

void lutInit(){

    //Sine
    double sineIndexToPhase = twopi / LUTSineSize;
    double phase;
    float s, is, prevs, previnv;
    float* ls = lutSine, *linv = lutInvSine;

    for (int i=0; i<= LUTSineSize; ++i){
        phase = i * sineIndexToPhase;
        s = sin(phase);
        is = 1./s;

        if(i>0){
            *(ls++) = s-prevs;
            *(linv++) = is-previnv;
        }

        *(ls++) = s;
        *(linv++) = 1./s;
        prevs=s;
        previnv=is;

    }


    for (int i=0; i<=17; ++i){
        lutInvSine[i] = lutInvSine[(LUTSineSize<<1)-i] = invSineBad;
        lutInvSine[LUTSineSize-i] = lutInvSine[LUTSineSize+i] = invSineBad;
    }


    //Midi
    double indexToMidi = 128.f/LUTMidiSize;
    float* tbl = lutMidi;
    float freq, prevfreq;

    for(int i=0; i<LUTMidiSize; ++i){
        freq = 440.f*pow(2,(i*indexToMidi-69.f)/12.f);
        if(i>0)
            *(tbl++) = freq-prevfreq;

        *(tbl++) = freq;
        prevfreq = freq;
    }



}
