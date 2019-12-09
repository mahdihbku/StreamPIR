/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: mahdi
 *
 * Created on August 23, 2019, 11:41 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <cstdlib>
#include <chrono>
#include <ctime>
#include <iomanip>

#include "ClientConnector.hpp"
#include "DBDescriptor.hpp"
#include "cxxopts.hpp"
#include "Query.hpp"
#include "PrimePIR256Query.hpp"
#include "PrimePIR512Query.hpp"
#include "RsaPIRQuery.hpp"
#include "HybridPIRQuery.hpp"

// TODO to remove
#include "lib.hpp"

using namespace std;

/*
 * 
 */

int main(int argc, char** argv) {
    string ip = "127.0.0.1";
    int port = 12345;
    string outputFile = "";
    int requestedFile = -1;
    bool noCatalog = true;
    int verbose = 0;
    int nthreads = 16;
    try {
        cxxopts::Options options("vPIR", "Truly practical private information retrieval");
        options.add_options()
            ("noCatalog", "No db catalog printing")
            ("ip", "Server ip -default:127.0.0.1", cxxopts::value<string>())
            ("p,port", "Server port -default:12345", cxxopts::value<int>())
            ("outputFile", "Received file location and name -default:currentDir/remoteFileName", cxxopts::value<string>())
            ("f,file", "Requested file number", cxxopts::value<int>())
            ("v,verbose", "Show additional details", cxxopts::value<int>())
            ("t,threads", "Number of parallel to use -default:16", cxxopts::value<int>())
            ("h,help", "Print help");
        auto result = options.parse(argc, argv);
        if (result.count("help")) {
            cout << options.help();
            exit(0);
        }
        if (result.count("verbose"))
            verbose = result["verbose"].as<int>();
        if (result.count("ip"))
            ip = result["ip"].as<string>();
        if (result.count("port"))
            port = result["port"].as<int>();
        if (result.count("outputFile"))
            outputFile = result["outputFile"].as<string>();
        if (result.count("file"))
            requestedFile = result["file"].as<int>();
        if (result.count("threads"))
            nthreads = result["threads"].as<int>();
    
        ClientConnector cn(ip, port, verbose);
        DBDescriptor* db;
        cout << "main: Initiating Connection..." << endl;
        cn.initiateConnection();
        cout << "main: Connected to the server: " << cn.getServerIP() << ":" << cn.getServerPort() << endl;
        cout << "main: Getting db info from the main server..." << endl;
        cn.getDBInfoFromServer();
        db = cn.getDB();

        if (!result.count("noCatalog")) {
            noCatalog = false;
            cn.getCatalogFromServer();
            cout << "main: Files on server:" << endl;
            db->displayDBcatalog();
        }

        if (requestedFile == -1) {
            cout << " file to request:";
            cin >> requestedFile;
        }

        size_t max_index = (db->getPirVersion() != 3 || db->getPirVersion() == 5) ? db->getN() : db->getN()*db->getN2();
        if(requestedFile >= max_index) error("Error: no file with such index");
        
        chrono::time_point<chrono::system_clock> start, end, t1, t2;
        chrono::duration<double> queryInitializationTime = start-start;
        chrono::duration<double> queryGenerationTime = start-start;
        chrono::duration<double> communicationTime = start-start;
        chrono::duration<double> replyDecodingTime = start-start;
        chrono::duration<double> RTTTime = start-start;
        
        start = chrono::system_clock::now();
        
        t1 = chrono::system_clock::now();
        Query* query;
        if(db->getPirVersion() == 1)    // PrimePIR256
            query = (Query*) new PrimePIR256Query(db, nthreads, verbose);
        else if (db->getPirVersion() == 2)  // RSAPIR256
            query = (Query*) new RsaPIRQuery(db, nthreads, verbose);
        else if (db->getPirVersion() == 3)  // HybridPIR
            query = (Query*) new HybridPIRQuery(db, nthreads, verbose);
        else if (db->getPirVersion() == 4)  // PrimePIR512
            query = (Query*) new PrimePIR512Query(db, nthreads, verbose);
//        else if (db->getPirVersion() == 5)
//            query = (Query*) new HybridPIR512Query(db, nthreads, verbose);
        t2 = chrono::system_clock::now();
        queryInitializationTime = t2-t1;
        
        t1 = chrono::system_clock::now();
        cout << "main: query initialized" << endl;
        query->generateQuery(requestedFile);
        cout << "main: query generated" << endl;
        t2 = chrono::system_clock::now();
        queryGenerationTime = t2-t1;
        
        t1 = chrono::system_clock::now();
        cn.sendQueryToServer(query->getQuery(), query->getQuerySize());
        cout << "main: query sent" << endl;
        unsigned char *reply = cn.getReplyFromServer(query->getReplySize());
        cout << "main: reply received" << endl;
        t2 = chrono::system_clock::now();
        communicationTime = t2-t1;
        
        t1 = chrono::system_clock::now();
        query->decodeReply(reply);
        cout << "main: reply decoded" << endl;
        t2 = chrono::system_clock::now();
        replyDecodingTime = t2-t1;
        
        size_t fileSize;
        if (noCatalog) {
            if (outputFile == "") outputFile = "receivedfile.data";
            fileSize = db->getM();
        } else {
            if (outputFile == "") outputFile = db->getFilesNames()->at(requestedFile);
            fileSize = db->getFilesSizes()->at(requestedFile);
        }
        query->writeDataToFile(outputFile, fileSize);
        cout << "main: reply saved to: " << outputFile << " (" << fileSize << "B)" << endl;
        
        cn.closeConnection();
        cout << "main: Connection closed" << endl;

        end = chrono::system_clock::now();
        RTTTime = end - start;
            
        ofstream ofs;
        ofs.open ("ClientStats.csv", ofstream::out | ofstream::app);
        ofs << db->getPirVersion() << ",";
        ofs << db->getM() << ",";
        ofs << db->getN() << ",";
        ofs << db->getN2() << ",";
        if (db->getPirVersion() == 3 || db->getPirVersion() == 5) // DB size in MB
            ofs << db->getN2()*db->getN()*db->getM()/1024/1024 << ",";
        else
            ofs << db->getN()*db->getM()/1024/1024 << ",";
        ofs << query->getQuerySize() << ",";
        ofs << query->getReplySize() << ",";
        ofs << queryInitializationTime.count() << ",";
        ofs << queryGenerationTime.count() << ",";
        ofs << communicationTime.count() << ",";
        ofs << replyDecodingTime.count() << ",";
        ofs << RTTTime.count() << ",";
        ofs << outputFile << endl;
        ofs.close();

        if (verbose) cout << "main: queryInitializationTime: " << queryInitializationTime.count() << "s" << endl;
        if (verbose) cout << "main: queryGenerationTime: " << queryGenerationTime.count() << "s" << endl;
        if (verbose) cout << "main: communicationTime: " << communicationTime.count() << "s" << endl;
        if (verbose) cout << "main: replyDecodingTime: " << replyDecodingTime.count() << "s" << endl;
        if (verbose) cout << "main: RTTTime: " << RTTTime.count() << "s" << endl;
        
    }
    catch (const cxxopts::OptionException& e) {
        cout << "main: error parsing options: " << e.what() << endl;
        exit(1);
    }
    
    return 0;
}
