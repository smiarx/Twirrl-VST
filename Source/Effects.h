#ifndef _EFFECTS_H
#define _EFFECTS_H

#include <cstdint>
#include <cmath>
#include <queue>
#include <iostream>

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
            float dsampl, dsampr;
            float ratetophaseinc;
            float lfo, lfophase, lfophaseinc, depth, predelay;


            int wphase;
            bool running;
        };

        struct Delay{
            Delay();
            ~Delay() {stop();};
            void clear(){
                    float *bfl=bufleft, *bfr=bufright;
                    for(int i=0; i<bufsize; ++i) *(bfl++)=*(bfr++)=0.;
                    speed=1.f;dsampinc=0.f;
                    while(!changes.empty()) changes.pop();
            };
            void start(double sampleRate, int samplePerBlock);
            void stop();
            void process(float* outleft, float* outright, int numSamples);

            void setDelay(float newdelay, float sR){
                if(!running){
                    timedelay=newdelay;
                    dsamp = newdelay*sR;
                    return;
                }

                float newspeed = timedelay/newdelay;
                int stop = wphase;
                speed *= newspeed;
                dsampinc = 1.f - speed;
                changes.push((speedchange) {newspeed,newdelay*sR,stop});
                timedelay=newdelay;
            };

            struct speedchange{
                float speed;
                float dsamp;
                int stop;
            };
            std::queue<speedchange> changes;
            float speed;
            float dsampinc;
            float timedelay;

            float *bufleft,*bufright;
            int bufsize, bufmask;
            float dsamp, feedback, wet;

            double dcy1left, dcy1right, dcx1left, dcx1right;
            double lpa0, lpb1, lpb2, lpy1left, lpy2left, lpy1right, lpy2right;
            double hpa0, hpb1, hpb2, hpy1left, hpy2left, hpy1right, hpy2right;

            int wphase;
            bool running;
        };



#define PHASER_STAGES 6
        struct Phaser {
            Phaser();
            void start(double sampleRate, int samplesPerBlock);
            void process(float* outleft, float* outright, int numSamples);
            void clear(void){ for(int j=0; j<PHASER_STAGES; j++) historyleft[j] = historyright[j] = 0.f;};

            float aleft[PHASER_STAGES], aright[PHASER_STAGES];//phaser coeff
            float depth[PHASER_STAGES];
            float historyleft[PHASER_STAGES+1], historyright[PHASER_STAGES+1];
            float feedback;

            float ratetophaseinc;
            float lfo, lfophase, lfophaseinc;

            bool running;

        };

        Effects(TwirrlAudioProcessor& parent);
        void start(double sampleRate, int samplesPerBlock);
        void stop() {chorus.stop(); delay.stop();};
        void process(float *outleft, float* outright, int numSamples);

        void setChorus(bool state) {chorus.running=state;};
        void setChorusRate(float rate){chorus.lfophaseinc = chorus.ratetophaseinc*rate;};

        void setPhaser(bool state)  {phaser.running=state; if(!state) phaser.clear();};
        void setPhaserRate(float rate){phaser.lfophaseinc = phaser.ratetophaseinc*rate;};
        void setPhaserFeedback(float feedback){phaser.feedback = feedback;};

        void setDelay(bool state)  {delay.running=state; if(!state) delay.clear();};
        void setDelayTime(float time){delay.setDelay(time,sampleRate);};
        void setDelayFeedback(float feedback)  {delay.feedback = feedback;};
        void setDelayWet(float wet)  {delay.wet = wet;};


    private:
        Chorus chorus;
        Phaser phaser;
        Delay delay;

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


inline float cubicinterp(float* tbl, int irphase1, int mask, float frac){
		long irphase2 = irphase1 - 1;
		long irphase3 = irphase1 - 2;
		long irphase0 = irphase1 + 1;

		float d0 = tbl[irphase0 & mask];
		float d1 = tbl[irphase1 & mask];
		float d2 = tbl[irphase2 & mask];
		float d3 = tbl[irphase3 & mask];

        float c0 = d1;
        float c1 = 0.5f * (d2 - d0);
        float c2 = d0 - 2.5f * d1 + 2.f * d2 - 0.5f * d3;
        float c3 = 0.5f * (d3 - d0) + 1.5f * (d1 - d2);

        return ((c3 * frac + c2) * frac + c1) * frac + c0;
}



inline float zapgremlins(float x)
{
	float absx = std::abs(x);
	// very small numbers fail the first test, eliminating denormalized numbers
	//    (zero also fails the first test, but that is OK since it returns zero.)
	// very large numbers fail the second test, eliminating infinities
	// Not-a-Numbers fail both tests and are eliminated.
	return (absx > (float)1e-15 && absx < (float)1e15) ? x : (float)0.;
}


inline double zapgremlins(double x)
{
	double absx = std::abs(x);
	// very small numbers fail the first test, eliminating denormalized numbers
	//    (zero also fails the first test, but that is OK since it returns zero.)
	// very large numbers fail the second test, eliminating infinities
	// Not-a-Numbers fail both tests and are eliminated.
	return (absx > (double)1e-15 && absx < (double)1e15) ? x : (double)0.;
}
#endif
