/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Query3Handler.hpp
 * Author: mahdi
 *
 * Created on November 4, 2019, 11:32 AM
 */

#ifndef QUERY3HANDLER_HPP
#define QUERY3HANDLER_HPP

#include <openssl/bn.h>
#include <openssl/rand.h>
#include <gmp.h>

#include "DBManager.hpp"
#include "QueryHandler.hpp"
#include "Query1Handler.hpp"
#include "Query2Handler.hpp"

//#define K 160		// MultiPIR modulus size (bits)
//#define Z 128		// MultiPIR size of random values
//#define L 257		// RsaPIR modulus size (bytes)
//#define B 128		// RsaPIR block size (bytes)
//#define MAXTHREADS 2048
//typedef struct {
//	unsigned long long a0;
//	unsigned long long a1;
//	unsigned long long a2;
//	unsigned long long a3;
//} uint256;

class Query3Handler : public QueryHandler {
public:
    Query3Handler();
    Query3Handler(DBManager* t_DB, const int t_nthreads, const int t_socket, const int t_verbose);
    int char_ptr2mpz_ptr (mpz_t* out, unsigned char* in, const size_t N, const size_t M);
    int processOneQuery(unsigned char* t_query);
    void process_sub_query(const unsigned int myid);
    unsigned char* getReply();
    size_t getReplySize();
    size_t getQuerySize();
    Query3Handler(const Query3Handler& orig);
    virtual ~Query3Handler();
private:
    DBManager* m_DB;
    // d1: first level of recursion -> MultiPIR
    size_t m_d1_DB_N, m_d1_DB_M;
    // d2: second level of recursion -> RSAPIR
    mpz_t* m_d2_DB_mpz;
    size_t m_d2_DB_N, m_d2_DB_M;
    Query1Handler* m_query1handler;
    Query2Handler* m_query2handler;
    int m_clientSocket;
    unsigned char* m_query;
    size_t m_querySize;
    int m_nthreads;
    int m_verbose;
    size_t m_fileBlocksCount;
};

#endif /* QUERY3HANDLER_HPP */

