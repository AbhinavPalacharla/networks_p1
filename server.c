#include "server.h"
#include "duckchat.h"
#include "shared.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct _channel channel;

typedef struct _user {
  char username[USERNAME_MAX_CHAR];
  char ip[INET_ADDRSTRLEN];
  unsigned short port;
  channel **subbed_channels;
} user;

typedef struct _channel {
  char name[CHANNEL_MAX_CHAR];
  user **subbed_users;
} channel;

typedef struct _users {
  int num_users;
  user *users;
} users;

void print_user(user *user) {
  if (user == NULL) {
    printf("User is NULL\n");
    return;
  }

  printf("USER INFO:\n");
  printf("  Username: %s\n", user->username);
  printf("  IP: %s\n", user->ip);
  printf("  Port: %d\n", user->port);

  // Print subscribed channels if any
  printf("  Subscribed Channels:\n");
  if (user->subbed_channels == NULL) {
    printf("    None\n");
  } else {
    int i = 0;
    while (user->subbed_channels[i] != NULL) {
      printf("    - %s\n", user->subbed_channels[i]->name);
      i++;
    }
  }
  printf("\n");
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
    printf("User %d:\n", i + 1);
    print_user(&users->users[i]);
  }
  printf("================\n\n");
}

typedef struct _channels {
  int num_channels;
  channel *channels;
} channels;

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

void view_request(request *request, struct sockaddr_in *client) {
  printf("\n");
  print_client_details(client);
  print_request(request);
  printf("\n");
}

int handle_request(request *request, struct sockaddr_in *client, users *users, channels *channels) {
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

    new_user->subbed_channels = NULL;
    users->num_users++;

    print_users(users);

    return SUCCESS;
  }

  // } else if (request->req_type == REQ_LOGOUT) {
  //   request_logout *rq_logout = (request_logout *)request;

  //   // Remove user from active channels and delete channels if the channels have no users
  // } else if (request->req_type == REQ_JOIN) {
  // }

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

  printf("SERVER STARTING...\n");

  while (1) {
    memset(buffer, 0, sizeof(buffer));
    memset(&client_addr, 0, sizeof(client_addr));

    // Receive message from client
    int msg_len =
        recvfrom(sockfd, (char *)buffer, SERVER_BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_addr_len);

    if (msg_len < 0) {
      perror("Failed to recieve message from client.");
      goto fail_exit;
    }

    printf("MESSAGE LEN: %d\n", msg_len);

    buffer[msg_len] = '\0';

    // printf("(CLIENT) >>> REQUEST_TYPE %d\n", ((request *)buffer)->req_type);
    // printf("REQUEST_TYPE: %d\n", buffer[0]);
    //
    // print_client_details(&client_addr);
    handle_request((request *)buffer, &client_addr, users_list, channels_list);

    // Send acknowledgment to client
    // const char *ack = "Message received!";
    // sendto(sockfd, ack, strlen(ack), 0, (struct sockaddr *)&client_addr, client_addr_len);
    // printf("Acknowledgment sent to client.\n");
  }

  close(sockfd);
  return 0;

fail_exit:
  close(sockfd);
  exit(EXIT_FAILURE);
}
