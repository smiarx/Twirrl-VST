#include "Voice.h"
#include "PluginProcessor.h"
#include "Parameters.h"




Voice::Voice(TwirrlAudioProcessor& prt, double sR) :
    parent(prt),
    sampleRate(sR),
    sampleDur(1./sR),
    freq(0.f),
    running(false),
    osc(Osc(*this, sR)),
    vcf(VCF(*this, prt.cutoff->get(), prt.res->get()))
{}


void Voice::start(float fr){
    freq=fr;
    osc.update();
    vcf.update();
    running=true;
}


void Voice::process(float* buf, int numSamples){
    osc.process(buf, numSamples);
    vcf.process(buf, numSamples);
}


void Voice::updateParameter(ParamID id, float value){
    switch(id){
        case cutoffID:
            vcf.updateCutoff(value);
            break;
        case resID:
            vcf.updateRes(value);
            break;
    }
}


Voice::Osc::Osc(Voice& vc, double sampleRate): voice(vc){
    freqtophaseinc =  LUTSineSize / sampleRate *65536. *0.5;
    phase = 0;//TODO random phase
};



void Voice::Osc::update(){
    phaseinc = voice.freq*freqtophaseinc;
    int32_t N = (int32_t)(voice.sampleRate*0.5/voice.freq);
    scale= 0.5/N;
    N2=2*N+1;

    y=-0.46f;//TODO initial value function of phase
    leak=0.9999f;


}


void Voice::Osc::process(float* buf, int numSamples){
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
        else{
            denom = LUTInterp(tbl, pfrac);
            y1= (num*denom-1)*scale + leak*y1;
        }


        *(buf++) = y1;
        phase+=phaseinc;

    }
    y=y1;


        
}






Voice::VCF::VCF(Voice& vc, float ctoff, float nk) : voice(vc), cutoff(ctoff), k(nk){
    s1 = s2 = s3 = s4 = 0.f;
}


void Voice::VCF::update(){
    double wcD = 2.0*voice.sampleRate*tan(voice.sampleDur*pi*voice.freq*cutoff);
    if(wcD<0)
        wcD=0;
    double TwcD = voice.sampleDur*wcD;

    b0=TwcD/(TwcD+2.);
    b0p4=b0*b0*b0*b0;
    a1=(TwcD-2.)/(TwcD+2.);
}

void Voice::VCF::updateCutoff(float ctoff){
    cutoff=ctoff;
    update();
}
void Voice::VCF::updateRes(float res){
    k=res;
}



void Voice::VCF::process(float*buf, int numSamples){
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


