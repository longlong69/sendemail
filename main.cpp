#include "email.h"
#include <iostream>
#include <string>

using namespace std;
using namespace email;

void usage(char* argv0) {
  cout << "usage: " << argv0 << " <username> <password | Authorization code> "
	 << "<from> <to> <title> <body> <hostname>" << endl; 
}

int main(int argc, char* argv[]) {
  if (argc < 8) {
    usage(argv[0]);
    return -1;
  }
  Email email(argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7]);
  if (!email.createConn()) {
    cout << "conn error!" << endl;
    return 1;
  }
  if (!email.login()) {
    cout << "login error!" << endl;
    return 2;
  }
  if (!email.send_email()) {
    cout << "send email error" << endl;
    return 3;
  }
  if (!email.send_end()) {
    cout << "end error!" << endl;
    return 4;
  }
  return 0;
}
