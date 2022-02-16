#ifndef _CLIENTINFO_H
#define _CLIENTINFO_H
#include<string>

struct clientInfo
{
    int Id;
    string Ip;
    int client_fd;

    clientInfo(int Id, string Ip, int client_fd): Id(Id), Ip(Ip.c_str()), client_fd(client_fd){}
};

#endif