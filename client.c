#include "client.h"
#include "duckchat.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
  int sockfd;
  char buffer[CLIENT_BUFFER_SIZE];
  char message_buffer[SAY_MAX];
  struct sockaddr_in servaddr;

  // Create socket file descriptor
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));

  // Filling server information
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(SERVER_PORT);
  servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);

  unsigned int n, len;

  while (1) {
    // Type message to send to server
    printf("> ");
    fgets(message_buffer, sizeof(message_buffer), stdin);
    message_buffer[strcspn(message_buffer, "\n")] = 0;

    // Check for commands
    if (strncmp(message_buffer, "/exit", strlen("/exit")) == 0) {
      printf("EXITING...\n");
      break;
    }

    // Send message to server
    sendto(sockfd, message_buffer, strlen(message_buffer), 0,
           (const struct sockaddr *)&servaddr, sizeof(servaddr));
    printf("Message sent to server.\n");

    // Receive message from server
    len = sizeof(servaddr);
    n = recvfrom(sockfd, (char *)buffer, CLIENT_BUFFER_SIZE, 0,
                 (struct sockaddr *)&servaddr, &len);
    buffer[n] = '\0';
    printf("Server: %s\n", buffer);
  }

  close(sockfd);
  return 0;
}
