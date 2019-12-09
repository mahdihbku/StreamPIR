/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   HybridPIRQuery.hpp
 * Author: mahdi
 *
 * Created on November 3, 2019, 4:07 PM
 */

#ifndef HYBRIDPIRQUERY_HPP
#define HYBRIDPIRQUERY_HPP

#include <openssl/bn.h>
#include <openssl/rand.h>
#include <gmp.h>

#include "Query.hpp"
#include "DBDescriptor.hpp"
#include "PrimePIR256Query.hpp"
#include "RsaPIRQuery.hpp"

//#define K 160		// PrimePIR modulus size (bits)
//#define Z 128		// PrimePIR size of random values
//#define L 257		// RsaPIR modulus size (bytes)
//#define B 128		// RsaPIR block size (bytes)
//typedef struct {
//	unsigned long long a0;
//	unsigned long long a1;
//	unsigned long long a2;
//	unsigned long long a3;
//} uint256;


class HybridPIRQuery : public Query {
public:
    HybridPIRQuery();
    HybridPIRQuery(DBDescriptor* t_DB, int t_nthreads, int t_verbose);
    int generateQuery(int t_requestedFile);
    int decodeReply(unsigned char* t_reply);
    HybridPIRQuery(const HybridPIRQuery& orig);
    virtual ~HybridPIRQuery();
    
private:
    PrimePIR256Query* m_primeQuery;
    RsaPIRQuery* m_rsaQuery;
    void create_sub_query(int myid);
    void decode_sub_reply(int myid);
};

#endif /* HYBRIDPIRQUERY_HPP */

