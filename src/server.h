#ifndef Z_SERVER_H
#define Z_SERVER_H

#include "common.h"
#include "map.h"
#define MAX_ENTITIES 1024
#define MAX_PLAYERS 16

enum EntityType{
	ENTITY_INVALID,
	ENTITY_PLAYER,
	ENTITY_PROJECTILE,
	MAX_ENTITY_TYPES
};

typedef struct Entity{
	real32 x,y;
	real32 w,h;
	real32 vx,vy;	
	u8 type;
	union{
		struct{
			bool onGround      :1;
			bool canDoubleJump :1;
			bool lookingRight  :1;
			bool crouching  :1;
			u8 playerId;
			u8 health;
			real32 fireDelay;
		} playerData;
		struct{
			u8 damage;
		} projectileData;
	};
} Entity;

typedef struct ControlState{
	bool valid :1;
	bool up    :1;
	bool down  :1;
	bool right :1;
	bool left  :1;
	bool shoot :1;
} ControlState;

typedef struct Player{
	char nick[32];
	real32 respawnCounter;
	bool valid     :1;
	bool hasEntity :1;
	ControlState controlState;
} Player;

typedef struct GameState{
	Map map;
	Entity entities[MAX_ENTITIES];
	Player players[MAX_PLAYERS];
} GameState;

void server_init();
void server_free();
void server_update();
bool server_new_entity(Entity entity);
GameState *server_get_gamestate();
bool server_register_player(char *nick, u8 *playerId);
void server_unregister_player(u8 playerId);
void server_push_control_state(u8 playerId, ControlState controlState);

#endif
