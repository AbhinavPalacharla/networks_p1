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

typedef struct _subbed_channel {
  channel *channel;
  struct _subbed_channel *prev;
  struct _subbed_channel *next;
} subbed_channel;

typedef struct _user {
  char username[USERNAME_MAX_CHAR];
  char ip[INET_ADDRSTRLEN];
  unsigned short port;
  subbed_channel *subbed_channels_head;
  struct _user *next;
  struct _user *prev;
} user;

typedef struct _subbed_user {
  user *user;
  struct _subbed_user *next;
  struct _subbed_user *prev;
} subbed_user;

typedef struct _channel {
  char name[CHANNEL_MAX_CHAR];
  subbed_user *subbed_users_head;
  struct _channel *next;
  struct _channel *prev;
} channel;

typedef struct _users {
  int num_users;
  user *users_head;
} users;

typedef struct _channels {
  int num_channels;
  channel *channels_head;
} channels;

channel *find_channel(channels *channel_list, const char *channel_name) {
  channel *current_channel = channel_list->channels_head;
  while (current_channel != NULL) {
    if (strncmp(current_channel->name, channel_name, CHANNEL_MAX_CHAR) == 0) {
      return current_channel;
    }
    current_channel = current_channel->next;
  }
  return NULL; // Channel not found
}

int create_channel(channels *channel_list, const char *channel_name) {
  if (find_channel(channel_list, channel_name)) {
    perror("Channel already exists");
    return FAILURE;
  }

  channel *new_channel = (channel *)malloc(sizeof(channel));

  strncpy(new_channel->name, channel_name, CHANNEL_MAX_CHAR);
  new_channel->subbed_users_head = NULL;
  new_channel->next = channel_list->channels_head;
  new_channel->prev = NULL;

  if (channel_list->channels_head) {
    channel_list->channels_head->prev = new_channel;
  }
  channel_list->channels_head = new_channel;
  channel_list->num_channels++;

  return SUCCESS;
}

void join_channel(user *user, channel *channel) {
  subbed_channel *new_subbed_channel = (subbed_channel *)malloc(sizeof(subbed_channel));
  if (!new_subbed_channel)
    return;

  new_subbed_channel->channel = channel;
  new_subbed_channel->next = user->subbed_channels_head;
  new_subbed_channel->prev = NULL;

  if (user->subbed_channels_head) {
    user->subbed_channels_head->prev = new_subbed_channel;
  }
  user->subbed_channels_head = new_subbed_channel;

  subbed_user *new_subbed_user = (subbed_user *)malloc(sizeof(subbed_user));
  if (!new_subbed_user)
    return;

  new_subbed_user->user = user;
  new_subbed_user->next = channel->subbed_users_head;
  new_subbed_user->prev = NULL;

  if (channel->subbed_users_head) {
    channel->subbed_users_head->prev = new_subbed_user;
  }
  channel->subbed_users_head = new_subbed_user;
}

void leave_channel(user *user, channel *channel) {
  // Remove the channel from the user's list of subscribed channels
  subbed_channel *subbed_channel_ptr = user->subbed_channels_head;
  while (subbed_channel_ptr) {
    if (subbed_channel_ptr->channel == channel) {
      // Update pointers to remove the subbed_channel from the user's list
      if (subbed_channel_ptr->prev) {
        subbed_channel_ptr->prev->next = subbed_channel_ptr->next;
      } else {
        user->subbed_channels_head = subbed_channel_ptr->next;
      }
      if (subbed_channel_ptr->next) {
        subbed_channel_ptr->next->prev = subbed_channel_ptr->prev;
      }
      free(subbed_channel_ptr); // Free the user's subscription node
      break;
    }
    subbed_channel_ptr = subbed_channel_ptr->next;
  }

  // Remove the user from the channel's list of subscribed users
  subbed_user *subbed_user_ptr = channel->subbed_users_head;
  while (subbed_user_ptr) {
    if (subbed_user_ptr->user == user) {
      // Update pointers to remove the subbed_user from the channel's list
      if (subbed_user_ptr->prev) {
        subbed_user_ptr->prev->next = subbed_user_ptr->next;
      } else {
        channel->subbed_users_head = subbed_user_ptr->next;
      }
      if (subbed_user_ptr->next) {
        subbed_user_ptr->next->prev = subbed_user_ptr->prev;
      }
      free(subbed_user_ptr); // Free the channel's subscription node
      break;
    }
    subbed_user_ptr = subbed_user_ptr->next;
  }
}

user *find_user(users *user_list, const char *username, struct sockaddr_in *client) {
  user *current_user = user_list->users_head;
  while (current_user != NULL) {
    if (username) {
      if (strncmp(current_user->username, username, USERNAME_MAX_CHAR) == 0) {
        return current_user;
      }
    } else if (client) {
      char ip[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &(client->sin_addr), ip, INET_ADDRSTRLEN);

      if ((strncmp(current_user->ip, ip, INET_ADDRSTRLEN) == 0) && current_user->port == ntohs(client->sin_port)) {
        return current_user;
      }
    }

    current_user = current_user->next;
  }
  return NULL; // User not found
}

