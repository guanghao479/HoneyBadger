/**
 * To run the test, do:
 * ./SgSimpleSockClientTest
 */

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/framework/StdOutFormatTarget.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <iostream>
#include <string>

#include "SimpleSockClient.h"
#include "../common/XStr.cpp"

using namespace std;
using namespace xercesc;

char const* sHost = "127.0.0.1";
int nPort         = kServerPort;

int total_passed = 0;
int total_tests = 0;
void log_without_connect();
void log_some_msgs();
void new_register(std::string, std::string, std::string);
void dup_register();
void valid_login();
void invalid_login();
void init_sync();
void event_sync();
void createRegisterDoc(DOMDocument* doc, string id, string pw, string email);
void log_some_msgs(string msg, int len);
void writeOutDOM(DOMDocument* myDoc, DOMImplementation* impl);
void createLoginDoc(DOMDocument* doc, string id, string pw, string hostid);
void requestLogin(string, string, string);

int main(int argc, char** argv) {
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
  printf("SgClient Unit Test:\n");

  log_without_connect();
  log_some_msgs();
  new_register("jfu", "123", "jfu@cs.rpi.edu");
  requestLogin("jfu", "123", "1234567890");
  dup_register();
  valid_login();
  invalid_login();
  init_sync();
  event_sync();

  std::cout <<std::endl << "---------------------------" << std::endl
    << "  " << total_passed << "/" << total_tests
    << " tests passed." << std::endl
    << "---------------------------" << std::endl;

fin:
  XMLPlatformUtils::Terminate();

  return OK;
}

void log_without_connect() {
  total_tests ++;
  std::cout << std::endl << "====>" << std::endl;
  //get a string to fire on wire
  int sendStringLen = kMsgLen;
  uint8_t* sendString = (uint8_t*) malloc(sizeof(uint8_t) * sendStringLen);
  int i;
  for( i = 0 ; i < sendStringLen; i++) {
    sendString[i] = 'a';
  }

  //add a client/connection
  client_t clients;
  memset(&clients, '\0', sizeof(client_t));
  client_t* client = &clients;

  int err = hb_log(client, sendString, sendStringLen);

  assert(err == NOT_READY);
  std::cout << "log_without_connect() test passed" << std::endl;

  total_passed ++;
}

void log_some_msgs() {
  total_tests ++;
  std::cout << std::endl << "====>" << std::endl;
  //get a string to fire on wire
  int sendStringLen = kMsgLen;
  uint8_t* sendString = (uint8_t*) malloc(sizeof(uint8_t) * sendStringLen);
  int i;
  for( i = 0 ; i < sendStringLen; i++) {
    sendString[i] = 'a';
  }

  //add a client/connection
  client_t clients;
  memset(&clients, '\0', sizeof(client_t));
  client_t* client = &clients;

  //try to connect
  if(hb_connect(sHost, nPort, client) != 0) {
    perror("hb_connect() failed\n");
    return;
    //exit(1);
  }

  //send kMsgNum messages synchronously
  int counter = 0;
  while(counter < 3) {
    int err = hb_log(client, sendString, sendStringLen);
    assert(err == OK);
    counter++;
    //printf("Got a correct reply from server, counter = %d\n", counter);
    //sleep(1);
  }

  //disconnect
  hb_disconnect(client);

  printf("All messages acknowledged, SgSimpleSockClientTest exit normally\n");
  std::cout << "log_some_msgs() test passed" << std::endl;
  total_passed ++;
}

void requestLogin(std::string id, std::string passwd, std::string hostid) {

  total_tests ++;
  std::cout << std::endl << "====>" << std::endl;

  DOMDocument*   myDoc;
  DOMImplementation* impl =  DOMImplementationRegistry::getDOMImplementation(X("Core"));
  assert(impl != NULL);
  myDoc = impl->createDocument(
      0,                    // root element namespace URI.
      X("HBMessage"),         // root element name (it doesn't like space in between)
      0);

  createLoginDoc(myDoc, id, passwd, hostid);
  writeOutDOM(myDoc, impl);

  std::cout << "loginRequest(): id="<<id <<", passwd="<<passwd << std::endl;
  total_passed++;
}

