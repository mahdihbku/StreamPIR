/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   QueryHandler.cpp
 * Author: mahdi
 * 
 * Created on September 13, 2019, 11:32 AM
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
#include <openssl/bn.h>
#include <openssl/rand.h>

#include "Query1Handler.hpp"
#include "lib.hpp"

using namespace std;

Query1Handler::Query1Handler() {}

Query1Handler::Query1Handler(DBManager* t_DB, const int t_nthreads, const int t_socket, const int t_verbose) :
    m_DB(t_DB), m_nthreads(t_nthreads), m_clientSocket(t_socket), m_verbose(t_verbose) {
    m_db_buffer = m_DB->getDBbuffer();
    m_N = m_DB->getN();
    m_M = m_DB->getM();
    initializeParameters();
}

Query1Handler::Query1Handler(unsigned char* t_db_buffer, size_t t_N, size_t t_M, int t_nthreads, const int t_socket, const int t_verbose) :
    m_db_buffer(t_db_buffer), m_N(t_N), m_M(t_M), m_nthreads(t_nthreads), m_clientSocket(t_socket), m_verbose(t_verbose) {
    initializeParameters();
}

int Query1Handler::initializeParameters() {
    m_querySize = m_N * K_256 + K_256/8; // K/8 to send p with the query
    m_replySize = m_M * K_256/8;
    m_reply = (unsigned char*) calloc (m_replySize, sizeof(unsigned char));
    m_p = BN_new();
    Q = (uint256 *)malloc(m_N * 256 * sizeof(uint256));
    q = (uint256 *)malloc(m_N * 8 * sizeof(uint256));
    n = (uint256 *)calloc(m_M, sizeof(uint256));
    res = (BIGNUM **)malloc(m_M * sizeof(BIGNUM *));
    for (size_t i = 0; i < m_M; i++)
        res[i] = BN_new();
    query_nelems_per_thread = m_N / m_nthreads;
    reply_nelems_per_thread = m_M / m_nthreads;
    return 0;
}

int Query1Handler::processOneQuery(unsigned char* t_query) {
    m_query = t_query;
    BN_bin2bn(&m_query[m_N*K_256], K_256/8, m_p);   // setting p
    thread tid[MAXTHREADS];
    unsigned int v, myid[MAXTHREADS];
    for (v = 0; v < m_nthreads; v++) {
        myid[v] = v;
        tid[v] = thread(&Query1Handler::expand_sub_query, this, v);
    }
    for (v = 0; v < m_nthreads; v++)
        tid[v].join();
    for (v = 0; v < m_nthreads; v++) {
        myid[v] = v;
        tid[v] = thread(&Query1Handler::process_sub_query, this, v);
    }
    for (v = 0; v < m_nthreads; v++)
        tid[v].join();
    return 0;
}

void Query1Handler::expand_sub_query(unsigned int myid) {
    size_t start = myid * query_nelems_per_thread;
    size_t end = (myid != m_nthreads-1) ? start + query_nelems_per_thread : m_N;
    size_t i, j, m, l;
    BIGNUM *r, *b;
    r = BN_new();
    b = BN_new();
    for (i = start; i < end; i++) {
        for (j = 0; j < 8; j++) {
            m = i*8+j;
            BN_bin2bn(&m_query[m * K_256/8], K_256/8, r);
            BN_copy(b, r);
            BN_mask_bits(b, 40);
            q[m].a0 = BN_get_word(b);
            BN_rshift(b, r, 40);
            BN_mask_bits(b, 40);
            q[m].a1 = BN_get_word(b);
            BN_rshift(b, r, 80);
            BN_mask_bits(b, 40);
            q[m].a2 = BN_get_word(b);
            BN_rshift(b, r, 120);
            BN_mask_bits(b, 40);
            q[m].a3 = BN_get_word(b);
        }
    }
    m = end << 3;
    for (i = start << 3; i < m; i += 8) {
        for (j = 0; j < 256; j++) {
            l = (i << 5) + j;
            memset(&Q[l], 0, sizeof(uint256));
            if (j & 0x01) {
                Q[l].a0 += q[i].a0;
                Q[l].a1 += q[i].a1;
                Q[l].a2 += q[i].a2;
                Q[l].a3 += q[i].a3;
            }
            if (j & 0x02) {
                Q[l].a0 += q[i+1].a0;
                Q[l].a1 += q[i+1].a1;
                Q[l].a2 += q[i+1].a2;
                Q[l].a3 += q[i+1].a3;
            }
            if (j & 0x04) {
                Q[l].a0 += q[i+2].a0;
                Q[l].a1 += q[i+2].a1;
                Q[l].a2 += q[i+2].a2;
                Q[l].a3 += q[i+2].a3;
            }
            if (j & 0x08) {
                Q[l].a0 += q[i+3].a0;
                Q[l].a1 += q[i+3].a1;
                Q[l].a2 += q[i+3].a2;
                Q[l].a3 += q[i+3].a3;
            }
            if (j & 0x10) {
                Q[l].a0 += q[i+4].a0;
                Q[l].a1 += q[i+4].a1;
                Q[l].a2 += q[i+4].a2;
                Q[l].a3 += q[i+4].a3;
            }
            if (j & 0x20) {
                Q[l].a0 += q[i+5].a0;
                Q[l].a1 += q[i+5].a1;
                Q[l].a2 += q[i+5].a2;
                Q[l].a3 += q[i+5].a3;
            }
            if (j & 0x40) {
                Q[l].a0 += q[i+6].a0;
                Q[l].a1 += q[i+6].a1;
                Q[l].a2 += q[i+6].a2;
                Q[l].a3 += q[i+6].a3;
            }
            if (j & 0x80) {
                Q[l].a0 += q[i+7].a0;
                Q[l].a1 += q[i+7].a1;
                Q[l].a2 += q[i+7].a2;
                Q[l].a3 += q[i+7].a3;
            }
        }
    }
}

