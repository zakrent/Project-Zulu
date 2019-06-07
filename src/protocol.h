#ifndef Z_PROTOCOL_H
#define Z_PROTOCOL_H

#define PORT 2500

enum ServerMsgType{
	SMSG_INVALID,
	SMSG_CONNECTED,
	SMSG_GAMESTATE_UPDATE,
	SMSG_KICK,
	MAX_SMSG
};

enum ClientMsgType{
	CMSG_INVALID,
	CMSG_CONNECT,
	CMSG_CONTROLSTATE_UPDATE,
	MAX_CMSG
};

typedef struct CVConnectPacket{
	u8 type; //Must be CMSG_Connect
	char nick[32];
} CVConnectPacket;

typedef struct CVControlStatePacket{
	u8 type; //Must be CMSG_CONTROLSTATE_UPDATE
	ControlState cs;
} CVControlStatePacket;

typedef struct SVConnectedPacket{
	u8 type; //Must be SMSG_CONNECTED
	u8 playerId;
} SVConnectedPacket;

typedef struct SVGamestatePacket{
	u8 type; //Must be SMSG_GAMESTATE_UPDATE
	GameState gs; //TODO(zakrent): compress and delta encode gamestate	
} SVGamestatePacket;

#endif
