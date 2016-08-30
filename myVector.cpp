/**
    Created June 23 2016
    by Varun Malhotra
*/

#include "myVector.h"
#include <Arduino.h>


  myVector::~myVector() {

    delete [] first;
  }


  myVector::myVector()
    :first(0), len(0), invalidRet("0")
  {
  }


  void myVector::insert(String element) {

    if (len == 0) {

      first = new String[1];
      first[0] = element;


    } else {

      String* tempFirst = new String[len + 1];

      for (int i = 0; i < len; i++) {

        tempFirst[i] = first[i];

      }

      tempFirst[len] = element;
      delete [] first;
      first = tempFirst;

    }

    len++;

  }

  int myVector::size() {

    return len;

  }


  String myVector::at(int index) {

    if (index < 0 || index >= size()) {

      return invalidRet;

    }

    return first[index];

  }


  String& myVector::operator[](int index) {

    if (index < 0 || index >= size()) {

      return  invalidRet;

    }

    return first[index];


  }


  String myVector::last() {

    return first[len - 1];

  }

  void myVector::flush() {

      delete [] first;
      len = 0;

  }

    void myVector::flushFirst() {

        String* tempFirst = new String[len - 1];
        for (int i = 1; i < len; i++) {

            tempFirst[i-1] = first[i];

        }

        delete [] first;
        first = tempFirst;
        --len;
    }
