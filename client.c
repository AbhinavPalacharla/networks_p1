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
  strncpy(say_packet.channel, user->current_channel, CHANNEL_MAX_CHAR);
  strncpy(say_packet.text, msg, SAY_MAX_CHAR);

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

// int handle_command(int sockfd, struct sockaddr_in servaddr, char *command, user_info *user) {

//   // Check for (/) commands from user
//   if (strncmp(command, "/exit", strlen("/exit")) == 0) {

//     send_packet(sockfd, servaddr, REQ_LOGOUT, NULL, NULL);

//     printf("Exiting...\n");
//     return SUCCESS_EXIT;

//   } else if (strncmp(command, "/join", strlen("/join")) == 0) {

//     char *channel = command + strlen("/join");
//     // Skip leading whitespace
//     while (*channel == ' ') {
//       channel++;
//     }
//     // Check if channel name is provided
//     if (*channel == '\0') {
//       printf("(CLIENT) >>> ERROR: Please provide a channel name to join.\nUsage: /join [channel]\n");
//       return NON_FATAL_ERR;
//     }

//     if (strlen(channel) > CHANNEL_MAX_CHAR) {
//       printf("(CLIENT) >>> ERROR: Channel name > %d characters.\n", CHANNEL_MAX_CHAR);
//       return NON_FATAL_ERR;
//     }

//     send_packet(sockfd, servaddr, REQ_JOIN, user, channel);

//     printf("> ");
//     fflush(stdout);

//   } else if (strncmp(command, "/leave", strlen("/leave")) == 0) {

//     char *channel = command + strlen("/leave");
//     // Skip leading whitespace
//     while (*channel == ' ') {
//       channel++;
//     }
//     // Check if channel name is provided
//     if (*channel == '\0') {
//       printf("(CLIENT) >>> ERROR: Please provide a channel name to leave.\nUsage: /leave [channel]\n");
//       return NON_FATAL_ERR;
//     }

//     if (strlen(channel) > CHANNEL_MAX_CHAR) {
//       printf("(CLIENT) >>> ERROR: Channel name > %d characters.\n", CHANNEL_MAX_CHAR);
//       return NON_FATAL_ERR;
//     }

//     send_packet(sockfd, servaddr, REQ_LEAVE, user, channel);

//     printf("> ");
//     fflush(stdout);

//   } else if (strncmp(command, "/list", strlen("/list")) == 0) {

//     send_packet(sockfd, servaddr, REQ_LIST, user, NULL);

//   } else if (strncmp(command, "/who", strlen("/who")) == 0) {

//     char *channel = command + strlen("/who");
//     // Skip leading whitespace
//     while (*channel == ' ') {
//       channel++;
//     }
//     // Check if channel name is provided
//     if (*channel == '\0') {
//       printf("(CLIENT) >>> ERROR: Please provide a channel name.\nUsage: /who [channel]\n");
//       return NON_FATAL_ERR;
//     }

//     if (strlen(channel) > CHANNEL_MAX_CHAR) {
//       printf("(CLIENT) >>> ERROR: Channel name > %d characters.\n", CHANNEL_MAX_CHAR);
//       return NON_FATAL_ERR;
//     }

//     // TODO: On server make sure channel exists otherwise throw error

//     send_packet(sockfd, servaddr, REQ_WHO, user, channel);

//   } else if (strncmp(command, "/switch", strlen("/switch")) == 0) {

//     char *channel = command + strlen("/switch");

//     // Skip leading whitespace
//     while (*channel == ' ') {
//       channel++;
//     }

//     // Check if channel name is provided
//     if (*channel == '\0') {
//       printf("(CLIENT) >>> ERROR: Please provide a channel name to switch to.\nUsage: /switch [channel]\n");
//       return NON_FATAL_ERR;
//     }

//     if (strlen(channel) > CHANNEL_MAX_CHAR) {
//       printf("(CLIENT) >>> ERROR: Channel name > %d characters.\n", CHANNEL_MAX_CHAR);
//       return NON_FATAL_ERR;
//     }

//     // TODO: Make sure user is subscribed to channel before using /switch to the channel

//     // Update user's current channel
//     strncpy(user->current_channel, channel, CHANNEL_MAX_CHAR);
//     printf("Switched to channel: %s\n", channel);

//     printf("> ");
//     fflush(stdout);

//   } else {
//     printf("(CLIENT) >>> ERROR: Unknown command\n");

//     printf("> ");
//     fflush(stdout);
//     return NON_FATAL_ERR;
//   }

//   return SUCCESS;
// }
//

