#include "Effects.h"
#include "Lut.h"
#include "PluginProcessor.h"
#include "Parameters.h"
#include "Constants.h"


#define CHORUS_DEPTH 0.001
#define CHORUS_PREDELAY 0.013

#define DELAY_MAX 2


Effects::Effects(TwirrlAudioProcessor& prt) :
    parent(prt)
{}

void Effects::start(double sR, int sPB){
    sampleRate=sR;
    samplesPerBlock=sPB;
    chorus.start(sR, sPB);
    phaser.start(sR, sPB);
    delay.start(sR, sPB);

    setChorusRate(parent.crate->get());
    setChorus(parent.chorus->get());

    setPhaserRate(parent.prate->get());
    setPhaserFeedback(parent.pfeedback->get());
    setPhaser(parent.phaser->get());

    setDelayTime(parent.dtime->get());
    setDelayFeedback(parent.dfeedback->get());
    setDelayWet(parent.dwet->get());
    setDelay(parent.delay->get());
}


void Effects::process(float* outleft, float* outright, int numSamples){
    if(chorus.running)
        chorus.process(outleft, outright, numSamples);
    if(phaser.running)
        phaser.process(outleft, outright, numSamples);
    if(delay.running)
        delay.process(outleft, outright, numSamples);
}


Effects::Chorus::Chorus() : bufleft(nullptr), bufright(nullptr), running(false)
{}

void Effects::Chorus::start(double sR, int sPB){
    bufsize = nextpow2(sR*(CHORUS_DEPTH*2+CHORUS_PREDELAY));
    bufmask = bufsize-1;
    bufleft = new float[bufsize];
    bufright = new float[bufsize];

    float *bfl=bufleft, *bfr = bufright;
    for(int i=0; i<bufsize; ++i)
        *(bfl++) = *(bfr++) = 0.;

    depth = sR * CHORUS_DEPTH;
    predelay = sR * (CHORUS_PREDELAY+CHORUS_DEPTH);
    lfophase = 0.f;
    ratetophaseinc = 4./sR*sPB;
    //lfophaseinc = ratetophaseinc*RATE;
    wphase=0;
    lfo=depth;
    dsampl = predelay+lfo;
    dsampr = predelay-lfo;

}


void Effects::Chorus::stop(){
    if(bufleft != nullptr){
        delete bufleft;
        delete bufright;
        bufleft = bufright = nullptr;
    }
}



void Effects::Chorus::process(float *outbufleft, float *outbufright, int numSamples){

    ////LFO
    float nlfo;
    lfophase += lfophaseinc;
    if(lfophase < 1.f){
        float z = lfophase;
        nlfo = 1.f - z*z;
    } else if(lfophase < 3.f){
        float z = lfophase - 2.f;
        nlfo = z*z - 1.f;
    } else {
        lfophase -= 4.f;
        float z = lfophase;
        nlfo = 1.f - z*z;
    }
    nlfo *= depth;
    float lfo_slop = (nlfo - lfo)/numSamples;


    //chorus
    int _wphase = wphase;
    float _dsampl = dsampl;
    float _dsampr = dsampr;
    float *outleft=outbufleft, *outright=outbufright;

    while(numSamples--){

        //left
        _dsampl += lfo_slop;
        int idsampl = (int) _dsampl;
        float pfracl = _dsampl-idsampl;
        int rphasel=_wphase-idsampl;
        bufleft[_wphase & bufmask] =  *outleft;
        *(outleft++) += cubicinterp(bufleft, rphasel, bufmask, pfracl) * invsqrt2;


        //right
        _dsampr -= lfo_slop;
        int idsampr = (int) _dsampr;
        float pfracr = _dsampr-idsampr;
        int rphaser=_wphase-idsampr;
        bufright[_wphase & bufmask] =  *outright;
        *(outright++) += cubicinterp(bufright, rphaser, bufmask, pfracr) * invsqrt2;
        
        _wphase++;
    }


    lfo = nlfo;
    wphase=_wphase;
    dsampl = _dsampl;
    dsampr = _dsampr;

}






////////////DELAY/////////////
#define DELAYLEAK 0.99f
#define DELAYLP 2381.f
#define DELAYHP 183.f

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









#define PHASER_RATE 0.8
#define PHASER_COEFFS  0.8271, 0.6321, 0.3521,    0.9,    0.7,    0.7
#define PHASER_DEPTHS    0.14,  0.341,   0.32, 0.0635,   0.29,   0.24

Effects::Phaser::Phaser() :
    aleft{PHASER_COEFFS},
    aright{PHASER_COEFFS},
    depth{PHASER_DEPTHS},
    historyleft{},
    historyright{},
    running(false),
    lfo(0.f),
    lfophase(1.f)
{
}
void Effects::Phaser::start(double sR, int sPB){

    ratetophaseinc = 4./sR*sPB;
    //lfophaseinc = ratetophaseinc*PHASER_RATE;


}


#define phaser_sign_left +=
#define phaser_sign_right -=

#define DOPHASER(chan) \
    float previn##chan = history##chan[j];\
    history##chan[j] = out##chan;\
    out##chan = (a##chan[j] phaser_sign_##chan lfo_slop*depth[j]) * (out##chan + history##chan[j+1]) - previn##chan;

void Effects::Phaser::process(float *outbufleft, float *outbufright, int numSamples){

    ////LFO
    float _lfo;
    lfophase += lfophaseinc;
    if(lfophase < 1.f){
        float z = lfophase;
        _lfo = 1.f - z*z;
    } else if(lfophase < 3.f){
        float z = lfophase - 2.f;
        _lfo = z*z - 1.f;
    } else {
        lfophase -= 4.f;
        float z = lfophase;
        _lfo = 1.f - z*z;
    }
    float lfo_slop = (_lfo - lfo)/numSamples;


    //PHASER
    float outleft = historyleft[PHASER_STAGES];
    float outright = historyright[PHASER_STAGES];
    while(numSamples--){

        //stages inputs
        outleft = *(outbufleft) + outleft*feedback;
        outright = *(outbufright) + outright*feedback;


        for(int j=0; j<PHASER_STAGES; ++j){
            DOPHASER(left);
            DOPHASER(right);

        }

        historyleft[PHASER_STAGES] = outleft;
        historyright[PHASER_STAGES] = outright;

        *outbufleft++ += outleft;
        *outbufright++ += outright;
    }

    lfo = _lfo;

}

