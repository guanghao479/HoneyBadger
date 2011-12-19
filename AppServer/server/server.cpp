#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/bufferevent_ssl.h>
#include <event2/buffer.h>
#include <event2/thread.h>
#include <arpa/inet.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include <glog/logging.h>

#include <xercesc/util/PlatformUtils.hpp>

#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <iostream>

#include "server.h"
#include "../common/HBcommon.h"
#include "../common/XStr.cpp"
#include "Message.cpp" // TODO: add header file!

//using namespace std;
using namespace xercesc;

pthread_t threads[10240]; // TODO: err, magic number.. need a more proper way to manage threads
int conn_count = 0;

int read_out_buffer(struct evbuffer* input, char** precord, uint32_t* precord_len) {
  size_t buffer_len = evbuffer_get_length(input);
  if(buffer_len < sizeof(frame_t)) {
    LOG(WARNING) << "Warning: now buffer_len=" << buffer_len << ", The size field hasn't arrived yet";
    return BAD_BUFFER;
  };

  uint32_t record_len; // TODO: endianess?
  evbuffer_copyout(input, &record_len, sizeof(uint32_t));
  if(buffer_len < record_len + 4) {
    LOG(WARNING) << "Warning: The record hasn't arrived";
    return BAD_BUFFER;
  }

  // note: caller responsible for free mem
  // note 2: alloc an extra byte for string terminator
  char* record = (char*) malloc(record_len+1);
  if( record == NULL ) return OUT_OF_MEMORY;
  memset(record, '\0', record_len+1);

  evbuffer_drain(input, 4);
  evbuffer_remove(input, record, record_len);

  *precord = record;
  *precord_len = record_len;

  //printd("size: %d, content: %s, real_len: %d\n", record_len, record, strlen(record));

  return OK;
}

// interpret and process this request in record string
int process_request(char* record, uint32_t record_len, string* reply_str) {
  ErrorCode ret;
  LOG(INFO) << "process_request(): record="<<record << ", length="<< record_len;

  string str_record(record);
  Message msg(str_record, (size_t) record_len);
  ret =  msg.parseXML();

  *reply_str = msg.getReplyStr();

  return ret;
}

/* This callback is invoked when there is data to read on bev. */
  static void
echo_read_cb(struct bufferevent *bev, void *ctx)
{
  LOG(INFO) << ("echo_read_cb():");

  int fd = (int) bufferevent_getfd(bev);

  LOG(INFO) << "now its fd=" << fd ;
  // TODO: this is a quick hack, each thread is alloc'ed to do an event.
  //       preferably make each therad handles each _active_ connections maybe?
  int rc = pthread_create(&threads[fd], NULL, run_thread, (void *)bev);
  if (rc){
    LOG(ERROR) << "ERROR; return code from pthread_create() is "<< rc << ", fd=" << fd;
    exit(-1);
  }
}

void* run_thread(void* ctx) {
  struct bufferevent *bev = (struct bufferevent*) ctx;

  struct evbuffer *input = bufferevent_get_input(bev);
  struct evbuffer *output = bufferevent_get_output(bev);

  // get the buffer size and content out
  char* record;
  uint32_t record_len;
  assert(read_out_buffer(input, &record, &record_len) == OK);

  // process this request
  string reply_str;

  ErrorCode ret = (ErrorCode) process_request(record, record_len, &reply_str);
  if (ret == OK) {
    //fire off the reply message
    evbuffer_add(output, reply_str.c_str(), reply_str.length());
  }
  else if (ret == BAD_XML) {
    // do some extra processing for invalid xml messages?
    evbuffer_add(output, reply_str.c_str(), reply_str.length());
  }

  free(record);

  pthread_exit(NULL);
}

  static void
echo_write_cb(struct bufferevent *bev, void *ctx)
{
   LOG(INFO) << "echo_write_cb()" ;
}
  static void
