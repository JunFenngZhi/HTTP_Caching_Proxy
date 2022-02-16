#ifndef _PARSERREQUEST_H
#define _PARSERREQUEST_H
#include <string>
#include <iostream>

using namespace std;

class parserRequest {
 public:
  string request;  // whole request
  string host;
  string port;
  string method;
  string requestline;

 public:
  parserRequest() {}
  ~parserRequest() {}

  void parse(const string & request) {
    this->request = request;
    checkRequestFormat();
    parseRequestLine();
    parseMethod();
    parseHost();
  }

  void parseRequestLine();

  void parseMethod();

  void parseHost();

  void checkRequestFormat();

  inline void printResult() {
    cout << "p.request:" << request << endl;
    cout << "p.requestline:" << requestline << endl;
    cout << "p.host:" << host << endl;
    cout << "p.method:" << method << endl;
    cout << "p.port:" << port << endl;
    cout << "----------------------------------------------------------" << endl;
  }
};

#endif
