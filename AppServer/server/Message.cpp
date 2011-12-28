#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

#include <map>
#include <string>
#include <fstream>

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
#include <xercesc/validators/common/Grammar.hpp>

#include <glog/logging.h>
#include "server.h"
#include "../common/HBcommon.h"
#include "../common/XStr.cpp"

using namespace std;
using namespace xercesc;

/**
 *
 * This class deals with XML message parsing and message processing.
 *
 */

struct Message{

  public:
	Message(string record, size_t len);
	~Message();
  string getReplyStr();
  ErrorCode parseXML();

  private:
  ErrorCode getUserInfo(DOMElement* requestElement, user_info* user);
  ErrorCode createUserDataDir(string uid);
  void setReplyStr(string reply);
  string generateRegisterReplyStr(ErrorCode, user_info*);
  string generateLoginReplyStr(ErrorCode, user_info*);
  DOMDocument* createHBMessage();
  ErrorCode createRegisterOrLoginResponseDoc(DOMDocument* doc, string response_type, string id, string code, string detail);
  string writeOutDOM(DOMDocument* myDoc, DOMImplementation* impl);

  private:
  string record;
  size_t record_len;
  msg_t msg;
  string replyStr;
};

Message::Message(string rec, size_t len) {
  record = rec;
  record_len = len;
}

ErrorCode Message::parseXML() {
  ErrorCode ret;

  // get a parser first
  XercesDOMParser* parser = new XercesDOMParser(); // looked up online, using a DOMBuilder would solve the problem, but where is Builder included?
  // http://www.velocityreviews.com/forums/t392321-validation-of-xml-file-against-external-xsd-schema-using-xerces-cdt.html

  ErrorHandler* errHandler = (ErrorHandler*) new HandlerBase();
  parser->setErrorHandler(errHandler);
  //parser->loadGrammar("../common/honeyBadger.xsd", Grammar::SchemaGrammarType, true);
  /* Schema Validation */
  /*
  parser->setExternalNoNamespaceSchemaLocation("../common/honeyBadger.xsd");
  parser->setLoadSchema(true);
  parser->setDoSchema(true);
  parser->setValidationScheme(XercesDOMParser::Val_Always);
  parser->setValidationSchemaFullChecking(true);
  parser->setDoNamespaces(true);    // optional
  */
  //parser->setExitOnFirstFatalError(true); // this line doesn't do anything even if the xml is bad
  //parser->setValidationConstraintFatal(true); // this line will break the xml to exception, even if the xml is legit

  // create an input source from string
  MemBufInputSource xml_buf((XMLByte*)record.c_str(),(XMLSize_t) (record_len ), "test", false);

  // try to parse this document and get fields
  // TODO: maybe a better idea to put these functions into a msg class
  //msg msg;
  try {
    parser->parse(xml_buf);
    // no need to free this, owned by parent parser project
    DOMDocument* xmlDoc = parser->getDocument();
    assert(xmlDoc != NULL);
    // Get the top-level element
    DOMElement* elementRoot = xmlDoc->getDocumentElement();

    if( !elementRoot ) { goto bad_xml;}
    //cout << "root: " << XMLString::transcode(elementRoot->getTagName())<< endl;

    DOMNodeList*      children = elementRoot->getChildNodes();
    const  XMLSize_t nodeCount = children->getLength();
    //cout <<" node count: " << nodeCount << endl;

    //for( XMLSize_t xx = 0; xx < nodeCount; ++xx )  //TODO: why need to loop?
    // we know message type tag follows
    DOMNode* typeNode = children->item(1);
    DOMElement* typeElement;
    if( typeNode->getNodeType() &&  // true is not NULL
        typeNode->getNodeType() == DOMNode::ELEMENT_NODE ) {
      typeElement = dynamic_cast< xercesc::DOMElement* >( typeNode );
      if( XMLString::equals(typeElement->getTagName(), X("registerRequest"))) {
        cout << "this is a registerRequest" << endl;
        ErrorCode user_ret = (ErrorCode) getUserInfo(typeElement, &msg.user);
        assert(user_ret == OK);
        // now grab msg.user and do whatever processing of register
        //
        // suppose this user ID is unique and everything is legit, now create
        // empty dir for this user
        assert (createUserDataDir(msg.user.uid) == OK);

        setReplyStr(generateRegisterReplyStr(user_ret, &msg.user));

        ret = OK;
        goto done;
      }
      else if( XMLString::equals(typeElement->getTagName(), X("loginRequest"))) {
        cout << "this is a loginRequest" << endl;
        ErrorCode user_ret = getUserInfo(typeElement, &msg.user);
        assert(user_ret == OK);

        // now check user database and see if (uid, passwd) matches

        setReplyStr(generateLoginReplyStr(user_ret, &msg.user));

        ret = OK;
        goto done;
      }
      else if( XMLString::equals(typeElement->getTagName(), X("newfileRequest"))) {
        cout << "this is a newfileRequest" << endl;
        assert (getUserInfo(typeElement, &msg.user) == (int) OK);
        setReplyStr("newFile_OK");

        ret = OK;
        goto done;
      }
      else {
        LOG(ERROR) << "Error: unknown message type" << endl;
      }
    }
    else {
      cout << "Error: can't parse this xml" << endl;
      LOG(ERROR) << "Error: can't parse this xml" << endl;
    }

  } // end of try
  catch (const XMLException& toCatch) {
    char* message = XMLString::transcode(toCatch.getMessage());
    LOG(ERROR) << "Exception message is: \n"
      << message << "\n";
    XMLString::release(&message);
    goto bad_xml;
  }
  catch (const DOMException& toCatch) {
    char* message = XMLString::transcode(toCatch.msg);
    LOG(ERROR) << "Exception message is: \n"
      << message << "\n";
    XMLString::release(&message);
    goto bad_xml;
  }
  catch (...) {
    LOG(ERROR) << "Unexpected Exception \n" ;
    goto bad_xml;
  }

bad_xml:
  ret = BAD_XML;
  setReplyStr("BAD_XML");

done:
  delete parser;
  delete errHandler;

  return ret;
}

