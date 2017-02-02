#include "Voicer.h"
#include "Voice.h"

#include <iostream>
//Inspired by Pure Data poly element


Voicer::Voicer(Voice** voices): voices(voices), serial(0)
{
}




void Voicer::noteOn(int note, int vel){

    Voice *vc=*voices, *firston=nullptr, *firstoff=nullptr;
    uint32_t serialon, serialoff;
    serialon = serialoff = 0xFFFFFFFF;

    for(int i=0; i< NVOICES; ++vc, ++i){
        if(firstoff==nullptr && vc->isRunning() && vc->getSerial() < serialon)
            firston=vc, serialon=vc->getSerial();
        else if(!vc->isRunning() && vc->getSerial() < serialoff){
            firstoff=vc, serialoff=vc->getSerial();
        }
    }

    if(firstoff != nullptr){
        firstoff->start(note);
        firstoff->setSerial(serial++);
    } else {
        firston->start(note);
        firston->setSerial(serial++);
    }

}

void Voicer::noteOff(int note){
    Voice *vc=*voices, *firston=nullptr;
    int32_t serialon = 0xFFFFFFFF;

    for(int i=0; i<NVOICES; ++vc, ++i)
        if(vc->getGate() && vc->getMidiNote() == note && vc->getSerial() < serialon)
            firston = vc, serialon=vc->getSerial();
    if(firston != nullptr){
        firston->release();
        firston->setSerial(serial++);
    }
}
