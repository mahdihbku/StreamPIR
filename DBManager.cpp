/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   DBManager.cpp
 * Author: mahdi
 * 
 * Created on August 28, 2019, 2:41 PM
 */

#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <experimental/filesystem>
#include <assert.h>
#include <emmintrin.h>
#include <iomanip>
#include <bitset>
#include <sys/mman.h>
#include <sys/stat.h>
#include <chrono>
#include <math.h>

#include "DBManager.hpp"
#include "lib.hpp"
#include "ProgressBar.hpp"
#include "RandomDBgenerator.hpp"

using namespace std;

DBManager::DBManager(const DBManager& orig) {}

DBManager::DBManager(const int t_verbose, const int t_oneFileDB, const int t_pirVersion) : 
        m_DBdirectory(""), m_N(0), m_M(0), m_verbose(t_verbose), m_oneFileDB(t_oneFileDB), m_pirVersion(t_pirVersion) {}

int DBManager::processDBDirectory(const string t_directory) {
    if (m_verbose) cout << "DBManager::processDBDirectory: Processing directory " << m_DBdirectory << "..." << endl;
    m_DBdirectory   = t_directory;
    size_t numberOfFiles=0, fileSize=0, maxSize=0;
    DIR *directory;
    struct dirent *dir;
    directory = opendir(m_DBdirectory.c_str());
    string errorMsg = "Unable to open the directory: " + m_DBdirectory;
    if (!directory) error(errorMsg.c_str());
    while ((dir = readdir(directory)) != nullptr)
    {
        if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0 && 
            strcmp((m_DBdirectory+dir->d_name).c_str(), (m_DBdirectory+dummyFileName).c_str()) != 0 &&
            dir->d_type != DT_DIR)
        {
            //m_dbFilesNamesList.push_back(m_DBdirectory+dir->d_name);
            m_dbFilesNamesList.push_back(dir->d_name);
            fileSize = getFileSize(m_DBdirectory+dir->d_name);
            m_dbFilesSizesList.push_back(fileSize);
            numberOfFiles++;
            if (fileSize > maxSize) maxSize = fileSize;
        }
    }
    m_M = maxSize;
    if (m_pirVersion == 3 || m_pirVersion == 2 || m_pirVersion == 5) {
        m_fileBlocksCount = (m_M%B == 0) ? m_M/B : m_M/B+1;
        m_M = m_fileBlocksCount*B;
    }   // That makes M always multiple of B

    m_N = numberOfFiles;
    if (m_pirVersion == 3 || m_pirVersion == 5) {    // hybridPIR
        size_t sqrt_N = sqrt(m_N);
        m_N = (m_N%sqrt_N == 0) ? sqrt_N : m_N/sqrt_N+1;
        m_N2 = sqrt_N;
    } else
        m_N2 = 0;
    if (m_verbose) cout << "DBManager::processDBDirectory: Real number of files= " << numberOfFiles << endl;
    if (m_verbose) cout << "DBManager::processDBDirectory: M= " << m_M << endl;
    if (m_verbose) cout << "DBManager::processDBDirectory: N= " << m_N << endl;
    if (m_verbose) cout << "DBManager::processDBDirectory: N2= " << m_N2 << endl;
    return 0;
    // should call loadEntireDBtoMemory() right after that
}

int DBManager::processDBFile(const string t_DBfile, const size_t t_M, const size_t t_N) {
    if (m_verbose) cout << "DBManager::processDBFile: Processing db file " << m_DBfile << "..." << endl;
    m_DBfile = t_DBfile;
    m_M = t_M;
    if (m_pirVersion == 3 || m_pirVersion == 2 || m_pirVersion == 5) {
        m_fileBlocksCount = (m_M%B == 0) ? m_M/B : m_M/B+1;
        m_M = m_fileBlocksCount*B;
    }   // That makes M always multiple of B
    m_N = t_N;
    size_t filesCount;
    if (m_pirVersion == 3 || m_pirVersion == 5) {    // hybridPIR
        size_t sqrt_N = sqrt(m_N);
        m_N = (m_N%sqrt_N == 0) ? sqrt_N : m_N/sqrt_N+1;
        m_N2 = sqrt_N;
        filesCount = m_N*m_N2;
    } else {
        m_N2 = 0;
        filesCount = m_N;
    }
    for (size_t i=0; i<filesCount; i++) {
        m_dbFilesNamesList.push_back("randfile\0");
        m_dbFilesSizesList.push_back(m_M);
    }
    if (m_verbose) cout << "DBManager::processDBFile: M= " << m_M << endl;
    if (m_verbose) cout << "DBManager::processDBFile: N= " << m_N << endl;
    if (m_verbose) cout << "DBManager::processDBFile: N2= " << m_N2 << endl;
    return 0;
    // should call loadEntireDBtoMemory() right after that
}

