/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   QueryHandler.hpp
 * Author: mahdi
 *
 * Created on September 13, 2019, 11:32 AM
 */

#ifndef QUERY1HANDLER_HPP
#define QUERY1HANDLER_HPP


#include <string>
#include <openssl/bn.h>
#include <openssl/rand.h>

#include "DBManager.hpp"
#include "QueryHandler.hpp"

#define K_256 160		// modulus size
#define Z 128		// size of random values
#define MAXTHREADS 2048
typedef struct {
    unsigned long long a0;
    unsigned long long a1;
    unsigned long long a2;
    unsigned long long a3;
} uint256;

class Query1Handler : public QueryHandler {
public:
    Query1Handler();
    Query1Handler(DBManager* t_DB, int t_nthreads, const int t_socket, const int t_verbose);
    Query1Handler(unsigned char* t_db_buffer, size_t t_N, size_t t_M, int t_nthreads, const int t_socket, const int t_verbose);
    int initializeParameters();
    int processOneQuery(unsigned char* t_query);
    void expand_sub_query(unsigned int myid);
    void process_sub_query(unsigned int myid);
    unsigned char* getReply();
    size_t getReplySize();
    size_t getQuerySize();
    Query1Handler(const Query1Handler& orig);
    virtual ~Query1Handler();
private:
    DBManager* m_DB;
    unsigned char *m_db_buffer;
    size_t m_N, m_M;
    int m_clientSocket;
    unsigned char* m_query;
    size_t m_querySize;
    unsigned char* m_reply;
    size_t m_replySize;
    int m_nthreads;
    BIGNUM **res; // result returned by server
    BIGNUM *m_p;
    uint256 *q; // query sent by client
    uint256 *Q; // expanded query
    uint256 *n; // client's result
    int m_verbose;
    size_t query_nelems_per_thread;
    size_t reply_nelems_per_thread;
};

#endif /* QUERY1HANDLER_HPP */

