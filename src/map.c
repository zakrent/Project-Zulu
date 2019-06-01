#include "map.h"
#include "common.h"
#include "resources.h"
#include "renderer.h"

u8 *map_tile(Map map, u32 x, u32 y){
	return map.tiles+x+y*map.w;
}

Map map_generate(u32 w, u32 h){
	Map map = (Map){.w = w, .h = h};
	map.tiles = _malloc(w*h*sizeof(u8));
	for(int x = 0; x < w; x++){
		for(int y = 0; y < h; y++){
			if(h-y < 5 || x == 0 || y == 0 || x == w-1){
				*map_tile(map, x, y) = TILE_STONE;
			}
			else if(h-y == 10 && w-x < 8){
				*map_tile(map, x, y) = TILE_STONE;
			}
			else{
				*map_tile(map, x, y) = TILE_AIR;
			}
		}
	}
	return map;
}

bool map_collision_p(Map map, real32 x, real32 y){
	if(x > map.w || y > map.h || y < 0 || x < 0)
		return false;
	return *map_tile(map, (u32)x, (u32)y) == TILE_STONE;
}

bool map_collision(Map map, real32 x, real32 y, real32 w, real32 h){
	for(u32 mx = (u32)x; mx <= (u32)x+(u32)w; mx++){
		for(u32 my = (u32)y; my <= (u32)y+(u32)h; my++){
			if(*map_tile(map, mx, my) == TILE_STONE)
				return true;
		}
	}
	return false;
}
