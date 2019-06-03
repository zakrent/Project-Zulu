#include "resources.h"

void resources_init(){
	resources_load_tex(TEX_MAP_TILES, "MAP_DEFAULT.bmp");
}

void resources_free(){
	for(int i = 0; i < MAX_TEX; i++){
		if(textures[i] != NULL){
			SDL_DestroyTexture(textures[i]);
		}
	}
}

void resources_load_tex(u8 texType, char* filename){
	SDL_Surface* tempSurface = NULL;
	tempSurface = SDL_LoadBMP(filename);
	printf("TEMP: %u\n", tempSurface);
	textures[texType] = renderer_load_tex(tempSurface);
	SDL_FreeSurface(tempSurface);
}
