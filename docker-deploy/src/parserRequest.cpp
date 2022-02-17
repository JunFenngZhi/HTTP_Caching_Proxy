#include "parserRequest.h"

#include "exception.h"

/*
  check whether request is empty, if yes throw exception.
*/
void parserRequest::checkRequestFormat() {
  if (request == "" || request == "\r" || request == "\n" || request == "\r\n") {
    throw MyException("Wrong request format. request is empty\n");
  }
}

/*
    get the request line from the whole message.    
*/
void parserRequest::parseRequestLine() {
  size_t requestline_end = request.find_first_of("\r\n");
  if (requestline_end == string::npos) {
    throw MyException("Wrong request format. No /r/n, Can not find request line. \n");
  }

  this->requestline = request.substr(0, requestline_end);
}

/*
    extract method from the request line.
*/
void parserRequest::parseMethod() {
  size_t method_end = requestline.find_first_of(" ");
  this->method = requestline.substr(0, method_end);
  if (this->method != "GET" && this->method != "POST" && this->method != "CONNECT") {
    throw MyException("Request method does not exist.\n");
  }
}

/*
    extract host and port from the whole request.
*/
void parserRequest::parseHost() {
  // get host line
  size_t hostLine_begin = request.find("Host: ");
  if (hostLine_begin == string::npos) {
    throw MyException("Host not found in request\n");
  }
  size_t hostLine_end = request.find("\r\n", hostLine_begin);
  string hostLine =
      request.substr(hostLine_begin + 6, hostLine_end - (hostLine_begin + 6));

  // extract port and host from host line.
  size_t delimiter = hostLine.find(":");  //没有端口号，':'也不存在
  if (delimiter == string::npos) {
    this->host = hostLine;
    this->port = "80";
  }
  else {
    this->host = hostLine.substr(0, delimiter);
    this->port = hostLine.substr(delimiter + 1);
  }
}