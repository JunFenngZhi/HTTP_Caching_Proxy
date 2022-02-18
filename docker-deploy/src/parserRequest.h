#ifndef _PARSERREQUEST_H
#define _PARSERREQUEST_H
#include <iostream>
#include <string>
#include <unordered_map>

using namespace std;

class parserRequest {
 public:
  string request;  // whole request
  string host;
  string port;
  string method;
  string requestline;
  unordered_map<string, string> list;
  int head_length;

 public:
  parserRequest() {}
  ~parserRequest() {}

  void parse(const string & request) {
    this->request = request;
    checkRequestFormat();
    parseRequestLine();
    parseMethod();
    parseHost();
    parseHeaderContent();
  }

  void parseRequestLine();

  void parseMethod();

  void parseHost();

  void checkRequestFormat();

  void parseHeaderContent();

  void parseHead_Length();

  inline void printResult() {
    cout << "----------------------------------------------------------" << endl;
    cout << "p.request:" << request << endl;
    cout << "p.requestline:" << requestline << endl;
    cout << "p.host:" << host << endl;
    cout << "p.method:" << method << endl;
    cout << "p.port:" << port << endl;
    cout << "----------------------------------------------------------" << endl;
  }
};

#endif
