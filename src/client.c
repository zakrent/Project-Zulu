#include <SDL.h>
#include "client.h"
#include "globals.h"
#include "map.h"
#include "server.h"
#include "renderer.h"
#include "net_client.h"

local ControlState controlState;
local u8 playerId;

void client_init(){
	if(listening){
		server_register_player("local", &playerId);
	}
	else{
	}
	controlState = (ControlState){0};
	controlState.valid = true;
	printf("ID: %u\n", playerId);
}

void client_free(){
	controlState = (ControlState){0};
	playerId = 0;
}

void client_update(){
	if(listening){
		//server_push_control_state(playerId, controlState);
	}
	else{
		net_client_push_control_state(controlState);
	}
	controlState.up = false;
	ui_update();
}

void client_handle_event(SDL_Event *event){
	switch(event->type){
		case SDL_KEYDOWN:
			if(event->key.repeat == false){
				switch(event->key.keysym.sym){
					case SDLK_SPACE:
						controlState.shoot = true;
						break;
					case SDLK_w:
						controlState.up = true;
						break;
					case SDLK_d:
						controlState.right = true;
						break;
					case SDLK_a:
						controlState.left = true;
						break;
					case SDLK_s:
						controlState.down = true;
						break;
					default:
						break;
				}
			}
			break;
		case SDL_KEYUP:
			if(event->key.repeat == false){
				switch(event->key.keysym.sym){
					case SDLK_SPACE:
						controlState.shoot = false;;
						break;
					case SDLK_w:
						controlState.up = false;
						break;
					case SDLK_d:
						controlState.right = false;
						break;
					case SDLK_a:
						controlState.left = false;
						break;
					case SDLK_s:
						controlState.down = false;
						break;
					default:
						break;
				}
			}
			break;
		default:
			break;
	}
}

void client_render(){
	GameState *gamestate;
	if(listening){ 
		gamestate = server_get_gamestate();
	}
	else{
		gamestate = net_client_get_gamestate();
	}
	renderer_draw_map(gamestate->map);
	for(int i = 0; i < MAX_ENTITIES; i++){
		Entity *e = gamestate->entities+i;
		if(e->type != ENTITY_INVALID){
			real32 x = e->x;
			real32 y = e->y;
			renderer_draw_rect(e->x, e->y, e->w, e->h);
		}
	}
	for(int i = 0; i < MAX_ENTITIES; i++){
		Entity *e = gamestate->entities+i;
		if(e->type == ENTITY_PLAYER && e->playerData.playerId == playerId){
			renderer_camera_track(e->x, e->y);
			break;
		}
	}
	ui_render();
}
