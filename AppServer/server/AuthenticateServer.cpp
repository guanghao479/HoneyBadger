#include <map>
#include <string>
#include <glog/logging.h>


using namespace std;

/**
 * This class deals with user registration and user logon. It writes all registration information into a txt file when the class object is destroyed. It can also be initialized from a txt file during construction.
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
		int registerUID();
		int loginCheck();

	private:
		map<string, string> users;
		string uIDFileName;
}	


authenticateServer::authenticateServer()
{
	users.clear();
	uIDFileName = "users.txt";
	int result = loadFromFile();
	if (result != 0)
	{
	}
	else
	{
	}
}

authenticateServer::authenticateServer(string uIDFiles)
{

	int result  = loadFromFile();
	

}
