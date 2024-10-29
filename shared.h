#ifndef SHARED_H
#define SHARED_H

#include "duckchat.h"
#include <stdio.h>

#define SUCCESS 0
#define FAILURE -1
#define NON_FATAL_ERR 1
#define SUCCESS_EXIT 2
#define FAIL_EXIT 3

#define MAX_NUM_USERS 10
#define MAX_NUM_CHANNELS 10

void print_text_say(text_say *txt) {
  printf("TEXT SAY:\n");
  printf("  Type: %d (TXT_SAY)\n", txt->txt_type);
  printf("  Channel: %s\n", txt->channel);
  printf("  Username: %s\n", txt->username);
  printf("  Text: %s\n", txt->text);
}

void print_text_list(text_list *txt) {
  printf("TEXT LIST:\n");
  printf("  Type: %d (TXT_LIST)\n", txt->txt_type);
  printf("  Number of Channels: %d\n", txt->n_channel);
  printf("  Channels: ");
  if (txt->n_channel == 0) {
    printf("NONE\n");
  } else {
    printf("\n");
    for (int k = 0; k < txt->n_channel; k++) {
      printf("    - %s\n", txt->channels[k].channel);
    }
  }
}

void print_text_who(text_who *txt) {
  printf("TEXT WHO:\n");
  printf("  Type: %d (TXT_WHO)\n", txt->txt_type);
  printf("  Channel: %s\n", txt->channel);
  printf("  Number of Users: %d\n", txt->n_username);
  printf("  Users: ");
  if (txt->n_username == 0) {
    printf("NONE\n");
  } else {
    printf("\n");
    for (int i = 0; i < txt->n_username; i++) {
      printf("    - %s\n", txt->users[i].username);
    }
  }
}

void print_text_error(text_error *txt) {
  printf("TEXT ERROR:\n");
  printf("  Type: %d (TXT_ERROR)\n", txt->txt_type);
  printf("  Error Message: %s\n", txt->txt_error);
}

// Helper function to print any text type
void print_text(text *txt) {
  if (txt == NULL) {
    printf("TEXT is NULL\n");
    return;
  }

  switch (txt->txt_type) {
  case TXT_SAY:
    print_text_say((text_say *)txt);
    break;
  case TXT_LIST:
    print_text_list((text_list *)txt);
    break;
  case TXT_WHO:
    print_text_who((text_who *)txt);
    break;
  case TXT_ERROR:
    print_text_error((text_error *)txt);
    break;
  default:
    printf("UNKNOWN TEXT TYPE: %d\n", txt->txt_type);
  }
}

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

#endif
