// custom exception for invalid arguments.
// Written by Tim Murphy <tim@murphy.org> 2023

#ifndef INVALIDARGUMENT_H
#define INVALIDARGUMENT_H

#include <exception>
#include <string>

class InvalidArgument : public std::exception
{
  private:
    std::string errMsg;

  public:
    InvalidArgument(const std::string &message)
        : errMsg(message)
    { }

    inline const char * const what()
    {
        return errMsg.c_str();
    }
};

#endif // defined INVALIDARGUMENT_H