#include <SDL.h>
#include "common.c"
#include "server.c"
#include "client.c"
#include "map.c"
#include "renderer.c"
#include "resources.c"

int main(){
	SDL_Init(SDL_INIT_EVERYTHING);
	printf("%i\n", sizeof(GameState));

	u32 targetFrameTime = (u32)(DT*1000.0);
	bool running = true;
	u32 frameBegin = 0; 

	renderer_init(1920, 1080);
	resources_init();
	server_init();
	client_init();

	while(running){
		frameBegin = SDL_GetTicks();

		SDL_Event event = {0};
		while(SDL_PollEvent(&event)){
			client_handle_event(&event);
			switch(event.type){
				case SDL_QUIT:
					running = false;
					break;
				default:
					break;
			}
		}

		client_update();
		server_update();

		renderer_start_frame();
		client_render();
		renderer_end_frame();

		u32 frameTime = SDL_GetTicks() - frameBegin;
		if(frameTime < targetFrameTime){
			SDL_Delay(targetFrameTime-frameTime);
		}
	}

	return 0;
}
