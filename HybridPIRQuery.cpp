/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   HybridPIRQuery.cpp
 * Author: mahdi
 * 
 * Created on November 3, 2019, 4:07 PM
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

#include "HybridPIRQuery.hpp"
#include "lib.hpp"

using namespace std;

#define HEX(x) setw(2) << setfill('0') << hex << (int)(x)

HybridPIRQuery::HybridPIRQuery() {}

HybridPIRQuery::HybridPIRQuery(DBDescriptor* t_DB, int t_nthreads, int t_verbose) {
    m_DB = t_DB;
    m_nthreads = t_nthreads;
    m_verbose = t_verbose;
    m_primeQuery = new PrimePIR256Query(m_DB, m_nthreads, m_verbose);
    m_rsaQuery = new RsaPIRQuery(m_DB, m_nthreads, m_verbose);
    m_querySize = m_primeQuery->getQuerySize() + m_rsaQuery->getQuerySize();
    m_query = (unsigned char *) malloc (m_querySize * sizeof(unsigned char));
    m_replySize = m_rsaQuery->getReplySize();
}

int HybridPIRQuery::generateQuery(int t_requestedFile) {
    m_fileIndex = t_requestedFile;
    int d1_index = m_fileIndex / m_DB->getN2();
    int d2_index = m_fileIndex % m_DB->getN2();
    m_primeQuery->generateQuery(d1_index);
    memcpy(m_query, m_primeQuery->getQuery(), m_primeQuery->getQuerySize());
    m_rsaQuery->generateQuery(d2_index);
    memcpy(&m_query[m_primeQuery->getQuerySize()], m_rsaQuery->getQuery(), m_rsaQuery->getQuerySize());
    return 0;
}

int HybridPIRQuery::decodeReply(unsigned char* t_reply) {
    m_rsaQuery->decodeReply(t_reply);
    m_primeQuery->decodeReply(m_rsaQuery->getData());
    m_dataSize = m_primeQuery->getDataSize();
    m_data = m_primeQuery->getData();
    return 0;
}

HybridPIRQuery::HybridPIRQuery(const HybridPIRQuery& orig) {}
HybridPIRQuery::~HybridPIRQuery() {}
