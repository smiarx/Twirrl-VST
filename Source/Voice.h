#ifndef _OSC_H
#define _OSC_H

#include "Lut.h"
#include "Constants.h"
#include <cmath>


#define ENVCURVE (-4.f)

class TwirrlAudioProcessor;


class Voice{

    public:


        struct Env{
            Env(Voice& vc, int samplesPerBlock, float sR, float a, float d, float s, float r);
            ~Env();
            void attack(){ gate=true; changeStage(0);};
            void release(){ gate=false; changeStage(3);};
            void changeStage(int stg);
            void process(int numSamples);

            Voice& voice;
            bool gate;
            float* buf;
            float level,s;
            float a2,b1,growth;
            int stage;
            int32_t count;
            int32_t a,d,r;
        };

        struct Osc{
            Osc(Voice& vc, double sampleRate, float vib, float sawlvl, float sqlvl);

            void process(int numSamples);
            void update();

            Voice& voice;

            int32_t phase; // Q15 fixed float
            int32_t phaseinc;
            int32_t N2;//number of harmonics (odd)
            double freqtophaseinc;
            float scale;

            int32_t vibrato;
            float sawlvl, sqlvl;
            float saw, sq;
            float leak;


        };

        struct  VCF{
            VCF(Voice& vc, float cutoff, float nk);

            void process(int numSumples);
            void update();

            Voice& voice;
            int32_t midibase, cutoff, lfomod;
            float k;
            double b0, b0p4, a1;
            double s1,s2,s3,s4; //first order filter state

        };

        Voice(TwirrlAudioProcessor& prt, double sR, int sPB, int32_t* lfoBuf);
        ~Voice();

        void process(float* outbuf, int numSamples);
        void start(int midinote);
        void release(){ env.release();};
        void stop(){ running=false;};
        bool isRunning(){ return running;};
        bool getGate() { return env.gate;};
        int getMidiNote() {return midinum;};
        uint32_t getSerial(){ return serial;};
        void setSerial(uint32_t srl){serial=srl;};


        void updateVibrato(float vib){ osc.vibrato=vib*(1<<16);}
        void updateCutoff(float ctoff){ vcf.cutoff=ctoff*VCFMidiMul;}
        void updateVCFLFO(float lfomod){ vcf.lfomod=lfomod*VCFMidiMul;}
        void updateRes(float res){ vcf.k=res;}
        void updateAttack(float a){ env.a=static_cast<int32_t>(a*sampleRate);}
        void updateDecay(float d){ env.d=static_cast<int32_t>(d*sampleRate);}
        void updateSustain(float s){ env.s=s;}
        void updateRelease(float r){ env.r=static_cast<int32_t>(r*sampleRate);}
        void updateSaw(float saw){ osc.sawlvl=saw;}
        void updateSq(float sq){ osc.sqlvl=sq;}

    private:
        TwirrlAudioProcessor& parent;
        Env env;
        Osc osc;
        VCF vcf;

        bool running;
        float* audiobuf;
        uint32_t serial;//serial number
        int32_t* lfoBuf;
        float freq;
        int midinum;
        float* midilut;

        int samplesPerBlock;
        double sampleRate;
        double sampleDur;
};






#endif
