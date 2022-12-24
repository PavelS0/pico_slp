#ifndef CONTROL_H
#define CONTROL_H

uint16_t control_pack_test_packet(void* o, uint8_t* buf);
void* control_unpack_test_packet(uint8_t* buf);
void control_recv_test_packet(void* o);

#endif