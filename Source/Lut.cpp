//Lookup Tables


#include "Lut.h"
#include "Constants.h"




float lutSine[(LUTSineSize<<1)+1];
float lutInvSine[(LUTSineSize<<1)+1];


float lutMidi[(LUTMidiSize<<1)-1];


float lutVCFb[(LUTVCFSize<<1)];
float lutVCFa[(LUTVCFSize<<1)];




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


    //VCF midi to pulsation
    double wcD;
    double b,a,prevb,preva;
    float *tblb=lutVCFb, *tbla=lutVCFa;
    for(int i=0; i<LUTVCFSize; ++i){
        wcD = 2.f*tan(pi/2.f * pow(2, VCFMaxMidi * (i-LUTVCFSize)/(12.f*LUTVCFSize)));
        b = wcD/(wcD+2.);
        a = (wcD-2.)/(wcD+2.);
        if(i>0){
            *(tblb++) = b-prevb;
            *(tbla++) = a-preva;
        }
        *(tblb++) = b;
        *(tbla++) = a;
        prevb=b;
        preva=a;
    }
    tblb[0] = tblb[-2];
    tbla[0] = tbla[-2];



}
