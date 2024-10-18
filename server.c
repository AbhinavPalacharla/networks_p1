#include "server.h"
#include "duckchat.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
  server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
  server_addr.sin_port = htons(PORT);

  // Bind the socket with the server address
  if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("bind failed");
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  unsigned int client_addr_len =
      sizeof(client_addr); // length of the client address

  char buffer[SERVER_BUFFER_SIZE];

  while (1) {
    memset(buffer, 0, sizeof(buffer));

    // Receive message from client
    int msg_len = recvfrom(sockfd, (char *)buffer, SERVER_BUFFER_SIZE, 0,
                           (struct sockaddr *)&client_addr, &client_addr_len);

    if (msg_len < 0) {
      perror("Failed to recieve message from client.");
      close(sockfd);
      exit(EXIT_FAILURE);
    }

    buffer[msg_len] = '\0';

    printf("(CLIENT) >>> %s\n", buffer);

    // Send acknowledgment to client
    const char *ack = "Message received!";
    sendto(sockfd, ack, strlen(ack), 0, (struct sockaddr *)&client_addr,
           client_addr_len);
    printf("Acknowledgment sent to client.\n");
  }

  close(sockfd);
  return 0;
}
