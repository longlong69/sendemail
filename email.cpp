#include "email.h"
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <memory.h>
#include <cstring>
#include <string>
#include <iostream>

using namespace std;

namespace email {

Email::EmailInfo::EmailInfo(string user_name,
                            string password,
                            string from,
                            string to,
                            string title,
                            string data,
                            string srv_name)
    : user_name_{user_name},
      password_{password},
      from_{from},
      to_{to},
      title_{title},
      data_{data},
      srv_name_{srv_name} {
}


Email::Email(string user_name,
             string password,
             string from,
             string to,
             string title,
             string data,
             string srv_name)
    : ei(user_name, password, from, to, title, data, srv_name) {
}

Email::~Email() {
  close(fd_sock_);
}

string Email::char2base64(const char* source_str, size_t length) {
  const char base64_code[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  int point = 2;
  char rv = 0; //Residual value
  size_t index = 0;
  string res;
  for (int i = 0; i < length; i++) {
    if (2 == point) {
      index = ((*source_str) >> 2) & 0x3f;
    } else if (4 == point) {
      index = ((*source_str) >> 4) & 0xf;
    } else if (6 == point) {
      index = ((*source_str) >> 6) & 0x3;
    }
    index += rv;
    res.push_back(base64_code[index]);
    rv = ((*source_str) << (6 - point));
    rv = rv & 0x3f;
    point += 2;
    if (point == 8) { //third char end
      index = ((*source_str)) & 0x3f;
      res.push_back(base64_code[index]);
      rv = 0;
      point = 2;
    }
    source_str++;
  }
  if (rv != 0) {
    res.push_back(base64_code[rv]);
  }
  if (length % 3 == 2) {
    res.push_back('=');
  } else if (length % 3 == 1) {
    res.push_back('=');
    res.push_back('=');
  }
  return res;
}

bool Email::login() {
  char recvbuf[1024];
  recv(fd_sock_, recvbuf, 1024, 0);
  string send_str = "HELO smtp.163.com\r\n";
  send(fd_sock_, send_str.c_str(), send_str.length(), 0);
  memset(recvbuf, 0, 1024);
  recv(fd_sock_, recvbuf, 1024, 0);
  if (strncmp("250", recvbuf, 3)) {
    return false;
  }
  send_str = "AUTH LOGIN\r\n";
  send(fd_sock_, send_str.c_str(), send_str.length(), 0);
  memset(recvbuf, 0, 1024);
  recv(fd_sock_, recvbuf, 1024, 0);
  if (strncmp("334", recvbuf, 3)) {
    return false;
  }
  send_str = char2base64(ei.user_name_.c_str(), ei.user_name_.length());
  send_str += "\r\n";
  send(fd_sock_, send_str.c_str(), send_str.length(), 0);
  memset(recvbuf, 0, 1024);
  recv(fd_sock_, recvbuf, 1024, 0);
  if (strncmp("334", recvbuf, 3)) {
    return false;
  }
  send_str = char2base64(ei.password_.c_str(), ei.password_.length());
  send_str += "\r\n";
  send(fd_sock_, send_str.c_str(), send_str.length(), 0);
  memset(recvbuf, 0, 1024);
  recv(fd_sock_, recvbuf, 1024, 0);
  if (strncmp("235", recvbuf, 3)) {
    return false;
  }
  return true;
}

bool Email::send_email() {
  string send_str = "mail from: <" + ei.from_  + ">\r\n";
  char recvbuf[1024] = { 0 };
  send(fd_sock_, send_str.c_str(), send_str.length(), 0);
  recv(fd_sock_, recvbuf, 1024, 0);
  if (strncmp("250", recvbuf, 3)) {
    return false;
  }
  send_str = "rcpt to: <" + ei.to_ + ">\r\n";
  send(fd_sock_, send_str.c_str(), send_str.length(), 0);
  memset(recvbuf, 0, 1024);
  recv(fd_sock_, recvbuf, 1024, 0);
  if (strncmp("250", recvbuf, 3)) {
    return false;
  }
  send_str = "data\r\n";
  send(fd_sock_, send_str.c_str(), send_str.length(), 0);
  memset(recvbuf, 0, 1024);
  recv(fd_sock_, recvbuf, 1024, 0);
  if (strncmp("354", recvbuf, 3)) {
    return false;
  }
  send_str = "TO: " + ei.to_ + "\r\n";
  send(fd_sock_, send_str.c_str(), send_str.length(), 0);
  send_str = "FROM: " + ei.from_ + "\r\n";
  send(fd_sock_, send_str.c_str(), send_str.length(), 0);
  send_str = "SUBJECT:" + ei.title_ + "\r\n\r\n";
  send(fd_sock_, send_str.c_str(), send_str.length(), 0);
  send_str = ei.data_ + "\r\n.\r\n";
  send(fd_sock_, send_str.c_str(), send_str.length(), 0);
  memset(recvbuf, 0, 1024);
  recv(fd_sock_, recvbuf, 1024, 0);
  if (strncmp("250", recvbuf, 3)) {
    return false;
  }
  return true;
}

bool Email::send_end() {
  string send_str = "quit\r\n";
  send(fd_sock_, send_str.c_str(), send_str.length(), 0);
  char recvbuf[1024] = { 0 };
  recv(fd_sock_, recvbuf, 1024, 0);
  if (strncmp("221", recvbuf, 3)) {
    return false;
  }
  return true;
}

bool Email::createConn() {
  struct hostent* host;
  host = gethostbyname(ei.srv_name_.c_str());
  struct sockaddr_in addr_in;
  addr_in.sin_family = AF_INET;
  addr_in.sin_port = htons(25);
  addr_in.sin_addr.s_addr = 0;
  memcpy(reinterpret_cast<char*>(&(addr_in.sin_addr)), host->h_addr, host->h_length);
  fd_sock_ = socket(AF_INET, SOCK_STREAM, 0);
  if (connect(fd_sock_, reinterpret_cast<struct sockaddr*>(&addr_in), sizeof(addr_in)) != 0) {
    return false;
  }
  return true;
}

} //namesapce email
