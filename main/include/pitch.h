#ifndef PITCH_H
#define PITCH_H

#include "math.h"

static long base_period[12] = { 
	61162,
	57703,
	54466,
	51414,
	48544,
	45809,
	43234,
	40816,
	38521,
	36364,
	34317,
	32394
	};

long period(int note) {
	return base_period[note%12] / pow(2, (note/12));
}

#endif