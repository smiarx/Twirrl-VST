#ifndef _LUT_H
#define _LUT_H


#define LUTSineSize 8192
#define LUTSineMask (LUTSineSize-1)

#define LUTInterp(tlb,pfrac) (tbl[0] + (tbl[1]-tbl[0])*pfrac)


#include <cstdint>
#include <iostream>


extern float lutSine[LUTSineSize+1];
extern float lutInvSine[LUTSineSize+1];


const float invSineBad = 1e20f; // Value for 1/sin(x) when sin(x)~=0

void lutInit();



inline float PhaseFrac(uint32_t inPhase)
{
	union { uint32_t itemp; float ftemp; } u;
	u.itemp = 0x3F800000 | (0x007FFF80 & ((inPhase)<<7));
	return u.ftemp - 1.f;
}

#endif