int handle_command(int sockfd, struct sockaddr_in servaddr, char *command, user_info *user) {
  char *cmd_copy = strdup(command); // Create a copy since strtok modifies the string
  if (!cmd_copy) {
    printf("(CLIENT) >>> ERROR: Memory allocation failed\n");
    printf("> ");
    fflush(stdout);
    return NON_FATAL_ERR;
  }

  // Get command name (first token)
  char *cmd_name = strtok(cmd_copy, " \t\n");
  if (!cmd_name) {
    free(cmd_copy);
    printf("> ");
    fflush(stdout);
    return NON_FATAL_ERR;
  }

  // Get argument (second token)
  char *argument = strtok(NULL, " \t\n");

  // Check for any additional unexpected arguments
  char *extra_arg = strtok(NULL, " \t\n");

  if (strcmp(cmd_name, "/exit") == 0) {
    if (argument != NULL) {
      printf("(CLIENT) >>> ERROR: /exit command takes no arguments\n");
      printf("> ");
      fflush(stdout);
      free(cmd_copy);
      return NON_FATAL_ERR;
    }
    send_packet(sockfd, servaddr, REQ_LOGOUT, NULL, NULL);
    printf("Exiting...\n");
    free(cmd_copy);
    return SUCCESS_EXIT;

  } else if (strcmp(cmd_name, "/join") == 0) {
    if (!argument) {
      printf("(CLIENT) >>> ERROR: Please provide a channel name to join.\nUsage: /join [channel]\n");
      printf("> ");
      fflush(stdout);
      free(cmd_copy);
      return NON_FATAL_ERR;
    }
    if (extra_arg) {
      printf("(CLIENT) >>> ERROR: Too many arguments.\nUsage: /join [channel]\n");
      printf("> ");
      fflush(stdout);
      free(cmd_copy);
      return NON_FATAL_ERR;
    }
    if (strlen(argument) > CHANNEL_MAX_CHAR) {
      printf("(CLIENT) >>> ERROR: Channel name > %d characters.\n", CHANNEL_MAX_CHAR);
      printf("> ");
      fflush(stdout);
      free(cmd_copy);
      return NON_FATAL_ERR;
    }
    send_packet(sockfd, servaddr, REQ_JOIN, user, argument);

  } else if (strcmp(cmd_name, "/leave") == 0) {
    if (!argument) {
      printf("(CLIENT) >>> ERROR: Please provide a channel name to leave.\nUsage: /leave [channel]\n");
      printf("> ");
      fflush(stdout);
      free(cmd_copy);
      return NON_FATAL_ERR;
    }
    if (extra_arg) {
      printf("(CLIENT) >>> ERROR: Too many arguments.\nUsage: /leave [channel]\n");
      printf("> ");
      fflush(stdout);
      free(cmd_copy);
      return NON_FATAL_ERR;
    }
    if (strlen(argument) > CHANNEL_MAX_CHAR) {
      printf("(CLIENT) >>> ERROR: Channel name > %d characters.\n", CHANNEL_MAX_CHAR);
      printf("> ");
      fflush(stdout);
      free(cmd_copy);
      return NON_FATAL_ERR;
    }
    send_packet(sockfd, servaddr, REQ_LEAVE, user, argument);

  } else if (strcmp(cmd_name, "/list") == 0) {
    if (argument) {
      printf("(CLIENT) >>> ERROR: /list command takes no arguments\n");
      printf("> ");
      fflush(stdout);
      free(cmd_copy);
      return NON_FATAL_ERR;
    }
    send_packet(sockfd, servaddr, REQ_LIST, user, NULL);

  } else if (strcmp(cmd_name, "/who") == 0) {
    if (!argument) {
      printf("(CLIENT) >>> ERROR: Please provide a channel name.\nUsage: /who [channel]\n");
      printf("> ");
      fflush(stdout);
      free(cmd_copy);
      return NON_FATAL_ERR;
    }
    if (extra_arg) {
      printf("(CLIENT) >>> ERROR: Too many arguments.\nUsage: /who [channel]\n");
      printf("> ");
      fflush(stdout);
      free(cmd_copy);
      return NON_FATAL_ERR;
    }
    if (strlen(argument) > CHANNEL_MAX_CHAR) {
      printf("(CLIENT) >>> ERROR: Channel name > %d characters.\n", CHANNEL_MAX_CHAR);
      printf("> ");
      fflush(stdout);
      free(cmd_copy);
      return NON_FATAL_ERR;
    }
    send_packet(sockfd, servaddr, REQ_WHO, user, argument);

  } else if (strcmp(cmd_name, "/switch") == 0) {
    if (!argument) {
      printf("(CLIENT) >>> ERROR: Please provide a channel name to switch to.\nUsage: /switch [channel]\n");
      printf("> ");
      fflush(stdout);
      free(cmd_copy);
      return NON_FATAL_ERR;
    }
    if (extra_arg) {
      printf("(CLIENT) >>> ERROR: Too many arguments.\nUsage: /switch [channel]\n");
      printf("> ");
      fflush(stdout);
      free(cmd_copy);
      return NON_FATAL_ERR;
    }
    if (strlen(argument) > CHANNEL_MAX_CHAR) {
      printf("(CLIENT) >>> ERROR: Channel name > %d characters.\n", CHANNEL_MAX_CHAR);
      printf("> ");
      fflush(stdout);
      free(cmd_copy);
      return NON_FATAL_ERR;
    }
    strncpy(user->current_channel, argument, CHANNEL_MAX_CHAR);
    printf("Switched to channel: %s\n", argument);

  } else {
    printf("(CLIENT) >>> ERROR: Unknown command\n");
    printf("> ");
    fflush(stdout);
    free(cmd_copy);
    return NON_FATAL_ERR;
  }

  printf("> ");
  fflush(stdout);
  free(cmd_copy);
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

    printf("\r\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");

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

    printf("\r\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");

    printf("%s", str);
  } else if (response->txt_type == TXT_SAY) {
    text_say *res = (text_say *)response;

    printf("\r\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");

    printf("[%s][%s]: %s\n", res->channel, res->username, res->text);

  } else if (response->txt_type == TXT_ERROR) {
    // TODO: Handle Error Messages
  }

  printf("> ");
  fflush(stdout);

  return SUCCESS;
}

