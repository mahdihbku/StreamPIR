/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Query4Handler.cpp
 * Author: mahdi
 * 
 * Created on November 24, 2019, 4:29 PM
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

#include "Query4Handler.hpp"
#include "lib.hpp"

using namespace std;

Query4Handler::Query4Handler() {}

Query4Handler::Query4Handler(DBManager* t_DB, const int t_nthreads, const int t_socket, const int t_verbose) :
    m_DB(t_DB), m_nthreads(t_nthreads), m_clientSocket(t_socket), m_verbose(t_verbose) {
    m_db_buffer = m_DB->getDBbuffer();
    m_N = m_DB->getN();
    m_M = m_DB->getM();
    initializeParameters();
}

Query4Handler::Query4Handler(unsigned char* t_db_buffer, size_t t_N, size_t t_M, int t_nthreads, const int t_socket, const int t_verbose) :
    m_db_buffer(t_db_buffer), m_N(t_N), m_M(t_M), m_nthreads(t_nthreads), m_clientSocket(t_socket), m_verbose(t_verbose) {
    initializeParameters();
}

int Query4Handler::initializeParameters() {
    m_querySize = m_N * K_512 + K_512/8 + 1;     // K/8 to send p with the query; last byte for query aggregation
    m_p = BN_new();
    Q = (uint512 *)malloc(m_N * 256 * sizeof(uint512));
    q = (uint512 *)malloc(m_N * 8 * sizeof(uint512));
    n = (uint512 *)calloc(m_M, sizeof(uint512));
    query_nelems_per_thread = m_N / m_nthreads;
    setNbrCharsPerCipher(default_chars_per_ciphertext);
    return 0;
}

int Query4Handler::setNbrCharsPerCipher(int t_chars_per_ciphertext) {
    chars_per_ciphertext = t_chars_per_ciphertext;
    ciphertext_count = m_M/chars_per_ciphertext;
    if (m_M % chars_per_ciphertext != 0) ciphertext_count++;
    m_replySize = ciphertext_count * K_512/8;
    m_reply = (unsigned char*) calloc (m_replySize, sizeof(unsigned char));
    res = (BIGNUM **)malloc(ciphertext_count * sizeof(BIGNUM *));
    for (size_t i = 0; i < ciphertext_count; i++)
        res[i] = BN_new();
    reply_nelems_per_thread = m_M / m_nthreads / chars_per_ciphertext * chars_per_ciphertext;   // always multiple of files_per_ciphertext
    return 0;
}

int Query4Handler::processOneQuery(unsigned char* t_query) {
    m_query = t_query;
    if (m_query[m_N * K_512 + K_512/8] == 0x1) ; // without query aggregation
    else if (m_query[m_N * K_512 + K_512/8] == 0x2) // query aggregation
        setNbrCharsPerCipher(1);
    else
        error("Query4Handler::processOneQuery: Incorrect query aggregation version");        
    BN_bin2bn(&m_query[m_N*K_512], K_512/8, m_p);   // setting p
    thread tid[MAXTHREADS];
    unsigned int v, myid[MAXTHREADS];
    for (v = 0; v < m_nthreads; v++) {
        myid[v] = v;
        tid[v] = thread(&Query4Handler::expand_sub_query, this, v);
    }
    for (v = 0; v < m_nthreads; v++)
        tid[v].join();
    for (v = 0; v < m_nthreads; v++) {
        myid[v] = v;
        tid[v] = thread(&Query4Handler::process_sub_query, this, v);
    }
    for (v = 0; v < m_nthreads; v++)
        tid[v].join();
    return 0;
}

