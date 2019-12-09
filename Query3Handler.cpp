/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Query3Handler.cpp
 * Author: mahdi
 * 
 * Created on November 4, 2019, 11:32 AM
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

#include "Query3Handler.hpp"
#include "lib.hpp"

using namespace std;

Query3Handler::Query3Handler() {}

Query3Handler::Query3Handler(DBManager* t_DB, const int t_nthreads, const int t_socket, const int t_verbose) :
    m_DB (t_DB), m_nthreads(t_nthreads), m_clientSocket(t_socket), m_verbose(t_verbose) {
    m_d1_DB_N = m_DB->getN(), m_d1_DB_M = m_DB->getN2()*m_DB->getM();
    m_d2_DB_N = m_DB->getN2(), m_d2_DB_M = m_DB->getM()*K_256/8;
    m_query1handler = new Query1Handler(m_DB->getDBbuffer(), m_d1_DB_N, m_d1_DB_M, m_nthreads, m_clientSocket, m_verbose);
    m_fileBlocksCount = m_d2_DB_M / B;  // RSAPIR uses blocks not char*
    m_query2handler = new Query2Handler(NULL, m_d2_DB_N, m_d2_DB_M, m_fileBlocksCount, m_nthreads, m_clientSocket, m_verbose);
    m_querySize = m_query1handler->getQuerySize() + m_query2handler->getQuerySize();
    size_t m_dbBlocksCount = m_fileBlocksCount * m_d2_DB_N;
    m_d2_DB_mpz = (mpz_t *)malloc(m_dbBlocksCount * sizeof(mpz_t));
    for (size_t i = 0; i < m_dbBlocksCount; i++)
        mpz_init(m_d2_DB_mpz[i]);
}

int Query3Handler::processOneQuery(unsigned char* t_query) {
    m_query = t_query;
    m_query1handler->processOneQuery(m_query);
    char_ptr2mpz_ptr(m_d2_DB_mpz, m_query1handler->getReply(), m_d2_DB_N, m_d2_DB_M);
    m_query2handler->setDBmpzBuffer(m_d2_DB_mpz);
    m_query2handler->processOneQuery(&m_query[m_query1handler->getQuerySize()]);
    return 0;
}

int Query3Handler::char_ptr2mpz_ptr (mpz_t* out, unsigned char* in, const size_t N, const size_t M) {
    for (size_t k=0; k<N; k++)
        for (size_t l=0; l<m_fileBlocksCount; l++)
            mpz_import(out[k*m_fileBlocksCount+l], B, 1, 1, 1, 0, &in[k*M+l*B]);
    return 0;
}

size_t Query3Handler::getQuerySize() { return m_querySize; }
unsigned char* Query3Handler::getReply() { return m_query2handler->getReply(); }
size_t Query3Handler::getReplySize() { return m_query2handler->getReplySize(); }

Query3Handler::Query3Handler(const Query3Handler& orig) {}
Query3Handler::~Query3Handler() {}
