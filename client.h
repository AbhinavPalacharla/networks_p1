#ifndef CLIENT_H
#define CLIENT_H

#include "duckchat.h"
#include "shared.h"
#include <stdio.h>

// Largest packet is list packet sent from server to client
//  4 bytes = 32-bit message type identifier
//  4 byte = int total number of channels
// CHANNEL_MAX * 32 = one 32 byte slot for each channel
#define CLIENT_BUFFER_SIZE (4 + 4 + (MAX_NUM_CHANNELS * CHANNEL_MAX_CHAR))

typedef struct _subbed_channel {
  char channel_name[CHANNEL_MAX_CHAR];
  struct _subbed_channel *prev;
  struct _subbed_channel *next;
} subbed_channel;

typedef struct _user {
  char username[USERNAME_MAX_CHAR];
  subbed_channel *current_channel;
  subbed_channel *subbed_channels_head;
} user;

#endif