string Message::generateRegisterReplyStr(ErrorCode user_ret, user_info* user) {
  string ret_str;
  if(user_ret == OK) {
    // set new reply xml message here
    DOMDocument*   myDoc = createHBMessage();
    DOMImplementation* impl =  DOMImplementationRegistry::getDOMImplementation(X("Core"));
    assert(impl != NULL);
    assert( createRegisterOrLoginResponseDoc(myDoc, string("registerResponse"), user->uid, string("1"), string("Register succeed. Wecome to HB!")) == OK);
    ret_str = writeOutDOM(myDoc, impl);
  }
  return ret_str;
}

string Message::generateLoginReplyStr(ErrorCode user_ret, user_info* user) {
  string ret_str;
  if(user_ret == OK) {
    // set new reply xml message here
    DOMDocument*   myDoc = createHBMessage();
    DOMImplementation* impl =  DOMImplementationRegistry::getDOMImplementation(X("Core"));
    assert(impl != NULL);
    assert( createRegisterOrLoginResponseDoc(myDoc, string("loginResponse"), user->uid, string("2"), string("Login succeed. You balling!")) == OK);
    ret_str = writeOutDOM(myDoc, impl);
  }
  return ret_str;
}

//TODO: add exception handling
ErrorCode Message::getUserInfo(DOMElement* requestElement, user_info* user) {
  // we know User tag follows
  //DOMNodeList* children = requestElement->getChildNodes();
  //DOMNode* userNode = children->item(1);
  DOMElement* userElement = requestElement;
  if( XMLString::equals(userElement->getTagName(), X("registerRequest")) ||
      XMLString::equals(userElement->getTagName(), X("loginRequest"))    ) {
    DOMNodeList* children = userElement->getChildNodes();
    const  XMLSize_t nodeCount = children->getLength();
    for( XMLSize_t xx = 0; xx < nodeCount; ++xx ) {
      DOMNode* crtNode = children->item(xx);
      DOMElement* crtElement;
      if( crtNode->getNodeType() &&  // true is not NULL
          crtNode->getNodeType() == DOMNode::ELEMENT_NODE ) {
        crtElement = dynamic_cast< xercesc::DOMElement* >( crtNode );
        if( XMLString::equals(crtElement->getTagName(), X("userid"))) {
          user->uid = XMLString::transcode(crtElement->getTextContent()) ;
        }
        else if( XMLString::equals(crtElement->getTagName(), X("password"))) {
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
    LOG(ERROR) << "error getting user tag";
    return BAD_XML;
  }
  cout <<  "getUserInfo(): uid = " << user->uid << ", passwd = " << user->passwd
                           << ", hostid = " << user->hostid << ", email = "<< user->email << endl;

  return OK;
}
Message::~Message() {
}

void Message::setReplyStr(string reply) {
  replyStr = reply;
}

string Message::getReplyStr() {
  return replyStr;
}

ErrorCode Message::createUserDataDir(string uid) {

  DIR* dir = NULL;
  dir = opendir(kHBDataRootPath);
  //if non-exist, create it
  if(dir == NULL) {
    int status = mkdir(kHBDataRootPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if(status != 0) {
      LOG(ERROR) << "can't create data root dir at " << kHBDataRootPath << endl;
      exit(1);
    }
  }
  else {
    assert(closedir(dir) == 0);
  }

  // if it reaches here, root path exist
  // TODO: integrate emma's logic with user dir creation

  return OK;
}

DOMDocument* Message::createHBMessage() {
    DOMImplementation* impl =  DOMImplementationRegistry::getDOMImplementation(X("Core"));
    assert(impl != NULL);
    return impl->createDocument(
            0,                    // root element namespace URI.
            X("HBMessages"),         // root element name (it doesn't like space in between)
            0);
}

ErrorCode Message::createRegisterOrLoginResponseDoc(DOMDocument* doc, string response_type, string id, string code, string details){
  ErrorCode errorCode ;

  try
  {
    DOMElement* rootElem = doc->getDocumentElement();

    DOMElement*  typeElem = doc->createElement(X(response_type.c_str()));
    rootElem->appendChild(typeElem);

    DOMElement*  catElem = doc->createElement(X("userid"));
    typeElem->appendChild(catElem);

    DOMText*    catDataVal = doc->createTextNode(X(id.c_str()));
    catElem->appendChild(catDataVal);

    DOMElement*  codeElem = doc->createElement(X("result"));
    typeElem->appendChild(codeElem);

    DOMText*    codeDataVal = doc->createTextNode(X(code.c_str()));
    codeElem->appendChild(codeDataVal);

    DOMElement*  devByElem = doc->createElement(X("details"));
    typeElem->appendChild(devByElem);

    DOMText*  devByDataVal = doc->createTextNode(X(details.c_str()));
    devByElem->appendChild(devByDataVal);

    errorCode = OK;
    //doc->release();
  }
  catch (const OutOfMemoryException&)
  {
    cerr << "OutOfMemoryException" << endl;
    errorCode = OUT_OF_MEMORY;
  }
  catch (const DOMException& e)
  {
    cerr << "DOMException code is:  " << e.code << endl;
    errorCode = UNKNOWN_EXCEPTION;
  }
  catch (...)
  {
    cerr << "An error occurred creating the document" << endl;
    errorCode = UNKNOWN_EXCEPTION;
  }

  return errorCode;
}

string Message::writeOutDOM(DOMDocument* myDoc, DOMImplementation* impl){
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

  string output((char*)myFormTarget->getRawBuffer());

  return output;
}
