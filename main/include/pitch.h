#ifndef PITCH_H
#define PITCH_H

static long base_period[11] = { 
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
	return base_period[note%11] / (note/11 + 1);
}

#endif