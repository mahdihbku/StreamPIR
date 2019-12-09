/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   DBManager.hpp
 * Author: mahdi
 *
 * Created on August 28, 2019, 2:41 PM
 */

#ifndef DBMANAGER_HPP
#define DBMANAGER_HPP

#include <vector>
#include <string>
#include <gmp.h>

class DBManager {
public:
    DBManager(const int t_verbose, const int t_oneFileDB, const int t_pirVersion);
    int processDBDirectory(const std::string t_directory);
    int processDBFile(const std::string t_DBfile, const size_t t_M, const size_t t_N);
    int loadEntireDBtoMemory();
    int loadEntireDBtoMemory_mpz();
    DBManager(const DBManager& orig);
    virtual ~DBManager();
    
    //Class setters and getters
    size_t getN();
    size_t getN2();
    size_t getM();
    int getPirVersion();
    int getDBinOneFile();
    unsigned char *getDBbuffer();
    size_t getDBbufferSize();
    mpz_t *getDB_mpz_buffer();
    size_t getDBBlocksCount();
    size_t getFileBlocksCount();
    std::vector<std::string>* getDBfilesNamesList();
    std::vector<std::size_t>* getDBfilesSizesList();
    std::string getDBDirectory();
    std::string getDBFile();
private:
    //members
    int B = 128;		// block size (bytes)
    size_t m_N, m_N2, m_M;      // N:file size, M:number of files, N2:for hybridPIR (dimension=2)
    int m_oneFileDB = 0;        // boolean0:db from multiple files, 1:db in a single file
    int m_pirVersion = 1;       // PIR version: 1:multiPIR, 2:rsaPRI, 3:hybridPIR
    unsigned char *m_buffer;             // buffer is one block of (chunkLength X m/8)
    size_t m_bufferSize;        // db buffer (db chunk) size in number of bytes
    size_t m_fileBlocksCount = 0;   // number of blocks per file
    mpz_t *m_mpz_buffer;        // buffer is one block 
    size_t m_dbBlocksCount = 0; // number of blocks per file
    std::string m_DBdirectory;
    std::string m_DBfile;
    std::vector<std::string> m_dbFilesNamesList;
    std::vector<size_t> m_dbFilesSizesList;
    int m_verbose = 0;
    std::string dummyFileName = "dummyFile.data";
    
};

#endif /* DBMANAGER_HPP */

