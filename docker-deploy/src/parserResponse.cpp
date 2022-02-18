#include "parserResponse.h"

void parserResponse::parseStatusLine() {
  size_t pos = this->response.find_first_of("\r\n");
  this->status_line = response.substr(0, pos);
}

void parserResponse::parseHead_Length() {
  size_t head_end = response.find("\r\n\r\n");
  this->head_length = head_end + 4;
}

void parserResponse::parseContent_Length() {
  size_t pos;
  if ((pos = response.find("Content-Length: ")) != std::string::npos) {
    size_t end = response.find("\r\n", pos);
    this->content_length = response.substr(pos + 16, end - pos - 16);
  }
}

void parserResponse::parseChunked() {
  size_t pos;
  if ((pos = response.find("chunked")) != std::string::npos) {
    this->chunked = true;
  }
  else {
    this->chunked = false;
  }
}

void parserResponse::parseHeaderContent() {
  size_t findPosStart;  // Header ": " start pos
  size_t findPosEnd;    // One Header field line end pos
  string HeaderKey;     //Header Field Key
  string HeaderValue;   //Header Field Value

  size_t pos = this->response.find_first_of("\r\n");
  //find the Header field start pos
  size_t Header_Start_pos = pos + 2;
  //find the Header field end pos
  size_t Header_End_pos = this->response.find("\r\n\r\n") + 4;
  string Header_Content =
      this->response.substr(Header_Start_pos, Header_End_pos - Header_Start_pos);

  while (findPosStart != string::npos) {
    findPosStart = Header_Content.find_first_of(": ");
    findPosEnd = Header_Content.find_first_of("\r\n");
    HeaderKey = Header_Content.substr(0, findPosStart);
    HeaderValue = Header_Content.substr(findPosStart + 2, findPosEnd - findPosStart - 2);
    this->list.insert({HeaderKey, HeaderValue});
    Header_Content = Header_Content.substr(findPosEnd + 2);
  }
}
