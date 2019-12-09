/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: mahdi
 *
 * Created on August 24, 2019, 12:10 AM
 */

#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <cmath>
#include <random> 
#include <cstring>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <chrono>
#include <thread>

#include "ServerOps.hpp"
#include "DBManager.hpp"
#include "RandomDBgenerator.hpp"
#include "lib.hpp"
#include "cxxopts.hpp"

using namespace std;

/*
 * 
 */

int main(int argc, char** argv) {
    string dbDir = "../../1GB/";
    int a = 1;      // aggregation
    size_t M = 1048576;    // file size
    size_t N = 1024; // number of files
    int portNumber = 12345;
    int verbose = 0;
    int pirVersion = 1;
    int DBinOneFile = 0;
    string dbFile = "";
    int nthreads = 128;
    
    try
    {
        cxxopts::Options options("hybridPIR", "Truly practical private information retrieval");
        options.add_options()
            // Mandatory:
            ("d,directory", "Working DB directory -default:../../10GB", cxxopts::value<string>())
            // or
            ("oneFileDB", "The database is saved in one file", cxxopts::value<string>())
            // Optional:
            ("N", "Number of files in DB", cxxopts::value<int>())
            ("M", "Size of one file (in bytes)", cxxopts::value<int>())
            ("a", "Aggregation parameter", cxxopts::value<int>())
            ("version", "PIR method (1:primePIR256, 2:rsaPRI, 3:hybridPIR, 4:primePIR512) -default:1", cxxopts::value<int>()) // PIR version: 1:primePIR256, 2:rsaPRI, 3:hybridPIR, 4:primePIR512
            ("generateRandom", "Generate random DB files (requires N and M)")  // requires setting  N and M
            ("port", "Server port number -default:12345", cxxopts::value<int>())
            ("t,threads", "Number of parallel to use -default:16", cxxopts::value<int>())
            // General:
            ("v,verbose", "Show additional details", cxxopts::value<int>())
            ("h,help", "Print help");
            // ("test", "Test program features...TODO to be removed");
        auto result = options.parse(argc, argv);
        if (result.count("help")) {
            cout << options.help();
            exit(0);
        }
        if (!result.count("directory") && !(result.count("oneFileDB") && result.count("M") && result.count("N")))
            error("Error, usage: ./server -d DBdirectory\n    or: ./server -oneFileDB dbfile.bin -M 1024 -N 1024");
        if (result.count("generateRandom") && !(result.count("directory") && result.count("M") && result.count("N")))
            error("Error, usage: vPIR -generateRandom -m oneFileSize -n numberOfFiles -d DBdirectory");
        
        if (result.count("directory"))
            dbDir = result["directory"].as<string>();
        if (result.count("oneFileDB")){
            DBinOneFile = 1;
            dbFile = result["oneFileDB"].as<string>();            
        }
        if (result.count("threads"))
            nthreads = result["threads"].as<int>();
        if (result.count("verbose"))
            verbose = result["verbose"].as<int>();
        if (result.count("M"))
            M = result["M"].as<int>();
        if (result.count("N"))
            N = result["N"].as<int>();
        if (result.count("a"))
            a = result["a"].as<int>();
        if (result.count("version"))
            pirVersion = result["version"].as<int>();
        if (result.count("port"))
            portNumber = result["port"].as<int>();
        if (result.count("generateRandom")) {
            RandomDBgenerator randomDB;
            randomDB.setParameters(dbDir, N, M, verbose);
            size_t dbSize = N * M;
            if (verbose) cout << "dbSize=" << dbSize << endl;
            if (dbSize < 1024*1024*1024) randomDB.generate();
            else randomDB.fastGenerate();
        }
        
        DBManager newDB(verbose, DBinOneFile, pirVersion);
        if (!DBinOneFile)
            newDB.processDBDirectory(dbDir);
        else
            newDB.processDBFile(dbFile, M, N);
        
        if (pirVersion == 1 || pirVersion == 3 || pirVersion == 4)
            newDB.loadEntireDBtoMemory();
        else if (pirVersion == 2 || pirVersion == 5)
            newDB.loadEntireDBtoMemory_mpz();
        cout << "main: The DB has been loaded successfully to memory." << endl;

        ServerOps serverOperator(&newDB, portNumber, nthreads, verbose);
        serverOperator.openConnection();
    } 
    catch (const cxxopts::OptionException& e) {
        cout << "main: error parsing options: " << e.what() << endl;
        exit(1);
    }
    return 0;
}
