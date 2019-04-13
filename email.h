// Copyright 2019 hrbeu
// License()
// Author: wangyanlong (383538204@qq.com)
// This is a program for send email

#ifndef EMAIL_H_
#define EMAIL_H_
#include <string>
using namespace std;

namespace email {

class Email {
 private:
  struct EmailInfo {
    string user_name_;
    string password_; //Authorization code
    string from_;
    string to_;
    string title_;
    string data_;
    string srv_name_;
    EmailInfo(string user_name,
               string password,
               string from,
               string to,
               string title,
               string data,
               string srv_name);
  };
 public:
  Email(string user_name,
	string password,
	string from,
	string to,
	string title,
	string data,
	string srv_name);
  ~Email();

  bool login();
  bool send_email();
  bool send_end();
  bool createConn();

 protected:
  string char2base64(const char* source_str, size_t length);

 private:
  int fd_sock_;
  EmailInfo ei;
};

} //namespace email
#endif
