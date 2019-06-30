#include <SDL.h>
#include <enet/enet.h>
#include <x86intrin.h>
#include "globals.h"
#include "common.h"
#include "common.c"
#include "ui.c"
#include "server.c"
#include "client.c"
#include "map.c"
#include "renderer.c"
#include "resources.c"
#include "menu.c"
#include "net_server.c"
#include "net_client.c"

void main_switch_state(u8 nextState){
	switch(nextState){
		case STATE_QUIT:
			state = STATE_QUIT;
			break;
		case STATE_MENU:
			menu_free();
			menu_init();
			ui_free();
			ui_init();
			state = STATE_MENU;
			break;
		case STATE_GAME:
			if(listening){
				server_free();
				server_init();
				net_server_init();
			}
			else{
				net_client_init();
			}
			resources_free();
			resources_init();
			ui_free();
			ui_init();
			client_free();
			client_init();
			state = STATE_GAME;
			break;
		default:
			break;
	}
}

bool main_frame(void){
	if(dedicated){
		server_update();
	}
	else{
		switch(state){
			case STATE_START:
				main_switch_state(STATE_MENU);
				break;
			case STATE_QUIT:
				return false;
			case STATE_MENU:
				{
					u8 nextState = menu_update();
					if(nextState != STATE_MENU){
						main_switch_state(nextState);
					}
					renderer_start_frame();
					menu_render();
					renderer_end_frame();
					break;
				}
			case STATE_GAME:
				client_update_controls();
				if(listening){
					server_update();
					net_server_update();
				}
				renderer_start_frame();
				client_update();
				client_render();
				if(!listening)
					net_client_update();
				renderer_end_frame();
				break;
			default:
				state = STATE_MENU;
				break;
		}
	}
	return true;
}

void main_event(SDL_Event *event){
	if(!dedicated){
		switch(state){
			case STATE_MENU:
				ui_handle_event(event);
				break;
			case STATE_GAME:
				if(!ui_handle_event(event))
					client_handle_event(event);
				break;
			default:
				break;
		}
	}
}


int main(int argc, char **argv){
	SDL_Init(SDL_INIT_EVERYTHING);

	printf("S: %u\n", sizeof(GameState));

	listening = true;

	for(int i = 1; i < argc; i++){
		switch(argv[i][1]){
			case 'd':
				dedicated = true;
				break;
			case 'c':
				listening = false;
		}
	}

	u32 targetFrameTime = (u32)(DT*1000.0);
	bool running = true;
	u32 frameBegin = 0; 

	if(!dedicated){
		renderer_init(RESX, RESY);
	}
	else{
		server_init();
	}


	while(running){
		frameBegin = SDL_GetTicks();

		SDL_Event event = {0};
		while(SDL_PollEvent(&event)){
			main_event(&event);
			switch(event.type){
				case SDL_QUIT:
					running = false;
					break;
				default:
					break;
			}
		}

		running &= main_frame();

		u32 frameTime = SDL_GetTicks() - frameBegin;
		if(frameTime < targetFrameTime){
			SDL_Delay(targetFrameTime-frameTime);
		}
	}

	return 0;
}