void new_register(std::string id, std::string passwd, std::string email) {
  total_tests ++;
  std::cout << std::endl << "====>" << std::endl;

  DOMDocument*   myDoc;
  DOMImplementation* impl =  DOMImplementationRegistry::getDOMImplementation(X("Core"));
  assert(impl != NULL);
  myDoc = impl->createDocument(
      0,                    // root element namespace URI.
      X("HBMessage"),         // root element name (it doesn't like space in between)
      0);

  createRegisterDoc(myDoc, id, passwd, email);
  writeOutDOM(myDoc, impl);

  std::cout << "new_register(): id="<<id <<", passwd="<<passwd << std::endl;
  total_passed++;
}

void writeOutDOM(DOMDocument* myDoc, DOMImplementation* impl){

  const XMLSize_t elementCount = myDoc->getElementsByTagName(X("*"))->getLength();
  //impl          = DOMImplementationRegistry::getDOMImplementation(X("LS"));
  DOMLSSerializer   *theSerializer = ((DOMImplementationLS*)impl)->createLSSerializer();
  DOMConfiguration *theConfig = theSerializer->getDomConfig();
  theConfig->setParameter(X("format-pretty-print"), true);
  //if ( theSerializer->canSetFeature(XMLUni::fgDOMWRTFormatPrettyPrint, true) )
  //      theSerializer->setFeature(XMLUni::fgDOMWRTFormatPrettyPrint, true);
  DOMLSOutput       *theOutputDesc = ((DOMImplementationLS*)impl)->createLSOutput();
  //XMLFormatTarget *myFormTarget = new StdOutFormatTarget();
  MemBufFormatTarget *myFormTarget = new MemBufFormatTarget();
  theOutputDesc->setByteStream(myFormTarget);
  theSerializer->write(myDoc, theOutputDesc);
  cout << "MemBuf len: "<< myFormTarget->getLen() << ", content: \n"<< myFormTarget->getRawBuffer() << endl;

  int msg_len = myFormTarget->getLen();
  string output((char*)myFormTarget->getRawBuffer());
  log_some_msgs(output, msg_len);
}


void dup_register() {
}

void valid_login() {
}

void invalid_login() {
}

void init_sync() {
}

void event_sync() {
}

void createRegisterDoc(DOMDocument* doc, string id, string pw, string email){
  int errorCode = 0;

  try
  {
    /*
       doc = impl->createDocument(
       0,                    // root element namespace URI.
       X("company"),         // root element name
       0);                   // document type object (DTD).
       */

    DOMElement* rootElem = doc->getDocumentElement();

    DOMElement*  prodElem = doc->createElement(X("MessageType"));
    rootElem->appendChild(prodElem);

    prodElem->setAttribute(X("Name"), X("Register"));

    DOMElement* nextElem = doc->createElement(X("ActionType"));
    prodElem->appendChild(nextElem);
    nextElem->setAttribute(X("Name"), X("Request"));

    DOMElement* userElem = doc->createElement(X("User"));
    nextElem->appendChild(userElem);

    DOMElement*  catElem = doc->createElement(X("uid"));
    userElem->appendChild(catElem);

    //catElem->setAttribute(X("idea"), X("great"));

    DOMText*    catDataVal = doc->createTextNode(X(id.c_str()));
    catElem->appendChild(catDataVal);

    DOMElement*  devByElem = doc->createElement(X("passwd"));
    userElem->appendChild(devByElem);

    DOMText*  devByDataVal = doc->createTextNode(X(pw.c_str()));
    devByElem->appendChild(devByDataVal);

    DOMElement*  evaByElem = doc->createElement(X("email"));
    userElem->appendChild(evaByElem);

    DOMText*  evaByDataVal = doc->createTextNode(X(email.c_str()));
    evaByElem->appendChild(evaByDataVal);

    //
    // Now count the number of elements in the above DOM tree.
    //

    const XMLSize_t elementCount = doc->getElementsByTagName(X("*"))->getLength();
    cout << "The tree just created contains: " << elementCount << " elements." << endl;

    //doc->release();
  }
  catch (const OutOfMemoryException&)
  {
    cerr << "OutOfMemoryException" << endl;
    errorCode = 5;
  }
  catch (const DOMException& e)
  {
    cerr << "DOMException code is:  " << e.code << endl;
    errorCode = 2;
  }
  catch (...)
  {
    cerr << "An error occurred creating the document" << endl;
    errorCode = 3;
  }

}

