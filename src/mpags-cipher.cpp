#include "CipherFactory.hpp"
#include "CipherMode.hpp"
#include "CipherType.hpp"
#include "ProcessCommandLine.hpp"
#include "TransformChar.hpp"
#include "ExceptionMisArg.hpp"
#include "ExceptionInvalidKey.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <future>
#include <cmath>
#include <mutex>

int main(int argc, char* argv[])
{
    std::mutex outputTextMutex;

    // Convert the command-line arguments into a more easily usable form
    const std::vector<std::string> cmdLineArgs{argv, argv + argc};

    // Options that might be set by the command-line arguments
    ProgramSettings settings{false, false, "", "", {}, {}, CipherMode::Encrypt};

    // Process command line arguments
    //const bool cmdLineStatus{processCommandLine(cmdLineArgs, settings)};


    // Any failure in the argument processing means we can't continue
    // Use a non-zero return value to indicate failure
    //if (!cmdLineStatus) {
    //    return 1;
    //}

    //cmd caller edited so that rather that returning false, it throws an error with an associated message
    try {
        processCommandLine(cmdLineArgs, settings);
    } catch ( const MissingArgument& e) { // if processCommandLine throws an error
        std::cerr << "[error] Incorrect argument call - " << e.what() << std::endl;
    } //write different catch for different error types

    // Handle help, if requested
    if (settings.helpRequested) {
        // Line splitting for readability
        std::cout
            << "Usage: mpags-cipher [-h/--help] [--version] [-i <file>] [-o <file>] [-c <cipher>] [-k <key>] [--encrypt/--decrypt]\n\n"
            << "Encrypts/Decrypts input alphanumeric text using classical ciphers\n\n"
            << "Available options:\n\n"
            << "  -h|--help        Print this help message and exit\n\n"
            << "  --version        Print version information\n\n"
            << "  -i FILE          Read text to be processed from FILE\n"
            << "                   Stdin will be used if not supplied\n\n"
            << "  -o FILE          Write processed text to FILE\n"
            << "                   Stdout will be used if not supplied\n\n"
            << "                   Stdout will be used if not supplied\n\n"
            << "  --multi-cipher N Specify the number of ciphers to be used in sequence\n"
            << "                   N should be a positive integer - defaults to 1"
            << "  -c CIPHER        Specify the cipher to be used to perform the encryption/decryption\n"
            << "                   CIPHER can be caesar, playfair, or vigenere - caesar is the default\n\n"
            << "  -k KEY           Specify the cipher KEY\n"
            << "                   A null key, i.e. no encryption, is used if not supplied\n\n"
            << "  --encrypt        Will use the cipher to encrypt the input text (default behaviour)\n\n"
            << "  --decrypt        Will use the cipher to decrypt the input text\n\n"
            << std::endl;
        // Help requires no further action, so return from main
        // with 0 used to indicate success
        return 0;
    }

    // Handle version, if requested
    // Like help, requires no further action,
    // so return from main with zero to indicate success
    if (settings.versionRequested) {
        std::cout << "0.5.0" << std::endl;
        return 0;
    }

    // Initialise variables
    char inputChar{'x'};
    std::string cipherText;

    // Read in user input from stdin/file
    if (!settings.inputFile.empty()) {
        // Open the file and check that we can read from it
        std::ifstream inputStream{settings.inputFile};
        if (!inputStream.good()) {
            std::cerr << "[error] failed to create istream on file '"
                      << settings.inputFile << "'" << std::endl;
            return 1;
        }

        // Loop over each character from the file
        while (inputStream >> inputChar) {
            cipherText += transformChar(inputChar);
        }

    } else {
        // Loop over each character from user input
        // (until Return then CTRL-D (EOF) pressed)
        while (std::cin >> inputChar) {
            cipherText += transformChar(inputChar);
        }
    }

    // Request construction of the appropriate cipher(s)
    std::vector<std::unique_ptr<Cipher>> ciphers;
    std::size_t nCiphers{settings.cipherType.size()};
    ciphers.reserve(nCiphers);
    for (std::size_t iCipher{0}; iCipher < nCiphers; ++iCipher) {
        try {
            ciphers.push_back(CipherFactory::makeCipher( //This is the line which generates and pushes back the cipher. 
                settings.cipherType[iCipher], settings.cipherKey[iCipher]));
        } catch (const InvalidKey& e) { //if error in creating cipher due to invalid key
            std::cerr << "[error] Invalid key try for " << e.what() << " entry at position" << iCipher << std::endl;
        }

        // Check that the cipher was constructed successfully
        if (!ciphers.back()) {
            std::cerr << "[error] problem constructing requested cipher"
                      << std::endl;
            return 1;
        }
    }

    // If we are decrypting, we need to reverse the order of application of the ciphers
    if (settings.cipherMode == CipherMode::Decrypt) {
        std::reverse(ciphers.begin(), ciphers.end());
    }

    //Threading starts here
    //We have now assembled cipherText
    //Cannot do threads before this point as we need
    //We will split up ciphertext into seperate threads, and then apply cipher on each of them 

    //Calculate size of each substring
    std::size_t numThreads{4};
    const std::size_t substringSize{static_cast<std::size_t>(std::floor(cipherText.length()/numThreads))};
    const std::size_t excessSize{cipherText.length() % numThreads}; //calculate remainder, i.e. number of strings to get +1 to size
    std::size_t iStart{0};
    std::size_t iEntries{0};
    //vector will store the various 'futures'
    std::vector<std::future<std::string>> futures;
    //Create and launch thread operations
    for (std::size_t iThread = 0; iThread < numThreads; ++iThread) {
        // Calculate range of segment being looped over by each thread
        if (iThread < excessSize) {
            iStart = (substringSize + 1) * iThread;
            iEntries = substringSize + 1;
        } else {
            iStart = excessSize + (iThread * substringSize);
            iEntries = substringSize;
        }
        //Extract out substring
        std::string ciphersubText = cipherText.substr(iStart, iEntries);
        std::cout << ciphersubText << std::endl; //chunk division functions correctly
        //Start a new thread to process the chunk and push_back enciphered substrings into the futures vector
        futures.push_back(std::async(std::launch::async, [&, ciphersubText]() { //pulls from outside the scope, will edit the 'loaded' values as the for loop is executed 'serially', loading existent values into the threads
            std::string ciphersubTextLocal = ciphersubText;
            //apply ciphers to local copy of ciphertext
            for (const auto& cipher : ciphers) {
                ciphersubTextLocal = cipher->applyCipher(ciphersubTextLocal, settings.cipherMode); //apply the cipher set to the substring
            }
            return ciphersubTextLocal;
        }));
    }

    //wait for all threads to finish
    for (auto& future : futures) {
        future.wait();
    }

    //Combine the substrings into 1 big cipherText
    std::string outputText{""};
    for (auto& future : futures) {
        std::lock_guard<std::mutex> lock(outputTextMutex);
        outputText += future.get();
    }
    cipherText = outputText;

    // Output the encrypted/decrypted text to stdout/file
    if (!settings.outputFile.empty()) {
        // Open the file and check that we can write to it
        std::ofstream outputStream{settings.outputFile};
        if (!outputStream.good()) {
            std::cerr << "[error] failed to create ostream on file '"
                      << settings.outputFile << "'" << std::endl;
            return 1;
        }

        // Print the encrypted/decrypted text to the file
        outputStream << cipherText << std::endl;

    } else {
        // Print the encrypted/decrypted text to the screen
        std::cout << cipherText << std::endl;
    }

    // No requirement to return from main, but we do so for clarity
    // and for consistency with other functions
    return 0;
}
