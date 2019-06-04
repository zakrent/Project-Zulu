#include "net_server.h"
#include <enet/enet.h>
#include "common.h"
#include "protocol.h"
#include "server.h"

enum NetClientState{
	NC_INVALID,
	NC_CONNECTING,
	NC_CONNECTED,
	MAX_NC
};

typedef struct NetClientData{
	u8 state;	
	u8 playerId;
} NetClientData;

ENetHost *server;
ENetAddress serverAddress;

void net_server_init(){
	serverAddress.host = ENET_HOST_ANY;
	serverAddress.port = PORT;
	server = enet_host_create (&serverAddress, 
								MAX_PLAYERS+10, //10 more for future queue
								1,
								0,
								0);
}

void net_server_free(){
	enet_host_destroy(server);
}

void net_server_update(){
	ENetPacket * packet = enet_packet_create(server_get_gamestate(), sizeof(GameState), 0);
	enet_host_broadcast(server, 0, packet);
	ENetEvent event;
	while (enet_host_service (server, &event, 0) > 0){
		switch (event.type){
			case ENET_EVENT_TYPE_CONNECT:
				printf ("A new client connected from %x:%u.\n", 
				event.peer->address.host,
				event.peer->address.port);
				NetClientData *peerData = _malloc(sizeof(NetClientData));
				peerData->state = NC_CONNECTING;
				event.peer->data = peerData;
				break;
			case ENET_EVENT_TYPE_RECEIVE:
				{
					ControlState *cs = (ControlState*)event.packet->data;
					server_push_control_state(0, *cs);
					enet_packet_destroy (event.packet);
					break;
				}
			case ENET_EVENT_TYPE_DISCONNECT:
				printf ("%s disconnected.\n", event.peer -> data);
				/* Reset the peer's client information. */
				NetClientData *data = event.peer->data;
				server_unregister_player(data->playerId);
				_free(data);
				event.peer->data = NULL;
				break;
			default:
				break;
		}
	}
}
