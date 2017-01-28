#include "Voice.h"
#include "PluginProcessor.h"
#include "Parameters.h"




Voice::Voice(TwirrlAudioProcessor& prt, double sR, int sPB, float* lfoBuf) :
    parent(prt),
    samplesPerBlock(sPB),
    sampleRate(sR),
    sampleDur(1./sR),
    lfoBuf(lfoBuf),
    freq(0.f),
    running(false),
    env(Env(*this, sPB, sR, prt.a->get(), prt.d->get(), prt.s->get(), prt.r->get())),
    osc(Osc(*this, sR, prt.saw->get(), prt.sq->get())),
    vcf(VCF(*this, prt.cutoff->get(), prt.res->get()))
{}


void Voice::start(float fr){
    freq=fr;
    env.attack();
    osc.update();
    vcf.update();
    running=true;
}


void Voice::release(){ env.release();}
void Voice::stop(){ running=false;}


void Voice::process(float* buf, int numSamples){
    env.process(numSamples);
    osc.process(buf, numSamples);
    vcf.process(buf, numSamples);

    float* envbuf = env.buf;
    for(int i=0; i<numSamples; ++i)
        *(buf++) *= *(envbuf++);
}



///////ENV

Voice::Env::Env(Voice& vc, int samplesPerBlock, float sR, float af, float df, float sf, float rf):
    voice(vc),
    level(0.)
{
    buf=new float[samplesPerBlock];
    a=std::max(static_cast<int32_t>(af*sR),1);
    d=std::max(static_cast<int32_t>(df*sR),1);
    s=sf;
    r=std::max(static_cast<int32_t>(rf*sR),1);
}


Voice::Env::~Env(){ delete [] buf;}

void Voice::Env::attack(){ changeStage(0);}
void Voice::Env::release(){ changeStage(3);}

void Voice::Env::changeStage(int stg){
    float goalLevel;

    stage=stg;
    switch(stage){
        case 0:
            goalLevel = 1.f;
            count=a;
            break;
        case 1:
            goalLevel = s;
            count=d;
            break;
        case 2:
            level=s;
            return;
            break;
        case 3:
            goalLevel = 0.f;
            count=r;
            break;
        case 4:
            level=0.f;
            voice.stop();
            return;
            break;
        default:
            return;
    }
    b1 = (goalLevel-level) / (1.f - exp(ENVCURVE));
    a2 = level+b1;
    growth = exp(ENVCURVE/count);

}


void Voice::Env::process(int numSamples){
    float* bf=buf;
    if(stage==2){

        for(int i=0; i<numSamples; ++i)
            *(bf++) = s;
        return;
    }

    while(numSamples){
        int decr=std::min(count,numSamples);
        for (int i=0; i<decr; ++i){
            b1*=growth;
            level=a2-b1;
            *(bf++) = level;
        }

        numSamples-=decr;
        count-=decr;
        if(count==0){
            changeStage(stage+1);
            //finish buffer
            if(stage==2 || stage==4){
                for(int i=0; i<numSamples; ++i)
                    *(bf++) = level;
                numSamples=0;
            }
        }
    }
}



/////OSC


Voice::Osc::Osc(Voice& vc, double sampleRate, float sawl, float sql): voice(vc){
    freqtophaseinc =  LUTSineSize / sampleRate *65536. *0.5;
    sawlvl = sawl;
    sqlvl = sql;
    phase = 0;//TODO random phase
    saw=-0.5f;//TODO initial value function of phase
    sq=-0.5f;
};



void Voice::Osc::update(){
    phaseinc = voice.freq*freqtophaseinc;
    int32_t N = (int32_t)(voice.sampleRate*0.5/voice.freq);
    scale= 0.5/N;
    N2=2*N+1;

    leak=0.9999f;


}


inline float blit(int32_t phase, int N2, float scale){
    float* tbl;
    float blt, num, denom, pfrac;
    int nphase;

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
            blt= 1.f + scale;
        }
        else
            blt=num/denom*scale;

    }
    else{
        denom = LUTInterp(tbl, pfrac);
        blt= num*denom*scale;
    }

}

void Voice::Osc::process(float* buf, int numSamples){
    float blt, saw1, sq1;
    float pulsepos,pulseneg;
    bool dosaw,dosq;

    saw1=saw;
    sq1=sq;

    dosaw=sawlvl>0;
    dosq=sawlvl>0;


    for (int i=0; i< numSamples; ++i){

        if(dosaw){
            blt=blit(phase,N2,scale);
            saw1 =  blt-scale + saw1*leak;
            *buf +=  saw1*sawlvl;
        }


        if(dosq){
            pulsepos=blit(phase,N2,scale);
            pulseneg=blit(phase+(LUTSineSize << 14) ,N2,scale);
            //TODO weird shit with this -> pulseneg=blit(phase+LUTSineSize << 18 ,N2,scale);
            sq1 = pulsepos - pulseneg + sq1*leak;

            *buf += sq1*sqlvl;
        }
        buf++;
        phase+=phaseinc;

    }
    saw=saw1;
    sq=sq1;



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


