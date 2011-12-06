#ifndef HB_SERVER_H_
#define HB_SERVER_H_

using namespace std;

typedef struct user_info_s {
  string uid;
  string passwd;
  string hostid;
  string email;
} user_info;

typedef struct msg_s {
  user_info user;
  string msg_type;
  string action_type;
} msg_t;

void* run_thread(void* ctx);

#endif