int DBManager::loadEntireDBtoMemory() {
    m_bufferSize = (m_pirVersion == 1 || m_pirVersion == 2 || m_pirVersion == 4) ? m_N*m_M : m_N*m_N2*m_M;
    size_t filesCount = (m_pirVersion == 1 || m_pirVersion == 2 || m_pirVersion == 4) ? m_N : m_N*m_N2;
    m_buffer = (unsigned char *) malloc (m_bufferSize * sizeof (unsigned char));
    if (!m_oneFileDB) {
        size_t k=0, l=0;
        for (; k<m_dbFilesNamesList.size(); k++) {
            ifstream fileStream(m_DBdirectory+m_dbFilesNamesList[k], ios::binary);
            if (!fileStream.is_open()) {
                string errorMsg = "DBManager::loadEntireDBtoMemory: Unable to open the file: " + m_dbFilesNamesList[k];
                error(errorMsg.c_str());
            }
            //Reading from one file
            fileStream.read((char *)m_buffer+k*m_M, m_dbFilesSizesList[k]);
            for (l=m_dbFilesSizesList[k]; l<m_M; l++)      //adding padding
                m_buffer[k*m_M+l]='\0';
            fileStream.close();
        }
        for (; k<filesCount; k++) {
            for (l=0; l<m_M; l++)      //adding padding
                m_buffer[k*m_M+l]='\0';
        }
    } else {
        size_t fileSize = getFileSize(m_DBfile);
        ifstream fileStream(m_DBfile, ios::binary);
        if (fileSize < m_bufferSize) {
            fileStream.read((char *)m_buffer, fileSize);
            for (size_t i=fileSize; i<m_bufferSize; i++)
                m_buffer[i] = '\0';
        }
        else
            fileStream.read((char*)m_buffer, m_bufferSize);
        fileStream.close();
    }
}

int DBManager::loadEntireDBtoMemory_mpz() { // RSAPIR
    m_fileBlocksCount = m_M / B;
    m_dbBlocksCount = m_fileBlocksCount * m_N;
    m_mpz_buffer = (mpz_t *)malloc(m_dbBlocksCount * sizeof(mpz_t));    // data will be written in hex not 
    unsigned char *temp_buff_bin = (unsigned char*)malloc(B * sizeof(unsigned char));
    for (size_t i = 0; i < m_dbBlocksCount; i++)
        mpz_init(m_mpz_buffer[i]);
    if (!m_oneFileDB) {
        size_t k=0, l=0;
        for (; k<m_dbFilesNamesList.size(); k++) {
            ifstream fileStream(m_DBdirectory+m_dbFilesNamesList[k], ios::binary);
            if (!fileStream.is_open()) {
                string errorMsg = "DBManager::loadEntireDBtoMemory_mpz: Unable to open the file: " + m_dbFilesNamesList[k];
                error(errorMsg.c_str());
            }
            //Reading from one file
            size_t bytesToRead = m_dbFilesSizesList[k];
            for (l=0; l<m_fileBlocksCount; l++) {
                bzero(temp_buff_bin, B);
                fileStream.read((char *)temp_buff_bin, min(B, bytesToRead));
                mpz_import(m_mpz_buffer[k*m_fileBlocksCount+l], B, 1, 1, 1, 0, temp_buff_bin);
                bytesToRead -= B;
                if (bytesToRead < 0) bytesToRead = 0;
            }
            fileStream.close();
        }
        for (; k<m_N; k++) {
            for (l=0; l<m_fileBlocksCount; l++) {      //adding padding
                mpz_t r;
                mpz_init(r);
                mpz_set_ui(r, 0);
                mpz_set(m_mpz_buffer[k*m_fileBlocksCount+l], r);
            }
        }
    } else {
        size_t fileSize = getFileSize(m_DBfile);
        ifstream fileStream(m_DBfile, ios::binary);
        size_t parser=0;
        size_t bytesToRead = fileSize;
        while (bytesToRead > 0 && parser<m_dbBlocksCount) {
            bzero(temp_buff_bin, B);
            fileStream.read((char *)temp_buff_bin, B);
            mpz_import(m_mpz_buffer[parser], B, 1, 1, 1, 0, temp_buff_bin);
            parser++;
            bytesToRead -= B;
        }
        for(; parser<m_dbBlocksCount; parser++) {
            mpz_t r;
            mpz_init(r);
            mpz_set_ui(r, 0);
            mpz_set(m_mpz_buffer[parser], r);
        }
        fileStream.close();
    }
    return 0;
}

string DBManager::getDBDirectory() { return m_DBdirectory; }
size_t DBManager::getN() { return m_N; }
size_t DBManager::getN2() { return m_N2; }
size_t DBManager::getM(){ return m_M; }
int DBManager::getPirVersion() { return m_pirVersion; }
unsigned char* DBManager::getDBbuffer() { return m_buffer; }
size_t DBManager::getDBbufferSize() { return m_bufferSize; }
mpz_t* DBManager::getDB_mpz_buffer() { return m_mpz_buffer; }
size_t DBManager::getDBBlocksCount() { return m_dbBlocksCount; }
size_t DBManager::getFileBlocksCount() { return m_fileBlocksCount; }
vector<string>* DBManager::getDBfilesNamesList() { return &m_dbFilesNamesList; }
vector<size_t>* DBManager::getDBfilesSizesList() { return &m_dbFilesSizesList; }
string DBManager::getDBFile() { return m_DBfile; }
int DBManager::getDBinOneFile() { return m_oneFileDB; }

DBManager::~DBManager() {}
