/**
    Created June 23 2016
    by Varun Malhotra
*/
#include <Arduino.h>

class myVector {

private:
  String element;
  String* first;
  int len;
  String invalidRet;
public:

  ~myVector();
  myVector();
  void insert(String element);
  int size();
  String at(int index);
  String& operator[](int index);
  String last();
  void flush();
  void flushFirst();

};
