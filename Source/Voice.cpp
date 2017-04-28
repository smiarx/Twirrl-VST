#include "Voice.h"
#include "PluginProcessor.h"
#include "Parameters.h"
#include <stdlib.h>



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
    env(Env(*this, sPB)),
    osc(Osc(*this, sR)),
    noise(Noise(*this, rand())),
    vcf(VCF(*this))
{
    updateAttack(prt.a->get());
    updateDecay(prt.d->get());
    updateSustain(prt.s->get());
    updateRelease(prt.r->get());

    updateVibrato(prt.vibrato->get());
    updateSaw(prt.saw->get());
    updateSq(prt.sq->get());
    updateSub(prt.sub->get());

    updateNoise(prt.noise->get());

    updateCutoff(prt.cutoff->get());
    updateRes(prt.res->get());
    updateVCFLFO(prt.vcflfo->get());
    updateVCFEnv(prt.vcfenv->get());
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
    noise.process(numSamples);
    vcf.process(numSamples);

    envbuf = env.buf, bf=audiobuf;
    for(int i=0; i<numSamples; ++i)
        *(outbuf++) += (*(bf++)) * (*(envbuf++));
}



///////ENV

Voice::Env::Env(Voice& vc, int samplesPerBlock):
    voice(vc),
    level(0.)
{
    buf=new float[samplesPerBlock];
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


Voice::Osc::Osc(Voice& vc, double sampleRate) :
    voice(vc)
{
    freqtophaseinc =  LUTSineSize / sampleRate *65536. *0.5;//65536=2^16
    phase = rand();
    phasediff= rand();//phase difference between saw & square

    //initial level from random phase
    saw = 0.5 - ((float) ((phase-1) & ((LUTSineSize << 15)-1))) / (LUTSineSize*(1<<15));
    sq= 0.5  - (float) (((phase+phasediff-1) & ((LUTSineSize << 15)-1)) > (LUTSineSize << 14));
    sub= 0.5  - (float) ((( (phase>>1) +phasediff-1) & ((LUTSineSize << 15)-1)) > (LUTSineSize << 14));

};



void Voice::Osc::update(){
    phaseinc = voice.freq*freqtophaseinc;
    int32_t N = (int32_t)(voice.sampleRate*0.5/voice.freq);
    int32_t Nsub = (int32_t)(voice.sampleRate/voice.freq);
    scale= 0.5/N;
    scalesub= 0.5/Nsub;
    N2=2*N+1;
    N2sub=2*Nsub+1;

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
    float blt, saw1, sq1, sub1;
    float pulsepos,pulseneg;
    float subpos,subneg;
    bool dosaw,dosq, dosub;

    float *midilut=voice.midilut;
    int32_t *lfobuf=voice.lfoBuf;
    int32_t phaseinc1=phaseinc, phase1=phase, phasesq, phasesub;

    saw1=saw;
    sq1=sq;
    sub1=sub;

    dosaw=sawlvl>0;
    dosq=sqlvl>0;
    dosub=sublvl>0.;


    float* envbuf=voice.env.buf;
    int32_t pwmphase;

    for (int i=0; i< numSamples; ++i){

        if(dosaw){
            blt=blit(phase1,N2,scale);
            saw1 =  blt-scale + saw1*leak;
            *buf +=  saw1*sawlvl;
        }


        if(dosq){
            //pwm
            pwmphase=(LUTSineSize << 14) + (*(envbuf++))*pwm;

            phasesq=phase1+phasediff;

            pulsepos=blit(phasesq,N2,scale);
            pulseneg=blit(phasesq+pwmphase ,N2,scale);
            //TODO weird shit with this -> pulseneg=blit(phase+LUTSineSize << 18 ,N2,scale);
            sq1 = pulsepos - pulseneg + sq1*leak;

            *buf += sq1*sqlvl;
        }

        if(dosub){
            phasesub=(phase1>>1)+phasediff;
            subpos=blit(phasesub, N2sub, scalesub);
            subneg=blit(phasesub+(LUTSineSize << 14), N2sub, scalesub);
            sub1 = subpos - subneg + sub1*leak;

            *buf += sub1*sublvl;
        }
        buf++;


        //vibrato
        int32_t vib = Q16MUL(*(lfobuf++), vibrato);
        int32_t ivib = LUTMidiIndex(vib);
        float fvib = MidiFrac(vib);
        float freq = midilut[ivib] + fvib*midilut[ivib+1];
        phaseinc1 = freqtophaseinc*freq;

        phase1+=phaseinc1;
    }
    phaseinc=phaseinc1;
    phase=phase1;
    saw=saw1;
    sq=sq1;
    sub=sub1;



}





//////NOISE

inline int32_t hash(int32_t inKey)
{
    // Thomas Wang's integer hash.
    // http://www.concentric.net/~Ttwang/tech/inthash.htm
    // a faster hash for integers. also very good.
    uint32_t hash = (uint32_t)inKey;
    hash += ~(hash << 15);
    hash ^=   hash >> 10;
    hash +=   hash << 3;
    hash ^=   hash >> 6;
    hash += ~(hash << 11);
    hash ^=   hash >> 16;
    return (int32)hash;
}

Voice::Noise::Noise(Voice& vc, uint32_t seed) : voice(vc){
    seed=hash(seed);
    s1 = 1243598713U ^ seed; if (s1 <  2) s1 = 1243598713U;
    s2 = 3093459404U ^ seed; if (s2 <  8) s2 = 3093459404U;
    s3 = 1821928721U ^ seed; if (s3 < 16) s3 = 1821928721U;
}

//SC rand functions
inline uint32 trand( uint32& s1, uint32& s2, uint32& s3 )
{
	s1 = ((s1 &  (uint32)-2) << 12) ^ (((s1 << 13) ^  s1) >> 19);
	s2 = ((s2 &  (uint32)-8) <<  4) ^ (((s2 <<  2) ^  s2) >> 25);
	s3 = ((s3 & (uint32)-16) << 17) ^ (((s3 <<  3) ^  s3) >> 11);
	return s1 ^ s2 ^ s3;
}
inline float frand2( uint32& s1, uint32& s2, uint32& s3 )
{
	// return a float from -1.0 to +0.999...
	union { uint32 i; float f; } u;
	u.i = 0x40000000 | (trand(s1,s2,s3) >> 9);
	return u.f - 3.f;
}

void Voice::Noise::process(int numSamples){
    uint32_t ts1=s1, ts2=s3, ts3=s3;
    float *buf=voice.audiobuf;

    for(int i=0; i<numSamples; ++i)
        *(buf++) += frand2(ts1,ts2,ts3)*level;
    s1=ts1, s2=ts2, s3=ts3;
}





//VCF


Voice::VCF::VCF(Voice& vc) : voice(vc){
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
    float * envbuf=voice.env.buf, envsustain=voice.env.s;
    int32_t cut;
    float fcut;
    float *tblb, *tbla;
    float in, out, u, prev, next;


    for (int i=0; i<numSamples; i++){

        //compute pulsation
        cut = midibase + cutoff + Q16MUL(*(lfobuf++), lfomod) + (*(envbuf++)-envsustain)*envmod;
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


