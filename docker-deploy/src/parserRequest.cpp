#include "parserRequest.h"

#include "exception.h"

/*
  check whether request is empty, if yes throw exception.
*/
void parserRequest::checkRequestFormat() {
  if (request == "" || request == "\r" || request == "\n" || request == "\r\n") {
    throw MyException("Wrong request format. request is empty\n");
  }
  //add one more case: HTTP/1.1 omit
  if (request.find("HTTP/1.1") == string::npos) {
    throw MyException("Wrong request format. omit the HTTP/1.1\n");
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

void parserRequest::parseHeaderContent() {
  size_t findPosStart;  // Header ": " start pos
  size_t findPosEnd;    // One Header field line end pos
  string HeaderKey;     //Header Field Key
  string HeaderValue;   //Header Field Value

  size_t pos = this->request.find_first_of("\r\n");
  //find the Header field start pos
  size_t Header_Start_pos = pos + 2;
  //find the Header field end pos
  size_t Header_End_pos = this->request.find("\r\n\r\n") + 4;
  string Header_Content =
      this->request.substr(Header_Start_pos, Header_End_pos - Header_Start_pos);

  while (findPosStart != string::npos) {
    findPosStart = Header_Content.find_first_of(": ");
    findPosEnd = Header_Content.find_first_of("\r\n");
    HeaderKey = Header_Content.substr(0, findPosStart);
    HeaderValue = Header_Content.substr(findPosStart + 2, findPosEnd - findPosStart - 2);
    this->list.insert({HeaderKey, HeaderValue});
    Header_Content = Header_Content.substr(findPosEnd + 2);
  }
}

void parserRequest::parseURI() {
  size_t URI_start_pos = this->requestline.find(" ") + 1;
  size_t URI_end_pos = this->requestline.find(" HTTP");
  this->URI = this->requestline.substr(URI_start_pos, URI_end_pos - URI_start_pos);
}