void createLoginDoc(DOMDocument* doc, string id, string pw, string hostid){
  int errorCode = 0;

  try
  {
    /*
       doc = impl->createDocument(
       0,                    // root element namespace URI.
       X("company"),         // root element name
       0);                   // document type object (DTD).
       */

    DOMElement* rootElem = doc->getDocumentElement();

    DOMElement*  prodElem = doc->createElement(X("MessageType"));
    rootElem->appendChild(prodElem);

    prodElem->setAttribute(X("Name"), X("Login"));

    DOMElement* nextElem = doc->createElement(X("ActionType"));
    prodElem->appendChild(nextElem);
    nextElem->setAttribute(X("Name"), X("Request"));

    DOMElement* userElem = doc->createElement(X("User"));
    nextElem->appendChild(userElem);

    DOMElement*  catElem = doc->createElement(X("uid"));
    userElem->appendChild(catElem);

    //catElem->setAttribute(X("idea"), X("great"));

    DOMText*    catDataVal = doc->createTextNode(X(id.c_str()));
    catElem->appendChild(catDataVal);

    DOMElement*  devByElem = doc->createElement(X("passwd"));
    userElem->appendChild(devByElem);

    DOMText*  devByDataVal = doc->createTextNode(X(pw.c_str()));
    devByElem->appendChild(devByDataVal);

    DOMElement*  evaByElem = doc->createElement(X("hostid"));
    userElem->appendChild(evaByElem);

    DOMText*  evaByDataVal = doc->createTextNode(X(hostid.c_str()));
    evaByElem->appendChild(evaByDataVal);

    //
    // Now count the number of elements in the above DOM tree.
    //

    const XMLSize_t elementCount = doc->getElementsByTagName(X("*"))->getLength();
    cout << "The tree just created contains: " << elementCount << " elements." << endl;

    //doc->release();
  }
  catch (const OutOfMemoryException&)
  {
    cerr << "OutOfMemoryException" << endl;
    errorCode = 5;
  }
  catch (const DOMException& e)
  {
    cerr << "DOMException code is:  " << e.code << endl;
    errorCode = 2;
  }
  catch (...)
  {
    cerr << "An error occurred creating the document" << endl;
    errorCode = 3;
  }

}
void log_some_msgs(string msg, int len) {
  //get a string to fire on wire
  assert(len > 0);
  int sendStringLen = len;
  string sendString(msg);

  //add a client/connection
  client_t clients;
  memset(&clients, '\0', sizeof(client_t));
  client_t* client = &clients;

  //try to connect
  if(hb_connect(sHost, nPort, client) != 0) {
    perror("hb_connect() failed\n");
    return;
    //exit(1);
  }

  //send kMsgNum messages synchronously
  int counter = 0;
  while(counter < 1) {
    int err = hb_log(client, (uint8_t*)sendString.c_str(), sendStringLen);
    assert(err == OK);
    counter++;
    //printf("Got a correct reply from server, counter = %d\n", counter);
    //sleep(1);
  }

  //disconnect
  hb_disconnect(client);

  printf("All messages acknowledged.\n");
}
