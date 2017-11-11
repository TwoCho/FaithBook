// headers for C
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <regex.h>

// headers for C++
#include <string>
#include <iostream>
#include <deque>

#define SERVER_IPADDR "127.0.0.1"
#define SERVER_PORTNO 9000
#define MAX_CONNECTIONS 1000

#define MAX_BUFFER 1024
#define MAX_NAME 20

using namespace std;

///////////////////////////////////////////////
// data structure (mutex not implemented!)
///////////////////////////////////////////////

// beforehand declare for compile
class User;
class UserList;
class ChatRoom;     
class ChatRoomList;

//--------------------------------------------
// User class definition
//--------------------------------------------
class ConnInfo {
public:
  ConnInfo(int id, int fd, sockaddr_in addr):
    conn_id(id), sockfd(fd), saddr(addr), user(NULL) {}
  friend ostream& operator<<(ostream& out, const ConnInfo& obj);
  int conn_id;
  int sockfd;
  sockaddr_in saddr;
  const User *get_user() const { return user; }
  void set_user(const User *u) { user = u; }
private:
  User *user;
};


class User {
public:
  User();
  User(const string& n, const string& e);
  User(const string& n, const string& e, const ConnInfo* ci);
  const string& get_name() const;
  const string& get_email() const;
  const deque<ChatRoom *>& get_belong() const;
  const ConnInfo* get_conn() const;
  void set_conn(ConnInfo *ci);
  friend ostream& operator<<(ostream& out, const User& u);
  deque<ChatRoom *> rlist;
private:
  string name;
  string email;
  ChatRoom* current;
  ConnInfo* conn;
  User* next;
  friend class UserList;
};

User::User() : name(""), email(""), conn(NULL) {};
User::User(const string& n, const string& e) :
  name(n), email(e), conn(NULL) {}
User::User(const string& n, const string& e, const ConnInfo* ci) :
  name(n), email(e), conn(ci) {}
const string& User::get_name() const { return name; }
const string& User::get_email() const { return email; }
const ConnInfo* User::get_conn() const { return conn; }
void User::set_conn(ConnInfo *ci) {
  if (ci == NULL) {
    delete conn;
    conn = NULL;
    return;
  }
  
  if (conn != NULL)
    delete conn;
  else {
    conn = ci;
    ci->set_user(this);
  }
}


//--------------------------------------------
// UserList class definition
//--------------------------------------------
class UserList {
public:
  UserList() {}
  User* find_by_name(const string& name);
  User* find_by_email(const string& email);
  //User* find_by_conn_id(const int conn_id);
  //User* find_by_sockfd(const int sockfd);
  int find_idx_by_name(const string& name);
  void add(const User *u);
  User* remove(const string& name);
  friend ostream& operator<<(ostream& out, const UserList& obj);
  deque<User *> list;
};

#if 0
User* UserList::find_by_conn_id(const int conn_id){
  User *u;
  for (int i = 0; i < list.size(); i++) {
    u  = list.at(i);
    if (u->conn_id == conn_id)
      return u;
  }
  return NULL;
}

User* UserList::find_by_sockfd(const int sockfd){
  User *u;
  for (int i = 0; i < list.size(); i++) {
    u  = list.at(i);
    if (u->sockfd == sockfd)
      return u;
  }
  return NULL;
}
#endif

User* UserList::find_by_email(const string& email){
  User *u;
  for (int i = 0; i < list.size(); i++) {
    u  = list.at(i);
    if (u->email.compare(email) == 0)
      return u;
  }
  return NULL;
}

User *UserList::find_by_name(const string& name) {
  User *u;
  for (int i = 0; i < list.size(); i++) {
    u  = list.at(i);
    if (u->name.compare(name) == 0)
      return u;
  }
  return NULL;
}

int UserList::find_idx_by_name(const string& name) {
  User *u;
  for (int i = 0; i < list.size(); i++) {
    u  = list.at(i);
    if (u->name.compare(name) == 0)
      return i;
  }
  return -1;
}

void UserList::add(const User *u){
  list.push_back((User *) u);
}

User* UserList::remove(const string& name){
  int idx = find_idx_by_name(name);
  //User *u = list.at(idx);
  list.erase(list.begin()+idx);
  //delete u;
}

//--------------------------------------------
// ChatRoom class definition
//--------------------------------------------
class ChatRoom {
public:
  ChatRoom(const string& name);
  bool isMember(const string& name);
  User *findMember(const string& name);
  bool invite(const User *u);
  bool leave(const string& name);
  friend ostream& operator<<(ostream& out, const ChatRoom& obj);
  string name;
  UserList ulist;
  //private:
};

ChatRoom::ChatRoom(const string& name) : name(name) {}

