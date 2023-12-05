#ifndef MPAGSCIPHER_EXCEPTIONINVALIDKEY_HPP
#define MPAGSCIPHER_EXCEPTIONINVALIDKEY_HPP

#include <stdexcept>
#include <string>

class InvalidKey : public std::invalid_argument { //This is the class, which is constructed from the parent class "invalid_argument"
public:
    InvalidKey( const std::string& msg ) : std::invalid_argument{msg} //This here is the constructor for the missing_argument class 
    {
    }
};

#endif

