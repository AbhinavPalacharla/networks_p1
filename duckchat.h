#ifndef DUCKCHAT_H
#define DUCKCHAT_H

/* Path names to unix domain sockets should not be longer than this */
#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX 108
#endif

/* This tells gcc to "pack" the structure. Normally, gcc will
 * inserting padding into a structure if it feels it is convenient.
 * When the structure is packed, gcc gaurantees that all the bytes
 * will fall exactly where specified. */
#define packed __attribute__((packed))

/* Define the length limits */
#define USERNAME_MAX_CHAR 32
#define CHANNEL_MAX_CHAR 32
#define SAY_MAX_CHAR 64

/* Define some types for designating request and text codes */
// typedef int request_t;
// typedef int TXT_TYPE;

/* Define codes for request types. These are the messages sent to the server. */
// #define REQ_LOGIN 0
// #define REQ_LOGOUT 1
// #define REQ_JOIN 2
// #define REQ_LEAVE 3
// #define REQ_SAY 4
// #define REQ_LIST 5
// #define REQ_WHO 6
// #define REQ_KEEP_ALIVE 7 /* Only needed by graduate students */

typedef enum { REQ_LOGIN, REQ_LOGOUT, REQ_JOIN, REQ_LEAVE, REQ_SAY, REQ_LIST, REQ_WHO, REQ_KEEP_ALIVE } REQUEST_TYPE;

/* Define codes for text types. These are the messages sent to the client. */
// #define TXT_SAY 0
// #define TXT_LIST 1
// #define TXT_WHO 2
// #define TXT_ERROR 3

typedef enum { TXT_SAY, TXT_LIST, TXT_WHO, TXT_ERROR } TXT_TYPE;

/* This structure is used for a generic request type, to the server. */
typedef struct _request {
  REQUEST_TYPE req_type;
} packed request;

/* Once we've looked at req_type, we then cast the pointer to one of
 * the types below to look deeper into the structure. Each of these
 * corresponds with one of the REQ_ codes above. */
typedef struct _request_login {
  REQUEST_TYPE req_type; /* = REQ_LOGIN */
  char username[USERNAME_MAX_CHAR];
} packed request_login;

typedef struct _request_logout {
  REQUEST_TYPE req_type; /* = REQ_LOGOUT */
} packed request_logout;

typedef struct _request_join {
  REQUEST_TYPE req_type; /* = REQ_JOIN */
  char channel[CHANNEL_MAX_CHAR];
} packed request_join;

typedef struct _request_leave {
  REQUEST_TYPE req_type; /* = REQ_LEAVE */
  char channel[CHANNEL_MAX_CHAR];
} packed request_leave;

typedef struct _request_say {
  REQUEST_TYPE req_type; /* = REQ_SAY */
  char channel[CHANNEL_MAX_CHAR];
  char text[SAY_MAX_CHAR];
} packed request_say;

typedef struct _request_list {
  REQUEST_TYPE req_type; /* = REQ_LIST */
} packed request_list;

typedef struct _request_who {
  REQUEST_TYPE req_type; /* = REQ_WHO */
  char channel[CHANNEL_MAX_CHAR];
} packed request_who;

typedef struct _request_keep_alive {
  REQUEST_TYPE req_type; /* = REQ_KEEP_ALIVE */
} packed request_keep_alive;

/* This structure is used for a generic text type, to the client. */
typedef struct _text {
  TXT_TYPE txt_type;
} packed text;

/* Once we've looked at txt_type, we then cast the pointer to one of
 * the types below to look deeper into the structure. Each of these
 * corresponds with one of the TXT_ codes above. */
typedef struct _text_say {
  TXT_TYPE txt_type; /* = TXT_SAY */
  char channel[CHANNEL_MAX_CHAR];
  char username[USERNAME_MAX_CHAR];
  char text[SAY_MAX_CHAR];
} packed text_say;

/* This is a substructure used by struct text_list. */
typedef struct _channel_info {
  char channel[CHANNEL_MAX_CHAR];
} packed channel_info;

typedef struct _text_list {
  TXT_TYPE txt_type; /* = TXT_LIST */
  int n_channel;
  channel_info *channels;
} packed text_list;

/* This is a substructure used by text_who. */
typedef struct _user_info {
  char username[USERNAME_MAX_CHAR];
  char current_channel[CHANNEL_MAX_CHAR];
} packed user_info;

typedef struct _text_who {
  TXT_TYPE txt_type; /* = TXT_WHO */
  int n_username;
  char channel[CHANNEL_MAX_CHAR]; // The channel requested
  user_info *users;
} packed text_who;

typedef struct _text_error {
  TXT_TYPE txt_type;            /* = TXT_ERROR */
  char txt_error[SAY_MAX_CHAR]; // Error message
} packed text_error;

#endif
