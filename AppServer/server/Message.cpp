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
  int getUserInfo(DOMElement* requestElement, user_info* user);
  void setReplyStr(string reply);
  string generateRegisterReplyStr(ErrorCode, user_info);

  public:

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
  XercesDOMParser* parser = new XercesDOMParser();

  //Use namespaces and schema
  parser->setDoSchema(true);
  //Set the validation scheme
  parser->setValidationScheme(XercesDOMParser::Val_Always);
  parser->setDoNamespaces(true);    // optional

  Grammar* grammar = parser->loadGrammar("../common/honeyBadger.xsd", Grammar::SchemaGrammarType, true);

  /*
  // jingfu: try following schema validation, ding's grammar wasn't checking
  // schema properly
  parser->setExternalNoNamespaceSchemaLocation("../common/honeyBadger.xsd");
  //Get all errors
  parser->setExitOnFirstFatalError(true);
  //All validation errors are fatal.
  parser->setValidationConstraintFatal(true);
  //Set schema validation
  //parser->setValidationSchemaFullChecking(false);
  */

  ErrorHandler* errHandler = (ErrorHandler*) new HandlerBase();
  parser->setErrorHandler(errHandler);

  // create an input source from string
  MemBufInputSource xml_buf((XMLByte*)record.c_str(),(XMLSize_t) (record_len ), "test", false);

  // try to parse this document and get fields
  // TODO: maybe a better idea to put these functions into a msg class
  //msg msg;
  LOG(INFO) << "schema check passed" << endl;
  cout << "schema check passed" << endl;
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

    //for( XMLSize_t xx = 0; xx < nodeCount; ++xx ) { //TODO: why need to loop?
    // we know message type tag follows
    DOMNode* typeNode = children->item(1);
    DOMElement* typeElement;
    if( typeNode->getNodeType() &&  // true is not NULL
        typeNode->getNodeType() == DOMNode::ELEMENT_NODE ) {
      typeElement = dynamic_cast< xercesc::DOMElement* >( typeNode );
      if( XMLString::equals(typeElement->getTagName(), X("registerRequestMessageType"))) {
        cout << "this is a registerRequestMessageType" << endl;
        ErrorCode user_ret;
        user_ret = (ErrorCode) getUserInfo(typeElement, &msg.user);
        assert(user_ret == OK);
        // now grab msg.user and do whatever processing of register
        //
        // suppose this user ID is unique and everything is legit, now create
        // empty dir for this user
        // assert (createDataDir(uid) = OK);

        setReplyStr(generateRegisterReplyStr(user_ret, msg.user));

        ret = OK;
        goto done;
      }
      else if( XMLString::equals(typeElement->getTagName(), X("loginRequestMessageType"))) {
        cout << "this is a loginRequestMessageType" << endl;
        assert (getUserInfo(typeElement, &msg.user) == (int) OK);
        setReplyStr("Login_OK");

        ret = OK;
        goto done;
      }
      else if( XMLString::equals(typeElement->getTagName(), X("newfileRequestMessageType"))) {
        cout << "this is a newfileRequestMessageType" << endl;
        assert (getUserInfo(typeElement, &msg.user) == (int) OK);
        setReplyStr("newFile_OK");

        ret = OK;
        goto done;
      }
      else {
        cout << "Error: unknow message type tag" << endl;
      }
    }
    else {
      cout << "Error: can't parse this xml" << endl;
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

string Message::generateRegisterReplyStr(ErrorCode user_ret, user_info user) {
  string ret_str;
  if(user_ret == OK) {
    // set new reply xml message here
  }
  return string("Register_OK");
}

//TODO: add exception handling
int Message::getUserInfo(DOMElement* requestElement, user_info* user) {
  // we know User tag follows
  //DOMNodeList* children = requestElement->getChildNodes();
  //DOMNode* userNode = children->item(1);
  DOMElement* userElement = requestElement;
  if( XMLString::equals(userElement->getTagName(), X("registerRequestMessageType")) ||
      XMLString::equals(userElement->getTagName(), X("loginRequestMessageType"))    ) {
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

/*
  try
  {
    DOMElement* rootElem = doc->getDocumentElement();

    DOMElement*  typeElem = doc->createElement(X("registerResponseMessageType"));
    rootElem->appendChild(typeElem);

    DOMElement*  catElem = doc->createElement(X("userid"));
    typeElem->appendChild(catElem);

    DOMText*    catDataVal = doc->createTextNode(X(id.c_str()));
    catElem->appendChild(catDataVal);

    DOMElement*  devByElem = doc->createElement(X("result"));
    typeElem->appendChild(devByElem);

    DOMText*  devByDataVal = doc->createTextNode(X(pw.c_str()));
    devByElem->appendChild(devByDataVal);


    DOMElement*  evaByElem = doc->createElement(X("details"));
    typeElem->appendChild(evaByElem);

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

DOMDocument* createHBMessage() {
    DOMImplementation* impl =  DOMImplementationRegistry::getDOMImplementation(X("Core"));
    assert(impl != NULL);
    return impl->createDocument(
            0,                    // root element namespace URI.
            X("HBMessages"),         // root element name (it doesn't like space in between)
            0);

}
*/