bool ChatRoom::isMember(const string& name){
  User *u = ulist.find_by_name(name);
  if (u == NULL)
    return false;
  else
    return true;
}

User *ChatRoom::findMember(const string& name){
  return ulist.find_by_name(name);
}

bool ChatRoom::invite(const User *n){
  if (isMember(n->get_name())) return false;

  ulist.add(n);
  n->rlist.push_back(this);
  return true;
}

bool ChatRoom::leave(const string& name){
  if (!isMember(name)) return false;

  ulist.remove(name);
  return true;
}

//--------------------------------------------
// ChatRoomList class definition
//--------------------------------------------
class ChatRoomList {
public:
  void insert(const ChatRoom* r);
  ChatRoom* find_by_name(const string& name);
  friend ostream& operator<<(ostream& out, const ChatRoomList& obj);
  deque<ChatRoom *> list;
};

void ChatRoomList::insert(const ChatRoom* r){
  list.push_back((ChatRoom*) r);
}

ChatRoom* ChatRoomList::find_by_name(const string& name){
  ChatRoom *room;
  for(int i = 0; i < list.size(); i++){
    room = list.at(i);
    if(room->name.compare(name) == 0)
      return room;
  }
  return NULL;
}

ostream& operator<<(ostream& out, const ConnInfo& obj){
  ChatRoom *r;
  out << "ConnInfo(" << obj.conn_id
      << ", " << obj.sockfd
      << ")" ;
  return out;
}

ostream& operator<<(ostream& out, const User& obj){
  ChatRoom *r;
  out << "User(" << obj.name
      << ", " << obj.email
      << ", " << *obj.conn
      << ", {" ;

  r = obj.rlist.at(0);
  cout << r->name;
    
  for (int i = 1; i < obj.rlist.size(); i++) {
    r = obj.rlist.at(i);
    cout << ", " << r->name;
  }
  cout << "} )";
  
  return out;
}

ostream& operator<<(ostream& out, const UserList& obj){
  User *u;
  for (int i = 0; i < obj.list.size(); i++) {
    u  = obj.list.at(i);
    cout << *u << endl;
  }
  //cout << endl;
  return out;
}

ostream& operator<<(ostream& out, const ChatRoom& obj){
  User *u;
  cout << obj.name << ":" << endl;
  cout << obj.ulist;
  //cout << endl;
  return out;
}

ostream& operator<<(ostream& out, const ChatRoomList& obj){
  ChatRoom* r;
  cout << "Chatroom List:" << endl;
  for (int i = 0; i < obj.list.size(); i++) {
    r  = obj.list.at(i);
    cout << "room #" << i << ":   ";
    cout << r->name << endl;
  }
  //cout << endl;
  return out;
}


///////////////////////////////////////////////
// global variables
///////////////////////////////////////////////
int conn_id; // total # of connections after the server boots
int n_conn; // number of connections
ChatRoomList *rlist;
ChatRoom *root;

///////////////////////////////////////////////
// system programming 
///////////////////////////////////////////////
void signal_handler(int signo) {
  if (signo == SIGINT || signo == SIGTERM) {
    // shutdown server gracefully.
    // close all socket descriptors.
    exit(0);
  }
}

int startup_server(char *ipaddr, int portno)
{
  struct sockaddr_in sockaddr;
  int sockfd;
  int val = 1;

  // create an unnamed socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  // set a socket option to reuse the server address
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) != 0) {
    printf("error: setsockopt(): %s\n", strerror(errno));
    return -1;
  }

  // name the socket with the server address
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_addr.s_addr = inet_addr(ipaddr);
  sockaddr.sin_port = htons(portno);
  if (bind(sockfd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) != 0) {
    printf("error: bind(): %s\n", strerror(errno));
    return -1;
  }

  // set the maximum number of pending connection requests
  if (listen(sockfd, 10) != 0) {
    printf("error: listen(): %s\n", strerror(errno));
    return -1;
  }

  return sockfd;
}

///////////////////////////////////////////////
// server logic
///////////////////////////////////////////////
regex_t regex_quit, regex_name, regex_msg, regex_who;
regmatch_t groups[3];

void regcomp_all() {
  /* compile regex patterns */
  regcomp(&regex_quit, "^/quit$", REG_EXTENDED);
  regcomp(&regex_name, "^/name ([a-zA-Z0-9_]{1,19})$", REG_EXTENDED);
  regcomp(&regex_msg, "^/msg ([a-zA-Z0-9_]{1,19}) (.*)$", REG_EXTENDED);
  regcomp(&regex_who, "^/who$", REG_EXTENDED);
}

