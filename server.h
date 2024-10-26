#ifndef SERVER_H
#define SERVER_H

#include "duckchat.h"
#include "shared.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 4949
#define SERVER_IP "127.0.0.1"

// Largest packet server recieves is say_request
// 4 byte =  packet identifier
// 32 byte = channel name
// 64 byte = text field
#define SERVER_BUFFER_SIZE (4 + 32 + 64)

typedef struct _channel channel;

typedef struct _user {
  char username[USERNAME_MAX_CHAR];
  char ip[INET_ADDRSTRLEN];
  unsigned short port;
  int n_subbed_channels;
  channel **subbed_channels;
} user;

typedef struct _channel {
  char name[CHANNEL_MAX_CHAR];
  int n_subbed_users;
  user **subbed_users;
} channel;

typedef struct _users {
  int num_users;
  user *users;
} users;

typedef struct _channels {
  int num_channels;
  channel *channels;
} channels;

void print_user(user *user) {
  if (user == NULL) {
    printf("User is NULL\n");
    return;
  }

  printf("[USER](%s, %s, %d)\n", user->username, user->ip, user->port);
  printf("↪[CHANNELS](");
  if (user->subbed_channels == NULL) {
    printf("NONE");
  } else {
    for (int i = 0; user->subbed_channels[i] != NULL; i++) {
      printf("%s, ", user->subbed_channels[i]->name);
    }
  }
  printf("\b\b)\n");
}

void print_users(users *users) {
  if (users == NULL) {
    printf("Users struct is NULL\n");
    return;
  }

  printf("=== USERS LIST ===\n");
  printf("Total Users: %d\n\n", users->num_users);

  if (users->num_users == 0) {
    printf("No users registered\n");
    return;
  }

  for (int i = 0; i < users->num_users; i++) {
    // printf("User %d:\n", i + 1);
    print_user(&users->users[i]);
  }
  printf("================\n\n");
}

void print_channel(channel *channel) {
  if (channel == NULL) {
    printf("Channel is NULL\n");
    return;
  }

  printf("[CHANNEL](%s)\n", channel->name);
  printf("↪[USERS](");
  if (channel->subbed_users == NULL) {
    printf("NONE");
  } else {
    for (int i = 0; channel->subbed_users[i] != NULL; i++) {
      printf("%s, ", channel->subbed_users[i]->username);
    }
  }
  printf("\b\b)\n");
}

void print_channels(channels *channels) {
  if (channels == NULL) {
    printf("Channels struct is NULL\n");
    return;
  }

  printf("=== CHANNELS LIST ===\n");
  printf("Total Channels: %d\n\n", channels->num_channels);

  if (channels->num_channels == 0) {
    printf("No channels exist\n");
    return;
  }

  for (int i = 0; i < channels->num_channels; i++) {
    // printf("Channel %d:\n", i + 1);
    print_channel(&channels->channels[i]);
  }
  printf("===================\n\n");
}

void print_request_login(request_login *req) {
  printf("REQUEST LOGIN:\n");
  printf("  Type: %d (REQ_LOGIN)\n", req->req_type);
  printf("  Username: %s\n", req->username);
}

void print_request_logout(request_logout *req) {
  printf("REQUEST LOGOUT:\n");
  printf("  Type: %d (REQ_LOGOUT)\n", req->req_type);
}

void print_request_join(request_join *req) {
  printf("REQUEST JOIN:\n");
  printf("  Type: %d (REQ_JOIN)\n", req->req_type);
  printf("  Channel: %s\n", req->channel);
}

void print_request_leave(request_leave *req) {
  printf("REQUEST LEAVE:\n");
  printf("  Type: %d (REQ_LEAVE)\n", req->req_type);
  printf("  Channel: %s\n", req->channel);
}

void print_request_say(request_say *req) {
  printf("REQUEST SAY:\n");
  printf("  Type: %d (REQ_SAY)\n", req->req_type);
  printf("  Channel: %s\n", req->channel);
  printf("  Text: %s\n", req->text);
}

void print_request_list(request_list *req) {
  printf("REQUEST LIST:\n");
  printf("  Type: %d (REQ_LIST)\n", req->req_type);
}

void print_request_who(request_who *req) {
  printf("REQUEST WHO:\n");
  printf("  Type: %d (REQ_WHO)\n", req->req_type);
  printf("  Channel: %s\n", req->channel);
}

void print_request_keep_alive(request_keep_alive *req) {
  printf("REQUEST KEEP ALIVE:\n");
  printf("  Type: %d (REQ_KEEP_ALIVE)\n", req->req_type);
}

