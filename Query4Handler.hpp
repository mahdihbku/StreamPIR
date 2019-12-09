/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Query4Handler.hpp
 * Author: mahdi
 *
 * Created on November 24, 2019, 4:29 PM
 */

#ifndef QUERY4HANDLER_HPP
#define QUERY4HANDLER_HPP

#include <string>
#include <openssl/bn.h>
#include <openssl/rand.h>

#include "DBManager.hpp"
#include "QueryHandler.hpp"

#define K_512 320                   // modulus size (bits)
#define index_512 239               // index in plaintext where data should be put
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

class Query4Handler : public QueryHandler {
public:
    Query4Handler();
    Query4Handler(DBManager* t_DB, int t_nthreads, const int t_socket, const int t_verbose);
    Query4Handler(unsigned char* t_db_buffer, size_t t_N, size_t t_M, int t_nthreads, const int t_socket, const int t_verbose);
    int initializeParameters();
    int processOneQuery(unsigned char* t_query);
    int setNbrCharsPerCipher(int t_chars_per_ciphertext);
    void expand_sub_query(unsigned int myid);
    void process_sub_query(unsigned int myid);
    void process_one_char(unsigned long long i, uint512 *n, unsigned char *bn);
    unsigned char* getReply();
    size_t getReplySize();
    size_t getQuerySize();
    Query4Handler(const Query4Handler& orig);
    virtual ~Query4Handler();
private:
    unsigned long long u40 = 0xFFFFFFFFFF;
    unsigned int default_chars_per_ciphertext = 10;   // number of chars encoded in one ct
    unsigned int chars_per_ciphertext;
    DBManager* m_DB;
    unsigned char *m_db_buffer;
    size_t m_N, m_M;
    size_t ciphertext_count;    // number of ciphertext to accommodate one file
    int m_clientSocket;
    unsigned char* m_query;
    size_t m_querySize;
    unsigned char* m_reply;
    size_t m_replySize;
    int m_nthreads;
    BIGNUM **res; // result returned by server
    BIGNUM *m_p;
    uint512 *q; // query sent by client
    uint512 *Q; // expanded query
    uint512 *n; // client's result
    int m_verbose;
    size_t query_nelems_per_thread;
    size_t reply_nelems_per_thread;
};

#endif /* QUERY4HANDLER_HPP */

