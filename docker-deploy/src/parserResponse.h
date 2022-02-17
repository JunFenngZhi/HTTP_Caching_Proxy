#ifndef _PARSERRESPONSE_H
#define _PARSERRESPONSE_H
#include <assert.h>

#include <iostream>
#include <string>

#include "exception.h"

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
  ~parserResponse() {}

  void parse(const string & response) {
    this->response = response;
    parseStatusLine();
    parseHead_Length();
    parseContent_Length();
    parseChunked();

    // make sure only one of the chunked and content-length is valid
    if ((this->chunked == true && !content_length.empty()) ||
        (this->chunked == false && content_length.empty())) {
      printResult();
      throw MyException("chunked and content_length is error.\n");
    }
  }

  void parseStatusLine();

  void parseHead_Length();

  void parseChunked();

  void parseContent_Length();

  inline void printResult() {
    cout << "----------------------------------------------------------" << endl;
    cout << "p.response:" << response << endl;
    cout << "p.status_line:" << status_line << endl;
    cout << "p.chunked:" << chunked << endl;
    cout << "p.content_length:" << content_length << endl;
    cout << "p.head_length:" << head_length << endl;
    cout << "----------------------------------------------------------" << endl;
  }
};

#endif
