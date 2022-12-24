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

uint16_t control_pack_test_packet(void* o, uint8_t* buf) {
    //cmp_init(&cmp, buf, file_reader, file_skipper, file_writer);
    return 32;
}

void* control_unpack_test_packet(uint8_t* buf) {
   
}

void control_recv_test_packet(void* o) {
   
}


void control_init() {
    
}

