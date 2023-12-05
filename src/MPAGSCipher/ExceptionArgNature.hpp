//This arg type is triggered when an argument is:
//- of incorrect type, e.g. non-convetable integer when should be string
//- not in dictionary of possibilities e.g. not a Caesar, Vigenere, or Playfair
//- argument not recognised at all, e.g. -x instead of -i, -p, etc

#ifndef MPAGSCIPHER_EXCEPTIONARGNATURE_HPP
#define MPAGSCIPHER_EXCEPTIONARGNATURE_HPP

#include <stdexcept>
#include <string>

class WrongTypeArgument : public std::invalid_argument { //This is the class, which is constructed from the parent class "invalid_argument"
public:
    WrongTypeArgument( const std::string& msg ) : std::invalid_argument{msg} //This here is the constructor for the missing_argument class 
    {
    }
};

#endif

