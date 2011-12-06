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
  parser->setValidationScheme(XercesDOMParser::Val_Always);
  parser->setDoNamespaces(true);    // optional
  ErrorHandler* errHandler = (ErrorHandler*) new HandlerBase();
  parser->setErrorHandler(errHandler);

  // create an input source from string
  MemBufInputSource xml_buf((XMLByte*)record.c_str(),(XMLSize_t) (record_len ), "test", false);

  // try to parse this document and get fields
  // TODO: maybe a better idea to put these functions into a msg class
  //msg msg;
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
      // suppose this user ID is unique and everything is legit, now create
      // empty dir for this user
      // assert (createDataDir(uid) = OK);
      setReplyStr("Register_OK");

      ret = OK;
      goto done;

    }
    else if (msg.msg_type.compare("Login") == 0) {
      // handle "Login"
      cout << "handling Login.." << endl;
      assert (getUserInfo(requestElement, &msg.user) == (int) OK);

      setReplyStr("Login_OK");

      ret = OK;
      goto done;
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

//TODO: add exception handling
int Message::getUserInfo(DOMElement* requestElement, user_info* user) {
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
      LOG(ERROR) << "error getting user tag";
            return BAD_XML;
    }
  }
  else {
    LOG(ERROR) << "error getting user type";
    return BAD_XML;
  }

  cout << "conn_count = " << -1 <<  ", getUserInfo(): uid = " << user->uid << ", passwd = " << user->passwd
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
