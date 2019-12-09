/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Query.cpp
 * Author: mahdi
 * 
 * Created on September 10, 2019, 11:54 AM
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

#include "PrimePIR256Query.hpp"
#include "DBDescriptor.hpp"
#include "lib.hpp"

using namespace std;

PrimePIR256Query::PrimePIR256Query() {}

PrimePIR256Query::PrimePIR256Query(DBDescriptor* t_DB, int t_nthreads, int t_verbose) {
    m_DB = t_DB;
    m_nthreads = t_nthreads;
    m_verbose = t_verbose;
    if (m_DB->getPirVersion() == 1) {
        m_N = m_DB->getN();
        m_M = m_DB->getM();
    } else if (m_DB->getPirVersion() == 3) {
        m_N = m_DB->getN();
        m_M = m_DB->getN2()*m_DB->getM();
    } else
        error("PrimePIR256Query::PrimePIR256Query: PirVersion incorrect");
    initializeParameters();
}

int PrimePIR256Query::initializeParameters() {
    m_querySize = m_N * K_256 + K_256/8; // K/8 to send p with the query
    m_query = (unsigned char *) calloc(m_querySize, sizeof(unsigned char));
    m_replySize = (m_DB->getPirVersion() == 1) ? m_M * K_256/8 : m_M/m_DB->getN2() * K_256/8;   // if hybridPIR, the reply will be of size M not M*N2
    m_reply = (unsigned char *)malloc(m_replySize * sizeof(unsigned char));
    m_dataSize = (m_DB->getPirVersion() == 1) ? m_M : m_M/m_DB->getN2();
    m_data = (unsigned char *)malloc(m_dataSize * sizeof(unsigned char));
    query_nelems_per_thread = m_N / m_nthreads;
    reply_nelems_per_thread = m_dataSize / m_nthreads;
    BN_CTX *ctx = BN_CTX_new();
    m_p = BN_new();
    m_k = BN_new();
    m_invk = BN_new();
    BN_generate_prime_ex(m_p, K_256, 0, NULL, NULL, NULL);
    BN_rand_range(m_k, m_p);
    BN_mod_inverse(m_invk, m_k, m_p, ctx);
    res = (BIGNUM **)malloc(m_dataSize * sizeof(BIGNUM *));
    for (size_t i = 0; i < m_dataSize; i++)
        res[i] = BN_new();
    return 0;
}

int PrimePIR256Query::generateQuery(int t_requestedFile) {
    m_fileIndex = t_requestedFile;
    thread tid[MAXTHREADS];
    unsigned int v, myid[MAXTHREADS];
    unsigned char p[K_256/8];
    for (int v = 0; v < m_nthreads; v++) {
        myid[v] = v;
        tid[v] = thread(&PrimePIR256Query::create_sub_query, this, v);
    }
    for (v = 0; v < m_nthreads; v++)
        tid[v].join();
    // adding p at the end of the query buffer
    bn2binpad(m_p, p, K_256/8);
    memcpy(&m_query[m_N*K_256], p, K_256/8);
    return 0;
}

void PrimePIR256Query::create_sub_query(int myid) {
    size_t start = myid * query_nelems_per_thread;
    size_t end = (myid != m_nthreads-1) ? start + query_nelems_per_thread : m_N;
    BIGNUM *r, *a, *b;
    size_t i, j, m;
    BN_CTX *ctx = BN_CTX_new();

    a = BN_new();
    b = BN_new();
    r = BN_new();
    for (i = start; i < end; i++) {
        if (i == m_fileIndex) {
            BN_one(a);
            BN_lshift(a, a, index_256);
            for (j = 0; j < 8; j++) {
                BN_rand(r, Z, Z, Z);
                BN_lshift(b, a, j);
                BN_add(r, r, b);
                BN_mod_mul(r, r, m_k, m_p, ctx);
                m = (i*8+j) * K_256/8;
                bn2binpad(r, &m_query[m], K_256/8);
            }
        } else {
            for (j = 0; j < 8; j++) {
                BN_rand(r, Z, Z, Z);
                BN_mod_mul(r, r, m_k, m_p, ctx);
                m = (i*8+j) * K_256/8;
                bn2binpad(r, &m_query[m], K_256/8);
            }
        }
    }
}

int PrimePIR256Query::decodeReply(unsigned char* t_reply) {
    m_reply = t_reply;
    thread tid[MAXTHREADS];
    unsigned int v, myid[MAXTHREADS];
    for (v = 0; v < m_nthreads; v++) {
        myid[v] = v;
        tid[v] = thread(&PrimePIR256Query::decode_sub_reply, this, v);
    }
    for (v = 0; v < m_nthreads; v++)
        tid[v].join();
    return 0;
}

void PrimePIR256Query::decode_sub_reply(int myid) {
    size_t start = myid * reply_nelems_per_thread;
    size_t end = (myid != m_nthreads-1) ? start + reply_nelems_per_thread : m_dataSize;
    size_t i;
    BN_CTX *ctx = BN_CTX_new();
    for (i = start; i < end; i++) {
        BN_bin2bn(&m_reply[i*K_256/8], K_256/8, res[i]);
        BN_mod_mul(res[i], res[i], m_invk, m_p, ctx);
        BN_rshift(res[i], res[i], index_256);
        m_data[i] = (unsigned char) BN_get_word(res[i]);
    }
}

PrimePIR256Query::PrimePIR256Query(const PrimePIR256Query& orig) {}
PrimePIR256Query::~PrimePIR256Query() {}
