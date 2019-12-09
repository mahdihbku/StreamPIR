/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   DBDescriptor.cpp
 * Author: mahdi
 * 
 * Created on August 29, 2019, 11:12 AM
 */

#include <cmath>
#include <random> 
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <netdb.h>

#include "DBDescriptor.hpp"
#include "lib.hpp"
#include "sha512.hpp"

using namespace std;

DBDescriptor::DBDescriptor() {}

DBDescriptor::DBDescriptor(const size_t t_N, const size_t t_N2, const size_t t_M, const int t_verbose, const int t_pirVersion, const string t_serverIP, const int t_serverPort)
    : m_serverIP(t_serverIP), m_serverPort(t_serverPort), m_N(t_N), m_N2(t_N2), m_M(t_M), m_pirVersion(t_pirVersion), m_verbose(t_verbose) {
    m_fileBlocksCount = m_M / B;
    if (m_M % B != 0) m_fileBlocksCount++;
    m_dbBlocksCount = m_fileBlocksCount * m_N;
}

int DBDescriptor::loadFilesNamesFromBuffer(unsigned char *buff) {
    unsigned char* parser = buff;
    size_t filesCount = (m_pirVersion == 3) ? m_N2*m_N : m_N;
    for (size_t i=0; i<filesCount; i++) {
        string fileName((char *)parser, CATALOG_NAME_SIZE);
        m_dbFilesNamesList.push_back(fileName);
        size_t fileSize = *((size_t*)(parser+CATALOG_NAME_SIZE));
        m_dbFilesSizesList.push_back(fileSize);
        parser += CATALOG_NAME_SIZE+sizeof(size_t);
    }
    return 0;
}

int DBDescriptor::displayDBcatalog() {  // TODO for HybridPIR it's not like that... (N*N2)
    for (size_t i=0; i<m_dbFilesNamesList.size(); i++)
        cout << i << "." << m_dbFilesNamesList.at(i) << " (" << m_dbFilesSizesList.at(i) << ')' << endl;
}

size_t DBDescriptor::getM() { return m_M; }
size_t DBDescriptor::getN2() { return m_N2; }
size_t DBDescriptor::getN() { return m_N; }
vector<string>* DBDescriptor::getFilesNames() { return &m_dbFilesNamesList; }
vector<size_t>* DBDescriptor::getFilesSizes() { return &m_dbFilesSizesList; }
int DBDescriptor::getPirVersion() { return m_pirVersion; }
std::string DBDescriptor::getServerIP() { return m_serverIP; }
int DBDescriptor::getServerPort() { return m_serverPort; }
size_t DBDescriptor::getDBBlocksCount() { return m_dbBlocksCount; }
size_t DBDescriptor::getFileBlocksCount() { return m_fileBlocksCount; }

DBDescriptor::DBDescriptor(const DBDescriptor& orig) {}
DBDescriptor::~DBDescriptor() {}

