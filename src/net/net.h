#ifndef NET_H
#define NET_H

#define NET_STATUS_OK 0
#define NET_STATUS_ARCH_INIT_FAILED 1
#define NET_STATUS_WIFI_CONNECT_FAILED 2
#define NET_STATUS_TCP_SERVER_INIT_FAILED 3
#define NET_STATUS_TCP_SERVER_OPEN_FAILED 4

#include "pico/stdlib.h"

#define NET_BUF_SIZE 2048

typedef uint8_t NET_STATUS;

typedef void (*NET_RECIVED)(void* arg, uint8_t* buf, uint16_t len);

typedef struct NET_SERVER_ {
    struct tcp_pcb *server_pcb;
    struct tcp_pcb *client_pcb;
    bool complete;
    uint8_t buffer_sent[NET_BUF_SIZE];
    uint8_t buffer_recv[NET_BUF_SIZE];
    int sent_buf_len;
    int sent_len;
    int recv_packet_len;
    int recv_len;
    int run_count;
    NET_STATUS status;
    NET_RECIVED recv_callback;
    void* recv_arg;
} NET_SERVER;



typedef struct NET_SERVER_INIT_ {
    NET_SERVER* handle;
    NET_STATUS status;
} NET_SERVER_INIT;


NET_SERVER_INIT net_init(NET_RECIVED recv_callback, void* recv_arg);
bool net_poll(NET_SERVER*);
void net_deinit(NET_SERVER* );
bool net_send(NET_SERVER*, uint8_t* buf, uint16_t len);
#endif