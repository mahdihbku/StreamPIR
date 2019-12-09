/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   RandomDBgenerator.cpp
 * Author: mahdi
 * 
 * Created on January 6, 2018, 2:50 PM
 */

#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <cmath>
#include <random> 
#include <cstring>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>

#include "RandomDBgenerator.hpp"
#include "lib.hpp"
#include "ProgressBar.hpp"

using namespace std;

RandomDBgenerator::RandomDBgenerator() {}

RandomDBgenerator::RandomDBgenerator(const RandomDBgenerator& orig) {}

RandomDBgenerator::~RandomDBgenerator() {}

int RandomDBgenerator::setParameters(string t_directory, size_t t_numberOfFiles, size_t t_fileSize, int t_verbose) {
    m_directory = t_directory;
    m_numberOfFiles = t_numberOfFiles;
    m_fileSize = t_fileSize;
    m_verbose = t_verbose;
}

int RandomDBgenerator::generateRandomFile(string t_fileName, size_t t_size) {
    mt19937_64 prng(1);
    uniform_int_distribution<uint64_t> udist(0,256);
    uint8_t rd=0;
    ofstream FileStream(t_fileName.c_str(), ofstream::out);
        string errorMsg = "Unable to open the file: " + t_fileName;
        if (!FileStream.is_open()) error(errorMsg.c_str());
        for (int j=0; j<t_size;j++) {
            rd = udist(prng)% 256;
            char xdbDataByte = (char)rd;
            FileStream.write(&xdbDataByte,1);
        }
        FileStream.close();
}

int RandomDBgenerator::generate() {
    int n = 100;
    cout << "Generating Random files:" << endl;
    ProgressBar *bar1 = new ProgressBar(n, "");
    bar1->Progressed(0);
    mt19937_64 prng(1);
    uniform_int_distribution<uint64_t> udist(0,256);
    int status = mkdir(m_directory.c_str(), 0777);
    string errorMsg = "Unable to create directory: " + m_directory;
    if (!status) error(errorMsg.c_str());
    uint64_t rd=0;
    for (size_t i=0; i<m_numberOfFiles; i++) {
        ostringstream oss;
        oss << m_directory << "file" << i << ".data";
        string fileName = oss.str();
        ofstream FileStream(fileName.c_str());
        string errorMsg = "Unable to open the file: " + fileName;
        if (!FileStream.is_open()) error(errorMsg.c_str());
        size_t j=0;
        for (; j+8<m_fileSize;j+=8) {
            rd = udist(prng);
            FileStream.write((char*)&rd,8);
        }
        for (; j<m_fileSize;j++) {
            rd = udist(prng);
            FileStream.write((char*)&rd,1);
        }
        FileStream.close();
        bar1->Progressed(i*100/m_numberOfFiles);
    }
    bar1->Progressed(100);
    cout << endl;
}

int RandomDBgenerator::fastGenerate() {
    //Using Marsaglia's xorshift generator
    uint64_t t=0, x=123456789, y=362436069, z=521288629;
    int n = 100;
    cout << "Generating Random files - fast:" << endl;
    ProgressBar *bar1 = new ProgressBar(n, "");
    bar1->Progressed(0);
    int status = mkdir(m_directory.c_str(), 0777);
    string errorMsg = "Unable to create directory: " + m_directory;
    if (!status) error(errorMsg.c_str());
    for (int i=0; i<m_numberOfFiles; i++) {
        ostringstream oss;
        oss << m_directory << "file" << i << ".data";
        string fileName = oss.str();
        ofstream FileStream(fileName.c_str());
        string errorMsg = "Unable to open the file: " + fileName;
        if (!FileStream.is_open()) error(errorMsg.c_str());
        size_t j=0;
        for (; j+8<m_fileSize;j+=8) {
            x ^= x << 16;
            x ^= x >> 5;
            x ^= x << 1;
            t = x;
            x = y;
            y = z;
            z = t ^ x ^ y;
            FileStream.write((char*)&z,8);
        }
        for (; j<m_fileSize;j++) {
            x ^= x << 16;
            x ^= x >> 5;
            x ^= x << 1;
            t = x;
            x = y;
            y = z;
            z = t ^ x ^ y;
            FileStream.write((char*)&z,1);
        }
        FileStream.close();
        bar1->Progressed(i*100/m_numberOfFiles);
    }
    bar1->Progressed(100);
    cout << endl;
}
