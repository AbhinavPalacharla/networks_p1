#ifndef CLIENT_H
#define CLIENT_H

#include "duckchat.h"
#include "shared.h"
#include <stdio.h>

#define SERVER_PORT 4949
#define SERVER_IP "127.0.0.1"

// Largest packet is list packet sent from server to client
//  4 bytes = 32-bit message type identifier
//  4 byte = int total number of channels
// CHANNEL_MAX * 32 = one 32 byte slot for each channel
#define CLIENT_BUFFER_SIZE (4 + 4 + (MAX_NUM_CHANNELS * CHANNEL_MAX_CHAR))

#endif
