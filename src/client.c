#include <SDL.h>
#include "client.h"
#include "globals.h"
#include "map.h"
#include "server.h"
#include "renderer.h"
#include "net_client.h"

local ControlState controlState = {0};
local u8 playerId = 0;

local GameState clientGamestate = {0};
local GameState serverGamestate = {0};

local ClientSideEffects clientSideEffects = {0};

local ParticleGroup explosionPGTemplate = (ParticleGroup){.valid = true, .x=0, .y=0, .ph=0.2, .pw=0.2, .groupLife = 10, .particlesPerFrame = 50, .particleLife = 20, .particleInitialVel = 20, .particleVelDecay = 0.2, .particleCount = 0};

void client_init(){
	if(listening){
		server_register_player("local", &playerId);
	}
	else{
		playerId = net_client_get_playerId();
	}
	controlState = (ControlState){0};
	controlState.valid = true;
	printf("ID: %u\n", playerId);

}

void client_free(){
	controlState = (ControlState){0};
	playerId = 0;
}

void client_update_controls(){
	if(listening){
		server_push_control_state(playerId, controlState);
	}
	else{
		net_client_push_control_state(controlState);
	}
	controlState.up = false;
}

void client_update_gamestate(){
	GameState gamestate = clientGamestate;
	for(int i = 0; i < MAX_ENTITIES; i++){
		Entity *e = gamestate.entities+i;
		switch(e->type){
		case ENTITY_PLAYER:
			{
				Player *player = gamestate.players+e->playerData.playerId;	
				if(player->valid && player->controlState.valid){
					static const real32 speed = 30.0;
					static const real32 accel = speed*4;
					static const real32 jumpVel = 30.0;
					e->x += 100*DT;
					e->vx += (e->vx < speed)*player->controlState.right*accel*DT-(e->vx > - speed)*player->controlState.left*accel*DT;
					e->vx -= 2.0*((e->vx>0)-(e->vx<0))*accel*DT*e->playerData.onGround*!player->controlState.left*!player->controlState.right;
					if(abs(e->vx) < 8.0 && !player->controlState.right && !player->controlState.left)
						e->vx = 0.0;
					if((e->playerData.onGround || e->playerData.canDoubleJump) && player->controlState.up){
						e->vy -= jumpVel;
						if(!e->playerData.onGround){
							e->playerData.canDoubleJump = false;
						}
					}

					if(player->controlState.right){
						e->playerData.lookingRight = true;
					}
					else if(player->controlState.left){
						e->playerData.lookingRight = false;
					}

					if(player->controlState.down && !e->playerData.crouching){
						e->playerData.crouching = true;
						e->h = 1.5;
						e->y += 0.5;
					}
					else if(!player->controlState.down && e->playerData.crouching){
						e->playerData.crouching = false;
						e->h = 2.0;
						e->y -= 0.5;
					}

					player->controlState.valid = false;
				}

				if(e->playerData.health == 0){
					e->type = ENTITY_INVALID;	
					if(player->valid){
						player->hasEntity = false;
						player->respawnCounter = 3.0;
					}
				}

				static const real32 g = 50.0;
				e->vy += g*DT;

				e->playerData.onGround = false;
				e->x += e->vx*DT;
				if(map_collision(&gamestate.map, e->x, e->y, e->w, e->h)){
					e->x -= e->vx*DT;
					e->vx = 0.0;
				}
				e->y += e->vy*DT;
				if(map_collision(&gamestate.map, e->x, e->y, e->w, e->h)){
					e->y -= e->vy*DT;
					if(e->vy > 0){
						e->playerData.onGround = true;
						e->playerData.canDoubleJump = true;
					}
					e->vy *= 0.5;
				}
				break;
			}
		case ENTITY_PROJECTILE:
			{
				e->x += e->vx*DT;
				e->y += e->vy*DT;
				if(map_collision(&gamestate.map, e->x, e->y, e->w, e->h)){
					e->type = ENTITY_INVALID;
					explosionPGTemplate.x = e->x;
					explosionPGTemplate.y = e->y;
					client_new_particle_group(&explosionPGTemplate);
				}
				for(int j = 0; j < MAX_ENTITIES; j++){
					Entity* e2 = gamestate.entities+j;
					if(e2->type == ENTITY_PLAYER && entity_collision(e, e2)){
						if(e2->playerData.health < e->projectileData.damage){
							e2->playerData.health = 0;
						}	
						else{
							e2->playerData.health -= e->projectileData.damage;
						}
						explosionPGTemplate.x = e->x;
						explosionPGTemplate.y = e->y;
						client_new_particle_group(&explosionPGTemplate);
						e->type = ENTITY_INVALID;
						break;
					}
				}
				break;
			}
		default:
			break;
		}
	}
	clientGamestate = gamestate;
}

