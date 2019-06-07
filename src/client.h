#ifndef Z_CLIENT_H
#define Z_CLIENT_H

void client_init();
void client_free();
void client_update();
void client_handle_event(SDL_Event *event);
void client_set_playerId(u8 playerId);
void client_render();

#endif
