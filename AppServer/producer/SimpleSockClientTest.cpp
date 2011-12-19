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
void send_new_register(std::string, std::string, std::string);
void send_text_file(std::string, std::string, std::string);
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
DOMDocument* createHBMessage();
void createAndAppendTextFileDoc(DOMDocument* doc, string userid, string hostid,
        string filepath);

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
  send_new_register("jfu", "good_password_123", "fuj@cs.rpi.edu");
  requestLogin("jfu", "im_a_wrong_password", "1234567890");
  send_text_file("fmaj7", "k234324io2u3", "abc/def/ksdl");
  dup_register();
  valid_login();
  invalid_login();
  init_sync();
  event_sync();

  std::cout <<std::endl << "---------------------------" << std::endl
    << "  " << total_passed << "/" << total_tests
    << " tests passed." << std::endl
    << "---------------------------" << std::endl;

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
  myDoc = createHBMessage();

  createLoginDoc(myDoc, id, passwd, hostid);
  writeOutDOM(myDoc, impl);

  std::cout << "loginRequest(): id="<<id <<", passwd="<<passwd << std::endl;
  total_passed++;
}

void send_new_register(std::string id, std::string passwd, std::string email) {
  total_tests ++;
  std::cout << std::endl << "====>" << std::endl;

  DOMDocument*   myDoc;
  DOMImplementation* impl =  DOMImplementationRegistry::getDOMImplementation(X("Core"));
  assert(impl != NULL);
  myDoc = createHBMessage();

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

void send_text_file(std::string userid, std::string hostid, std::string filepath) {
    total_tests ++;
    std::cout << std::endl << "====>" << std::endl;

    DOMImplementation* impl =  DOMImplementationRegistry::getDOMImplementation(X("Core"));
    DOMDocument* myDoc = createHBMessage();
    createAndAppendTextFileDoc(myDoc, userid, hostid, filepath);

    writeOutDOM(myDoc, impl);
    std::cout << "send_text_file(): userid="<<userid <<", hostid="<<hostid <<", filepath="<<filepath << std::endl;
    total_passed++;
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

void createRegisterDoc(DOMDocument* doc, string id, string pw, string email) {
  int errorCode = 0;

  try
  {
    DOMElement* rootElem = doc->getDocumentElement();

    DOMElement*  typeElem = doc->createElement(X("registerRequestMessageType"));
    rootElem->appendChild(typeElem);

    /*
    prodElem->setAttribute(X("name"), X("registerRequestMessageType"));

    DOMElement* nextElem = doc->createElement(X("ActionType"));
    prodElem->appendChild(nextElem);
    nextElem->setAttribute(X("Name"), X("Request"));

    DOMElement* userElem = doc->createElement(X("User"));
    nextElem->appendChild(userElem);
    */

    DOMElement*  catElem = doc->createElement(X("userid"));
    typeElem->appendChild(catElem);

    //catElem->setAttribute(X("idea"), X("great"));

    DOMText*    catDataVal = doc->createTextNode(X(id.c_str()));
    catElem->appendChild(catDataVal);

    DOMElement*  devByElem = doc->createElement(X("password"));
    typeElem->appendChild(devByElem);

    DOMText*  devByDataVal = doc->createTextNode(X(pw.c_str()));
    devByElem->appendChild(devByDataVal);

    // new schema got rid of email info
    /*
    DOMElement*  evaByElem = doc->createElement(X("email"));
    typeElem->appendChild(evaByElem);

    DOMText*  evaByDataVal = doc->createTextNode(X(email.c_str()));
    evaByElem->appendChild(evaByDataVal);
    */

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

DOMDocument* createHBMessage() {
    DOMImplementation* impl =  DOMImplementationRegistry::getDOMImplementation(X("Core"));
    assert(impl != NULL);
    return impl->createDocument(
            0,                    // root element namespace URI.
            X("HBMessages"),         // root element name (it doesn't like space in between)
            0);

}

DOMElement* createTextNode(DOMDocument* doc, string name, string val) {
    DOMElement* textNodeElem = doc->createElement(X(name.c_str()));
    DOMText* textNodeVal = doc->createTextNode(X(val.c_str()));
    textNodeElem->appendChild(textNodeVal);
    return textNodeElem;
}

void createAndAppendTextFileDoc(DOMDocument* doc, string userid, string hostid,
        string filepath) {

    int errorCode = 0;
    try
    {
        DOMElement* rootElem = doc->getDocumentElement();

        DOMElement* typeElem = doc->createElement(X("Type"));
        rootElem->appendChild(typeElem);
        typeElem->setAttribute(X("Type"), X("File"));

        DOMElement* requestElem = doc->createElement(X("Request"));
        rootElem->appendChild(requestElem);

        DOMElement* uidElem = createTextNode(doc, "uid", userid);
        requestElem->appendChild(uidElem);

        DOMElement* hostidElem = createTextNode(doc, "hostid", hostid);
        requestElem->appendChild(hostidElem);

        DOMElement* fileElem = doc->createElement(X("file"));
        requestElem->appendChild(fileElem);

        DOMElement* filepathElem = createTextNode(doc, "path", filepath);
        fileElem->appendChild(filepathElem);

        //string filecontent = getTextFileContent(string filepath);
        string filecontent = "test";
        DOMElement* filecontentElem = createTextNode(doc, "content", filecontent);
        fileElem->appendChild(filecontentElem);

        // Now count the number of elements in the above DOM tree.
        const XMLSize_t elementCount = doc->getElementsByTagName(X("*"))->getLength();
        cout << "The tree just created contains: " << elementCount << " elements." << endl;
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


void createLoginDoc(DOMDocument* doc, string id, string pw, string email) {
  int errorCode = 0;

  try
  {
    DOMElement* rootElem = doc->getDocumentElement();

    DOMElement*  typeElem = doc->createElement(X("loginRequestMessageType"));
    rootElem->appendChild(typeElem);

    DOMElement*  catElem = doc->createElement(X("userid"));
    typeElem->appendChild(catElem);

    //catElem->setAttribute(X("idea"), X("great"));

    DOMText*    catDataVal = doc->createTextNode(X(id.c_str()));
    catElem->appendChild(catDataVal);

    DOMElement*  devByElem = doc->createElement(X("password"));
    typeElem->appendChild(devByElem);

    DOMText*  devByDataVal = doc->createTextNode(X(pw.c_str()));
    devByElem->appendChild(devByDataVal);

    // new schema got rid of email info
    /*
    DOMElement*  evaByElem = doc->createElement(X("email"));
    typeElem->appendChild(evaByElem);

    DOMText*  evaByDataVal = doc->createTextNode(X(email.c_str()));
    evaByElem->appendChild(evaByDataVal);
    */

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
