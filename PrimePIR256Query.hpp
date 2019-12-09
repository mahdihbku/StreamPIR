/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Query.hpp
 * Author: mahdi
 *
 * Created on September 10, 2019, 11:54 AM
 */

#ifndef PRIME256QUERY_HPP
#define PRIME256QUERY_HPP

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <openssl/bn.h>
#include <openssl/rand.h>

#include "Query.hpp"
#include "DBDescriptor.hpp"

#define K_256 160	// modulus size (bits)
#define index_256 151   // index in plaintext where data should be put
#define Z 128		// size of random values
#define MAXTHREADS 2048
typedef struct {
	unsigned long long a0;
	unsigned long long a1;
	unsigned long long a2;
	unsigned long long a3;
} uint256;

class PrimePIR256Query : public Query {
public:
    PrimePIR256Query();
    PrimePIR256Query(DBDescriptor* t_DB, int t_nthreads, int t_verbose);
    int initializeParameters();
    int generateQuery(int t_requestedFile);
    int decodeReply(unsigned char* t_reply);
    int decodeHybridReply(unsigned char* t_reply);
    PrimePIR256Query(const PrimePIR256Query& orig);
    virtual ~PrimePIR256Query();
    
private:
//    DBDescriptor* m_DB;
    size_t m_N, m_M;
//    int m_nthreads;
//    int m_fileIndex;
    BIGNUM *m_p, *m_k, *m_invk;  // query prime
//    std::string m_outputFile;
//    uint256 *m_query_uint256;
//    size_t m_querySize;     // in bytes
//    unsigned char *m_reply;
//    size_t m_replySize;     // in bytes
    BIGNUM **res;       // result returned by server
//    size_t m_dataSize;      // in bytes
//    unsigned char* m_data;
//    int m_verbose = 0;
    size_t query_nelems_per_thread;
    size_t reply_nelems_per_thread;
    void create_sub_query(int myid);
    void decode_sub_reply(int myid);
};

#endif /* PRIME256QUERY_HPP */
