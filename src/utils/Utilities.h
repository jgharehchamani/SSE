#ifndef UTILITIES_H
#define UTILITIES_H
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <chrono>
#include <stdlib.h>
#include <sse/crypto/hash.hpp>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <math.h>

class Utilities {
private:
    static int parseLine(char* line);
public:
    Utilities();
    static std::string base64_encode(const char* bytes_to_encode, unsigned int in_len);
    static std::string base64_decode(std::string const& enc);
    static std::string XOR(std::string value, std::string key);
    static void startTimer(int id);
    static double stopTimer(int id);
    static std::map<int, std::chrono::time_point<std::chrono::high_resolution_clock> > m_begs;
    static std::map<std::string, std::ofstream*> handlers;
    static void logTime(std::string filename, std::string content);
    static void initializeLogging(std::string filename);
    static void finalizeLogging(std::string filename);
    static std::array<uint8_t, 16> convertToArray(std::string value);
    static int getMem();
    static double getTimeFromHist(int id);
    static int getBid(std::string srchIndex);
    static std::array<uint8_t, 16> encode(std::string keyword);
    static std::string decode(std::array<uint8_t, 16> data);
    static unsigned char key[16], iv[16];
    static int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, unsigned char *plaintext);
    static int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext);
    static void handleErrors(void);    
    virtual ~Utilities();
};

#endif /* UTILITIES_H */

