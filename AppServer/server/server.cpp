#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/thread.h>
#include <arpa/inet.h>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/framework/StdOutFormatTarget.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/sax/InputSource.hpp>

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <iostream>

#include "../common/HBcommon.h"
#include "../common/XStr.cpp"

using namespace std;
using namespace xercesc;

typedef struct user_info_s {
  string uid;
  string passwd;
  string hostid;
  string email;
} user_info;

typedef struct msg_s {
  user_info user;
  string msg_type;
  string action_type;
} msg;

int read_out_buffer(struct evbuffer* input, char** precord, uint32_t* precord_len) {
  size_t buffer_len = evbuffer_get_length(input);
  if(buffer_len < sizeof(frame_t)) {
    printd("Warning: now buffer_len=%d, The size field hasn't arrived yet\n", buffer_len);
    return BAD_BUFFER;
  };

  uint32_t record_len; // TODO: endianess?
  evbuffer_copyout(input, &record_len, sizeof(uint32_t));
  if(buffer_len < record_len + 4) {
    printd("Warning: The record hasn't arrived\n");
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

//TODO: add exception handling
int getUserInfo(DOMElement* requestElement, user_info* user) {
  // we know User tag follows
  DOMNodeList* children = requestElement->getChildNodes();
  DOMNode* userNode = children->item(1);
  DOMElement* userElement;
  if( userNode->getNodeType() &&  // true is not NULL
      userNode->getNodeType() == DOMNode::ELEMENT_NODE ) {
    userElement = dynamic_cast< xercesc::DOMElement* >( userNode );
    if( XMLString::equals(userElement->getTagName(), X("User"))) {
      //const XMLCh* xmlch_user = userElement->getAttribute(X("Name"));
      //msg.action_type = XMLString::transcode(xmlch_user);
      cout << "is User " << endl;
      children = userElement->getChildNodes();
      const  XMLSize_t nodeCount = children->getLength();
      for( XMLSize_t xx = 0; xx < nodeCount; ++xx ) {
        DOMNode* crtNode = children->item(xx);
        DOMElement* crtElement;
        if( crtNode->getNodeType() &&  // true is not NULL
            crtNode->getNodeType() == DOMNode::ELEMENT_NODE ) {
          crtElement = dynamic_cast< xercesc::DOMElement* >( crtNode );
          if( XMLString::equals(crtElement->getTagName(), X("uid"))) {
            user->uid = XMLString::transcode(crtElement->getTextContent()) ;
          }
          else if( XMLString::equals(crtElement->getTagName(), X("passwd"))) {
            user->passwd = XMLString::transcode(crtElement->getTextContent()) ;
          }
          else if( XMLString::equals(crtElement->getTagName(), X("hostid"))) {
            user->hostid = XMLString::transcode(crtElement->getTextContent()) ;
          }
          else if( XMLString::equals(crtElement->getTagName(), X("email"))) {
            user->email = XMLString::transcode(crtElement->getTextContent()) ;
          }
        }
      }
    }
    else {
      cout << "error getting user tag" << endl;
            return BAD_XML;
    }
  }
  else {
    cout << "error getting user type" << endl;
    return BAD_XML;
  }

  cout << "getUserInfo(): uid = " << user->uid << ", passwd = " << user->passwd
                          << ", hostid = " << user->hostid << ", email = "<< user->email << endl;

  return OK;
}

// interpret and process this request in record string
// TODO: process it
int process_request(char* record, uint32_t record_len, string* reply_str) {
  ErrorCode ret;
  printd("process_request(): record=%s, length=%d", record, record_len);

  // get a parser first
  XercesDOMParser* parser = new XercesDOMParser();
  parser->setValidationScheme(XercesDOMParser::Val_Always);
  parser->setDoNamespaces(true);    // optional
  ErrorHandler* errHandler = (ErrorHandler*) new HandlerBase();
  parser->setErrorHandler(errHandler);

  // create an input source from string
  MemBufInputSource xml_buf((XMLByte*)record,(XMLSize_t) (record_len ), "test", false);

  // try to parse this document and get fields
  // TODO: maybe a better idea to put these functions into a msg class
  msg msg;
  try {
    parser->parse(xml_buf);
    // no need to free this, owned by parent parser project
    DOMDocument* xmlDoc = parser->getDocument();
    // Get the top-level element
    DOMElement* elementRoot = xmlDoc->getDocumentElement();
    if( !elementRoot ) { ret = BAD_XML; goto done;}
    cout << "root: " << XMLString::transcode(elementRoot->getTagName())<< endl;

    DOMNodeList*      children = elementRoot->getChildNodes();
    const  XMLSize_t nodeCount = children->getLength();
    cout <<" node count: " << nodeCount << endl;

    //for( XMLSize_t xx = 0; xx < nodeCount; ++xx ) { //TODO: why need to loop?
    // we know message type tag follows
    DOMNode* typeNode = children->item(1);
    DOMElement* typeElement;
    if( typeNode->getNodeType() &&  // true is not NULL
        typeNode->getNodeType() == DOMNode::ELEMENT_NODE ) {
      typeElement = dynamic_cast< xercesc::DOMElement* >( typeNode );
      if( XMLString::equals(typeElement->getTagName(), X("MessageType"))) {
        const XMLCh* xmlch_type = typeElement->getAttribute(X("Name"));
        msg.msg_type = XMLString::transcode(xmlch_type);
        cout << "msg type name= " << msg.msg_type << endl;
      }
      else {
        cout << "Error: wrong message type tag" << endl;
      }
    }
    else {
      cout << "Error getting msg type\n" << endl;
    }

    // we know action type tag follows
    children = typeElement->getChildNodes();
    DOMNode* requestNode = children->item(1);
    DOMElement* requestElement;
    if( requestNode->getNodeType() &&  // true is not NULL
        requestNode->getNodeType() == DOMNode::ELEMENT_NODE ) {
      requestElement = dynamic_cast< xercesc::DOMElement* >( requestNode );
      if( XMLString::equals(requestElement->getTagName(), X("ActionType"))) {
        const XMLCh* xmlch_request = requestElement->getAttribute(X("Name"));
        msg.action_type = XMLString::transcode(xmlch_request);
        cout << "msg action type = " << msg.action_type << endl;
      }
      else {
        cout << "error getting action type tag" << endl;
      }
    }
    else {
      cout << "error getting action type" << endl;
    }

    // now process message
    if (msg.msg_type.compare("Register") == 0) {
      // handle "Register"
      cout << "handling Register.." << endl;
      assert (getUserInfo(requestElement, &msg.user) == (int) OK);
      // now grab msg.user and do whatever processing of register
      //
    }
    else if (msg.msg_type.compare("Login") == 0) {
      // handle "Login"
      cout << "handling Login.." << endl;
      assert (getUserInfo(requestElement, &msg.user) == (int) OK);
    }


  } // end of try
  catch (const XMLException& toCatch) {
    char* message = XMLString::transcode(toCatch.getMessage());
    cout << "Exception message is: \n"
      << message << "\n";
    XMLString::release(&message);
    ret = BAD_XML;
    goto done;
  }
  catch (const DOMException& toCatch) {
    char* message = XMLString::transcode(toCatch.msg);
    cout << "Exception message is: \n"
      << message << "\n";
    XMLString::release(&message);
    ret = BAD_XML;
    goto done;
  }
  catch (...) {
    cout << "Unexpected Exception \n" ;
    ret = BAD_XML;
    goto done;
  }

  *reply_str = "REGISTER_OK";
  ret = OK;

done:
  delete parser;
  delete errHandler;

  //sleep(10);

  return ret;
}

/* This callback is invoked when there is data to read on bev. */
  static void
echo_read_cb(struct bufferevent *bev, void *ctx)
{
  printd("echo_read_cb():");
  struct evbuffer *input = bufferevent_get_input(bev);
  struct evbuffer *output = bufferevent_get_output(bev);

  // get the buffer size and content out
  char* record;
  uint32_t record_len;
  assert(read_out_buffer(input, &record, &record_len) == OK);

  // process this request
  //string reply_str("REGISTER_OK");
  string reply_str;

  ErrorCode ret = (ErrorCode) process_request(record, record_len, &reply_str);
  if (ret == OK) {
    //fire off the reply message
    evbuffer_add(output, reply_str.c_str(), reply_str.length());
  }
  else if (ret == BAD_XML) {
    reply_str = "INVALID_XML message!";
    evbuffer_add(output, reply_str.c_str(), reply_str.length());
  }

  free(record);
}

  static void
echo_write_cb(struct bufferevent *bev, void *ctx)
{
  std::cout << "echo_write_cb()" << std::endl;
}
  static void
echo_event_cb(struct bufferevent *bev, short events, void *ctx)
{
  printd("echo_event_cb():");
  if (events & BEV_EVENT_ERROR)
    perror("Error from bufferevent");
  if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
    bufferevent_free(bev);
    printf("buffer event freed\n");
  }
}

  static void
accept_conn_cb(struct evconnlistener *listener,
    evutil_socket_t fd, struct sockaddr *address, int socklen,
    void *ctx)
{
  cout << "accept_conn_cb(): fd=" << fd <<", "<< "socklen="<<socklen
                             << endl;
  /* We got a new connection! Set up a bufferevent for it. */
  struct event_base *base = evconnlistener_get_base(listener);
  struct bufferevent *bev = bufferevent_socket_new(
      base, fd, BEV_OPT_CLOSE_ON_FREE); // TODO: can't make it thread-safe(thread support not active?)
  assert(bev != NULL);

  bufferevent_setcb(bev, echo_read_cb, echo_write_cb, echo_event_cb, NULL);
  bufferevent_enable(bev, EV_READ|EV_WRITE);
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

  int
main(int argc, char **argv)
{
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

  cout << "Server uses libevent version:\t " << event_get_version() << endl;
  // Note this function needs to link with libevent as well as libevent_pthreads
  cout << "Server libevent pthread support:\t " << ((evthread_use_pthreads()==1)?"ON":"OFF") << endl;

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

  // need to terminate xerces-c stuff too
fin:
  XMLPlatformUtils::Terminate();

  return 0;
}
