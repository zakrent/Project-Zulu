#ifndef Z_CLIENT_H
#define Z_CLIENT_H

#define MAX_PARTICLES 65536
#define MAX_PARTICLE_GROUPS 16

typedef struct Particle{
	bool valid;
	real32 x,y;
	real32 vx,vy;
	u32 life;
} Particle;

typedef struct ParticleGroup{
	bool valid;
	real32 x,y;
	real32 pw,ph;
	u32 groupLife;
	u32 particlesPerFrame;
	u32 particleLife;
	real32 particleInitialVel;
	real32 particleVelDecay;
	u32 firstParticle;
	u32 lastParticle;
	u32 particleCount;
	Particle particles[MAX_PARTICLES];
} ParticleGroup;

typedef struct ClientSideEffects{
	ParticleGroup particleGroups[MAX_PARTICLE_GROUPS];
} ClientSideEffects;

void client_init();
void client_free();
void client_update();
void client_update_controls();
void client_handle_event(SDL_Event *event);
void client_set_playerId(u8 playerId);
bool client_new_particle_group(ParticleGroup *npg);
void client_render();

#endif
