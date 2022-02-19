#ifndef _PARSERRESPONSE_H
#define _PARSERRESPONSE_H
#include <assert.h>

#include <iostream>
#include <string>
#include <unordered_map>

#include "exception.h"

using namespace std;

class parserResponse {
 public:
  string response;  // whole request
  int head_length;
  string content_length;
  bool chunked;
  string status_line;
  unordered_map<string, string> list;
  bool
      cache_control;  //true means have cache control field, false means have no cache control field
  int max_age;  // -1 means no max-age
  bool revalidate;
  bool no_store;

 public:
  parserResponse() {}
  ~parserResponse() {}

  void parse(const string & response) {
    this->response = response;
    parseStatusLine();
    parseHead_Length();
    parseContent_Length();
    parseChunked();
    parseHeaderContent();
    parseCacheControl();
    parseMaxage();
    parseRevalidate();
    parseNostore();

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

  void parseHeaderContent();

  void parseMaxage();

  void parseCacheControl();

  void parseRevalidate();

  void parseNostore();

  inline void printResult() {
    cout << "-------------------RESPONSE-----------------------------" << endl;
    //cout << "p.response:" << response << endl;
    cout << "p.status_line:" << status_line << endl;
    cout << "p.head_length:" << head_length << endl;
    for (auto it : list) {
      cout << it.first << ":" << it.second << endl;
    }
    cout << "----------------------------------------------------------" << endl;
  }
};

#endif
