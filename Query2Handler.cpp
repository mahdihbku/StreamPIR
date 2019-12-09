/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Query2Handler.cpp
 * Author: mahdi
 * 
 * Created on November 1, 2019, 8:31 PM
 */

#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cmath>
#include <random> 
#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>
#include <chrono>
#include <ctime>
#include <thread> 
#include <iomanip>

#include "Query2Handler.hpp"
#include "lib.hpp"

using namespace std;

Query2Handler::Query2Handler() {}

Query2Handler::Query2Handler(DBManager* t_DB, int t_nthreads, const int t_socket, const int t_verbose) :
    m_DB(t_DB), m_nthreads(t_nthreads), m_clientSocket(t_socket), m_verbose(t_verbose) {
    m_mpz_db_buffer = m_DB->getDB_mpz_buffer();
    m_N = m_DB->getN();
    m_M = m_DB->getM();
    m_fileBlocksCount = m_DB->getFileBlocksCount();
    initializeParameters();
}

Query2Handler::Query2Handler(mpz_t* t_mpz_db_buffer, size_t t_N, size_t t_M, size_t t_fileBlocksCount, int t_nthreads, const int t_socket, const int t_verbose) :
    m_mpz_db_buffer(t_mpz_db_buffer), m_N(t_N), m_M(t_M), m_fileBlocksCount(t_fileBlocksCount), m_nthreads(t_nthreads), m_clientSocket(t_socket), m_verbose(t_verbose) {
    initializeParameters();
}

int Query2Handler::initializeParameters() {
    nelems_per_thread = m_fileBlocksCount / m_nthreads;
    m_querySize = L * m_N + L; // in bytes. Last K bytes for n
    m_replySize = L * m_fileBlocksCount;
    m_reply = (unsigned char*) calloc (m_replySize, sizeof(unsigned char));
    res = (mpz_t *) malloc(m_fileBlocksCount * sizeof(mpz_t));
    for (size_t i = 0; i < m_fileBlocksCount; i++)
        mpz_init(res[i]);
    Q = (mpz_t *) malloc(m_N * sizeof(mpz_t));
    for (size_t i = 0; i < m_N; i++)
        mpz_init(Q[i]);
    mpz_init(m_n);
    return 0;
}

int Query2Handler::processOneQuery(unsigned char* t_query) {
    m_query = t_query;
    for (size_t i=0; i<m_N; i++)
        mpz_import(Q[i], L, 1, 1, 1, 0, &m_query[i*L]);
    mpz_import(m_n, L, 1, 1, 1, 0, &m_query[m_N*L]);
    thread tid[MAXTHREADS];
    unsigned int v, myid[MAXTHREADS];
    for (v = 0; v < m_nthreads; v++) {
        myid[v] = v;
        tid[v] = thread(&Query2Handler::process_sub_query, this, v);
    }
    for (v = 0; v < m_nthreads; v++)
        tid[v].join();
}

void Query2Handler::process_sub_query(unsigned int myid) {
    size_t start = myid * nelems_per_thread;
    size_t end = (myid != m_nthreads-1) ? start + nelems_per_thread : m_fileBlocksCount;
    size_t i, j, k;
    for (i = 0; i < m_N; i++) {
        k = i * m_fileBlocksCount + start;
        for (j = start; j < end; j++)
            mpz_addmul(res[j], Q[i], m_mpz_db_buffer[k++]);
    } 
    for (i = start; i < end; i++) {
        mpz_mod(res[i], res[i], m_n);
        mpz_export(&m_reply[i*L], NULL, 1, L, 1, 0, res[i]);
    }
}

int Query2Handler::setDBmpzBuffer(mpz_t* t_mpz_db_buffer) { m_mpz_db_buffer=t_mpz_db_buffer; }
unsigned char* Query2Handler::getReply() { return m_reply; }
size_t Query2Handler::getReplySize() { return m_replySize; }
size_t Query2Handler::getQuerySize() { return m_querySize; }

Query2Handler::Query2Handler(const Query2Handler& orig) {}
Query2Handler::~Query2Handler() {}
