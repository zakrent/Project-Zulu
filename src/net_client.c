#include "net_client.h"
#include <enet/enet.h>
#include "common.h"
#include "protocol.h"

GameState netState = {0};

ENetHost *client;
ENetPeer *peer;
local u8 netPlayerId = 0;

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

	CVConnectPacket connectPacket = (CVConnectPacket){.type = CMSG_CONNECT, .nick="REMOTE"};
	ENetPacket *packet = enet_packet_create(&connectPacket, sizeof(CVConnectPacket), 0);
	enet_peer_send(peer, 0, packet);

	while(enet_host_service (client, & event, 5000) > 0 && event.type == ENET_EVENT_TYPE_RECEIVE && *((u8*)event.packet->data) == SMSG_CONNECTED){
		SVConnectedPacket *cpacket = (SVConnectedPacket*)event.packet->data;
		netPlayerId = cpacket->playerId;
	}
}

u8 net_client_get_playerId(){
	return netPlayerId;
}

void net_client_free(){
	enet_host_destroy(client);
}

void net_client_update(){
	ENetEvent event;
	while (enet_host_service (client, &event, 0) > 0){
		switch (event.type){
			case ENET_EVENT_TYPE_RECEIVE:
				{
					GameState *newState = event.packet->data;
					netState = *newState;
					enet_packet_destroy (event.packet);
					break;
				}
			case ENET_EVENT_TYPE_DISCONNECT:
				break;
			default:
				break;
		}
	}
}

void net_client_push_control_state(ControlState cs){
	CVControlStatePacket packetData;
	packetData.type = CMSG_CONTROLSTATE_UPDATE;
	packetData.cs = cs;
	ENetPacket *packet = enet_packet_create(&packetData, sizeof(CVControlStatePacket), 0);
	enet_peer_send(peer, 0, packet);
}

GameState *net_client_get_gamestate(){
	return &netState;
}
