#include "devset.h"

//
// wifi heart beat time
//
uint16_t et_hbt_time;

// wifi udp heart beat
uint16_t et_udp_time;

//
// device id
//
uint64_t self_dev_id = 0x0201f0fff800;

//
// wifi heart beat sequence
//
uint64_t g_seq;
//uint64_t g_set_seq;
uint64_t g_list_seq;

DevSet_t devset;
DevState_t devstate;