void Query4Handler::expand_sub_query(unsigned int myid) {
    size_t start = myid * query_nelems_per_thread;
    size_t end = (myid != m_nthreads-1) ? start + query_nelems_per_thread : m_N;
    size_t i, j, m, l;
    BIGNUM *r, *b;
    r = BN_new();
    b = BN_new();
    for (i = start; i < end; i++) {
        for (j = 0; j < 8; j++) {
            m = i*8+j;
            BN_bin2bn(&m_query[m * K_512/8], K_512/8, r);
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
            BN_rshift(b, r, 160);
            BN_mask_bits(b, 40);
            q[m].a4 = BN_get_word(b);
            BN_rshift(b, r, 200);
            BN_mask_bits(b, 40);
            q[m].a5 = BN_get_word(b);
            BN_rshift(b, r, 240);
            BN_mask_bits(b, 40);
            q[m].a6 = BN_get_word(b);
            BN_rshift(b, r, 280);
            BN_mask_bits(b, 40);
            q[m].a7 = BN_get_word(b);
        }
    }
    m = end << 3;
    for (i = start << 3; i < m; i += 8) {
        for (j = 0; j < 256; j++) {
            l = (i << 5) + j;
            memset(&Q[l], 0, sizeof(uint512));
            if (j & 0x01) {
                Q[l].a0 += q[i].a0;
                Q[l].a1 += q[i].a1;
                Q[l].a2 += q[i].a2;
                Q[l].a3 += q[i].a3;
                Q[l].a4 += q[i].a4;
                Q[l].a5 += q[i].a5;
                Q[l].a6 += q[i].a6;
                Q[l].a7 += q[i].a7;
            }
            if (j & 0x02) {
                Q[l].a0 += q[i+1].a0;
                Q[l].a1 += q[i+1].a1;
                Q[l].a2 += q[i+1].a2;
                Q[l].a3 += q[i+1].a3;
                Q[l].a4 += q[i+1].a4;
                Q[l].a5 += q[i+1].a5;
                Q[l].a6 += q[i+1].a6;
                Q[l].a7 += q[i+1].a7;
            }
            if (j & 0x04) {
                Q[l].a0 += q[i+2].a0;
                Q[l].a1 += q[i+2].a1;
                Q[l].a2 += q[i+2].a2;
                Q[l].a3 += q[i+2].a3;
                Q[l].a4 += q[i+2].a4;
                Q[l].a5 += q[i+2].a5;
                Q[l].a6 += q[i+2].a6;
                Q[l].a7 += q[i+2].a7;
            }
            if (j & 0x08) {
                Q[l].a0 += q[i+3].a0;
                Q[l].a1 += q[i+3].a1;
                Q[l].a2 += q[i+3].a2;
                Q[l].a3 += q[i+3].a3;
                Q[l].a4 += q[i+3].a4;
                Q[l].a5 += q[i+3].a5;
                Q[l].a6 += q[i+3].a6;
                Q[l].a7 += q[i+3].a7;
            }
            if (j & 0x10) {
                Q[l].a0 += q[i+4].a0;
                Q[l].a1 += q[i+4].a1;
                Q[l].a2 += q[i+4].a2;
                Q[l].a3 += q[i+4].a3;
                Q[l].a4 += q[i+4].a4;
                Q[l].a5 += q[i+4].a5;
                Q[l].a6 += q[i+4].a6;
                Q[l].a7 += q[i+4].a7;
            }
            if (j & 0x20) {
                Q[l].a0 += q[i+5].a0;
                Q[l].a1 += q[i+5].a1;
                Q[l].a2 += q[i+5].a2;
                Q[l].a3 += q[i+5].a3;
                Q[l].a4 += q[i+5].a4;
                Q[l].a5 += q[i+5].a5;
                Q[l].a6 += q[i+5].a6;
                Q[l].a7 += q[i+5].a7;
            }
            if (j & 0x40) {
                Q[l].a0 += q[i+6].a0;
                Q[l].a1 += q[i+6].a1;
                Q[l].a2 += q[i+6].a2;
                Q[l].a3 += q[i+6].a3;
                Q[l].a4 += q[i+6].a4;
                Q[l].a5 += q[i+6].a5;
                Q[l].a6 += q[i+6].a6;
                Q[l].a7 += q[i+6].a7;
            }
            if (j & 0x80) {
                Q[l].a0 += q[i+7].a0;
                Q[l].a1 += q[i+7].a1;
                Q[l].a2 += q[i+7].a2;
                Q[l].a3 += q[i+7].a3;
                Q[l].a4 += q[i+7].a4;
                Q[l].a5 += q[i+7].a5;
                Q[l].a6 += q[i+7].a6;
                Q[l].a7 += q[i+7].a7;
            }
        }
    }
}

