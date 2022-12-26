#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmp.h"
#include "packet_mgr.h"

void tcp_recv_callback(void* arg, uint8_t* buf, uint16_t len) {
    PACKET_MGR* handle = (PACKET_MGR*) arg;
    uint16_t type;
    memcpy(&type, buf, sizeof(type));
    if (type < MAX_PACKETS) {
        PACKET pt = handle->cmds[type];
        if (pt.unpack != NULL && pt.recv != NULL) {
            pt.unpack(&buf[sizeof(type)], handle->packet);
            pt.recv(handle, handle->packet);
        }
    }
    
}

bool packet_mgr_init(PACKET_MGR* handle) {
    NET_SERVER_INIT res = net_init(tcp_recv_callback, handle);
    handle->packet = malloc(MAX_PACKET_SIZE);
    if (handle->packet == NULL) {
        panic("Unable to allocate memory");
    }
    if (res.status == NET_STATUS_OK) {
        handle->tcp = res.handle;
        return true;
    }
    return false;
}


void packet_mgr_reg(PACKET_MGR* handle, uint16_t type, PACKET_MGR_PACK pack, PACKET_MGR_UNPACK unpack, PACKET_MGR_RECIVED recv) {
    PACKET* pt = &handle->cmds[type];
    
    pt->pack = pack;
    pt->unpack = unpack;
    pt->recv = recv;
}

bool packet_mgr_send(PACKET_MGR* handle, uint16_t type, void* obj) {
    if(type < MAX_PACKETS) {
        memcpy(handle->send_buf, &type, sizeof(type));
        PACKET* pt = &handle->cmds[type];
        if (pt->pack != NULL) {
            uint16_t len = pt->pack(obj, handle->send_buf + sizeof(type));
            if (net_send(handle->tcp, handle->send_buf, len + sizeof(type)))
            {
                return true;
            }
        }
        
    }
    return false;
}

void packet_mgr_pool(PACKET_MGR* handle) {
    net_poll(handle->tcp);
}