void regfree_all() {
  regfree(&regex_quit);
  regfree(&regex_name);
  regfree(&regex_msg);
  regfree(&regex_who);
}

int regexec_with_args(regex_t *regex, char *msg, int ngroups,
		      regmatch_t *groups, char *arg1, char *arg2) {
  int ret = regexec(regex, msg, ngroups, groups, 0);
  if (ret == 0)	{
    int len;
    if (ngroups > 1) {
      len = groups[1].rm_eo - groups[1].rm_so;
      memcpy(arg1, msg + groups[1].rm_so, len);
      arg1[len] = '\0';
    }
    if (ngroups > 2) {
      len = groups[2].rm_eo - groups[2].rm_so;
      memcpy(arg2, msg + groups[2].rm_so, len);
      arg2[len] = '\0';
    }
  }
  return !ret;
}

int process_msg(ConnInfo *ci, char *msg) {
  size_t len = 0;
  int quit_flag = 0;
  char buffer[MAX_BUFFER], *p;
  int id = ci->conn_id;
  int sockfd = ci->sockfd;
  User *user = ci->get_user();
  char uname[MAX_NAME], pname[MAX_NAME];
  char message[MAX_BUFFER], privmsg[MAX_BUFFER];
  User *peer;
  int peerfd;

  if (regexec_with_args(&regex_quit, msg, 0, NULL, NULL, NULL)) {
    if (user != NULL)
      user->set_conn(NULL);
    n_conn--;
    close(sockfd);
    quit_flag = 1;
    goto cleanup;
  }
  else if (regexec_with_args(&regex_name, msg, 2, groups, uname, NULL)) {
    if ((user = root->findMember(uname)) != NULL)
      user->set_conn(ci);
    else {
      char reply[] = "server: unknown user!\n";
      if (write(sockfd, reply, strlen(reply)) < 0) {
	printf("thread[%d]: error: write(): %s\n", id, strerror(errno));
	quit_flag = 1;
	goto cleanup;
      }
    }
  }
  else if (regexec_with_args(&regex_msg, msg, 3, groups, pname, privmsg)) {
    ConnInfo *pi;
    strcpy(uname, user->get_name().c_str());
    if ((peer = root->findMember(pname)) == NULL) {
      char reply[] = "server: no such user\n";
      if (write(sockfd, reply, strlen(reply)) < 0) {
	printf("thread[%d]: error: write(): %s\n", id, strerror(errno));
	quit_flag = 1;
	goto cleanup;
      }
      goto cleanup;
    }

    if ((pi = peer->get_conn()) == NULL) {
      char reply[] = "server: not online user\n";
      if (write(sockfd, reply, strlen(reply)) < 0) {
	printf("thread[%d]: error: write(): %s\n", id, strerror(errno));
	quit_flag = 1;
	goto cleanup;
      }
      goto cleanup;
    }
    
    peerfd = pi->sockfd;
    sprintf(message, "%s: %s\n", uname, privmsg);
    if (write(peerfd, message, strlen(message)) < 0) {
      printf("thread[%d]: error: write(): %s\n", id, strerror(errno));
      quit_flag = 1;
      goto cleanup;
    }
  }
  else if (regexec_with_args(&regex_who, msg, 0, NULL, NULL, NULL)) {
    User *u;
    message[0] = 0;
    for (int i = 0; i < root->ulist.list.size(); i++) {
      u = root->ulist.list.at(i);
      if (u->get_conn() == NULL) continue;
      strcat(message, u->get_name().c_str());
      strcat(message, " ");
    }
    strcat(message, "\n");
    if (write(sockfd, message, strlen(message)) < 0) {
      printf("thread[%d]: error: write(): %s\n", id, strerror(errno));
      quit_flag = 1;
      goto cleanup;
    }
  }
  else {
    strcpy(uname, user->get_name().c_str());
    sprintf(message, "%s: %s\n", uname, msg);

    for (int i = 0; i < root->ulist.list.size(); i++) {
      peer = root->ulist.list.at(i);
      if (peer->get_conn() == NULL) continue;
      peerfd = peer->get_conn()->sockfd;
      if (write(peerfd, message, strlen(message)) < 0) {
	printf("thread[%d]: error: write(): %s\n", id, strerror(errno));
	quit_flag = 1;
	goto cleanup;
      }
    }
  }

 cleanup:
  return quit_flag;
}

void print_string(char *p) {
  while (*p != 0) printf("%c_", *p++);
  printf("\n");
}

