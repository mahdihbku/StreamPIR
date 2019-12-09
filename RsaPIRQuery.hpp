/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   RsaPIRQuery.hpp
 * Author: mahdi
 *
 * Created on November 1, 2019, 12:29 AM
 */

#ifndef RSAPIRQUERY_HPP
#define RSAPIRQUERY_HPP

#include <gmp.h>

#include "DBDescriptor.hpp"
#include "Query.hpp"

#define L 257		// modulus size (bytes)
#define B 128		// block size (bytes)
#define K 160		// MultiPIR modulus size (bits)
#define MAXTHREADS 2048

class RsaPIRQuery : public Query {
public:
    RsaPIRQuery();
    RsaPIRQuery(DBDescriptor* t_DB, int t_nthreads, int t_verbose);
    int initializeParameters();
    int generateQuery(int t_requestedFile);
    int decodeReply(unsigned char* t_reply);
    RsaPIRQuery(const RsaPIRQuery& orig);
    virtual ~RsaPIRQuery();
    
private:
//    DBDescriptor* m_DB;
    size_t m_N, m_M;
//    int m_nthreads;
//    int m_fileIndex;
//    std::string m_outputFile;
    size_t m_blocksCount;
//    unsigned char *m_query;
//    size_t m_querySize; // in bytes
//    unsigned char *m_reply;
//    size_t m_replySize; // in bytes
//    unsigned char* m_data;
//    size_t m_dataSize;
//    int m_verbose = 0;
    size_t query_nelems_per_thread;
    size_t reply_nelems_per_thread;
    void create_sub_query(int myid);
    void decode_sub_reply(int myid);
    
    mpz_t *Q;       // query sent by client
    mpz_t *res;     // result sent to client
    mpz_t n, p, q;  // RSA modulus
};

#endif /* RSAPIRQUERY_HPP */

