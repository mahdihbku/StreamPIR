/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   PrimePIR512Query.cpp
 * Author: mahdi
 * 
 * Created on November 19, 2019, 12:12 PM
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

#include "PrimePIR512Query.hpp"
#include "DBDescriptor.hpp"
#include "lib.hpp"

using namespace std;

PrimePIR512Query::PrimePIR512Query() {}

PrimePIR512Query::PrimePIR512Query(DBDescriptor* t_DB, int t_nthreads, int t_verbose) {
    m_DB = t_DB;
    m_nthreads = t_nthreads;
    m_verbose = t_verbose;
    if (m_DB->getPirVersion() == 4) {
        m_N = m_DB->getN();
        m_M = m_DB->getM();
    } else if (m_DB->getPirVersion() == 5) {
        m_N = m_DB->getN();
        m_M = m_DB->getN2()*m_DB->getM();
    } else
        error("PrimePIR512Query::PrimePIR512Query: PirVersion incorrect");
    initializeParameters();
}

int PrimePIR512Query::initializeParameters() {
    if (m_DB->getPirVersion() == 4) {
        m_dataSize = m_M;
        ciphertext_count = m_M/chars_per_ciphertext;
        if (m_M % chars_per_ciphertext != 0) ciphertext_count++;
//    } else if (m_DB->getPirVersion() == 5) {   // if hybridPIR, the reply will be of size M not M*N2
//        m_dataSize = m_M/m_DB->getN2();
//        ciphertext_count = m_M/m_DB->getN2()/chars_per_ciphertext;
//        if (m_M/m_DB->getN2() % chars_per_ciphertext != 0) ciphertext_count++;
    } else
        error("PrimePIR512Query::initializeParameters: Incorrect PIR version");
    m_querySize = m_N * K_512 + K_512/8 + 1; // K/8 to send p with the query; last byte for query aggregation
    m_query = (unsigned char *) calloc(m_querySize, sizeof(unsigned char));
    m_replySize = ciphertext_count * K_512/8;
    m_reply = (unsigned char *)malloc(m_replySize * sizeof(unsigned char));
    m_data = (unsigned char *)malloc(m_dataSize * sizeof(unsigned char));
    query_nelems_per_thread = m_N / m_nthreads;
    reply_nelems_per_thread = m_M / m_nthreads / chars_per_ciphertext * chars_per_ciphertext;   // always multiple of chars_per_ciphertext
    BN_CTX *ctx = BN_CTX_new();
    m_p = BN_new();
    m_k = BN_new();
    m_invk = BN_new();
    BN_generate_prime_ex(m_p, K_512, 0, NULL, NULL, NULL);
    BN_rand_range(m_k, m_p);
    BN_mod_inverse(m_invk, m_k, m_p, ctx);
    res = (BIGNUM **)malloc(ciphertext_count * sizeof(BIGNUM *));
    for (size_t i = 0; i < ciphertext_count; i++)
        res[i] = BN_new();
    return 0;
}

int PrimePIR512Query::generateQuery(int t_requestedFile) {
    m_fileIndex = t_requestedFile;
    thread tid[MAXTHREADS];
    unsigned int v, myid[MAXTHREADS];
    unsigned char p[K_512/8];
    for (int v = 0; v < m_nthreads; v++) {
        myid[v] = v;
        tid[v] = thread(&PrimePIR512Query::create_sub_query, this, v);
    }
    for (v = 0; v < m_nthreads; v++)
        tid[v].join();
    // adding p at the end of the query buffer
    bn2binpad(m_p, p, K_512/8);
    memcpy(&m_query[m_N*K_512], p, K_512/8);
    m_query[m_N*K_512+K_512/8] = 0x1;   // no query aggregation TODO continue from here. when there is query agg
    return 0;
}

void PrimePIR512Query::create_sub_query(int myid) {
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
            BN_lshift(a, a, index_512);
            for (j = 0; j < 8; j++) {
                BN_rand(r, Z, Z, Z);
                BN_lshift(b, a, j);
                BN_add(r, r, b);
                BN_mod_mul(r, r, m_k, m_p, ctx);
                m = (i*8+j) * K_512/8;
                bn2binpad(r, &m_query[m], K_512/8);
            }
        } else {
            for (j = 0; j < 8; j++) {
                BN_rand(r, Z, Z, Z);
                BN_mod_mul(r, r, m_k, m_p, ctx);
                m = (i*8+j) * K_512/8;
                bn2binpad(r, &m_query[m], K_512/8);
            }
        }
    }
}

int PrimePIR512Query::decodeReply(unsigned char* t_reply) {
    m_reply = t_reply;
    thread tid[MAXTHREADS];
    unsigned int v, myid[MAXTHREADS];
    for (v = 0; v < m_nthreads; v++) {
        myid[v] = v;
        tid[v] = thread(&PrimePIR512Query::decode_sub_reply, this, v);
    }
    for (v = 0; v < m_nthreads; v++)
        tid[v].join();
    return 0;
}

void PrimePIR512Query::decode_sub_reply(int myid) {
    size_t start = myid * reply_nelems_per_thread;
    size_t end = (myid != m_nthreads-1) ? start + reply_nelems_per_thread : m_dataSize;
    size_t i, current_ct;
    BN_CTX *ctx = BN_CTX_new();
    for (i = start; i+chars_per_ciphertext <= end; i += chars_per_ciphertext) {
        current_ct = i/chars_per_ciphertext;
        BN_bin2bn(&m_reply[current_ct*K_512/8], K_512/8, res[current_ct]);
        BN_mod_mul(res[current_ct], res[current_ct], m_invk, m_p, ctx);
        BN_rshift(res[current_ct], res[current_ct], index_512);
        bn2binpad(res[current_ct], &m_data[i], chars_per_ciphertext);
    }
    if (i < end) {
        current_ct = i/chars_per_ciphertext;
        BN_bin2bn(&m_reply[current_ct*K_512/8], K_512/8, res[current_ct]);
        BN_mod_mul(res[current_ct], res[current_ct], m_invk, m_p, ctx);
        BN_rshift(res[current_ct], res[current_ct], index_512);
        bn2binpad(res[current_ct], &m_data[i], end-i);
    }
}

PrimePIR512Query::PrimePIR512Query(const PrimePIR512Query& orig) {}
PrimePIR512Query::~PrimePIR512Query() {}
