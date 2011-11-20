
/**
 * SgSimpleSockClient.h
 *
 * @author  jingfu
 */

#ifndef HB_SIMPLESOCK_CLIENT_H_
#define HB_SIMPLESOCK_CLIENT_H_

#include <assert.h>
#include <time.h>
#include <arpa/inet.h> /* for sockaddr_in and inet_addr() */
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../common/HBcommon.h"

#define kSendTimeout 180
#define kRecvTimeout 180

typedef enum {
  test_echo = 0,
  test_read = 1,
  prod = 2,
  test = 3,
} test_mode_t;

//client structure on client side
typedef struct client_s {
  int id; //client id
  int isRunning; //if it is running

  //for sync mode
  char const* sHost; //host name
  int nPort;
  int sock; //socket descriptor
  struct sockaddr_in servAddr; // server address
  char* recvBuf; // to keep ack info
  uint8_t sendBuf[kSingleMsgSizeLimit]; // buffer to do framing stuff
  int sendSize;

  int send_tmo;
  int recv_tmo;

  test_mode_t mode;
} client_t;

// the connection struct that producer uses to connect to server
typedef struct client_s hb_conn_t;

#ifdef __cplusplus
extern "C" {
#endif
// connect to server, if succeed return 0, otherwise non-zero
int hb_connect(char const* host, int port, hb_conn_t* conn);
// method for producer to log a string, will return after data written to disk
int hb_log(hb_conn_t* conn, uint8_t* buf, int nbuf);
// disconnect with server
int hb_disconnect(hb_conn_t* conn);

#ifdef __cplusplus
}
#endif
#endif
