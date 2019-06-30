#include <stdlib.h>
#include "common.h"

void *_malloc(size_t size){
	return malloc(size);
}

void _free(void *ptr){
	free(ptr);
}

local bool randInit = false;
real32 randf(real32 min, real32 max){
	if(!randInit){
		srand(100);
		randInit = true;
	}
	return (real32)rand()/(real32)(RAND_MAX)*(max-min)+min;
}
