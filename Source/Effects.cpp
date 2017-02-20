#include "Effects.h"
#include "Lut.h"
#include "PluginProcessor.h"
#include "Parameters.h"
#include "Constants.h"


#define CHORUS_DEPTH 0.002
#define CHORUS_PREDELAY 0.01

#define DELAY_MAX 2


Effects::Effects(TwirrlAudioProcessor& prt) :
    parent(prt)
{}

void Effects::start(double sR, int sPB){
    sampleRate=sR;
    samplesPerBlock=sPB;
    chorus.start(sR, sPB);
    delay.start(sR, sPB);

    setChorus(parent.chorus->get());

    setDelayTime(parent.dtime->get());
    setDelayFeedback(parent.dfeedback->get());
    setDelayWet(parent.dwet->get());
    setDelay(parent.delay->get());
}


void Effects::process(float* outleft, float* outright, int numSamples){
    if(chorus.running)
        chorus.process(outleft, outright, numSamples);
    if(delay.running)
        delay.process(outleft, outright, numSamples);
}


Effects::Chorus::Chorus() : bufleft(nullptr), bufright(nullptr)
{}

#define RATE 0.8
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

    }


    lfo = nlfo;
    wphase = wphase1;

}






////////////DELAY/////////////
#define DELAYLEAK 0.99f
#define DELAYLP 2932.f
#define DELAYHP 167.f

Effects::Delay::Delay() : bufleft(nullptr), bufright(nullptr), running(false)
{}

void Effects::Delay::start(double sR, int sPB){
    bufsize = nextpow2(sR*DELAY_MAX);
    bufmask = bufsize-1;
    bufleft = new float[bufsize];
    bufright = new float[bufsize];

    clear();

    //Leak
    dcy1left = dcy1right = dcx1left = dcx1right = 0.f;

    //LPF
    double C = 1./tan(DELAYLP * pi / sR);
    double C2 = C*C;
    double sqrt2C = C*sqrt2;
    lpa0 = 1.f / (1.f + sqrt2C + C2);
    lpb1 = -2.f * (1.f - C2) * lpa0;
	lpb2 = -(1.f - sqrt2C + C2) * lpa0;
    lpy1left = lpy1right = lpy2left = lpy2right = 0;

    //HPF
    C = tan(DELAYHP * pi / sR);
    C2 = C*C;
    sqrt2C = C*sqrt2;
	hpa0 = 1. / (1. + sqrt2C + C2);
	hpb1 = 2. * (1. - C2) * hpa0 ;
	hpb2 = -(1. - sqrt2C + C2) * hpa0;
    hpy1left = hpy1right = hpy2left = hpy2right = 0;

}


void Effects::Delay::stop(){
    if(bufleft != nullptr){
        delete bufleft;
        delete bufright;
        bufleft = bufright = nullptr;
    }
}

#define PREPARE_DELAY(chan) \
    double _dcy1##chan = dcy1##chan; \
    double _dcx1##chan = dcx1##chan; \
    \
    double _lpy1##chan = lpy1##chan; \
    double _lpy2##chan = lpy2##chan; \
    \
    double _hpy1##chan = hpy1##chan; \
    double _hpy2##chan = hpy2##chan; \




#define DO_DELAY(chan) \
    float taprd##chan = cubicinterp(buf##chan, rphase, bufmask, pfrac); \
    float tapwr##chan = *out##chan + feedback*taprd##chan; \
\
    /* HPF */ \
    double hpy0##chan = tapwr##chan + hpb1 * _hpy1##chan + hpb2 * _hpy2##chan; \
    tapwr##chan = hpa0 * (hpy0##chan - 2 * _hpy1##chan + _hpy2##chan); \
    _hpy2##chan = _hpy1##chan; \
    _hpy1##chan = hpy0##chan; \
\
    /* LPF */ \
    double lpy0##chan = tapwr##chan + lpb1 * _lpy1##chan + lpb2 * _lpy2##chan; \
    tapwr##chan = lpa0 * (lpy0##chan + 2 * _lpy1##chan + _lpy2##chan); \
    _lpy2##chan = _lpy1##chan; \
    _lpy1##chan = lpy0##chan; \
\
\
    /* LEAKDC */ \
    float x0##chan = tapwr##chan; \
    tapwr##chan = _dcy1##chan = x0##chan - _dcx1##chan + DELAYLEAK * _dcy1##chan; \
    _dcx1##chan = x0##chan; \
\
\
    buf##chan[_wphase & bufmask] =  tapwr##chan; \
    *(out##chan++) += wet*taprd##chan; \


#define FINISH_DELAY(chan) \
    lpy1##chan = zapgremlins(_lpy1##chan); \
    lpy2##chan = zapgremlins(_lpy2##chan); \
    \
    hpy1##chan = zapgremlins(_hpy1##chan); \
    hpy2##chan = zapgremlins(_hpy2##chan); \
    \
    dcy1##chan = zapgremlins(_dcy1##chan); \
    dcx1##chan = _dcx1##chan;



void Effects::Delay::process(float *outbufleft, float *outbufright, int numSamples){
    int _wphase = wphase;
    float _dsamp = dsamp;
    float *outleft=outbufleft, *outright=outbufright;

    PREPARE_DELAY(left);
    PREPARE_DELAY(right);

    if(changes.empty()){
changesempty:
        while(numSamples--){
            int idsamp = (int) _dsamp;
            float pfrac = _dsamp-idsamp;
            int rphase=_wphase-idsamp;
            DO_DELAY(left);
            DO_DELAY(right);

            _wphase++;
        }
    } else {
        speedchange nextchange = changes.front();
        while(numSamples--){
            _dsamp += dsampinc;
            int idsamp = (int) _dsamp;
            float pfrac = _dsamp-idsamp;
            int rphase=_wphase-idsamp;
            DO_DELAY(left);
            DO_DELAY(right);

            _wphase++;
            if(rphase >= nextchange.stop){
                //_dsamp = nextchange.dsamp;
                changes.pop();
                if(changes.empty()){
                    speed=1.f;
                    dsampinc=0.f;
                    goto changesempty;
                } else {
                    speed /= nextchange.speed;
                    dsampinc = 1.f-speed;
                    nextchange = changes.front();
                }
            }
        }
    }
    dsamp=_dsamp;
    wphase=_wphase;
    FINISH_DELAY(left);
    FINISH_DELAY(right);
}

