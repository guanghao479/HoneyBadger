/**
 * SgSimpleSockClient.c
 *
 * @author  jingfu
 */

#include "SgSimpleSockClient.h"

/**
 * Method to set up a connection to SgServrer for client
 * It will initiate the connection structure and setup a connection
 *
 * @param: host ip address of remote server
 * @param: port port number of remote server
 * @param: conn pointer to the actual connection object
 *
 * @return: 0 if non-error
 */
int hb_connect(char const* host, int port, hb_conn_t* client) {
  if(client->isRunning == 1) {
    printf("Error - already connected\n");
    return NOT_READY;
  }

  //client->mode = test; //TODO uncomment when run in prod

  client->send_tmo = kSendTimeout;
  client->recv_tmo = kRecvTimeout;

  client->sHost = host;
  client->nPort = port;

  /* Create a reliable, stream socket using TCP */
  if ((client->sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    printf("socket() failed\n");
    return NOT_READY;
  }

  /* Construct the server address structure */
  memset(&(client->servAddr), 0, sizeof(client->servAddr));// Zero out
  client->servAddr.sin_family      = AF_INET;   // Internet address family
  client->servAddr.sin_addr.s_addr = inet_addr(client->sHost); // Server IP
  client->servAddr.sin_port        = htons(client->nPort); // Server port

  if(connect(client->sock, (struct sockaddr*) &client->servAddr,
             sizeof(client->servAddr)) < 0) {
    printf("connect() failed\n");
    return NOT_READY;
  }

  //set up timeout value
  struct timeval tmo;
  tmo.tv_sec = client->recv_tmo;
	// the following two asserts fails on fz but works on my macbook
  //assert (setsockopt(client->sock, SOL_SOCKET, SO_RCVTIMEO, &tmo, sizeof(tmo)) == 0);
  tmo.tv_sec = client->send_tmo;
  //assert (setsockopt(client->sock, SOL_SOCKET, SO_SNDTIMEO, &tmo, sizeof(tmo)) == 0);

  //other set up work
  client->recvBuf = (char*)malloc(sizeof(char) * kAckLength);
  client->isRunning = 1;

  return OK;
}

/**
 * Method that would try to log a message for you
 *
 * @param:  client  pointer to our client connection structure
 * @param:  buf     the message to send
 * @param:  nbuf    the size of the message to send
 *
 * @return: 0 if non-error
 */
int hb_log(hb_conn_t* client, uint8_t* buf, int nbuf) {

  if(client->isRunning != 1) {
    printf("Error - never connected\n");
    return NOT_READY;
  }

  //do framing
  frame_t len = nbuf;
  assert((nbuf+sizeof(len)) <= kSingleMsgSizeLimit);
  memcpy(client->sendBuf, &len, sizeof(len));
  client->sendSize = sizeof(len);
  if(client->mode == test) {
    buf[0]='$';
    int i;
    for(i = 1; i < nbuf -1; i++) buf[i] = (buf[i]+1 -48)%(122-48) + 48;//ASCII
    buf[nbuf-1] = '#';
  }
  memcpy(&client->sendBuf[client->sendSize], buf, nbuf);
  client->sendSize += nbuf;
	//printd("buf: %s\n", &client->sendBuf[4]);

  if( send(client->sock, client->sendBuf, client->sendSize, 0)
      != client->sendSize ) {
    printf("send() got a problem\n");
    return SEND_FAIL;
  }

  int totalBytesRcvd = 0, bytesRcvd;
	int replyMsgMaxSize = 512; //TODO: need to properly define this
	free(client->recvBuf);
	client->recvBuf = (char*) malloc(replyMsgMaxSize);
	memset(client->recvBuf, 0, replyMsgMaxSize);
  //while( totalBytesRcvd < 10)
	{
    if((bytesRcvd = recv(client->sock, client->recvBuf, replyMsgMaxSize, 0)) <= 0) {
        printf("recv() got a problem\n");
        return RECV_FAIL;
    }
    totalBytesRcvd += bytesRcvd;
  }

	printf("hb_log: sent %d bytes, received %d bytes: %s \n",
					client->sendSize, totalBytesRcvd, client->recvBuf);

  //check buffer correctness
  //if(strcmp(client->recvBuf, kAckStr) != 0) {
  //  printf("The Ack message from server doesn't look right\n");
  //  return SERVER_IS_CRAZY;
  //}

  return OK;
}

/**
 * Method that disconnect the socket and clean up memory alloc'd earlier
 *
 * @param:  client  pointer to the structure that contains connection
 *
 * @return: return 0 if non-error. What error could happen when close a socket?
 */
int hb_disconnect(hb_conn_t* client) {
  if(client->isRunning == 1) {
    close(client->sock);
    free(client->recvBuf);
    client->isRunning = 0;
    return OK;
  }
  else {
    printf("Error - never connected\n");
    return NOT_READY;
  }
}
