#ifndef _VOICER_H
#define _VOICER_H

#define NVOICES 12
#include <cstdint>

class Voice;

class Voicer{

    public:
        Voicer(Voice** voices);
        void noteOn(int note, int vel);
        void noteOff(int note);

    private:
        Voice** voices;
        uint32_t serial;

};



#endif