void client_update(){
   	//Update gamestate client-side so we can 
	//get client side effects going
	client_update_gamestate();
	GameState newGamestate;
	if(listening){ 
		newGamestate = *server_get_gamestate();
	}
	else if(net_client_has_new_gamestate()){
		newGamestate = *net_client_get_gamestate();
		serverGamestate = newGamestate;
		//lerp newGamestate with old one
		for(int i = 0; i < MAX_ENTITIES; i++){
			Entity *e = clientGamestate.entities+i;
			Entity *ne = newGamestate.entities+i;
			if(e->type != ENTITY_INVALID //&& (e->x - ne->x > 20 || e->y - ne->y > 20 || e->x - ne->x > 2000 || e->y - ne->y > 2000)
					){
				ne->x = e->x*0.6 + ne->x*0.4;
				ne->y = e->y*0.6 + ne->y*0.4;
				ne->vx = e->vx*0.5 + ne->vx*0.5;
				ne->vy = e->vy*0.5 + ne->vy*0.5;
			}
		}
	}
	else{
		newGamestate = clientGamestate;
	}
	//Replace gamestate with newGamestate from the server, they should 
	//be nearly the same if everything's all right
	clientGamestate = newGamestate;
	//Why am I commenting this one function when I haven't commented 
	//anything else? IDK maybe i'm bored

	//update gamestate so we see the world as it is when our control
	//packet arrive AKA client side prediction
	//TODO: measure delay
	if(!listening){
		for(int i = 0; i < 0; i++){
			client_update_gamestate();
		}
	}

	//Update CS effects;
	for(int i = 0; i < MAX_PARTICLE_GROUPS; i++){
		ParticleGroup *pg = clientSideEffects.particleGroups+i;
		if(pg->valid){
			if(pg->groupLife){
				pg->groupLife--;
			}
			else if(pg->particleCount == 0){
				pg->valid = false;
			}
			else{
				printf("%u!\n", pg->particleCount);
				printf("%uf\n", pg->firstParticle);
				printf("%ul\n", pg->lastParticle);
				printf("%uv\n", pg->particles[pg->lastParticle].valid);
				printf("%ulif\n", pg->particles[pg->lastParticle].life);
			}

			for(int j = pg->firstParticle; j != pg->lastParticle+1; j++){
				if(j >= MAX_PARTICLES){
					j = 0;
				}
				Particle *p = pg->particles+j;
				p->x += p->vx*DT;
				p->y += p->vy*DT;
				p->vx *= 1.0-pg->particleVelDecay;
				p->vy *= 1.0-pg->particleVelDecay;
			}
			for(int j = pg->firstParticle; j != pg->lastParticle+1; j++){
				if(j >= MAX_PARTICLES){
					j = 0;
				}
				Particle *p = pg->particles+j;

				if(p->life){
					p->life--;
				}
				else if(p->valid){
					p->valid = false;
					pg->particleCount--;
					if(j == pg->firstParticle){
						while(!pg->particles[pg->firstParticle].valid && pg->particleCount != 0){
							pg->firstParticle++;
							if(pg->firstParticle >= MAX_PARTICLES){
								pg->firstParticle = 0;
							}
						}
					}
				}
			}

			if(pg->groupLife){
				u32 lastParticle = pg->lastParticle;
				for(int j = 0; j < pg->particlesPerFrame; j++){
					lastParticle++;
					if(pg->particleCount == 0){
						lastParticle = 0;
					}
					if(lastParticle >= MAX_PARTICLES){
						lastParticle = 0;
					}
					Particle *lp = pg->particles+lastParticle;
					real32 vx = randf(-1.0,1.0);
					real32 vy = randf(-1.0,1.0);
					real32 mxy = sqrtf(vx*vx+vy*vy);
					vx /= mxy;
					vy /= mxy;
					vx *= pg->particleInitialVel;
					vy *= pg->particleInitialVel;
					if(lp->valid){
						pg->particleCount--;
					}
					*lp = (Particle){.valid = true, .x = pg->x, .y = pg->y, .vx = vx, .vy = vy, .life = pg->particleLife*randf(0.8,1.0)};
					pg->particleCount++;
				}
				pg->lastParticle = lastParticle;
			}
		}
	}

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

bool client_new_particle_group(ParticleGroup *npg){
	for(int i = 0; i < MAX_PARTICLE_GROUPS; i++){
		ParticleGroup *pg = clientSideEffects.particleGroups+i;
		if(!pg->valid){
			*pg = *npg;
			return true;
		}
	}
	return false;
}

void client_render(){
	renderer_draw_map(clientGamestate.map);
	for(int i = 0; i < MAX_ENTITIES; i++){
		Entity *e = clientGamestate.entities+i;
		if(e->type != ENTITY_INVALID){
			real32 x = e->x;
			real32 y = e->y;
			renderer_draw_rect(e->x, e->y, e->w, e->h);
		}
	}
#if 0
	for(int i = 0; i < MAX_ENTITIES; i++){
		Entity *e = serverGamestate.entities+i;
		if(e->type != ENTITY_INVALID){
			real32 x = e->x;
			real32 y = e->y;
			renderer_draw_rectb(e->x, e->y, e->w, e->h);
		}
	}
#endif
	for(int i = 0; i < MAX_ENTITIES; i++){
		Entity *e = clientGamestate.entities+i;
		if(e->type == ENTITY_PLAYER && e->playerData.playerId == playerId){
			renderer_camera_track(e->x, e->y);
			break;
		}
	}

	for(int i = 0; i < MAX_PARTICLE_GROUPS; i++){
		ParticleGroup *pg = clientSideEffects.particleGroups+i;
		if(pg->valid){
			for(int j = pg->firstParticle; j != pg->lastParticle; j++){
				if(j >= MAX_PARTICLES){
					j = 0;
				}
				Particle *p = pg->particles+j;
				if(p->valid){
					renderer_draw_rectb(p->x, p->y, pg->pw, pg->ph);
				}
			}
		}
	}
	ui_render();
}