void Query4Handler::process_sub_query(unsigned int myid) {
    size_t start = myid * reply_nelems_per_thread;
    size_t end = (myid != m_nthreads-1) ? start + reply_nelems_per_thread : m_M;
    unsigned long long i, j, k, current_ct;
    unsigned char *ptr1, *ptr2;
    unsigned long long m1, l1, m2, l2, m, l;
    unsigned char bn[43];
    BIGNUM *temp_res = BN_new();
    BN_CTX *ctx = BN_CTX_new();

    m = start;
    l = m_M<<1;
    size_t last_even_N = (m_N%2==0) ? m_N : m_N-1;
    for (i = 0; i < last_even_N; i += 2) {
        m1 = i << 8;
        m2 = m1 + 256;
        ptr1 = &m_db_buffer[m];
        ptr2 = &m_db_buffer[m+m_M];
        for (j = start; j < end; j++) {
            l1 = m1 + *(ptr1++);
            l2 = m2 + *(ptr2++);

            n[j].a0 += Q[l1].a0;
            n[j].a1 += Q[l1].a1;
            n[j].a2 += Q[l1].a2;
            n[j].a3 += Q[l1].a3;
            n[j].a4 += Q[l1].a4;
            n[j].a5 += Q[l1].a5;
            n[j].a6 += Q[l1].a6;
            n[j].a7 += Q[l1].a7;

            n[j].a0 += Q[l2].a0;
            n[j].a1 += Q[l2].a1;
            n[j].a2 += Q[l2].a2;
            n[j].a3 += Q[l2].a3;
            n[j].a4 += Q[l2].a4;
            n[j].a5 += Q[l2].a5;
            n[j].a6 += Q[l2].a6;
            n[j].a7 += Q[l2].a7;
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
            n[j].a4 += Q[l1].a4;
            n[j].a5 += Q[l1].a5;
            n[j].a6 += Q[l1].a6;
            n[j].a7 += Q[l1].a7;
        }
        m += l;
    }

    // merge ints and reduce mod p
    for (i = start; i+chars_per_ciphertext <= end; i+=chars_per_ciphertext) {
        current_ct = i/chars_per_ciphertext;
        process_one_char(i, n, bn);
        BN_bin2bn(bn, 43, res[current_ct]);
        BN_lshift(res[current_ct], res[current_ct], (chars_per_ciphertext-1)*8);
        for (k = 1; k < chars_per_ciphertext; k++) {
            process_one_char(i+k, n, bn);
            BN_bin2bn(bn, 43, temp_res);
            BN_lshift(temp_res, temp_res, (chars_per_ciphertext-k-1)*8);
            BN_add(res[current_ct], res[current_ct], temp_res);
        }
        BN_mod(res[current_ct], res[current_ct], m_p, ctx);
        bn2binpad(res[current_ct], &m_reply[current_ct*K_512/8], K_512/8);
    }
    if (i < end) {
        current_ct = i/chars_per_ciphertext;
        process_one_char(i, n, bn);
        BN_bin2bn(bn, 43, res[current_ct]);
        BN_lshift(res[current_ct], res[current_ct], (end-i-1)*8);
        for (k = 1; i+k < end; k++) {
            process_one_char(i+k, n, bn);
            BN_bin2bn(bn, 43, temp_res);
            BN_lshift(temp_res, temp_res, (end-i-k-1)*8);
            BN_add(res[current_ct], res[current_ct], temp_res);
        }
        BN_mod(res[current_ct], res[current_ct], m_p, ctx);
        bn2binpad(res[current_ct], &m_reply[current_ct*K_512/8], K_512/8);
    }
}

void Query4Handler::process_one_char(unsigned long long i, uint512 *n, unsigned char *bn) {
    unsigned long long j, carry;
    unsigned char *nptr, *bptr;
    carry = n[i].a0 >> 40;
    n[i].a1 += carry;
    n[i].a0 &= u40;
    carry = n[i].a1 >> 40;
    n[i].a2 += carry;
    n[i].a1 &= u40;
    carry = n[i].a2 >> 40;
    n[i].a3 += carry;
    n[i].a2 &= u40;
    carry = n[i].a3 >> 40;
    n[i].a4 += carry;
    n[i].a3 &= u40;
    carry = n[i].a4 >> 40;
    n[i].a5 += carry;
    n[i].a4 &= u40;
    carry = n[i].a5 >> 40;
    n[i].a6 += carry;
    n[i].a5 &= u40;
    carry = n[i].a6 >> 40;
    n[i].a7 += carry;
    n[i].a6 &= u40;

    bptr = bn;
    nptr = ((unsigned char *) &n[i].a7) + 7;
    for (j = 0; j < 8; j++) {
        memcpy(bptr, nptr, 1);
        nptr--;
        bptr++;
    }
    nptr = ((unsigned char *) &n[i].a6) + 4;
    for (j = 0; j < 5; j++) {
        memcpy(bptr, nptr, 1);
        nptr--;
        bptr++;
    }
    nptr = ((unsigned char *) &n[i].a5) + 4;
    for (j = 0; j < 5; j++) {
        memcpy(bptr, nptr, 1);
        nptr--;
        bptr++;
    }
    nptr = ((unsigned char *) &n[i].a4) + 4;
    for (j = 0; j < 5; j++) {
        memcpy(bptr, nptr, 1);
        nptr--;
        bptr++;
    }
    nptr = ((unsigned char *) &n[i].a3) + 4;
    for (j = 0; j < 5; j++) {
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
}

unsigned char* Query4Handler::getReply() { return m_reply; }
size_t Query4Handler::getReplySize() { return m_replySize; }
size_t Query4Handler::getQuerySize() { return m_querySize; }

Query4Handler::Query4Handler(const Query4Handler& orig) {}

Query4Handler::~Query4Handler() {}