void *thread_main(void *arg) {
  char buf[MAX_BUFFER], buffer[MAX_BUFFER];
  int nread, nconsumed, totread = 0;
  char *cpos, *npos, *rpos;
  int retval = 1;

  struct ConnInfo *ci = (struct ConnInfo *) arg;
  int id = ci->conn_id;
  int sockfd = ci->sockfd;

  printf("thread[%d] alive\n", id);
  memset(buffer, 0, sizeof(buffer));

  while (1) {
    memset(buf, 0, MAX_BUFFER);
    nread = read(sockfd, buf, sizeof(buf));
    if (nread < 0) {
      printf("thread[%d]: error: read(): %s\n", id, strerror(errno));
      pthread_exit((void *)&retval);
    }
    else if (nread == 0) {
      printf("thread[%d]: socket closed\n", id);
      pthread_exit((void *)&retval);
    }
    // else if (nread > 0)

    if (sizeof(buffer) - strlen(buffer) <= strlen(buf)) {
      printf("thread[%d]: too small buffer\n", id);
      pthread_exit((void *)&retval);
    }

    //strcpy(buffer, "a\nbb\nccc"); totread = 8;
    strcat(buffer, buf);
    totread += nread;
    cpos = buffer;
    while ((npos = (char *) memchr(cpos, '\n', totread)) != NULL) {
      *npos = 0; // replace '\n' with NULL
      if ((rpos = (char *) memchr(cpos, '\r', totread)) != NULL)
	*rpos = 0; // replace '\r' with NULL
      printf("thread[%d]: line = %s\n", id, cpos);
      if (process_msg(ci, cpos) != 0) pthread_exit((void *)&retval);
      nconsumed = npos - cpos + 1;
      totread -= nconsumed;
      cpos = npos + 1;
    }
    strcpy(buf, cpos);
    memset(buffer, 0, MAX_BUFFER);
    strcpy(buffer, buf);
    if (strlen(buffer) > 0) {
      printf("thread[%d]: buffer = %s (not completed)\n", id, buffer);
    }
  }
}


///////////////////////////////////////////////
// main function
///////////////////////////////////////////////

int main(int argc, char *argv[]) {
  int ch, flag_help = 0;
  char ipaddr[20] = SERVER_IPADDR;
  int portno = SERVER_PORTNO;
  int halfsd, fullsd; // socket descriptors
  struct sockaddr_in sockaddr;
  pthread_t tid;
  
  while( (ch = getopt(argc, argv, "hi:p:")) != -1) {
    switch(ch) {
    case 'h': flag_help = 1; break;
    case 'i': memcpy(ipaddr, optarg, strlen(optarg)); break;
    case 'c': portno = atoi(optarg); break;
    default:  printf("unknown option : %c\n", optopt); break;
    }
  }
  if (flag_help == 1) {
    printf("usage: %s [-h] [-i ipaddr] [-p portno] \n", argv[0]);
    exit(1);
  }
  printf("server address = %s:%d\n", ipaddr, portno);

  signal(SIGINT,  signal_handler);
  signal(SIGTERM, signal_handler);

  /////////////////////////////////////////////////
  // START: data structures built from DB
  /////////////////////////////////////////////////
  User *u;
  string s;
  ChatRoom *c;
  User *aa = new User("kanghee", "kim.kanghee@gmail.com");
  User *bb = new User("jaewook", "jaewook@gmail.com");
  User *cc = new User("surin", "jorin0723@gmail.com");
 
  rlist= new ChatRoomList();
  root = new ChatRoom("root");
  rlist->insert(root);
  root->invite(aa);
  root->invite(bb);
  root->invite(cc);
  /////////////////////////////////////////////////
  // END: data structures built from DB
  /////////////////////////////////////////////////

  halfsd = startup_server(ipaddr, portno);
  regcomp_all();
  
  while (1) {
    int len = sizeof(sockaddr);
    fullsd = accept(halfsd, (struct sockaddr *)&sockaddr, (socklen_t *)&len);
    if (fullsd < 0) {
      printf("error: accept(): %s\n", strerror(errno));
      exit(1);
    }

    if (n_conn == MAX_CONNECTIONS) {
      printf("error: max clients reached\n");
      close(fullsd);
      sleep(60); // wait for a thread to exit for 1 minute
      continue;
    }

    // create ConnInfo for a newly connected client
    ConnInfo *p = new ConnInfo(conn_id, fullsd, sockaddr);
    n_conn++;

    // create a thread to service the client
    if (pthread_create(&tid, NULL, &thread_main, (void *) p) < 0) {
      printf("error: pthread_create(): %s\n", strerror(errno));
      close(fullsd);
      continue;
    }
    pthread_detach(tid); // make pthread_join() unnecessary

    conn_id++; // increment conn_id for the next incoming client
  }    

  regfree_all();
  return 0;
}

///////////////////////////////////////////////
// end of file
///////////////////////////////////////////////