int main(int argc, char **argv) {

  if (argc != 4) {
    printf("Usage: ./client [server_hostname] [server_port] [username]\n");
    exit(EXIT_FAILURE);
  }

  // Socket setup
  int sockfd;
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  // Server address setup
  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(atoi(argv[2]));
  if (inet_pton(AF_INET, argv[1], &(servaddr.sin_addr)) <= 0) {
    perror("Invalid address/ Address not supported");
    goto fail_exit;
  }

  // User setup
  user_info *user = (user_info *)malloc(sizeof(user_info));

  if (strlen(argv[3]) > USERNAME_MAX_CHAR) {
    printf("(CLIENT) >>> ERROR: Username > %d characters.\n", USERNAME_MAX_CHAR);
    goto fail_exit;
  }

  strncpy(user->username, argv[argc - 1], USERNAME_MAX_CHAR);
  strncpy(user->current_channel, "Common", CHANNEL_MAX_CHAR); // Default channel

  printf("CLIENT STARTING...\n");

  // Send initial login packet
  send_packet(sockfd, servaddr, REQ_LOGIN, user, NULL);

  // Buffer setup
  socklen_t len = sizeof(servaddr);

  printf("> ");
  fflush(stdout);
  // Main loop
  while (1) {
    // Setup select() monitoring
    fd_set watch_fds;
    FD_ZERO(&watch_fds);
    FD_SET(sockfd, &watch_fds);
    FD_SET(STDIN_FILENO, &watch_fds);
    int maxfd = (sockfd > STDIN_FILENO) ? sockfd : STDIN_FILENO;

    // Wait for activity on either socket or stdin
    if (select(maxfd + 1, &watch_fds, NULL, NULL, NULL) < 0) {
      perror("select error");
      goto fail_exit;
    }

    // Handle socket activity (server messages)
    if (FD_ISSET(sockfd, &watch_fds)) {
      char server_buffer[CLIENT_BUFFER_SIZE];

      int n = recvfrom(sockfd, server_buffer, sizeof(server_buffer), 0, (struct sockaddr *)&servaddr, &len);
      if (n < 0) {
        perror("recvfrom() failed");
        goto fail_exit;
      }

      // printf("NUM BYTES RECEIVED: %d\n", n);
      // print_text((text *)server_buffer);
      handle_response((text *)server_buffer);
    }

    // Handle stdin activity (user input)
    if (FD_ISSET(STDIN_FILENO, &watch_fds)) {

      // Clear old inputs
      fflush(stdin);
      char raw_input[1024 * 10];
      char message_buffer[SAY_MAX_CHAR + 1];

      fgets(raw_input, 1024 * 10, stdin);

      // Prevent buffer from being overflowed by only allowing a max number of 64 chars
      strncpy(message_buffer, raw_input, SAY_MAX_CHAR);
      message_buffer[strcspn(message_buffer, "\n")] = '\0'; // If message didn't overflow then replace \n with end line
      message_buffer[SAY_MAX_CHAR] = '\0';                  // If message does overflow then place end line

      // skip empty messages
      if (strlen(message_buffer) == 0) {
        printf("> ");
        fflush(stdout);
        continue;
      }

      if (message_buffer[0] == '/') {
        int result = handle_command(sockfd, servaddr, message_buffer, user);
        if (result < 0) {
          goto fail_exit;
        } else if (result == SUCCESS_EXIT) {
          goto success_exit;
        }
      } else {

        if (strlen(message_buffer) > SAY_MAX_CHAR) {
          printf("(CLIENT) >>> ERROR: Message length > %d characters\n", SAY_MAX_CHAR);
        }

        send_message(sockfd, servaddr, user, message_buffer);
      }
    }
  }

success_exit:
  close(sockfd);
  free(user);
  exit(SUCCESS);

fail_exit:
  close(sockfd);
  free(user);
  exit(EXIT_FAILURE);
}
