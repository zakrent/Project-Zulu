#include "net_client.h"
#include <enet/enet.h>
#include "common.h"
#include "protocol.h"

GameState netState = {0};
local bool hasNewGamestate = false;

ENetHost *client;
ENetPeer *peer;
local u8 netPlayerId = 0;
local u32 lastClientGsId = 0;

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
					u8 *packetType = (u8*)event.packet->data;
					switch(*packetType){
						case SMSG_GAMESTATE_NEW:
							{
								SVGamestatePacket *gp = (SVGamestatePacket*)event.packet->data;
								netState = gp->gs;
								lastClientGsId = gp->lastGsId;
								CVGamestateAckPacket gap = (CVGamestateAckPacket){.type = CMSG_GAMESTATE_ACK, .lastGsId = lastClientGsId};
								ENetPacket *packet = enet_packet_create(&gap, sizeof(CVGamestateAckPacket), ENET_PACKET_FLAG_RELIABLE);
								enet_peer_send(peer, 0, packet);
								hasNewGamestate = true;
								break;
							}
						case SMSG_GAMESTATE_UPDATE:
							{
								SVGamestateUpdatePacket *gup = (SVGamestateUpdatePacket*)event.packet->data;
								if(gup->lastGsId > lastClientGsId){
									for(int i = 0; i < MAX_PLAYERS; i++){
										netState.players[i] = gup->players[i];
									}

									for(int i = 0; i < gup->nEntityUpdates; i++){
										netState.entities[gup->entityUpdates[i].entityIdx] = gup->entityUpdates[i].entity;
									}

									lastClientGsId = gup->lastGsId;
									CVGamestateAckPacket gap = (CVGamestateAckPacket){.type = CMSG_GAMESTATE_ACK, .lastGsId = lastClientGsId};
									ENetPacket *packet = enet_packet_create(&gap, sizeof(CVGamestateAckPacket), ENET_PACKET_FLAG_RELIABLE);
									enet_peer_send(peer, 0, packet);
									hasNewGamestate = true;
								}
								break;
							}
						default:
							break;
					}
					break;
				}
			case ENET_EVENT_TYPE_DISCONNECT:
				break;
			default:
				break;
		}
		enet_packet_destroy(event.packet);
	}
}

void net_client_push_control_state(ControlState cs){
	CVControlStatePacket packetData;
	packetData.type = CMSG_CONTROLSTATE_UPDATE;
	packetData.cs = cs;
	ENetPacket *packet = enet_packet_create(&packetData, sizeof(CVControlStatePacket), 0);
	enet_peer_send(peer, 0, packet);
}

bool net_client_has_new_gamestate(){
	return hasNewGamestate;
}

GameState *net_client_get_gamestate(){
	hasNewGamestate = false;
	return &netState;
}
