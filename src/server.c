#include "server.h"

local GameState gamestate = {0};

void server_init(){
	gamestate.map = map_generate(210,40);
	gamestate.entities[1] = (Entity){.type = ENTITY_PROJECTILE, .vx = 0.0, .x = 3.2, .y = 2.5, .projectileData.damage = 100};
}

void server_free(){
	gamestate = (GameState){0};
}

local bool entity_collision(Entity* e1, Entity* e2){
	return e1->x+e1->w > e2->x && e1->x < e2->x+e2->w && e1->y+e1->h > e2->y && e1->y < e2->y+e2->h;
}

bool server_new_entity(Entity entity){
	for(int i = 0; i < MAX_ENTITIES; i++){
		Entity *e = gamestate.entities+i;
		if(e->type == ENTITY_INVALID){
			*e = (Entity){0};
			*e = entity;
			return true;
		}
	}
	return false;
}

void server_update(){
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

					if(player->controlState.shoot && e->playerData.fireDelay <= 0.0){
						Entity projectile = (Entity){.type = ENTITY_PROJECTILE, .y = e->y+0.25*e->h, .vx = 40.0, .projectileData.damage = 40, .w=0.1, .h=0.1};
						e->playerData.fireDelay = 0.1;
						if(e->playerData.lookingRight){
							projectile.x = e->x+e->w*1.5;
						}
						else{
							projectile.x = e->x-e->w*0.5;
							projectile.vx *= -1;
						}
						server_new_entity(projectile);
					}
					else if(e->playerData.fireDelay > 0.0){
						e->playerData.fireDelay -= DT;
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
				}
				for(int j = 0; j < MAX_ENTITIES; j++){
					Entity* e2 = gamestate.entities+j;
					if(e2->type == ENTITY_PLAYER && entity_collision(e, e2)){
						printf("HIT! %i\n", j);
						if(e2->playerData.health < e->projectileData.damage){
							e2->playerData.health = 0;
						}	
						else{
							e2->playerData.health -= e->projectileData.damage;
						}
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
	for(int k = 0; k < MAX_PLAYERS; k++){
		Player *p = gamestate.players+k;
		if(p->valid && !p->hasEntity){
			if(p->respawnCounter > 0){
				p->respawnCounter -= DT;
			}
			else{
				Entity e = (Entity){.type = ENTITY_PLAYER, .playerData.playerId = k, .playerData.health = 100, .w = 1.0, .h = 2.0, .x = 5.0, .y = 2.0};
				p->hasEntity = server_new_entity(e);
			}
		}
	}
}

bool server_register_player(char *nick, u8 *playerId){
	for(int i = 0; i < MAX_PLAYERS; i++){
		if(!gamestate.players[i].valid){
			*playerId = i;
			strncpy(gamestate.players[i].nick, nick, 32);
			gamestate.players[i].nick[31] = 0;
			gamestate.players[i].valid = true;
			return true;
		}
	}
	return false;
}

void server_unregister_player(u8 playerId){
	gamestate.players[playerId].valid = false;
	for(int i = 0; i < MAX_ENTITIES; i++){
		Entity *e = gamestate.entities+i;
		if(e->type == ENTITY_PLAYER && e->playerData.playerId == playerId){
			e->type = ENTITY_INVALID;
		}
	}
}

void server_push_control_state(u8 playerId, ControlState controlState){
	gamestate.players[playerId].controlState = controlState;
}

GameState *server_get_gamestate(){
	return &gamestate;
}
