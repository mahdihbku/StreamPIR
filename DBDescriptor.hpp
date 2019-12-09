/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   DBDescriptor.hpp
 * Author: mahdi
 *
 * Created on August 29, 2019, 11:12 AM
 */

#ifndef DBDESCRIPTOR_HPP
#define DBDESCRIPTOR_HPP

#include <string>
#include <vector>

static const int CATALOG_NAME_SIZE = 20;    // max file name is 20 chars

class DBDescriptor {
public:
    DBDescriptor();
    DBDescriptor(size_t t_N, size_t t_N2,size_t t_M, int t_verbose, int t_pirVersion, std::string t_serverIP, int t_serverPort);
    int loadFilesNamesFromBuffer(unsigned char *buff);
    int displayDBcatalog();
    DBDescriptor(const DBDescriptor& orig);
    virtual ~DBDescriptor();
    
    //getters and setters
    size_t getN();
    size_t getM();
    size_t getN2();
    std::vector<size_t> *getFilesSizes();
    std::vector<std::string> *getFilesNames();
    std::string getServerIP();
    int getServerPort();
    int getPirVersion();
    size_t getDBBlocksCount();
    size_t getFileBlocksCount();
private:
    int B = 128;		// block size (bytes)
    std::string m_serverIP;
    int m_serverPort;
    size_t m_N, m_N2, m_M;      // N2 is used for hybridPIR
    std::vector<std::string> m_dbFilesNamesList;
    std::vector<size_t> m_dbFilesSizesList;
    size_t m_fileBlocksCount = 0;   // number of blocks per file
    size_t m_dbBlocksCount = 0;   // number of blocks per file
    int m_verbose = 0;
    int m_pirVersion = 0;
};

#endif /* DBDESCRIPTOR_HPP */

