/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   lib.hpp
 * Author: mahdi
 *
 * Created on December 12, 2018, 12:25 PM
 */

#ifndef LIB_HPP
#define LIB_HPP

#include <openssl/bn.h>
#include <gmp.h>

static const int INFO_BUFFER_SIZE = 1024;
static const int DATA_BUFFER_SIZE = 16384;

int error(const char *t_msg);
int min(const size_t t_a, const size_t t_b);
size_t getFileSize(const std::string fileName);
int char2int(unsigned char input);
int hex2bin(std::string src, unsigned char* target, size_t binSize);
int hex2bin(unsigned char* src, unsigned char* target, size_t binSize);
std::string bin2hex(const unsigned char* src, size_t binSize);
int sendData(unsigned char* buff, size_t buffSize, int socket);
int receiveData(unsigned char* buff, size_t buffSize, int socket);
int sendInfo(std::string info, int socket);
int sendInfo(unsigned char* buff, size_t buffSize, int socket);
int receiveInfo(unsigned char* buff, size_t buffSize, int socket);
int bn2binpad(const BIGNUM *a, unsigned char *to, int tolen);

#endif /* LIB_HPP */