user *create_user(users *user_list, const char *username, struct sockaddr_in *client) {

  if (find_user(user_list, username, NULL)) {
    perror("User already exists");
    return NULL; // User already exists
  }

  user *new_user = (user *)malloc(sizeof(user));

  strncpy(new_user->username, username, USERNAME_MAX_CHAR);
  inet_ntop(AF_INET, &(client->sin_addr), new_user->ip, INET_ADDRSTRLEN);
  new_user->port = ntohs(client->sin_port);
  new_user->subbed_channels_head = NULL;
  new_user->next = user_list->users_head;
  new_user->prev = NULL;

  if (user_list->users_head) {
    user_list->users_head->prev = new_user;
  }
  user_list->users_head = new_user;
  user_list->num_users++;

  return new_user;
}

void delete_user(users *user_list, user *user) {

  subbed_channel *subbed_channel_ptr = user->subbed_channels_head;

  while (subbed_channel_ptr) {
    subbed_channel *next_subbed = subbed_channel_ptr->next;
    leave_channel(user, subbed_channel_ptr->channel);
    subbed_channel_ptr = next_subbed;
  }

  if (user->prev) {
    user->prev->next = user->next;
  } else {
    user_list->users_head = user->next;
  }
  if (user->next) {
    user->next->prev = user->prev;
  }
  free(user);
  user_list->num_users--;
}

int delete_channel(channels *channel_list, channel *channel_to_delete) {
  // Check if there are users subscribed to this channel
  if (channel_to_delete->subbed_users_head != NULL) {
    perror("ERROR: Cannot delete channel as there are still users subscribed.\n");
    return FAILURE;
  }

  // Remove the channel from the channels list
  if (channel_to_delete->prev) {
    channel_to_delete->prev->next = channel_to_delete->next;
  } else {
    channel_list->channels_head = channel_to_delete->next;
  }

  if (channel_to_delete->next) {
    channel_to_delete->next->prev = channel_to_delete->prev;
  }

  // Free the channel memory
  free(channel_to_delete);
  channel_list->num_channels--; // Decrement the channel count

  return SUCCESS;
}

/***************************************************************************** */

void print_client_details(struct sockaddr_in *client) {
  char ip_str[INET_ADDRSTRLEN];

  // Convert IP address to string
  inet_ntop(AF_INET, &(client->sin_addr), ip_str, INET_ADDRSTRLEN);

  // Get port number (convert from network byte order to host byte order)
  unsigned short port = ntohs(client->sin_port);

  printf("CLIENT DETAILS\nIP: %s\nPORT: %d\n", ip_str, port);
}

// void print_channels(const channels *channel_list) {
//   channel *current_channel = channel_list->channels_head;
//   printf("[CHANNELS] ");
//   while (current_channel != NULL) {
//     printf("%s, ", current_channel->name);
//     current_channel = current_channel->next;
//   }
//   printf("\n");
// }

void view_request(request *request, struct sockaddr_in *client) {
  printf("\n");
  print_client_details(client);
  print_request(request);
  printf("\n");
}

void print_channels(const channels *channel_list) {
  channel *current_channel = channel_list->channels_head;
  printf("=== [CHANNELS] ===\n");

  while (current_channel != NULL) {
    printf("- %s\n↪ [USERS]", current_channel->name);

    // Check if there are any users subscribed to this channel
    if (current_channel->subbed_users_head == NULL) {
      printf(" ERROR: No users subscribed.\n"); // THIS SHOULD NOT HAPPEN
    } else {
      // Iterate through each user subscribed to the current channel
      subbed_user *current_subbed_user = current_channel->subbed_users_head;
      while (current_subbed_user != NULL) {
        printf(" %s |", current_subbed_user->user->username);
        current_subbed_user = current_subbed_user->next;
      }
      printf("\n");
    }

    // Move to the next channel
    current_channel = current_channel->next;
  }
}

void print_users(const users *user_list) {
  user *current_user = user_list->users_head;
  printf("=== [USERS] ===\n");

  while (current_user != NULL) {
    printf("- %s\n↪ [CHANNELS]", current_user->username);

    // Check if there are any channels this user is subscribed to
    if (current_user->subbed_channels_head == NULL) {
      printf(" NONE\n");
    } else {
      // Iterate through each channel this user is subscribed to
      subbed_channel *current_subbed_channel = current_user->subbed_channels_head;
      while (current_subbed_channel != NULL) {
        printf(" %s |", current_subbed_channel->channel->name);
        current_subbed_channel = current_subbed_channel->next;
      }
      printf("\n");
    }

    // Move to the next user
    current_user = current_user->next;
  }
}

#endif
