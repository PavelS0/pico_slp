#include <string.h>

#include "packet_mgr.h"
#include "cmp.h"

/* static cmp_ctx_t cmp = {0};

static bool file_reader(cmp_ctx_t *ctx, void *data, size_t limit) {
    return read_bytes(data, limit);
}

static bool file_skipper(cmp_ctx_t *ctx, size_t count) {
    
}

static size_t file_writer(cmp_ctx_t *ctx, const void *data, size_t count) {
    memcpy(ctx->buf, data, count);
    return count;
}

static void error_and_exit(const char *msg) {
    fprintf(stderr, "%s\n\n", msg);
    while (true);
}
 */


typedef struct {
    uint8_t arr[32];
} test_struct_t;

uint16_t control_pack_test_packet(void* o, uint8_t* buf) {
    test_struct_t* p = (test_struct_t*) o;
    uint8_t i;
    for (i = 0; i < 32; i++) {
        memcpy(&buf[i * sizeof(i)], &p->arr[i], sizeof(i));
    }
    return 32;
}

void* control_unpack_test_packet(uint8_t* buf, void* o) {
    test_struct_t* p = (test_struct_t*) o;
    uint8_t i;
    for (i = 0; i < 32; i++) {
        memcpy(&p->arr[i], &buf[i * sizeof(i)], sizeof(i));
    }
    p->arr[0] = 0;
    return NULL;
}

void control_recv_test_packet(PACKET_MGR* mgr, void* o) {
    test_struct_t* p = (test_struct_t*) o;
    p->arr[0] = 0;
    packet_mgr_send(mgr, 0, p);
}


void control_init() {
    
}

