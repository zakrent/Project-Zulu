#include "common.h"
#include "renderer.h"
#include "resources.h"

local SDL_Window   *window;
local SDL_Renderer *renderer;

local real32 aspectRatio = 1.0;
//window size
local u32 ww, wh;
//camera rect
local real32 cx, cy, cw, ch;

void renderer_init(u32 w, u32 h){
	ww = w;
	wh = h;
	SDL_CreateWindowAndRenderer(w,h, 0, &window, &renderer);
	aspectRatio = w/(1.0*h);
	int scale = 1;
	do{
		renderer_set_camera(0.0, 0.0, (wh*1.0/MAP_TILE_HEIGHT)/scale);
		scale++;
	}while(ch > 50);
}

void renderer_start_frame(){
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
}

void renderer_end_frame(){
	SDL_RenderPresent(renderer);
}

void renderer_set_camera(real32 x, real32 y, real32 h){
	cx = x;
	cy = y;
	cw = h*aspectRatio;
	ch = h;
}

void renderer_camera_track(real32 x, real32 y){
	real32 dx = x-cx-cw*0.5;
	real32 dy = y-cy-ch*0.5;
	cx += 10.0*dx*DT;
	cy += 10.0*dy*DT;
}

void renderer_draw_rect(real32 x, real32 y, real32 w, real32 h){
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	u32 tx, ty, tw, th;
	tx = (u32)((x - cx)/cw*ww+0.5);
	ty = (u32)((y - cy)/ch*wh+0.5);
	tw = (u32)(w/cw*ww+0.5);
	th = (u32)(h/ch*wh+0.5);
	const SDL_Rect rect = {tx, ty, tw, th};
	SDL_RenderDrawRect(renderer, &rect);
}

void renderer_draw_tex(u8 texType, u32 texX, u32 texY, u32 texW, u32 texH, real32 x, real32 y, real32 w, real32 h){
	SDL_Texture *t = textures[texType];
	if(t){
		u32 tx, ty, tw, th;
		tx = (u32)((x - cx)/cw*ww+0.5);
		ty = (u32)((y - cy)/ch*wh+0.5);
		tw = (u32)(w/cw*ww+0.5);
		th = (u32)(h/ch*wh+0.5);
		const SDL_Rect dstrect = {tx, ty, tw, th};
		const SDL_Rect srcrect = {texX, texY, texW, texH};
		SDL_RenderCopy(renderer, t, &srcrect, &dstrect);
	}
	else{
		renderer_draw_rect(x,y,w,h);
	}
}

void renderer_draw_map(Map map){
	static SDL_Texture* t;
	static u32 lastMapVersion;
	if(map.version > lastMapVersion || !t){
		if(!t){
			t = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_RGBA4444,SDL_TEXTUREACCESS_TARGET,map.w*MAP_TILE_WIDTH,map.h*MAP_TILE_HEIGHT);
		}
		SDL_SetRenderTarget(renderer, t);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		u32 size = map.w*map.h;
		SDL_Texture *tileTex = textures[TEX_MAP_TILES];
		for(int i = 0; i < size; i++){
			u32 px = MAP_TILE_WIDTH*(i%map.w);
			u32 py = MAP_TILE_HEIGHT*((i-(i%map.w))/map.w);
			const SDL_Rect rect = {px, py, 32, 32};
			if(map.tiles[i] != TILE_AIR){
				if(tileTex){
					const SDL_Rect tileSrcrect = {MAP_TILE_WIDTH*(map.tiles[i]+0), 0, MAP_TILE_WIDTH, MAP_TILE_HEIGHT};
					SDL_RenderCopy(renderer, tileTex, &tileSrcrect, &rect);
				}
				else
					SDL_RenderDrawRect(renderer, &rect);
			}
		}
		SDL_SetRenderTarget(renderer, 0);
	}
	u32 tx, ty, tw, th;
	tx = (u32)((0.0 - cx)/cw*ww+0.5);
	ty = (u32)((0.0 - cy)/ch*wh+0.5);
	tw = (u32)(map.w/cw*ww+0.5);
	th = (u32)(map.h/ch*wh+0.5);
	const SDL_Rect dstrect = {tx, ty, tw, th};
	const SDL_Rect srcrect = {0, 0, map.w*MAP_TILE_WIDTH, map.h*MAP_TILE_HEIGHT};
	//const SDL_Rect dstrect = srcrect;
	SDL_RenderCopy(renderer, t, &srcrect, &dstrect);
}

SDL_Texture* renderer_load_tex(SDL_Surface* s){
	return SDL_CreateTextureFromSurface(renderer, s);
}
