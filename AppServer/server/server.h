#ifndef HB_SERVER_H_
#define HB_SERVER_H_

#include <sstream>
using namespace std;

typedef struct user_info_s {
  string uid;
  string passwd;
  string hostid;
  string email;

  string printSelfStr() {
    string ret = "user_info: uid = " + uid + ", passwd = " + passwd +
                 ", hostid = " + hostid + ", email = " + email;
    return ret;
  }
} user_info;

typedef struct file_info_s {
  string fileid;
  string filepath;
  string content;
  int length;

  string printSelfStr() {
    stringstream lengthstr;
    lengthstr << length;
    string ret = "file_info: fileid = " + fileid + ", filepath = " + filepath +
                 ", content = " + content + ", length = " + lengthstr.str();
    return ret;
  }
} file_info;

typedef struct msg_s {
  user_info user;
  file_info file;
  string msg_type;
  string action_type;
} msg_t;

void* run_thread(void* ctx);

#endif
