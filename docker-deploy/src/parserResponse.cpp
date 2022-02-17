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
  else{
    this->chunked = false;
  }
}
