#include "net_client.h"
#include <enet/enet.h>
#include "common.h"
#include "protocol.h"

GameState netState = {0};

ENetHost *client;
ENetPeer *peer;

void net_client_init(){
	client = enet_host_create (NULL, 1, 1, 0, 0);
	ENetAddress address;
	address.port = PORT;
	enet_address_set_host_ip(&address, "127.0.0.1");
	peer = enet_host_connect(client, &address, 1, 0); 
	ENetEvent event;
	if (enet_host_service (client, & event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT){
		puts("Connection succeeded.\n");
	}
	else{
		puts("Connection not succeeded.\n");
	}
}

void net_client_free(){
	enet_host_destroy(client);
}

void net_client_update(){
	ENetEvent event;
	while (enet_host_service (client, &event, 0) > 0){
		switch (event.type){
			case ENET_EVENT_TYPE_RECEIVE:
				printf ("A packet of length %u \n",event.packet -> dataLength);
				GameState *newState = event.packet->data;
				netState = *newState;
				/* Clean up the packet now that we're done using it. */
				enet_packet_destroy (event.packet);
				break;
			case ENET_EVENT_TYPE_DISCONNECT:
				break;
			default:
				break;
		}
	}
}

void net_client_push_control_state(ControlState cs){
	ENetPacket *packet = enet_packet_create(&cs, sizeof(ControlState), 0);
	enet_peer_send(peer, 0, packet);
}

GameState *net_client_get_gamestate(){
	return &netState;
}
