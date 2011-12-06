#include <map>
#include <string>
#include <fstream>
#include <pthread.h>
//#include <mutex>

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
	map<string, string> users; //Both read and write on this structure should be locked for data consistency
	string uIDFileName;
	pthread_mutex_t maplock;
//	mutex mapLock;
};	


authenticateServer::authenticateServer() {
  

  pthread_mutex_init(&maplock, NULL);
 // maplock  = PTHREAD_MUTEX_INITIALIZER; 

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

  //maplock = PTHREAD_MUTEX_INITIALIZER; 
  pthread_mutex_init(&maplock, NULL);

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

  pthread_mutex_destroy(&maplock);
  
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
  fp.open(uIDFileName.c_str(), fstream::in);

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
  fp.open(uIDFileName.c_str(), fstream::out);

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

  // We'll simply lock both read and write equally for now. A read/write lock will have better performance
  pthread_mutex_lock(&maplock);

  // Find duplicates first
  map<string, string>::iterator tmp = users.find(uID);
  int result;
  // Valid, successful register
  if (tmp == users.end()) {
    users.insert(pair<string, string>(uID, passwd)); //add user
    result = 0;
  }
  // Invalid, register failed
  else {
	result  = -1;
  }

  pthread_mutex_unlock(&maplock);

  return result;
}

int authenticateServer::loginCheck(string uID, string passwd) { 

  // We'll simply lock both read and write equally for now. A read/write lock will have better performance
  pthread_mutex_lock(&maplock);

  map<string, string>::iterator tmp = users.find(uID);
  int result;

  //Valid, successful login
  if (tmp != users.end() && passwd.compare((*tmp).second) == 0) {
	result = 0;
  }
  //User doesn't exist
  else if (tmp == users.end()) {
	result = -1;
  }
  //User exists, but password is wrong
  else {
	result = -2;
  }

  pthread_mutex_unlock(&maplock);

  return result;
}

