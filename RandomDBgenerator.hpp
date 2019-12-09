/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   RandomDBgenerator.hpp
 * Author: mahdi
 *
 * Created on January 6, 2018, 2:50 PM
 */

#ifndef RANDOMDBGENERATOR_HPP
#define RANDOMDBGENERATOR_HPP

#include <string>

class RandomDBgenerator {
public:
    RandomDBgenerator();
    RandomDBgenerator(const RandomDBgenerator& orig);
    virtual ~RandomDBgenerator();
    int setParameters(std::string t_directory, size_t t_numberOfFiles, size_t t_fileSize, int t_verbose);
    int generateRandomFile(std::string t_fileName, size_t t_size);
    int generate();
    int fastGenerate();
private:
    std::string m_directory;
    size_t m_numberOfFiles;
    size_t m_fileSize;
    int m_verbose = 0;
};

#endif /* RANDOMDBGENERATOR_HPP */

