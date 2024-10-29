#include "client.h"
#include "duckchat.h"
#include "shared.h"
#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

int send_message(int sockfd, struct sockaddr_in servaddr, user_info *user, char *msg) {
  request_say say_packet;

  say_packet.req_type = REQ_SAY;
  strncpy(say_packet.channel, user->current_channel, strlen(user->current_channel));
  strncpy(say_packet.text, msg, strlen(msg));

  sendto(sockfd, &say_packet, sizeof(say_packet), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));

  return SUCCESS;
}

int send_packet(int sockfd, struct sockaddr_in servaddr, int packet_type, user_info *user, char *data) {
  if (packet_type == REQ_LOGIN) {
    request_login login_packet;
    login_packet.req_type = REQ_LOGIN;
    strncpy(login_packet.username, user->username, USERNAME_MAX_CHAR);

    sendto(sockfd, &login_packet, sizeof(login_packet), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));

  } else if (packet_type == REQ_LOGOUT) {
    request_logout logout_packet;
    logout_packet.req_type = REQ_LOGOUT;

    sendto(sockfd, &logout_packet, sizeof(logout_packet), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));

  } else if (packet_type == REQ_JOIN) {
    request_join join_packet;
    join_packet.req_type = REQ_JOIN;
    strncpy(join_packet.channel, data, CHANNEL_MAX_CHAR);

    sendto(sockfd, &join_packet, sizeof(join_packet), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));

  } else if (packet_type == REQ_LEAVE) {
    request_leave leave_packet;
    leave_packet.req_type = REQ_LEAVE;
    strncpy(leave_packet.channel, data, CHANNEL_MAX_CHAR);

    sendto(sockfd, &leave_packet, sizeof(leave_packet), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));

  } else if (packet_type == REQ_SAY) {
    request_say say_packet;
    say_packet.req_type = REQ_SAY;
    strncpy(say_packet.channel, user->current_channel, CHANNEL_MAX_CHAR);
    strncpy(say_packet.text, data, SAY_MAX_CHAR);

    sendto(sockfd, &say_packet, sizeof(say_packet), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));

  } else if (packet_type == REQ_LIST) {
    request_list list_packet;
    list_packet.req_type = REQ_LIST;

    sendto(sockfd, &list_packet, sizeof(list_packet), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));

  } else if (packet_type == REQ_WHO) {
    request_who who_packet;
    who_packet.req_type = REQ_WHO;
    strncpy(who_packet.channel, data, CHANNEL_MAX_CHAR);

    sendto(sockfd, &who_packet, sizeof(who_packet), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));

  } else {
    perror("Unknown packet type");
    return FAILURE;
  }

  return SUCCESS;
}

int handle_command(int sockfd, struct sockaddr_in servaddr, char *command, user_info *user) {
  // Check for (/) commands from user
  if (strncmp(command, "/exit", strlen("/exit")) == 0) {

    send_packet(sockfd, servaddr, REQ_LOGOUT, NULL, NULL);

    printf("Exiting...\n");
    return SUCCESS_EXIT;

  } else if (strncmp(command, "/join", strlen("/join")) == 0) {

    char *channel = command + strlen("/join");
    // Skip leading whitespace
    while (*channel == ' ') {
      channel++;
    }
    // Check if channel name is provided
    if (*channel == '\0') {
      printf("(CLIENT) >>> ERROR: Please provide a channel name to join.\nUsage: /join [channel]\n");
      return NON_FATAL_ERR;
    }

    send_packet(sockfd, servaddr, REQ_JOIN, user, channel);

  } else if (strncmp(command, "/leave", strlen("/leave")) == 0) {

    char *channel = command + strlen("/leave");
    // Skip leading whitespace
    while (*channel == ' ') {
      channel++;
    }
    // Check if channel name is provided
    if (*channel == '\0') {
      printf("(CLIENT) >>> ERROR: Please provide a channel name to leave.\nUsage: /leave [channel]\n");
      return NON_FATAL_ERR;
    }

    send_packet(sockfd, servaddr, REQ_LEAVE, user, channel);

  } else if (strncmp(command, "/list", strlen("/list")) == 0) {

    send_packet(sockfd, servaddr, REQ_LIST, user, NULL);

  } else if (strncmp(command, "/who", strlen("/who")) == 0) {

    char *channel = command + strlen("/who");
    // Skip leading whitespace
    while (*channel == ' ') {
      channel++;
    }
    // Check if channel name is provided
    if (*channel == '\0') {
      printf("(CLIENT) >>> ERROR: Please provide a channel name.\nUsage: /who [channel]\n");
      return NON_FATAL_ERR;
    }

    // TODO: On server make sure channel exists otherwise throw error

    send_packet(sockfd, servaddr, REQ_WHO, user, channel);

  } else if (strncmp(command, "/switch", strlen("/switch")) == 0) {

    char *channel = command + strlen("/switch");
    // Skip leading whitespace
    while (*channel == ' ') {
      channel++;
    }
    // Check if channel name is provided
    if (*channel == '\0') {
      printf("(CLIENT) >>> ERROR: Please provide a channel name to switch to.\nUsage: /switch [channel]\n");
      return NON_FATAL_ERR;
    }

    // TODO: Make sure user is subscribed to channel before using /switch to the channel

    // Update user's current channel
    strncpy(user->current_channel, channel, CHANNEL_MAX_CHAR);
    printf("Switched to channel: %s\n", channel);

  } else {
    printf("(CLIENT) >>> ERROR: Unknown command\n");
    return NON_FATAL_ERR;
  }

  return SUCCESS;
}