echo_event_cb(struct bufferevent *bev, short events, void *ctx)
{
  LOG(INFO) << ("echo_event_cb():");

  if (events & BEV_EVENT_ERROR)
    perror("Error from bufferevent");
  if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
    bufferevent_free(bev);
    LOG(INFO) << "buffer event freed" << ", now conn_count=" << --conn_count;
  }
}

  static void
accept_conn_cb(struct evconnlistener *listener,
    evutil_socket_t fd, struct sockaddr *address, int socklen,
    void *ctx)
{
  LOG(INFO) << "accept_conn_cb(): fd=" << fd <<", "<< "socklen="<<socklen
                             << ", conn_count=" << conn_count++ << endl;

  /* We got a new connection! Set up a bufferevent for it. */
  struct event_base *base = evconnlistener_get_base(listener);
  struct bufferevent *bev = bufferevent_socket_new(
      base, fd, BEV_OPT_CLOSE_ON_FREE|BEV_OPT_THREADSAFE); // so that we can pass it to another thread
  assert(bev != NULL);

  bufferevent_setcb(bev, echo_read_cb, echo_write_cb, echo_event_cb, NULL);
  bufferevent_enable(bev, EV_READ|EV_WRITE);

  LOG(INFO) << "end of accept_conn_cb()" << endl;
}

  static void
accept_error_cb(struct evconnlistener *listener, void *ctx)
{
  struct event_base *base = evconnlistener_get_base(listener);
  int err = EVUTIL_SOCKET_ERROR();
  fprintf(stderr, "Got an error %d (%s) on the listener. "
      "Shutting down.\n", err, evutil_socket_error_to_string(err));

  event_base_loopexit(base, NULL);
}

int main(int argc, char **argv)
{
  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);

  // to init xerces-c lib stuff
  try {
    XMLPlatformUtils::Initialize();
  }
  catch (const XMLException& toCatch) {
    // Do your failure processing here
    char* message = XMLString::transcode(toCatch.getMessage());
    cout << "Error during xerces-c initialization! :\n"
      << message << "\n";
    XMLString::release(&message);
    return 1;
  }

  struct event_base *base;
  struct evconnlistener *listener;
  SSL_CTX* ctx; // TODO: to add secure transport layer
  struct sockaddr_in sin;

  int port = kServerPort;

  if (argc > 1) {
    port = atoi(argv[1]);
  }
  if (port<=0 || port>65535) {
    puts("Invalid port");
    return 1;
  }

  base = event_base_new();
  if (!base) {
    puts("Couldn't open event base");
    return 1;
  }

  cout << "===============================================" << endl;
  // Check libevent version
  cout << "Server uses libevent version:\t " << event_get_version() << endl;
  // Note this function needs to link with libevent as well as libevent_pthreads
  cout << "Server libevent pthread support:\t " << ((evthread_use_pthreads()==0)?"ON":"OFF") << endl;
  // see http://google-glog.googlecode.com/svn/trunk/doc/glog.html for details
  cout << "Server is using glog, log path is /tmp/HB_log/" << endl;
  cout << "===============================================" << endl;

  /* Clear the sockaddr before using it, in case there are extra
   * platform-specific fields that can mess us up. */
  memset(&sin, 0, sizeof(sin));
  /* This is an INET address */
  sin.sin_family = AF_INET;
  /* Listen on 0.0.0.0 */
  //sin.sin_addr.s_addr = htonl(0);
  sin.sin_addr.s_addr = inet_addr("127.0.0.1");
  /* Listen on the given port. */
  sin.sin_port = htons(port);

  listener = evconnlistener_new_bind(base, accept_conn_cb, NULL,
      LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1,
      (struct sockaddr*)&sin, sizeof(sin));
  if (!listener) {
    perror("Couldn't create listener");
    return 1;
  }
  evconnlistener_set_error_cb(listener, accept_error_cb);

  event_base_dispatch(base);

  evconnlistener_free(listener);

  // need to terminate xerces-c stuff too
fin:
  XMLPlatformUtils::Terminate();

  return 0;
}
