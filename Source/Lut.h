#ifndef _LUT_H
#define _LUT_H


#define Q16MUL(x,y) (((int64_t) (x))*((int64_t) (y)) >> 16)

#define LUTSineSize ((int32_t) 8192)
#define LUTSineMask ((int32_t) (LUTSineSize-1))
#define LOOKUP(table, phase) ((table) + ((((phase)>>16) & LUTSineMask)<<1))


#define LUTMidiPrec 6
#define LUTMidiSize ((int32_t) 128<<LUTMidiPrec)
#define LUTMidiIndex(x) (((x) >> (16-LUTMidiPrec))<<1)
//#define LUTMidiMask ((int32_t) (LUTMidiSize-1))


#define LUTInterp(tlb,pfrac) (tbl[0] + tbl[1]*pfrac)



#include <cstdint>
#include <iostream>



extern float lutSine[(LUTSineSize<<1)+1];
extern float lutInvSine[(LUTSineSize<<1)+1];

const float invSineBad = 1e20f; // Value for 1/sin(x) when sin(x)~=0


extern float lutMidi[(LUTMidiSize<<1)-1];


void lutInit();



inline float PhaseFrac(uint32_t inPhase)
{
	union { uint32_t itemp; float ftemp; } u;
	u.itemp = 0x3F800000 | (0x007FFF80 & ((inPhase)<<7));
	return u.ftemp - 1.f;
    //return (float) (inPhase & 0x00007FFF)/(1<<17);
}


inline float MidiFrac(uint32_t inMidi)
{
    return (float) (inMidi & ((1<<(16-LUTMidiPrec)) -1))/(1<<(16-LUTMidiPrec));
}

#endif
