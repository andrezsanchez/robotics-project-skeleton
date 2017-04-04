#pragma once

#include "glitter.hpp"

#include <iostream>
#include <fstream>
#include <string>

class Exception : public std::exception {
  std::string message;
  virtual const char* what() const throw() {
    return message.c_str();
  }
public:
  Exception (std::string msg) : message(msg + "\n") {};
};


std::string loadFile(std::string filename) {
    filename = (PROJECT_SOURCE_DIR "/") + filename;

    std::ifstream fd;
    fd.exceptions(std::ifstream::failbit);
    try {
        fd.open(filename);
    }
    catch (std::ifstream::failure e) {
        Exception e2("Failed to open " + filename);
        throw e2;
    }
    auto src = std::string(
        (std::istreambuf_iterator<char>(fd)),
        std::istreambuf_iterator<char>()
    );
    return src;
}

