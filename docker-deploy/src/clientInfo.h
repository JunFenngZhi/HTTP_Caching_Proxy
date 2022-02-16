#ifndef _CLIENTINFO_H
#define _CLIENTINFO_H
#include <string>

using namespace std;

class clientInfo {
 public:
  int Id;
  string Ip;
  int client_fd;

 public:
  clientInfo(int client_id, string client_ip, int client_fd) :
      Id(client_id), Ip(client_ip.c_str()), client_fd(client_fd) {}
};

#endif