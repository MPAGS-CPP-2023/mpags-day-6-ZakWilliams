#ifndef MPAGSCIPHER_EXCEPTIONMISARG_HPP
#define MPAGSCIPHER_EXCEPTIONMISARG_HPP

#include <stdexcept>
#include <string>

class MissingArgument : public std::invalid_argument { //This is the class, which is constructed from the parent class "invalid_argument"
public:
    MissingArgument( const std::string& msg ) : std::invalid_argument{msg} //This here is the constructor for the missing_argument class 
    {
    }
};

#endif

