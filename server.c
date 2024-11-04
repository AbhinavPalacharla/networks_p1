#include "server.h"
#include "duckchat.h"
#include "shared.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int send_error(int sockfd, struct sockaddr_in *client, char *msg) {
  text_error res;

  res.txt_type = TXT_ERROR;
  strncpy(res.txt_error, msg, SAY_MAX_CHAR);

  sendto(sockfd, &res, sizeof(res), 0, (const struct sockaddr *)client, sizeof(struct sockaddr_in));

  return SUCCESS;
}

int handle_request(int sockfd, request *request, struct sockaddr_in *client, users *users, channels *channels) {
  if (request->req_type == REQ_LOGIN) {
    request_login *req = (request_login *)request;

    if (strlen(req->username) > USERNAME_MAX_CHAR) {
      send_error(sockfd, client, "Invalid username");
      return NON_FATAL_ERR;
    }

    user *new_user = create_user(users, req->username, client);
    if (new_user == NULL) {
      // TODO: Send error packet
      send_error(sockfd, client, "User already exists");
      return NON_FATAL_ERR;
    }

    channel *c = find_channel(channels, "Common");

    // Check if user is already in channel
    for (subbed_user *sub = c->subbed_users_head; sub != NULL; sub = sub->next) {
      if (sub->user == new_user) {
        send_error(sockfd, client, "Already in channel");
        return NON_FATAL_ERR;
      }
    }

    // Join user to common channel
    join_channel(new_user, c);

    print_users(users);
    print_channels(channels);
  } else if (request->req_type == REQ_JOIN) {
    request_join *req = (request_join *)request;

    if (strlen(req->channel) > CHANNEL_MAX_CHAR) {
      send_error(sockfd, client, "Invalid channel name");
      return NON_FATAL_ERR;
    }

    // if channel doesn't exist then create channel
    if (find_channel(channels, req->channel) == NULL) {
      create_channel(channels, req->channel);
    }

    user *u = find_user(users, NULL, client);

    if (u == NULL) {
      send_error(sockfd, client, "User not logged in");
      return NON_FATAL_ERR;
    }

    channel *c = find_channel(channels, req->channel);

    if (c == NULL) {
      send_error(sockfd, client, "Channel doesn't exist");
      return NON_FATAL_ERR;
    }

    // Check if user is already in channel
    for (subbed_user *sub = c->subbed_users_head; sub != NULL; sub = sub->next) {
      if (sub->user == u) {
        send_error(sockfd, client, "Already in channel");
        return NON_FATAL_ERR;
      }
    }

    join_channel(u, c);

    print_users(users);
    print_channels(channels);
  } else if (request->req_type == REQ_LEAVE) {
    request_leave *req = (request_leave *)request;

    if (strlen(req->channel) > CHANNEL_MAX_CHAR) {
      send_error(sockfd, client, "Invalid channel name");
      return NON_FATAL_ERR;
    }

    channel *c = find_channel(channels, req->channel);

    if (c == NULL) {
      send_error(sockfd, client, "Channel doesn't exist");
      return NON_FATAL_ERR;
    }

    user *u = find_user(users, NULL, client);

    if (u == NULL) {
      send_error(sockfd, client, "User not logged in");
      return NON_FATAL_ERR;
    }

    int is_subscribed = 0;
    for (subbed_user *sub = c->subbed_users_head; sub != NULL; sub = sub->next) {
      if (sub->user == u) {
        is_subscribed = 1;
        break;
      }
    }

    if (!is_subscribed) {
      send_error(sockfd, client, "User not subscribed to channel");
      return NON_FATAL_ERR;
    }

    leave_channel(u, c);

    // Prune channels with no users unless it is Common channel
    if (c->subbed_users_head == NULL && strncmp(c->name, "Common", strlen("Common")) != 0) {
      delete_channel(channels, c);
    }

    print_users(users);
    print_channels(channels);
  } else if (request->req_type == REQ_LOGOUT) {
    // request_leave *req = (request_leave *)request;

    user *u;

    if ((u = find_user(users, NULL, client)) == NULL) {
      send_error(sockfd, client, "User not logged in");
      return NON_FATAL_ERR;
    }

    delete_user(users, u);

    // Clean up empty channels after user deleted
    for (channel *c = channels->channels_head; c != NULL; c = c->next) {
      if (c->subbed_users_head == NULL) {
        delete_channel(channels, c);
      }
    }

    print_users(users);
    print_channels(channels);
  } else if (request->req_type == REQ_LIST) {
    int num_channels = 0;
    for (channel *c = channels->channels_head; c != NULL; c = c->next) {
      num_channels++;
    }

    size_t size = sizeof(text_list) + (sizeof(channel_info) * num_channels);

    text_list *res = (text_list *)malloc(size);

    res->txt_type = TXT_LIST;
    res->n_channel = num_channels;

    int i = 0;
    for (channel *c = channels->channels_head; c != NULL; c = c->next) {
      strncpy(res->channels[i].channel, c->name, CHANNEL_MAX_CHAR);
      i++;
    }

    print_text_list(res);

    sendto(sockfd, res, size, 0, (const struct sockaddr *)client, sizeof(struct sockaddr_in));
  } else if (request->req_type == REQ_WHO) {
    request_who *req = (request_who *)request;

    channel *c = find_channel(channels, req->channel);

    if (c == NULL) {
      send_error(sockfd, client, "Channel for /who not found");
      return NON_FATAL_ERR;
    }

    int num_users = 0;
    for (subbed_user *u = c->subbed_users_head; u != NULL; u = u->next) {
      num_users++;
    }

    size_t size = sizeof(text_who) + (sizeof(user_info) * num_users);

    text_who *res = (text_who *)malloc(size);

    res->txt_type = TXT_WHO;
    strncpy(res->channel, req->channel, CHANNEL_MAX_CHAR);
    res->n_username = num_users;

    int i = 0;
    for (subbed_user *u = c->subbed_users_head; u != NULL; u = u->next) {
      strncpy(res->users[i].username, u->user->username, USERNAME_MAX_CHAR);
      i++;
    }

    sendto(sockfd, res, size, 0, (const struct sockaddr *)client, sizeof(struct sockaddr_in));
  } else if (request->req_type == REQ_SAY) {
    request_say *req = (request_say *)request;

    // Find the sending user
    user *sender = find_user(users, NULL, client);
    if (sender == NULL) {
      send_error(sockfd, client, "User not logged in");
      return NON_FATAL_ERR;
    }

    // Find the channel
    channel *target_channel = find_channel(channels, req->channel);
    if (target_channel == NULL) {
      send_error(sockfd, client, "Channel not found");
      return NON_FATAL_ERR;
    }

    int is_subscribed = 0;
    for (subbed_user *sub = target_channel->subbed_users_head; sub != NULL; sub = sub->next) {
      if (sub->user == sender) {
        is_subscribed = 1;
        break;
      }
    }

    if (!is_subscribed) {
      send_error(sockfd, client, "Not subscribed to channel");
      return NON_FATAL_ERR;
    }

    text_say res;
    res.txt_type = TXT_SAY;
    strncpy(res.channel, req->channel, CHANNEL_MAX_CHAR);
    strncpy(res.username, sender->username, USERNAME_MAX_CHAR);
    strncpy(res.text, req->text, SAY_MAX_CHAR);

    // Send message to all users subscribed to the channel
    for (subbed_user *sub = target_channel->subbed_users_head; sub != NULL; sub = sub->next) {
      struct sockaddr_in recipient_addr;
      memset(&recipient_addr, 0, sizeof(recipient_addr));
      recipient_addr.sin_family = AF_INET;
      recipient_addr.sin_port = htons(sub->user->port);
      inet_pton(AF_INET, sub->user->ip, &recipient_addr.sin_addr);

      sendto(sockfd, &res, sizeof(res), 0, (const struct sockaddr *)&recipient_addr, sizeof(struct sockaddr_in));
    }
  }

  return SUCCESS;
}

