/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   lib.cpp
 * Author: mahdi
 * 
 * Created on December 12, 2017, 12:25 PM
 */
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <netdb.h>

// TODO check later on which libs are necessary for bzero, memcpy, write and read
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <cstring>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <math.h> 
#include <netinet/tcp.h>
#include <vector>
#include <random>
#include <algorithm>

#include "lib.hpp"

using namespace std;

int error(const char *t_msg) {
    perror(t_msg);
    abort();
}

int min(const size_t t_a, const size_t t_b) {
    return (t_a>t_b) ? t_b : t_a;
}

size_t getFileSize(const string fileName) {
    size_t size = 0;
    ifstream fileStream(fileName, ios::binary);
    string errorMsg = "Unable to open the file: " + fileName;
    if (!fileStream.is_open()) error(errorMsg.c_str());
    fileStream.seekg (0, fileStream.end);
    size = fileStream.tellg();
    fileStream.close();
    return size;
}

#define HEX(x) setw(2) << setfill('0') << hex << (int)(x)

int char2int(unsigned char input)
{
    if(input >= '0' && input <= '9')
        return input - '0';
    if(input >= 'A' && input <= 'F')
        return input - 'A' + 10;
    if(input >= 'a' && input <= 'f')
        return input - 'a' + 10;
    error("char2int:Invalid input string");
}

int sendData(unsigned char* buff, size_t buffSize, int socket) {
    int sockErr = 0;
    unsigned char *dataBuffer = (unsigned char *) malloc (DATA_BUFFER_SIZE * sizeof (unsigned char));
    bzero(dataBuffer, DATA_BUFFER_SIZE);
    size_t numberOfBytesToSend = buffSize;
    unsigned char* current = buff;
    size_t minSize = 0;
    do {
        minSize = min(numberOfBytesToSend, DATA_BUFFER_SIZE);
        memcpy(dataBuffer, current, minSize);
        current += minSize;
        sockErr = write(socket, dataBuffer, DATA_BUFFER_SIZE);
        if (sockErr < 0) error("ERROR sending data");
        numberOfBytesToSend -= minSize;
   } while(numberOfBytesToSend > 0);
    free(dataBuffer);
   return 0;
}

int receiveData(unsigned char* buff, size_t buffSize, int socket) {
    // buff should be malloc'd outside
    int sockErr = 0;
    unsigned char* dataBuffer = (unsigned char *) malloc (DATA_BUFFER_SIZE * sizeof (unsigned char));
    bzero((char *)dataBuffer, DATA_BUFFER_SIZE);
    size_t numberOfBytesToReceive = buffSize;
    size_t minSize = 0;
    unsigned char* current = buff;
    do {
        sockErr = 0;
        minSize = min(numberOfBytesToReceive, DATA_BUFFER_SIZE);
        while (sockErr < DATA_BUFFER_SIZE)
            sockErr += read(socket, dataBuffer+sockErr, DATA_BUFFER_SIZE-sockErr);
        if (sockErr < 0) error("ERROR receiving data");
        memcpy(current, dataBuffer, minSize);
        current += minSize;
        numberOfBytesToReceive -= minSize;
    } while(numberOfBytesToReceive > 0);
    free(dataBuffer);
    return 0;
}

int sendInfo(string info, int socket) {
    int sockErr = 0;
    unsigned char *infoBuffer = (unsigned char *) malloc (INFO_BUFFER_SIZE * sizeof (unsigned char));
    bzero(infoBuffer, INFO_BUFFER_SIZE);
    strcpy((char *)infoBuffer, info.c_str());
    sockErr = write(socket, infoBuffer, INFO_BUFFER_SIZE);
    if (sockErr < 0) error("ERROR writing to socket");
    free(infoBuffer);
   return 0;
}

int sendInfo(unsigned char* buff, size_t buffSize, int socket) {
    int sockErr = 0;
    unsigned char *infoBuffer;
    infoBuffer = (unsigned char *) malloc (INFO_BUFFER_SIZE * sizeof (unsigned char));
    bzero(infoBuffer, INFO_BUFFER_SIZE);
    memcpy(infoBuffer, buff, buffSize);
    sockErr = write(socket, infoBuffer, INFO_BUFFER_SIZE);
    if (sockErr < 0) error("ERROR writing to socket");
    free(infoBuffer);
   return 0;
}

int receiveInfo(unsigned char* buff, size_t buffSize, int socket) {
    // buff should be malloc'd outside
    int sockErr = 0;
    unsigned char* infoBuffer = (unsigned char *) malloc (INFO_BUFFER_SIZE * sizeof (unsigned char));
    bzero(infoBuffer, INFO_BUFFER_SIZE);
    sockErr = 0;
    while (sockErr<INFO_BUFFER_SIZE)
        sockErr += read(socket, infoBuffer+sockErr, INFO_BUFFER_SIZE-sockErr);
    if (sockErr < 0) error("ERROR while reading data");
    memcpy(buff, infoBuffer, buffSize);
    free(infoBuffer);
    return 0;
}

int bn2binpad(const BIGNUM *a, unsigned char *to, int tolen) {
    // this code is copied from BIGNUM library
    int n;
    size_t i, lasti, j, atop, mask;
    BN_ULONG l;
    /*
     * In case |a| is fixed-top, BN_num_bytes can return bogus length,
     * but it's assumed that fixed-top inputs ought to be "nominated"
     * even for padded output, so it works out...
     */
    n = BN_num_bytes(a);
    if (tolen == -1) {
        tolen = n;
    } else if (tolen < n) {     /* uncommon/unlike case */
        BIGNUM temp = *a;
        bn_correct_top(&temp);
        n = BN_num_bytes(&temp);
        if (tolen < n)
            return -1;
    }
    /* Swipe through whole available data and don't give away padded zero. */
    atop = a->dmax * BN_BYTES;
    if (atop == 0) {
        OPENSSL_cleanse(to, tolen);
        return tolen;
    }
    lasti = atop - 1;
    atop = a->top * BN_BYTES;
    for (i = 0, j = 0, to += tolen; j < (size_t)tolen; j++) {
        l = a->d[i / BN_BYTES];
        mask = 0 - ((j - atop) >> (8 * sizeof(i) - 1));
        *--to = (unsigned char)(l >> (8 * (i % BN_BYTES)) & mask);
        i += (i - lasti) >> (8 * sizeof(i) - 1); /* stay on last limb */
    }
    return tolen;
}

int hex2bin(string src, unsigned char* target, size_t binSize) {
    for (size_t i=0; i<binSize; i++)
        target[i] = char2int(src.at(i*2))*16 + char2int(src.at(i*2+1));
    return binSize;
}

string bin2hex(const unsigned char* src, size_t binSize) {
    stringstream seedSS;
    for (int i=0; i<binSize; i++)
        seedSS << HEX((uint8_t)src[i]);
    return seedSS.str();
}

int hex2bin(unsigned char* src, unsigned char* target, size_t binSize) {
    for (size_t i=0; i<binSize; i++)
        target[i] = char2int(src[i*2])*16 + char2int(src[i*2+1]);
    return binSize;
}
