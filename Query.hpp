/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Query.hpp
 * Author: mahdi
 *
 * Created on November 13, 2019, 8:32 PM
 */

#ifndef QUERY_HPP
#define QUERY_HPP

#include <string>

#include "DBDescriptor.hpp"

class Query {
public:
    Query();
    virtual int generateQuery(int t_requestedFile) {}
    size_t getQuerySize();
    size_t getReplySize();
    unsigned char *getData();
    size_t getDataSize();
    int writeDataToFile(std::string t_outputFile, size_t fileSize);
    virtual int decodeReply(unsigned char* t_reply) {}
    unsigned char *getQuery();
    Query(const Query& orig);
    virtual ~Query();
    
protected:
    DBDescriptor* m_DB;
    int m_nthreads;
    int m_fileIndex;
    std::string m_outputFile;
    unsigned char *m_query;
    size_t m_querySize; // in bytes
    unsigned char *m_reply;
    size_t m_replySize; // in bytes
    unsigned char *m_data;
    size_t m_dataSize; // in bytes
    int m_verbose = 0;
};

#endif /* QUERY_HPP */

