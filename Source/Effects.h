#ifndef _EFFECTS_H
#define _EFFECTS_H


#define CHORUS_DEPTH 0.002
#define CHORUS_PREDELAY 0.01


class TwirrlAudioProcessor;

class Effects{
    public:

        //stereo choruses
        struct Chorus{
            void start(double sampleRate, int samplesPerBlock);
            void stop();
            Chorus();
            ~Chorus() {stop();};
            void process(float* outleft, float* outright, int numSamples);

            float* bufleft, *bufright;
            int bufsize, bufmask;
            float ratetophaseinc;
            int lfo, lfophase, lfophaseinc, depth, predelay;

            int wphase;
            bool running;
        };

        Effects(TwirrlAudioProcessor& parent);
        void start(double sampleRate, int samplesPerBlock);
        void stop() {chorus.stop();};
        void process(float *outleft, float* outright, int numSamples);

        void setChorus(bool state) {chorus.running=state;};


    private:
        Chorus chorus;

        TwirrlAudioProcessor& parent;
        double sampleRate;
        int samplesPerBlock;

};



inline int nextpow2(int v){
    --v;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    return ++v;
}


inline float cubicinterp(float* tbl, int rphase1, int mask, float frac){
        int rphase0 = rphase1-1;
        int rphase2 = rphase1+1;
        int rphase3 = rphase1+2;

        float d0 = tbl[rphase0 & mask];
        float d1 = tbl[rphase1 & mask];
        float d2 = tbl[rphase2 & mask];
        float d3 = tbl[rphase3 & mask];

        float c0 = d1;
        float c1 = 0.5f * (d2 - d0);
        float c2 = d0 - 2.5f * d1 + 2.f * d2 - 0.5f * d3;
        float c3 = 0.5f * (d3 - d0) + 1.5f * (d1 - d2);

        return ((c3 * frac + c2) * frac + c1) * frac + c0;
}

#endif
