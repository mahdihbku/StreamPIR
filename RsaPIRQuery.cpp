/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   RsaPIRQuery.cpp
 * Author: mahdi
 * 
 * Created on November 1, 2019, 12:29 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <random> 
#include <fstream>
#include <iostream>
#include <cstring>
#include <string>
#include <sstream>
#include <iomanip>
#include <netdb.h>
#include <bitset>
#include <thread>

#include "RsaPIRQuery.hpp"
#include "DBDescriptor.hpp"
#include "lib.hpp"

using namespace std;

RsaPIRQuery::RsaPIRQuery() {}

RsaPIRQuery::RsaPIRQuery(DBDescriptor* t_DB, int t_nthreads, int t_verbose) {
    m_DB = t_DB;
    m_nthreads = t_nthreads;
    m_verbose = t_verbose;
    if (m_DB->getPirVersion() == 2) {
        m_N = m_DB->getN();
        m_M = m_DB->getM();
    } else if (m_DB->getPirVersion() == 3) {
        m_N = m_DB->getN2();
        m_M = m_DB->getM() * K/8;
    } else
        error("RsaPIRQuery::RsaPIRQuery: PirVersion incorrect");
    initializeParameters();
}

int RsaPIRQuery::initializeParameters() {
    m_querySize = L * m_N + L;  // in bytes. Last L bytes for n
    m_query = (unsigned char *) calloc(m_querySize, sizeof(unsigned char));
    m_blocksCount = m_M / B;
    if (m_M % B != 0) m_blocksCount++;
    m_replySize = L * m_blocksCount;    // == M * L / B
    m_reply = (unsigned char *) malloc(m_replySize * sizeof(unsigned char));
    m_dataSize = m_M;
    m_data = (unsigned char *) calloc(m_dataSize, sizeof(unsigned char));
    query_nelems_per_thread = m_N / m_nthreads;
    reply_nelems_per_thread = m_blocksCount / m_nthreads;
    
    // initialize mpz_t's
    res = (mpz_t *)malloc(m_blocksCount * sizeof(mpz_t));
    size_t i;
    for (i=0; i < m_blocksCount; i++)
        mpz_init(res[i]);
    Q = (mpz_t *)malloc(m_N * sizeof(mpz_t));
    for (i=0; i < m_N; i++)
        mpz_init(Q[i]);
    mpz_init(p);
    mpz_init(q);
    mpz_init(n);
    
    // Generating keys
    gmp_randstate_t state;
    gmp_randinit_default(state);
    gmp_randseed_ui(state, time(0));
    size_t j = L<<2;
    // compute primes p and q of equal size
    do {
        mpz_urandomb(p, state, j);
        mpz_nextprime(p, p);
    } while (mpz_sizeinbase(p, 2) != j);
    do {
        mpz_urandomb(q, state, j);
        mpz_nextprime(q, q);
    } while (mpz_sizeinbase(q, 2) != j);
    mpz_mul(n, p, q);
    return 0;
}

int RsaPIRQuery::generateQuery(int t_requestedFile) {
    m_fileIndex = t_requestedFile;
    thread tid[MAXTHREADS];
    unsigned int v, myid[MAXTHREADS];
    for (v = 0; v < m_nthreads; v++) {
        myid[v] = v;
        tid[v] = thread(&RsaPIRQuery::create_sub_query, this, v);
    }
    for (v = 0; v < m_nthreads; v++)
        tid[v].join();
    for (size_t i=0; i<m_N; i++)
        mpz_export(&m_query[i*L], NULL, 1, L, 1, 0, Q[i]);
    mpz_export(&m_query[m_N*L], NULL, 1, L, 1, 0, n);
    return 0;
}

void RsaPIRQuery::create_sub_query(int myid) {
    size_t start = myid * query_nelems_per_thread;
    size_t end = (myid != m_nthreads-1) ? start + query_nelems_per_thread : m_N;
    gmp_randstate_t state;
    gmp_randinit_default(state);
    gmp_randseed_ui(state, time(0));
    mpz_t r;
    mpz_init(r);
    for (size_t i = start; i < end; i++) {
        mpz_urandomm(r, state, n);
        mpz_mul(Q[i], p, r);
        mpz_mod(Q[i], Q[i], n);
        if (i == m_fileIndex) {
            mpz_add_ui(Q[i], Q[i], 1);
            mpz_mod(Q[i], Q[i], n);
        }
    }
}

int RsaPIRQuery::decodeReply(unsigned char* t_reply) {
    m_reply = t_reply;
    thread tid[MAXTHREADS];
    unsigned int v, myid[MAXTHREADS];
    for (v = 0; v < m_nthreads; v++) {
        myid[v] = v;
        tid[v] = thread(&RsaPIRQuery::decode_sub_reply, this, v);
    }
    for (v = 0; v < m_nthreads; v++)
        tid[v].join();
    return 0;
}

void RsaPIRQuery::decode_sub_reply(int myid) {
    size_t start = myid * reply_nelems_per_thread;
    size_t end = (myid != m_nthreads-1) ? start + reply_nelems_per_thread : m_blocksCount;
    for (size_t i = start; i < end; i++) {
        mpz_import(res[i], L, 1, 1, 1, 0, &m_reply[i*L]);
        mpz_mul(res[i], res[i], q);
        mpz_mod(res[i], res[i], n);
        mpz_divexact(res[i], res[i], q);
        mpz_export(&m_data[i*B], NULL, 1, B, 1, 0, res[i]);
    }
}

RsaPIRQuery::RsaPIRQuery(const RsaPIRQuery& orig) {}
RsaPIRQuery::~RsaPIRQuery() {}