int main(int argc, char **argv) {
  if (argc != 3) {
    printf("Usage: ./server [hostname] [port]\n");
    exit(EXIT_FAILURE);
  }

  int sockfd;
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));

  // Configure Server
  server_addr.sin_family = AF_INET;
  if (inet_pton(AF_INET, argv[1], &(server_addr.sin_addr)) <= 0) {
    perror("Invalid address/ Address not supported");
    goto fail_exit;
  }
  server_addr.sin_port = htons(atoi(argv[2]));

  // Bind the socket with the server address
  if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    perror("bind failed");
    goto fail_exit;
  }

  // unsigned int client_addr_len = sizeof(client_addr); // length of the client address
  char buffer[SERVER_BUFFER_SIZE];

  users *user_list = malloc(sizeof(users));
  channels *channel_list = malloc(sizeof(channels));

  // Create common channel
  create_channel(channel_list, "Common");

  print_channels(channel_list);

  printf("SERVER STARTING ON %s %d\n", argv[1], atoi(argv[2]));

  while (1) {
    // Reset client_addr
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    unsigned int client_addr_len = sizeof(client_addr);

    memset(buffer, 0, sizeof(buffer));

    // Receive message from client
    int len;
    if ((len = recvfrom(sockfd, (char *)buffer, SERVER_BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) {
      perror("Failed to recieve message from client.");
      continue;
    }

    validate_request((request *)buffer, len);

    print_client_details(&client_addr);
    print_request((request *)buffer);

    handle_request(sockfd, (request *)buffer, &client_addr, user_list, channel_list);
  }

  close(sockfd);
  return 0;

fail_exit:
  close(sockfd);
  exit(EXIT_FAILURE);
}
