/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   PrimePIR512Query.hpp
 * Author: mahdi
 *
 * Created on November 19, 2019, 12:12 PM
 */

#ifndef PRIMEPIR512QUERY_HPP
#define PRIMEPIR512QUERY_HPP

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <openssl/bn.h>
#include <openssl/rand.h>

#include "Query.hpp"
#include "DBDescriptor.hpp"

#define K_512 320                   // modulus size (bits)
#define index_512 239               // index in plaintext where data should be put
#define chars_per_ciphertext 10     // number of chars encoded in one ct
#define Z 128                       // size of random values
#define MAXTHREADS 2048
typedef struct {
	unsigned long long a0;
	unsigned long long a1;
	unsigned long long a2;
	unsigned long long a3;
	unsigned long long a4;
	unsigned long long a5;
	unsigned long long a6;
	unsigned long long a7;
} uint512;

class PrimePIR512Query : public Query {
public:
    PrimePIR512Query();
    PrimePIR512Query(DBDescriptor* t_DB, int t_nthreads, int t_verbose);
    int initializeParameters();
    int generateQuery(int t_requestedFile);
    int decodeReply(unsigned char* t_reply);
    int decodeHybridReply(unsigned char* t_reply);
    PrimePIR512Query(const PrimePIR512Query& orig);
    virtual ~PrimePIR512Query();
private:
//    DBDescriptor* m_DB;
    size_t m_N, m_M;
    size_t ciphertext_count;    // number of ciphertext to accommodate one file
//    int m_nthreads;
//    int m_fileIndex;
    BIGNUM *m_p, *m_k, *m_invk;
//    std::string m_outputFile;
//    uint512 *m_query_uint512;
//    size_t m_querySize;         // in bytes
//    unsigned char *m_reply;
//    size_t m_replySize;         // in bytes
    BIGNUM **res;               // result returned by server
//    size_t m_dataSize;          // in bytes
//    unsigned char* m_data;
//    int m_verbose = 0;
    size_t query_nelems_per_thread;
    size_t reply_nelems_per_thread;
    void create_sub_query(int myid);
    void decode_sub_reply(int myid);
};

#endif /* PRIMEPIR512QUERY_HPP */