int handle_response(text *response) {
  if (response->txt_type == TXT_LIST) {
    char *str = malloc(sizeof(char) * strlen("Existing channels:\n"));
    strncpy(str, "Existing channels:\n", strlen("Existing channels:\n"));

    text_list *res = (text_list *)response;

    for (int i = 0; i < res->n_channel; i++) {
      size_t new_size = strlen(str) + strlen(res->channels[i].channel) + 3;
      str = realloc(str, new_size);

      strcat(str, "\t");
      strcat(str, res->channels[i].channel);
      strcat(str, "\n");
    }

    printf("%s", str);
  } else if (response->txt_type == TXT_WHO) {
    text_who *res = (text_who *)response;

    char *str = malloc(sizeof(char) * (strlen("Users on Channel :\n") + strlen(res->channel)));
    strncpy(str, "Users on Channel ", strlen("Users on Channel "));
    strcat(str, res->channel);
    strcat(str, ":\n");

    for (int i = 0; i < res->n_username; i++) {
      size_t new_size = strlen(str) + strlen(res->users[i].username) + 3;
      str = realloc(str, new_size);

      strcat(str, "\t");
      strcat(str, res->users[i].username);
      strcat(str, "\n");
    }

    printf("%s", str);
  } else if (response->txt_type == TXT_SAY) {

  } else if (response->txt_type == TXT_ERROR) {
  }

  return SUCCESS;
}

int main(int argc, char **argv) {
  int sockfd;
  struct sockaddr_in servaddr;
  char server_buffer[SAY_MAX_CHAR];
  // 10 extra chars as buffer to handle command or extra spaces, will be cleaned
  char message_buffer[CLIENT_BUFFER_SIZE + 10];
  fd_set watch_fds;
  int maxfd;
  socklen_t len = sizeof(servaddr);
  int nErr = 0;

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));

  // Configure server info
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(SERVER_PORT);
  if (inet_pton(AF_INET, SERVER_IP, &(servaddr.sin_addr)) <= 0) {
    perror("Invalid address/ Address not supported");
    goto fail_exit;
  }

  // Configure user
  user_info *user = (user_info *)malloc(sizeof(user_info));
  if (argc == 2) {
    strncpy(user->username, argv[argc - 1], (sizeof(char) * 32));
  } else {
    // Default username for testing purposes...
    strncpy(user->username, "abhinavp", strlen("abhinavp"));
  }
  strncpy(user->current_channel, "common", strlen("common")); // Default channel for user to join

  printf("CLIENT STARTING...\n");

  // Automatically send login packet as last step of client initialization
  send_packet(sockfd, servaddr, REQ_LOGIN, user, NULL);

  // Send and recieve messages loop
  while (1) {
    // Configure fds to watch
    FD_ZERO(&watch_fds);
    FD_SET(sockfd, &watch_fds);
    FD_SET(STDIN_FILENO, &watch_fds);

    maxfd = (sockfd > STDIN_FILENO) ? sockfd : STDIN_FILENO;

    // Use select to wait for activity on either stdin or the socket
    int activity = select(maxfd + 1, &watch_fds, NULL, NULL, NULL);

    if (activity < 0) {
      perror("select error");
      goto fail_exit;
    }

    // Handle server activity
    if (FD_ISSET(sockfd, &watch_fds)) {
      int n = recvfrom(sockfd, (char *)server_buffer, CLIENT_BUFFER_SIZE, 0, (struct sockaddr *)&servaddr, &len);

      if (n < 0) {
        perror("recvfrom() failed.");
        goto fail_exit;
      }

      // printf("NUM BYTES RECIEVED: %d\n", n);

      server_buffer[n] = '\0';

      text *server_msg = (text *)server_buffer;

      print_text(server_msg);

      handle_response(server_msg);

      memset(server_buffer, 0, sizeof(server_buffer)); // clear server buffer for use in next server message
    }

    // Handle client stdin activity
    if (FD_ISSET(STDIN_FILENO, &watch_fds)) {
      fgets(message_buffer, sizeof(message_buffer), stdin);

      message_buffer[strcspn(message_buffer, "\n")] = '\0';

      if (message_buffer[0] == '/') {
        if ((nErr = handle_command(sockfd, servaddr, message_buffer, user)) < 0) {
          goto fail_exit;
        } else if (nErr == SUCCESS_EXIT) {
          goto success_exit;
        }
      } else {
        // Normal message to user active channel
        send_message(sockfd, servaddr, user, message_buffer);
      }

      memset(message_buffer, 0, sizeof(message_buffer)); // clear message buffer for use in next message
    }
  }

success_exit:
  close(sockfd);
  return 0;

fail_exit:
  close(sockfd);
  exit(EXIT_FAILURE);
}
