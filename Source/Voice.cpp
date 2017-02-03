#include "Voice.h"
#include "PluginProcessor.h"
#include "Parameters.h"




Voice::Voice(TwirrlAudioProcessor& prt, double sR, int sPB, int32_t* lfoBuf) :
    parent(prt),
    samplesPerBlock(sPB),
    sampleRate(sR),
    sampleDur(1./sR),
    serial(0),
    audiobuf(new float[sPB]),
    lfoBuf(lfoBuf),
    freq(0.f),
    midinum(0),
    midilut(nullptr),
    running(false),
    env(Env(*this, sPB, sR, prt.a->get(), prt.d->get(), prt.s->get(), prt.r->get())),
    osc(Osc(*this, sR, prt.vibrato->get(), prt.saw->get(), prt.sq->get())),
    vcf(VCF(*this, prt.cutoff->get(), prt.res->get()))
{
}


Voice::~Voice(){
    delete audiobuf;
}



void Voice::start(int midinote){

    midinum = midinote;
    midilut =  lutMidi + midinum*(LUTMidiSize>>6);
    freq=midilut[0];
    env.attack();
    osc.update();
    vcf.update();
    running=true;
}



void Voice::process(float* outbuf, int numSamples){
    float* bf, *envbuf;

    bf=audiobuf;
    for(int i=0; i<samplesPerBlock; ++i)
        *(bf++) = 0.f;

    env.process(numSamples);
    osc.process(numSamples);
    vcf.process(numSamples);

    envbuf = env.buf, bf=audiobuf;
    for(int i=0; i<numSamples; ++i)
        *(outbuf++) += (*(bf++)) * (*(envbuf++));
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


Voice::Osc::Osc(Voice& vc, double sampleRate, float vib, float sawl, float sql) :
    voice(vc),
    vibrato(vib)
{
    freqtophaseinc =  LUTSineSize / sampleRate *65536. *0.5;//65536=2^16
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

void Voice::Osc::process(int numSamples){
    float* buf= voice.audiobuf;
    float blt, saw1, sq1;
    float pulsepos,pulseneg;
    bool dosaw,dosq;

    float *midilut=voice.midilut;
    int32_t *lfobuf=voice.lfoBuf;

    saw1=saw;
    sq1=sq;

    dosaw=sawlvl>0;
    dosq=sqlvl>0;


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


        //vibrato
        int32_t vib = Q16MUL(*(lfobuf++), vibrato);
        int32_t ivib = LUTMidiIndex(vib);
        float fvib = MidiFrac(vib);
        float freq = midilut[ivib] + fvib*midilut[ivib+1];
        phaseinc = freqtophaseinc*freq;

        phase+=phaseinc;

    }
    saw=saw1;
    sq=sq1;



}






Voice::VCF::VCF(Voice& vc, float ctoff, float nk) : voice(vc), cutoff(ctoff), k(nk){
    lfomod=0;
    s1 = s2 = s3 = s4 = 0.f;
}


void Voice::VCF::update(){

    float sR2midi = 69.f + 12.f*log(voice.sampleRate/440.f)/log(2.f) - 12.f;
    midibase = (voice.midinum - sR2midi)*VCFMidiMul + (LUTVCFSize<<16);
}




void Voice::VCF::process(int numSamples){
    float *buf=voice.audiobuf;
    double s;
    int32_t* lfobuf=voice.lfoBuf;
    int32_t cut;
    float fcut;
    float *tblb, *tbla;
    float in, out, u, prev, next;


    for (int i=0; i<numSamples; i++){

        //compute pulsation
        cut = midibase + cutoff + Q16MUL(*(lfobuf++), lfomod);
        if(cut >= (LUTVCFSize<<16)) cut = (LUTVCFSize<<16)-1;

        tblb = VCFLOOKUP(lutVCFb, cut);
        tbla = VCFLOOKUP(lutVCFa, cut);
        fcut = CutFrac(cut);

        //filtering
        b0 = LUTInterp(tblb, fcut);
        b0p4=b0*b0*b0*b0;
        a1 = LUTInterp(tbla, fcut);

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


