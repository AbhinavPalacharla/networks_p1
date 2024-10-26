#include "server.h"
#include "duckchat.h"
#include "shared.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void view_request(request *request, struct sockaddr_in *client) {
  printf("\n");
  print_client_details(client);
  print_request(request);
  printf("\n");
}

int handle_request(int sockfd, request *request, struct sockaddr_in *client, users *users, channels *channels) {
  view_request(request, client);

  if (request->req_type == REQ_LOGIN) {
    // Register user in users list
    request_login *rq_login = (request_login *)request;

    users->users = realloc(users->users, (users->num_users + 1) * sizeof(user));

    if (users->users == NULL) {
      perror("Failed to resize user array using realloc()");
      return FAILURE;
    }

    user *new_user = &users->users[users->num_users];

    strncpy(new_user->username, rq_login->username, USERNAME_MAX_CHAR);     // username
    inet_ntop(AF_INET, &(client->sin_addr), new_user->ip, INET_ADDRSTRLEN); // IP
    new_user->port = ntohs(client->sin_port);                               // Port
    new_user->n_subbed_channels = 0;
    new_user->subbed_channels = NULL;
    users->num_users++;

    if ((join_channel(channels, new_user, "common")) < 0) {
      send_error(sockfd, client, "Failed to join common channel");
    }

    print_users(users);
    print_channels(channels);

    return SUCCESS;
  }

  return SUCCESS;
}

int main() {
  int sockfd;
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in server_addr, client_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  memset(&client_addr, 0, sizeof(client_addr));

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

  unsigned int client_addr_len = sizeof(client_addr); // length of the client address
  char buffer[SERVER_BUFFER_SIZE];

  users *users_list = malloc(sizeof(users));
  channels *channels_list = malloc(sizeof(channels));

  users_list->num_users = 0;
  users_list->users = NULL;
  channels_list->num_channels = 0;
  channels_list->channels = NULL;
  create_channel(channels_list, "common");

  printf("SERVER STARTING...\n");

  while (1) {
    memset(buffer, 0, sizeof(buffer));
    memset(&client_addr, 0, sizeof(client_addr));

    // Receive message from client
    int msg_len = recvfrom(sockfd, (char *)buffer, SERVER_BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_addr_len);

    if (msg_len < 0) {
      perror("Failed to recieve message from client.");
      goto fail_exit;
    }

    printf("MESSAGE LEN: %d\n", msg_len);

    buffer[msg_len] = '\0';

    handle_request(sockfd, (request *)buffer, &client_addr, users_list, channels_list);
  }

  close(sockfd);
  return 0;

fail_exit:
  close(sockfd);
  exit(EXIT_FAILURE);
}
