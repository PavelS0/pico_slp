#ifndef PACKET_MGR_H_
#define PACKET_MGR_H_

#include "net.h"

typedef struct _PACKET_MGR PACKET_MGR;

typedef uint16_t (*PACKET_MGR_PACK)(void* o, uint8_t* buf);
typedef void* (*PACKET_MGR_UNPACK)(uint8_t* buf, void* o);
typedef void (*PACKET_MGR_RECIVED)(PACKET_MGR* mgr, void* o);


#define MAX_PACKETS 64
#define MAX_PACKET_SIZE 2048

typedef struct
{
    PACKET_MGR_PACK pack;
    PACKET_MGR_UNPACK unpack;
    PACKET_MGR_RECIVED recv;
} PACKET;

typedef struct _PACKET_MGR
{
    NET_SERVER* tcp;
    uint8_t send_buf[MAX_PACKET_SIZE];
    void* packet;
    PACKET cmds[MAX_PACKETS];
} PACKET_MGR;

void packet_mgr_reg(PACKET_MGR* handle, uint16_t type, PACKET_MGR_PACK pack, PACKET_MGR_UNPACK unpack, PACKET_MGR_RECIVED recv);
bool packet_mgr_send(PACKET_MGR* handle, uint16_t type, void* obj);
bool packet_mgr_init(PACKET_MGR* handle);
void packet_mgr_pool(PACKET_MGR* handle);
#endif /* PACKET_MGR_H_ */