#include <string.h>
#include <stdlib.h>

#include "net/net.h"
#include "pico/cyw43_arch.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#define TCP_PORT 4242
#define DEBUG_printf printf
#define BUF_SIZE 2048
#define TEST_ITERATIONS 10
#define POLL_TIME_S 5


static NET_SERVER* tcp_server_init(void) {
    NET_SERVER *state = calloc(1, sizeof(NET_SERVER));
    if (!state) {
        DEBUG_printf("failed to allocate state\n");
        return NULL;
    }
    state->complete = false;
    return state;
}

static err_t tcp_server_close(void *arg) {
    NET_SERVER *state = (NET_SERVER*)arg;
    err_t err = ERR_OK;
    if (state->client_pcb != NULL) {
        tcp_arg(state->client_pcb, NULL);
        tcp_poll(state->client_pcb, NULL, 0);
        tcp_sent(state->client_pcb, NULL);
        tcp_recv(state->client_pcb, NULL);
        tcp_err(state->client_pcb, NULL);
        err = tcp_close(state->client_pcb);
        if (err != ERR_OK) {
            DEBUG_printf("close failed %d, calling abort\n", err);
            tcp_abort(state->client_pcb);
            err = ERR_ABRT;
        }
        state->client_pcb = NULL;
    }
    if (state->server_pcb) {
        tcp_arg(state->server_pcb, NULL);
        tcp_close(state->server_pcb);
        state->server_pcb = NULL;
    }
    return err;
}

static err_t tcp_server_result(void *arg, int status) {
    NET_SERVER *state = (NET_SERVER*)arg;
    if (status == 0) {
        DEBUG_printf("test success\n");
    } else {
        DEBUG_printf("test failed %d\n", status);
    }
    state->complete = true;
    return tcp_server_close(arg);
}

static err_t tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
    NET_SERVER *state = (NET_SERVER*)arg;
    DEBUG_printf("tcp_server_sent %u\n", len);
    state->sent_len += len;
    return ERR_OK;
}

err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    NET_SERVER *state = (NET_SERVER*)arg;
    if (!p) {
        return ERR_OK; //tcp_server_result(arg, -1);
    }
    // this method is callback from lwIP, so cyw43_arch_lwip_begin is not required, however you
    // can use this method to cause an assertion in debug mode, if this method is called when
    // cyw43_arch_lwip_begin IS needed
    cyw43_arch_lwip_check();
    if (p->tot_len > 0) {
        // handle new packet
        if (state->recv_packet_len == 0) {
            state->recv_len += pbuf_copy_partial(p, &state->recv_packet_len, sizeof(state->recv_packet_len), 0);
        }
        DEBUG_printf("tcp_server_recv %d/%d err %d\n", p->tot_len, state->recv_len, err);

        // Receive the buffer
        const uint16_t buffer_left = state->recv_packet_len - state->recv_len;
        state->recv_len += pbuf_copy_partial(p, state->buffer_recv + state->recv_len,
                                             p->tot_len > buffer_left ? buffer_left : p->tot_len, 0);
        tcp_recved(tpcb, p->tot_len);
    }
    
    pbuf_free(p);
    // Have we have received the whole buffer
    if (state->recv_len == state->recv_packet_len + sizeof(state->recv_packet_len)) {
        state->recv_callback(state->recv_arg, state->buffer_recv, state->recv_packet_len);
        state->recv_len = 0;
        state->recv_packet_len = 0;
    }

    return ERR_OK;
}

static err_t tcp_server_poll(void *arg, struct tcp_pcb *tpcb) {
    DEBUG_printf("tcp_server_poll_fn\n");
    return ERR_OK;
}

static void tcp_server_err(void *arg, err_t err) {
    if (err != ERR_ABRT) {
        DEBUG_printf("tcp_client_err_fn %d\n", err);
        tcp_server_result(arg, err);
    }
}

static err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err) {
    NET_SERVER *state = (NET_SERVER*)arg;
    if (err != ERR_OK || client_pcb == NULL) {
        DEBUG_printf("Failure in accept\n");
        tcp_server_result(arg, err);
        return ERR_VAL;
    }
    DEBUG_printf("Client connected\n");

    state->client_pcb = client_pcb;
    tcp_arg(client_pcb, state);
    tcp_sent(client_pcb, tcp_server_sent);
    tcp_recv(client_pcb, tcp_server_recv);
    tcp_poll(client_pcb, tcp_server_poll, POLL_TIME_S * 2);
    tcp_err(client_pcb, tcp_server_err);

    return err;
}

static bool tcp_server_open(void *arg) {
    NET_SERVER *state = (NET_SERVER*)arg;
    DEBUG_printf("Starting server at %s on port %u\n", ip4addr_ntoa(netif_ip4_addr(netif_list)), TCP_PORT);

    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) {
        DEBUG_printf("failed to create pcb\n");
        return false;
    }

    err_t err = tcp_bind(pcb, NULL, TCP_PORT);
    if (err) {
        DEBUG_printf("failed to bind to port %d\n");
        return false;
    }

    state->server_pcb = tcp_listen_with_backlog(pcb, 1);
    if (!state->server_pcb) {
        DEBUG_printf("failed to listen\n");
        if (pcb) {
            tcp_close(pcb);
        }
        return false;
    }

    tcp_arg(state->server_pcb, state);
    tcp_accept(state->server_pcb, tcp_server_accept);

    return true;
}

NET_SERVER_INIT net_init(NET_RECIVED recv_callback, void* recv_arg) {
    NET_SERVER_INIT init;
    if (cyw43_arch_init()) {
        printf("failed to initialise\n");
        
        init.status = NET_STATUS_ARCH_INIT_FAILED;
        return init;
    }

    cyw43_arch_enable_sta_mode();

    printf("Connecting to WiFi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        char* ch = WIFI_SSID;
        printf("failed to connect.\n");
        init.status = NET_STATUS_WIFI_CONNECT_FAILED;
        return init;
    } else {
        printf("Connected.\n");
    }

    // Running tcp server
    NET_SERVER *state = tcp_server_init();
    if (!state) {
        init.status = NET_STATUS_TCP_SERVER_INIT_FAILED;
        return init;
    }
    if (!tcp_server_open(state)) {
        tcp_server_result(state, -1);
        init.status = NET_STATUS_TCP_SERVER_OPEN_FAILED;
        return init;
    }

    state->recv_callback = recv_callback;
    state->recv_arg = recv_arg;
    init.status = NET_STATUS_OK;
    init.handle = state;

    return init;
}


bool net_poll(NET_SERVER* state) {
    cyw43_arch_poll();  
}


bool net_send(NET_SERVER* state, uint8_t* buf, uint16_t len)
{   
    memcpy(state->buffer_sent, &len, sizeof(len));
    memcpy(state->buffer_sent, &buf[sizeof(len)], sizeof(uint8_t) * len);

    state->sent_len = 0;
    state->sent_buf_len = len;

    DEBUG_printf("Writing %ld bytes to client\n", len);
    // this method is callback from lwIP, so cyw43_arch_lwip_begin is not required, however you
    // can use this method to cause an assertion in debug mode, if this method is called when
    // cyw43_arch_lwip_begin IS needed
    cyw43_arch_lwip_check();
    err_t err = tcp_write(state->client_pcb, state->buffer_sent, len, TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK) {
        DEBUG_printf("Failed to write data %d\n", err);
        tcp_server_result(state, -1);
        return false;
    }
    return true;
}


void net_deinit(NET_SERVER* state) {
    free(state);
    cyw43_arch_deinit();
}
