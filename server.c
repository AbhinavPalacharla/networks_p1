#include "server.h"
#include "duckchat.h"
#include "shared.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int handle_request(int sockfd, request *request, struct sockaddr_in *client, users *users, channels *channels) {
  if (request->req_type == REQ_LOGIN) {
    request_login *req = (request_login *)request;

    user *new_user = create_user(users, req->username, client);
    if (new_user == NULL) {
      // TODO: Send error packet
    }

    // Join user to common channel
    join_channel(new_user, find_channel(channels, "common"));

    print_users(users);
    print_channels(channels);
  } else if (request->req_type == REQ_JOIN) {
    request_join *req = (request_join *)request;

    // if channel doesn't exist then create channel
    if (find_channel(channels, req->channel) == NULL) {
      create_channel(channels, req->channel);
    }

    join_channel(find_user(users, NULL, client), find_channel(channels, req->channel));

    print_users(users);
    print_channels(channels);
  } else if (request->req_type == REQ_LEAVE) {
    request_leave *req = (request_leave *)request;

    channel *c = find_channel(channels, req->channel);
    if (c == NULL) {
      perror("Channel not found, could not leave channel");
      // TODO: send an error packet
    }

    user *u = find_user(users, NULL, client);

    leave_channel(u, c);

    if (c->subbed_users_head == NULL && strncmp(c->name, "common", strlen("common")) != 0) {
      delete_channel(channels, c);
    }

    print_users(users);
    print_channels(channels);
  } else if (request->req_type == REQ_LOGOUT) {
    request_leave *req = (request_leave *)request;

    user *u = find_user(users, NULL, client);

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
    // char *str = malloc((MAX_NUM_CHANNELS * (CHANNEL_MAX_CHAR + 2)) + strlen("Existing channels:\n"));
    // char *str = malloc(sizeof(char) * strlen("Existing channels:\n"));

    // for (channel *c = channels->channels_head; c != NULL; c = c->next) {
    //   size_t new_size = strlen(str) + strlen(c->name) + 3;
    //   str = realloc(str, new_size);

    //   strcat(str, "\t");
    //   strcat(str, c->name);
    //   strcat(str, "\n");
    // }

    text_list *res = malloc(sizeof(text_list));
    res->txt_type = TXT_LIST;

    int i = 0;
    for (channel *c = channels->channels_head; c != NULL; c = c->next) {
      i++;
    }

    res->n_channel = i;
    res->channels = malloc(sizeof(channel_info) * i);

    for (channel *c = channels->channels_head; c != NULL; c = c->next) {
      // strcpy(res.channels[i].channel, c->name);
      res->channels[i].channel[0] = 'R';
      printf("CHANNEL REGISTERED: %s\n", res->channels[i].channel);
    }

    // print_text((text *)&res);
    print_text_list(res);

    sendto(sockfd, res, sizeof(*res), 0, (const struct sockaddr *)client, sizeof(&client));
  }

  return SUCCESS;
}

int main() {
  int sockfd;
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));

  // Filling server information
  server_addr.sin_family = AF_INET; // IPv4
  if (inet_pton(AF_INET, SERVER_IP, &(server_addr.sin_addr)) <= 0) {
    perror("Invalid address/ Address not supported");
    goto fail_exit;
  }
  server_addr.sin_port = htons(PORT);

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
  create_channel(channel_list, "common");

  print_channels(channel_list);

  printf("SERVER STARTING...\n");

  while (1) {
    // Reset client_addr
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    unsigned int client_addr_len = sizeof(client_addr);

    memset(buffer, 0, sizeof(buffer));

    // Receive message from client
    if ((recvfrom(sockfd, (char *)buffer, SERVER_BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) {
      perror("Failed to recieve message from client.");
      goto fail_exit;
    }

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
