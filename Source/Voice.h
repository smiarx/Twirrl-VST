#ifndef _OSC_H
#define _OSC_H

#include "Lut.h"
#include "Constants.h"
#include "PluginProcessor.h"
#include <cmath>


#define NVOICES 1


class Voice{

    public:
    class Osc{
        public:
            Osc(Voice& vc, double sampleRate);
            
            void process(float* buf, int numSamples);
            void update();

        private:
            Voice& voice;

            int32_t phase; // Q15 fixed float
            int32_t phaseinc;
            int32_t N2;//number of harmonics (odd)
            double freqtophaseinc;
            float scale;

            float y;
            float leak;

    };  

    class VCF{
        public:
            VCF(Voice& vc, float cutoff, float nk);

            void process(float* buf, int numSumples);
            void update();
            void updateCutoff(float ctoff);
            void updateRes(float res);

        private:
            Voice& voice;
            float cutoff, k;
            double b0, b0p4, a1;
            double s1,s2,s3,s4; //first order filter state

    };

    public:
        Voice(TwirrlAudioProcessor& prt, double sR);

        void process(float* buf, int numSamples);
        void start(float fr);
        bool isRunning(){ return running;};
        void updateParameter(ParamID id, float value);

    private:
        TwirrlAudioProcessor& parent;
        Osc osc;
        VCF vcf;

        bool running;
        float freq;
        double sampleRate;
        double sampleDur;
};






#endif
