//Lookup Tables


#include "Lut.h"
#include "Constants.h"




float lutSine[(LUTSineSize<<1)+1];
float lutInvSine[(LUTSineSize<<1)+1];


void lutInit(){
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



}
