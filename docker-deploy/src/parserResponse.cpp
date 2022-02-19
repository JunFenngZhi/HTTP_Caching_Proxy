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

void parserResponse::parseCacheControl() {
  string cacheControl;
  cacheControl = this->list["Cache-Control"];
  if (cacheControl == "") {
    this->cache_control = false;  // cache control field does not exist
  }
  else {
    this->cache_control =
        true;  //cache control field exist, and we need to parse this filed further
  }
}

void parserResponse::parseMaxage() {
  //Cache control filed does not exist, so we set max_age = -1 to indicate that maxage does not exist
  if (this->cache_control == false) {
    this->max_age = -1;
  }
  else {
    string cache_control_value = list["Cache-Control"];
    if (cache_control_value.find("max-age=") == string::npos) {
      //does not have max-age, so set -1
      this->max_age = -1;
    }
    else {
      size_t max_age_start_pos = cache_control_value.find("max-age=") + 8;
      size_t max_age_end_pos = cache_control_value.find(",", max_age_start_pos);
      string max_age = cache_control_value.substr(max_age_start_pos,
                                                  max_age_end_pos - max_age_start_pos);
      this->max_age = stoi(max_age);
    }
  }
}

void parserResponse::parseRevalidate() {
  //Cache control filed does not exist, so we set revalidate = false to indicate that revalidate does not exist
  if (this->cache_control == false) {
    this->revalidate = false;
  }
  else {
    string cache_control_value = list["Cache-Control"];
    if (cache_control_value.find("must-revalidate") == string::npos &&
        cache_control_value.find("no-cache") == string::npos) {
      //does not have no cache and must-revalidate, so set false
      this->revalidate = false;
    }
    else {
      this->revalidate = true;
    }
  }
}

void parserResponse::parseNostore() {
  //Cache control filed does not exist, so we set no_store = false to indicate that no_store does not exist
  if (this->cache_control == false) {
    this->no_store = false;
  }
  else {
    string cache_control_value = list["Cache-Control"];
    if (cache_control_value.find("no-store") == string::npos) {
      //does not have no store, so set false
      this->no_store = false;
    }
    else {
      this->no_store = true;
    }
  }
}
