#ifndef _PARSERRESPONSE_H
#define _PARSERRESPONSE_H
#include <iostream>
#include <string>

using namespace std;

class parserResponse {
 public:
  string response;  // whole request
  int head_length;
  string content_length;
  bool chunked;
  string status_line;

 public:
  parserResponse() {}
  parserResponse() {}

  void parse(const string & response) {
    this->response = response;
    parseStatusLine();
    parseHead_Length();
    parseContent_Length();
    parseChunked();
  }

  void parseStatusLine();

  void parseHead_Length();

  void parseChunked();

  void parseContent_Length();
};

#endif
