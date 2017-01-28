#ifndef _OSC_H
#define _OSC_H

#include "Lut.h"
#include "Constants.h"
#include "PluginProcessor.h"
#include <cmath>


#define ENVCURVE (-4.f)


class Voice{

    public:


        struct Env{
            Env(Voice& vc, int samplesPerBlock, float sR, float a, float d, float s, float r);
            ~Env();
            void attack();
            void release();
            void changeStage(int stg);
            void process(int numSamples);

            Voice& voice;
            float* buf;
            float level,s;
            float a2,b1,growth;
            int stage;
            int32_t count;
            int32_t a,d,r;
        };

        struct Osc{
            Osc(Voice& vc, double sampleRate, float sawlvl, float sqlvl);

            void process(float* buf, int numSamples);
            void update();

            Voice& voice;

            int32_t phase; // Q15 fixed float
            int32_t phaseinc;
            int32_t N2;//number of harmonics (odd)
            double freqtophaseinc;
            float scale;

            float sawlvl, sqlvl;
            float saw, sq;
            float leak;

        };

        struct  VCF{
            VCF(Voice& vc, float cutoff, float nk);

            void process(float* buf, int numSumples);
            void update();
            void updateCutoff(float ctoff);
            void updateRes(float res);

            Voice& voice;
            float cutoff, k;
            double b0, b0p4, a1;
            double s1,s2,s3,s4; //first order filter state

        };

        Voice(TwirrlAudioProcessor& prt, double sR, int sPB, float* lfoBuf);

        void process(float* buf, int numSamples);
        void start(float fr);
        void release();
        void stop();
        bool isRunning(){ return running;};

        void updateCutoff(float ctoff){ vcf.cutoff=ctoff; vcf.update();}
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
        float* lfoBuf;
        float freq;
        int samplesPerBlock;
        double sampleRate;
        double sampleDur;
};






#endif