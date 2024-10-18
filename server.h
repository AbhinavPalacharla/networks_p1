#ifndef SERVER_H
#define SERVER_H

#include "duckchat.h"

#define PORT 4949
#define SERVER_IP "127.0.0.1"

// Largest packet server recieves is say_request
// 4 byte =  packet identifier
// 32 byte = channel name
// 64 byte = text field
#define SERVER_BUFFER_SIZE (4 + 32 + 64)

#endif
