/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Query.cpp
 * Author: mahdi
 * 
 * Created on November 13, 2019, 8:32 PM
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

#include "Query.hpp"
#include "lib.hpp"

using namespace std;

Query::Query() {}

int Query::writeDataToFile(std::string t_outputFile, size_t fileSize) {
    m_outputFile = t_outputFile;
    ofstream outputFileStream(m_outputFile, ofstream::out | ofstream::trunc);
    string errorMsg = "HybridPIRQuery::writeDataToFile: Unable to open the file: " + m_outputFile;
    if (!outputFileStream.is_open()) error(errorMsg.c_str());
    outputFileStream.close();
    outputFileStream.open(m_outputFile, ios::binary | ios::app);
    outputFileStream.write((char *)m_data, fileSize);
    outputFileStream.close();
    return 0;
}

unsigned char* Query::getQuery() { return m_query; }
size_t Query::getQuerySize() { return m_querySize; }
size_t Query::getReplySize() { return m_replySize; }
unsigned char *Query::getData() { return m_data; }
size_t Query::getDataSize() { return m_dataSize; }
    
Query::Query(const Query& orig) {}
Query::~Query() {}

