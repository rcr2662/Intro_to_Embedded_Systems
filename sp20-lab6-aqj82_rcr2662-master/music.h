#ifndef MUSIC_H
#define MUSIC_H
#include <stdint.h>
#include "Sound.h"
//timer0A is 32 bits, counts down at 80MHz
//12.5ns*2^32 = 53 seconds

#define bus 80000000
//cut time is quarter note is 250ms
#define quarter (bus/4)
#define half (quarter*2)
#define eighth (quarter/2)

struct note{
	uint32_t period; //number of 12.5ns ticks
	uint32_t length;
};

typedef const struct note note_t;


void Music_Init(void);
void Music_Play (note_t song[]);

#endif

