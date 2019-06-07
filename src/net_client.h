#ifndef Z_NET_SERVER_H
#define Z_NET_SERVER_H

#include "server.h"

void net_client_init();
void net_client_free();
void net_client_update();
void net_client_push_control_state(ControlState cs);
u8 net_client_get_playerId();
GameState *net_client_get_gamestate();

#endif