// Helper function to print any request type
void print_request(request *req) {
  switch (req->req_type) {
  case REQ_LOGIN:
    print_request_login((request_login *)req);
    break;
  case REQ_LOGOUT:
    print_request_logout((request_logout *)req);
    break;
  case REQ_JOIN:
    print_request_join((request_join *)req);
    break;
  case REQ_LEAVE:
    print_request_leave((request_leave *)req);
    break;
  case REQ_SAY:
    print_request_say((request_say *)req);
    break;
  case REQ_LIST:
    print_request_list((request_list *)req);
    break;
  case REQ_WHO:
    print_request_who((request_who *)req);
    break;
  case REQ_KEEP_ALIVE:
    print_request_keep_alive((request_keep_alive *)req);
    break;
  default:
    printf("UNKNOWN REQUEST TYPE: %d\n", req->req_type);
  }
}

void print_client_details(struct sockaddr_in *client) {
  char ip_str[INET_ADDRSTRLEN];

  // Convert IP address to string
  inet_ntop(AF_INET, &(client->sin_addr), ip_str, INET_ADDRSTRLEN);

  // Get port number (convert from network byte order to host byte order)
  unsigned short port = ntohs(client->sin_port);

  printf("CLIENT DETAILS\nIP: %s\nPORT: %d\n", ip_str, port);
}

channel *create_channel(channels *channels, const char *channel_name) {
  if (channels == NULL || channel_name == NULL) {
    return NULL;
  }

  // Reallocate channels array to make room for new channel
  channels->channels = (channel *)realloc(channels->channels, (channels->num_channels + 1) * sizeof(channel));

  if (channels->channels == NULL) {
    return NULL;
  }

  // Initialize the new channel
  channel *new_channel = &channels->channels[channels->num_channels];

  // Copy channel name
  strncpy(new_channel->name, channel_name, CHANNEL_MAX_CHAR);
  new_channel->name[CHANNEL_MAX_CHAR - 1] = '\0'; // Ensure null termination

  // Initialize subscribed users array
  new_channel->subbed_users = NULL;

  // Increment channel count
  channels->num_channels++;

  return new_channel;
}

channel *find_channel(channels *channels, const char *channel_name) {
  if (channels == NULL || channel_name == NULL) {
    return NULL;
  }

  for (int i = 0; i < channels->num_channels; i++) {
    if (strcmp(channels->channels[i].name, channel_name) == 0) {
      return &channels->channels[i];
    }
  }

  return NULL; // Channel not found
}

int send_error(int sockfd, struct sockaddr_in *client, char *error_reason) {
  text_error error_packet;
  error_packet.txt_type = TXT_ERROR;
  strncpy(error_packet.txt_error, error_reason, sizeof(error_packet.txt_error));

  if ((sendto(sockfd, &error_packet, sizeof(error_packet), 0, (const struct sockaddr *)client, sizeof(*client))) < 0) {
    perror("Failed to send to packet to client");
    return EXIT_FAILURE;
  }

  return SUCCESS;
}

int join_channel(channels *channels, user *target_user, const char *channel_name) {
  // if (channels == NULL || target_user == NULL || channel_name == NULL) {
  //   return FAILURE;
  // }

  // Find the channel
  channel *target_channel = find_channel(channels, channel_name);
  if (target_channel == NULL) {
    return FAILURE;
  }

  // Check if user is already subscribed to this channel
  for (int i = 0; i < target_user->n_subbed_channels; i++) {
    if (strcmp(target_user->subbed_channels[i]->name, channel_name) == 0) {
      return FAILURE;
    }
  }

  // Add channel to user's channels
  target_user->subbed_channels =
      (channel **)realloc(target_user->subbed_channels, ((target_user->n_subbed_channels + 1) * sizeof(channel *)));
  if (target_user->subbed_channels == NULL) {
    perror("(SERVER) >>> Realloc failed on subbed_channels");
    return FAILURE;
  }
  target_user->subbed_channels[target_user->n_subbed_channels] = target_channel;
  target_user->n_subbed_channels++;

  // Add user to channel
  target_channel->subbed_users = (user **)realloc(target_channel->subbed_users, ((target_channel->n_subbed_users + 1) * sizeof(user *)));
  if (target_channel->subbed_users == NULL) {
    perror("(SERVER) >>> Realloc failed on subbed_users");
    return FAILURE;
  }
  target_channel->subbed_users[target_channel->n_subbed_users] = target_user;
  target_channel->n_subbed_users++;

  return SUCCESS;
}

#endif
