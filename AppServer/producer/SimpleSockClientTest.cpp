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

using namespace std;
using namespace xercesc;
// ---------------------------------------------------------------------------
//  This is a simple class that lets us do easy (though not terribly efficient)
//  trancoding of char* data to XMLCh data.
// ---------------------------------------------------------------------------
class XStr
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    XStr(const char* const toTranscode)
    {
        // Call the private transcoding method
        fUnicodeForm = XMLString::transcode(toTranscode);
    }

    ~XStr()
    {
        XMLString::release(&fUnicodeForm);
    }


    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    const XMLCh* unicodeForm() const
    {
        return fUnicodeForm;
    }

private :
    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fUnicodeForm
    //      This is the Unicode XMLCh format of the string.
    // -----------------------------------------------------------------------
    XMLCh*   fUnicodeForm;
};

#define X(str) XStr(str).unicodeForm()

char const* sHost = "127.0.0.1";
int nPort         = kServerPort;

int total_passed = 0;
int total_tests = 0;
void log_without_connect();
void log_some_msgs();
void new_register(std::string, std::string);
void dup_register();
void valid_login();
void invalid_login();
void init_sync();
void event_sync();
void createIdPwDoc(DOMDocument* doc, string type, string id, string pw);
void log_some_msgs(string msg, int len);

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
	new_register("jfu", "123");
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
  while(counter < kMsgNum) {
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


void new_register(std::string id, std::string passwd) {
	total_tests ++;
	std::cout << std::endl << "====>" << std::endl;

	DOMDocument*   myDoc;
	DOMImplementation* impl =  DOMImplementationRegistry::getDOMImplementation(X("Core"));
	assert(impl != NULL);
	myDoc = impl->createDocument(
			0,                    // root element namespace URI.
			X("HB_Message"),         // root element name (it doesn't like space in between)
			0);

	createIdPwDoc(myDoc, "REGISTER", id, passwd);

	const XMLSize_t elementCount = myDoc->getElementsByTagName(X("*"))->getLength();

	//impl          = DOMImplementationRegistry::getDOMImplementation(X("LS"));
	DOMLSSerializer   *theSerializer = ((DOMImplementationLS*)impl)->createLSSerializer();
	//if ( theSerializer->canSetFeature(XMLUni::fgDOMWRTFormatPrettyPrint, true) )
	//			theSerializer->setFeature(XMLUni::fgDOMWRTFormatPrettyPrint, true);
	DOMLSOutput       *theOutputDesc = ((DOMImplementationLS*)impl)->createLSOutput();
	//XMLFormatTarget *myFormTarget = new StdOutFormatTarget();
	MemBufFormatTarget *myFormTarget = new MemBufFormatTarget();
	theOutputDesc->setByteStream(myFormTarget);
	theSerializer->write(myDoc, theOutputDesc);
	cout << "MemBuf len: "<< myFormTarget->getLen() << ", content: \n"<< myFormTarget->getRawBuffer() << endl;

	int msg_len = myFormTarget->getLen();
	string output((char*)myFormTarget->getRawBuffer());
	log_some_msgs(output, msg_len);
	std::cout << "new_register(): id="<<id <<", passwd="<<passwd << std::endl;
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

void createIdPwDoc(DOMDocument* doc, string type, string id, string pw) {
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

		DOMElement*  prodElem = doc->createElement(X("type"));
		rootElem->appendChild(prodElem);

		DOMText*    prodDataVal = doc->createTextNode(X(type.c_str()));
		prodElem->appendChild(prodDataVal);

		DOMElement*  catElem = doc->createElement(X("uid"));
		rootElem->appendChild(catElem);

		//catElem->setAttribute(X("idea"), X("great"));

		DOMText*    catDataVal = doc->createTextNode(X(id.c_str()));
		catElem->appendChild(catDataVal);

		DOMElement*  devByElem = doc->createElement(X("passwd"));
		rootElem->appendChild(devByElem);

		DOMText*    devByDataVal = doc->createTextNode(X(pw.c_str()));
		devByElem->appendChild(devByDataVal);

		//
		// Now count the number of elements in the above DOM tree.
		//

		const XMLSize_t elementCount = doc->getElementsByTagName(X("*"))->getLength();
		//cout << "The tree just created contains: " << elementCount << " elements." << endl;

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
