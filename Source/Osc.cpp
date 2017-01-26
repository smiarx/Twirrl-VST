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






VCF::VCF(double sampleRate, float freq, float nk){
        s1 = s2 = s3 = s4 = 0.f;
        k=nk;
        sR=sampleRate;
        sD=1./sampleRate;
        setFreq(freq);
}


void VCF::setFreq(float fr){
        freq=fr;
        double wcD = 2.0*sR*tan(sD*pi*freq);
        if(wcD<0)
                wcD=0;
        double TwcD = sD*wcD;

        b0=TwcD/(TwcD+2.);
        b0p4=b0*b0*b0*b0;
        a1=(TwcD-2.)/(TwcD+2.);
}



void VCF::process(float*buf, int numSamples){
        double s;
        float in, out, u, prev, next;


        for (int i=0; i<numSamples; i++){
                s = s4 + b0*(s3 + b0*(s2 + b0*s1));
                in=*buf;
                out=(b0p4*in + s) /(1.+ b0p4*k);
                *(buf++) = out;

                u = in - k*out;

                //update filters states
                prev = u;
                next = b0*prev + s1;
                s1 = b0*prev - a1*next;

                prev = next;
                next = b0*prev + s2;
                s2 = b0*prev - a1*next;

                prev = next;
                next = b0*prev + s3;
                s3 = b0*prev - a1*next;

                s4 = b0*next - a1*out;
        }
}


