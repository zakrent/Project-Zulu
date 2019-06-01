#ifndef Z_RESOURCES_H
#define Z_RESOURCES_H

#include <SDL.h>
#include "common.h"

#define MAP_TILE_WIDTH 32
#define MAP_TILE_HEIGHT 32

enum TextureType{
	TEX_SPRITES,
	TEX_MAP_TILES,
	MAX_TEX
};

SDL_Texture* textures[MAX_TEX];

void resources_init();
void resources_load_tex(u8 texType, char* filename);

#endif
