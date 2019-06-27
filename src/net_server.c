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
	u32 lastGsId;
} NetClientData;

local ENetHost *server;
local ENetAddress serverAddress;

#define GS_BUFFER_SIZE 60

local GameState gsBuffer[GS_BUFFER_SIZE];
local GameState* lastGs = gsBuffer;
local u32 lastGsId = 0;

void net_server_init(){
	serverAddress.host = ENET_HOST_ANY;
	serverAddress.port = PORT;
	server = enet_host_create (&serverAddress, 
								MAX_PLAYERS+10, //10 more for future queue
								1,
								0,
								0);
	*lastGs = *server_get_gamestate();
}

void net_server_free(){
	enet_host_destroy(server);
}

void net_server_update(){
	lastGs++;
	lastGsId++;
	if(lastGs >= gsBuffer + GS_BUFFER_SIZE){
		lastGs = gsBuffer;
	}
	*lastGs = *server_get_gamestate();
	
	ENetEvent event;
	while (enet_host_service (server, &event, 0) > 0){
		switch (event.type){
			case ENET_EVENT_TYPE_CONNECT:
				{
					NetClientData *peerData = _malloc(sizeof(NetClientData));
					*peerData = (NetClientData){};
					peerData->state = NC_CONNECTING;
					event.peer->data = peerData;
					break;
				}
			case ENET_EVENT_TYPE_RECEIVE:
				{
					u8 *packetType = (u8*)event.packet->data;
					NetClientData *peerData = event.peer->data;
					switch(*packetType){
						case CMSG_CONTROLSTATE_UPDATE:
							{
								if(event.packet->dataLength = sizeof(CVControlStatePacket) && peerData->state == NC_CONNECTED){
									CVControlStatePacket *cspacket = (CVControlStatePacket*)event.packet->data;
									ControlState cs = cspacket->cs;
									server_push_control_state(peerData->playerId, cs);
								}
								break;
							}
						case CMSG_CONNECT:
							{
								if(event.packet->dataLength = sizeof(CVConnectPacket) && peerData->state == NC_CONNECTING){
									CVConnectPacket *cpacket = (CVConnectPacket*)event.packet->data;
									SVConnectedPacket cdpacket;
									cdpacket.type = SMSG_CONNECTED;
									if(server_register_player(cpacket->nick, &(cdpacket.playerId))){
										peerData->state = NC_CONNECTED;
										peerData->playerId = cdpacket.playerId;
										ENetPacket *packet = enet_packet_create(&cdpacket, sizeof(SVConnectedPacket), 0);
										enet_peer_send(event.peer, 0, packet);
									}
								}
								break;
							}
						case CMSG_GAMESTATE_ACK:
							{
								if(event.packet->dataLength = sizeof(CVGamestateAckPacket) && peerData->state == NC_CONNECTED){
									CVGamestateAckPacket *gapacket = (CVGamestateAckPacket*)event.packet->data;
									peerData->lastGsId = gapacket->lastGsId;
								}
								break;
							}
						default:
							break;
					}
					enet_packet_destroy (event.packet);
					break;
				}
			case ENET_EVENT_TYPE_DISCONNECT:
				{
					NetClientData *data = event.peer->data;
					server_unregister_player(data->playerId);
					_free(data);
					event.peer->data = NULL;
					break;
				}
			default:
				break;
		}
	}

	for(int i = 0; i < server->connectedPeers; i++){
		ENetPeer *p = server->peers+i;
		NetClientData *pd = p->data;
		if(pd->lastGsId > lastGsId - GS_BUFFER_SIZE + 1){
			SVGamestateUpdatePacket *gup = _malloc(sizeof(SVGamestateUpdatePacket) + MAX_ENTITIES*sizeof((SVGamestateUpdatePacket *)0)->entityUpdates[0]);
			*gup = (SVGamestateUpdatePacket){.type = SMSG_GAMESTATE_UPDATE, .lastGsId = lastGsId, .nEntityUpdates = 0};
			GameState *lastPeerGs = lastGs - (lastGsId - pd->lastGsId);
			if(lastPeerGs < gsBuffer){
				lastPeerGs = gsBuffer + GS_BUFFER_SIZE - (gsBuffer - lastPeerGs);
			}

			for(int i = 0; i < MAX_PLAYERS; i++){
				gup->players[i] = lastGs->players[i];
			}

			for(int i = 0; i < MAX_ENTITIES; i++){
				if(lastGs->entities[i].type != ENTITY_INVALID || lastPeerGs->entities[i].type != ENTITY_INVALID){
					gup->entityUpdates[gup->nEntityUpdates].entityIdx = i;
					gup->entityUpdates[gup->nEntityUpdates].entity = lastGs->entities[i];
					gup->nEntityUpdates++;
				}
			}
			ENetPacket *packet = enet_packet_create(gup, sizeof(SVGamestateUpdatePacket)+gup->nEntityUpdates*sizeof((SVGamestateUpdatePacket *)0)->entityUpdates[0], 0);
			enet_peer_send(p, 0, packet);
			_free(gup);
		}
		else{
			SVGamestatePacket gp = (SVGamestatePacket){.type = SMSG_GAMESTATE_NEW, .gs = *lastGs, .lastGsId = lastGsId};
			ENetPacket *packet = enet_packet_create(&gp, sizeof(SVGamestatePacket), 0);
			enet_peer_send(p, 0, packet);
		}
	}
}
