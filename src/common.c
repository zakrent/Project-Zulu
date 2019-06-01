#include <stdlib.h>
#include "common.h"

void *_malloc(size_t size){
	return malloc(size);
}

void _free(void *ptr){
	free(ptr);
}
