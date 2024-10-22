#include "client.h"
#include "duckchat.h"
#include "shared.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

// int send_packet(int sockfd, struct sockaddr_in servaddr, int packet_type, user_info *user) {

//   if (packet_type == REQ_LOGIN) {
//     request_login *login_packet = (request_login *)malloc(sizeof(request_login));

//   } else if (packet_type == REQ_LOGOUT) {
//     request_logout *logout_packet = (request_logout *)malloc(sizeof(request_logout));

//     logout_packet->req_type = REQ_LOGOUT;

//     sendto(sockfd, logout_packet, sizeof(*logout_packet), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
//   } else {
//     perror("Unknown packet type");
//     return FAILURE;
//   }

//   return SUCCESS;
// }

// NOTE: char *data used for channel to join, channel to leave, message to say, etc.
int send_packet(int sockfd, struct sockaddr_in servaddr, int packet_type, user_info *user, char *data) {
  if (packet_type == REQ_LOGIN) {

    request_login *login_packet = (request_login *)malloc(sizeof(request_login));

    login_packet->req_type = REQ_LOGIN;
    strncpy(login_packet->username, user->username, USERNAME_MAX_CHAR);

    sendto(sockfd, login_packet, sizeof(*login_packet), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));

    free(login_packet);

  } else if (packet_type == REQ_LOGOUT) {

    request_logout *logout_packet = (request_logout *)malloc(sizeof(request_logout));

    logout_packet->req_type = REQ_LOGOUT;

    int bytes_sent =
        sendto(sockfd, logout_packet, sizeof(*logout_packet), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));

    printf("BYTES SENT: %d\n", bytes_sent);

    free(logout_packet);

  } else if (packet_type == REQ_JOIN) {

    request_join *join_packet = (request_join *)malloc(sizeof(request_join));

    join_packet->req_type = REQ_JOIN;
    strncpy(join_packet->channel, data, CHANNEL_MAX_CHAR); // data = channel name to join

    sendto(sockfd, join_packet, sizeof(*join_packet), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));

    // TODO: After confirmation recieved that user joined channel, set their current channel to that one
    //  (for sending purposes). Make this a seperate function called handle_recieved_packet() or something
    // This will not be handled in this function
    free(join_packet);

  } else if (packet_type == REQ_LEAVE) {

    request_leave *leave_packet = (request_leave *)malloc(sizeof(request_leave));

    leave_packet->req_type = REQ_LEAVE;
    strncpy(leave_packet->channel, data, CHANNEL_MAX_CHAR); // data = channel name to leave

    sendto(sockfd, leave_packet, sizeof(*leave_packet), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    free(leave_packet);

  } else if (packet_type == REQ_SAY) {

    request_say *say_packet = (request_say *)malloc(sizeof(request_say));

    say_packet->req_type = REQ_SAY;
    strncpy(say_packet->channel, user->current_channel, CHANNEL_MAX_CHAR);
    strncpy(say_packet->text, data, SAY_MAX_CHAR); // data = user message to say in channel

    sendto(sockfd, say_packet, sizeof(*say_packet), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    free(say_packet);

  } else if (packet_type == REQ_LIST) {

    request_list *list_packet = (request_list *)malloc(sizeof(request_list));

    list_packet->req_type = REQ_LIST;

    sendto(sockfd, list_packet, sizeof(*list_packet), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    free(list_packet);

  } else if (packet_type == REQ_WHO) {

    request_who *who_packet = (request_who *)malloc(sizeof(request_who));

    who_packet->req_type = REQ_WHO;
    strncpy(who_packet->channel, data, CHANNEL_MAX_CHAR);

    sendto(sockfd, who_packet, sizeof(*who_packet), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    free(who_packet);

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
  } else if (strncmp(command, "/join", strlen("/join")) == 0) {

    // // Check if user provided a channel name to join format -> /join [channel]
    // // so +2 to account for space and channel name being atleast 1 char
    // if (strlen(command) < (strlen("/join") + 2)) {
    //   printf("(CLIENT) >>> ERROR: Please provide a channel name to join.\nUsage: /join [channel]");
    // }

    // char *channel;

    // Check if user provided a channel name to join format -> /join [channel]
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
  } else if (strncmp(command, "/list", strlen("/list")) == 0) {
  } else if (strncmp(command, "/who", strlen("/who")) == 0) {
  } else if (strncmp(command, "/switch", strlen("/switch")) == 0) {
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
  servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);

  // TODO: change default username case before final submission
  user_info *user = (user_info *)malloc(sizeof(user_info));
  if (argc == 2) {
    strncpy(user->username, argv[argc - 1], (sizeof(char) * 32));
  } else {
    // Default username for testing purposes...
    strncpy(user->username, "abhinavp", strlen("abhinavp"));
  }
  strncpy(user->current_channel, "Common", strlen("Common")); // Default channel for user to join

  // TODO: Automatically send login packet as last step of client initialization

  printf("CLIENT STARTING...\n");
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
      } else if (n > 0) {
        server_buffer[n] = '\0';
        printf("Server: %s\n", server_buffer);

        memset(server_buffer, 0, sizeof(server_buffer)); // clear server buffer for use in next server message
      }
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
        // Normal message to channel
        sendto(sockfd, message_buffer, strlen(message_buffer), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
      }

      // printf("Message sent to server.\n");

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
