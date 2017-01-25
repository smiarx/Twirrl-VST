//Lookup Tables


#include "Lut.h"
#include "Constants.h"




float lutSine[LUTSineSize+1];
float lutInvSine[LUTSineSize+1];


void lutInit(){
    double sineIndexToPhase = twopi / LUTSineSize;
    for (int i=0; i<= LUTSineSize; ++i){
        double phase = i * sineIndexToPhase;
        float s = sin(phase);
        lutSine[i] = s;
        lutInvSine[i] = 1./s;

    }

    lutInvSine[0] = lutInvSine[LUTSineSize/2] = lutInvSine[LUTSineSize] = invSineBad;
    int sz2 = LUTSineSize>>1;

    for (int i=1; i<=8; ++i){
        lutInvSine[i] = lutInvSine[LUTSineSize-i] = invSineBad;
        lutInvSine[sz2-i] = lutInvSine[sz2+i] = invSineBad;
    }


}