void Query1Handler::process_sub_query(unsigned int myid) {
    size_t start = myid * reply_nelems_per_thread;
    size_t end = (myid != m_nthreads-1) ? start + reply_nelems_per_thread : m_M;
    size_t i, j, carry;
    unsigned char bn[23];
    unsigned char *nptr, *bptr;
    unsigned char *ptr1, *ptr2;
    unsigned long long m1, l1, m2, l2, m, l;
    BN_CTX *ctx = BN_CTX_new();
    unsigned long long u40 = 0xFFFFFFFFFF;

    m = start;
    l = m_M<<1;
    size_t last_even_N = (m_N%2==0) ? m_N : m_N-1;
    for (i = 0; i < last_even_N; i += 2) {
        m1 = i << 8;
        m2 = m1 + 256;
        ptr1 = (unsigned char *)&m_db_buffer[m];
        ptr2 = (unsigned char *)&m_db_buffer[m+m_M];
        for (j = start; j < end; j++) {
            l1 = m1 + *(ptr1++);
            l2 = m2 + *(ptr2++);
            n[j].a0 += Q[l1].a0;
            n[j].a1 += Q[l1].a1;
            n[j].a2 += Q[l1].a2;
            n[j].a3 += Q[l1].a3;
            n[j].a0 += Q[l2].a0;
            n[j].a1 += Q[l2].a1;
            n[j].a2 += Q[l2].a2;
            n[j].a3 += Q[l2].a3;
        }
        m += l;
    }
    if (m_N%2) {
        m1 = i << 8;
        m2 = m1 + 256;
        ptr1 = (unsigned char *)&m_db_buffer[m];
        for (j = start; j < end; j++) {
            l1 = m1 + *(ptr1++);
            n[j].a0 += Q[l1].a0;
            n[j].a1 += Q[l1].a1;
            n[j].a2 += Q[l1].a2;
            n[j].a3 += Q[l1].a3;
        }
        m += l;
    }

    // merge ints and reduce mod p
    for (i = start; i < end; i++) {
        carry = n[i].a0 >> 40;
        n[i].a1 += carry;
        n[i].a0 &= u40;
        carry = n[i].a1 >> 40;
        n[i].a2 += carry;
        n[i].a1 &= u40;
        carry = n[i].a2 >> 40;
        n[i].a3 += carry;
        n[i].a2 &= u40;

        bptr = bn;
        nptr = ((unsigned char *) &n[i].a3) + 7;
        for (j = 0; j < 8; j++) {
            memcpy(bptr, nptr, 1);
            nptr--;
            bptr++;
        }
        nptr = ((unsigned char *) &n[i].a2) + 4;
        for (j = 0; j < 5; j++) {
            memcpy(bptr, nptr, 1);
            nptr--;
            bptr++;
        }
        nptr = ((unsigned char *) &n[i].a1) + 4;
        for (j = 0; j < 5; j++) {
            memcpy(bptr, nptr, 1);
            nptr--;
            bptr++;
        }
        nptr = ((unsigned char *) &n[i].a0) + 4;
        for (j = 0; j < 5; j++) {
            memcpy(bptr, nptr, 1);
            nptr--;
            bptr++;
        }
        BN_bin2bn(bn, 23, res[i]);
        BN_mod(res[i], res[i], m_p, ctx);
        bn2binpad(res[i], &m_reply[i*K_256/8], K_256/8);
    }
}

unsigned char* Query1Handler::getReply() { return m_reply; }
size_t Query1Handler::getReplySize() { return m_replySize; }
size_t Query1Handler::getQuerySize() { return m_querySize; }

Query1Handler::Query1Handler(const Query1Handler& orig) {}
Query1Handler::~Query1Handler() {}
