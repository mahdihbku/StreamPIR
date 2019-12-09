/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Query2Handler.hpp
 * Author: mahdi
 *
 * Created on November 1, 2019, 8:31 PM
 */

#ifndef QUERY2HANDLER_HPP
#define QUERY2HANDLER_HPP

#include "DBManager.hpp"
#include "QueryHandler.hpp"

#include <gmp.h>

#define L 257		// modulus size (bytes)
#define B 128		// block size (bytes)
#define MAXTHREADS 2048

class Query2Handler : public QueryHandler {
public:
    Query2Handler();
    Query2Handler(DBManager* t_DB, int t_nthreads, const int t_socket, const int t_verbose);
    Query2Handler(mpz_t* t_mpz_db_buffer, size_t t_N, size_t t_M, size_t t_fileBlocksCount, int t_nthreads, const int t_socket, const int t_verbose);
    int initializeParameters();
    int processOneQuery(unsigned char* t_query);
    void process_sub_query(unsigned int myid);
    unsigned char* getReply();
    int setDBmpzBuffer(mpz_t* t_mpz_db_buffer);
    size_t getReplySize();
    size_t getQuerySize();
    Query2Handler(const Query2Handler& orig);
    virtual ~Query2Handler();
private:
    DBManager* m_DB;
    mpz_t* m_mpz_db_buffer;
    size_t m_N, m_M, m_fileBlocksCount;
    int m_clientSocket;
    unsigned char* m_query;
    size_t m_querySize;
    unsigned char* m_reply;
    size_t m_replySize;
    int m_nthreads;
    int m_verbose;
    mpz_t *Q;   // query sent by client
    mpz_t *res; // result sent to client
    mpz_t m_n;    // RSA modulus
    size_t nelems_per_thread;
};

#endif /* QUERY2HANDLER_HPP */

