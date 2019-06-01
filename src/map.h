#ifndef Z_MAP_H
#define Z_MAP_H

#define TILE_AIR 0
#define TILE_STONE 1

typedef struct Map{
	u32 w, h;
	u32 version;
	u8 *tiles;
} Map;

Map map_load(char *filename);
Map map_generate(u32 w, u32 h);
bool map_collision_p(Map map, real32 x, real32 y);
bool map_collision(Map map, real32 x, real32 y, real32 w, real32 h);
void map_free(Map map);

u8 *map_tile(Map map, u32 x, u32 y);

#endif
