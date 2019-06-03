#ifndef Z_GLOBALS_H
#define Z_GLOBALS_H
#include "common.h"

enum state{
	STATE_START,
	STATE_QUIT,
	STATE_MENU,
	STATE_GAME,
	MAX_STATES
};

static u8 state;
bool dedicated;
bool listening;
//TODO: implement settings
#define RESX 1280
#define RESY 720

#endif
