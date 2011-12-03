#include <map>
#include <string>
#include <fstream>

#include <glog/logging.h>


using namespace std;

/**
 * This class deals with user registration and user log-on. It writes all registration information into a text file when the class object is destroyed. It can also be initialized from a text file during construction.
 * It's multi-thread safe.
  **/
class authenticateServer {
  
  public:
	authenticateServer();
	authenticateServer(string uIDFiles);
	~authenticateServer();

  private: 
	int loadFromFile();
	int writeToFile();

  public:
	int registerUID(string uID, string passwd);
	int loginCheck(string uID, string passwd);
  
  private:
	map<string, string> users;
	string uIDFileName;
}	


authenticateServer::authenticateServer() {
  
  uIDFileName = "users.txt";
  users.clear();
  
  int result = loadFromFile();
  if (result != 0) {
	LOG(WARNING) << "Load from default user id file : " << uIDFileName << " failed";
  }
  else {
	LOG(INFO) << "Loading user ID from file: " << uIDFileName << ", " << users.size() << " items were loaded.";
  }
}

authenticateServer::authenticateServer(string uIDFiles) {
  
  uIDFileName = uIDFiles;
  users.clear();

  int result = loadFromFile();
  if (result != 0) {
	LOG(WARNING) << "Load from user id file : " << uIDFileName << " failed";
  }
  else {
	LOG(INFO) << "Loading user ID from file: " << uIDFileName << ", " << users.size() << " items were loaded.";
  }
}

authenticateServer::~authenticateServer(){
  
  int result = writeToFile();
  if (result != 0){
	LOG(ERROR) << "Writing user data to file: " << uIDFileName << " failed";	
  }
  else {
	LOG(INFO) << "Writing user data to file: " << uIDFileName << ", " << users.size() << " items were exported.";
  }

  users.clear();
}

int authenticateServer::loadFromFile() {

  fstream fp;
  fp.open(uIDFileName, fstream::in);

  int res = 0;
  if (fp.is_open()) {

	string uid, psw;

	/// Import user id and password to the map structure
	while(fp >> uid >> psw) {
	  users.insert(pair<string,string>(uid, psw));
	}
  	fp.close();
  }
  else {

	LOG(WARNING) << "The stream can't be opened";
	res = -1;
  }
  return res;
}

int authenticateServer::writeToFile() {
  
  fstream fp;
  fp.open(uIDFileName, fstream::out);

  int res  = 0;

  if (fp.is_open()) {
	/// Iterate through the map structure
	map<string, string>::iterator it;
	for (it = users.begin(); it != users.end(); it ++) {
	  fp << (*it).first << " " << (*it).second << endl;
	}
  }
  else {
    LOG(ERROR) << "The ostream can't be opened";
	res = -1;
  }

  return res;
}

int authenticateServer::registerUID(string uID, string passwd) {

}

int authenticateServer::loginCheck(string uID, string passwd) { 
  
}

