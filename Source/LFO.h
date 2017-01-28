#ifndef _LFO_H
#define _LFO_H



class LFO{
    public:
        LFO();
        ~LFO() {delete buf;};
        void start(float sampleRate, int samplesPerBlock, float rate);
        void process();
        void updateRate(float rate){phaseinc = ratetophaseinc*rate;};
        float* getBuf(){return buf;};

    private:
        float* buf;
        int bufSize;
        float ratetophaseinc;
        float phaseinc;
        float phase;
};

#endif
