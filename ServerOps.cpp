/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ServerOps.cpp
 * Author: mahdi
 * 
 * Created on January 7, 2018, 10:22 AM
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
#include <thread>
#include <vector>
#include <sys/mman.h>
#include <sys/stat.h>
#include <thread>
#include <chrono>
#include <libgen.h>
#include <netinet/tcp.h>

// TODO check later on which libs are necessary for bzero, memcpy, write and read
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <cstring>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <math.h> 
#include <netinet/tcp.h>
#include <vector>
#include <random>
#include <algorithm>

#include "ServerOps.hpp"
#include "DBManager.hpp"
#include "lib.hpp"
#include "Query1Handler.hpp"
#include "Query2Handler.hpp"
#include "Query3Handler.hpp"
#include "Query4Handler.hpp"

#define HEX(x) setw(2) << setfill('0') << hex << (int)(x)   // TODO to remove if not needed

using namespace std;

const string ServerOps::DBINFO_REQUEST = "DBInfo request";
const string ServerOps::CATALOG_REQUEST = "Catalog request";
const string ServerOps::QUERY_REQUEST = "Query request";
const string ServerOps::CLOSE_CONNECTION = "Close Connection";

ServerOps::ServerOps() {}

ServerOps::ServerOps(const ServerOps& orig) {}

ServerOps::ServerOps(DBManager* t_DB, const int t_serverPort, const int t_nthreads, const int t_verbose) : 
    m_DB(t_DB), m_serverPort(t_serverPort), m_nthreads(t_nthreads), m_verbose(t_verbose) {}

int ServerOps::openConnection() {
    int sockfd, newsockfd, pid;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    
    thread clientThread[MAX_CLIENTS];
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");
    
    int yes = 1;
    setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (void*) &yes, sizeof(yes));
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(m_serverPort);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR on binding");
    listen(sockfd, MAX_CLIENTS);
    clilen = sizeof(cli_addr);
    
    cout << "ServerOps::ServerOps: Waiting for clients..." << endl;
    
    int noClientsThread = 0;
    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) error("ServerOps::openConnection: Error on accepting socket");
        clientThread[noClientsThread] = thread(&ServerOps::serveNewClient, this, newsockfd);
        if (m_verbose > 1) cout << "ServerOps::openConnection: ->new client thread created=" << newsockfd << endl;
        noClientsThread++;
    }
    close(sockfd);
    return 0;
}

int ServerOps::serveNewClient(int t_clientSocket) {
    unsigned char *infoBuffer = (unsigned char *) malloc (INFO_BUFFER_SIZE * sizeof (unsigned char));
    while (1) {
        QueryHandler* queryHandler;
        if (m_DB->getPirVersion() == 1)
            queryHandler = (QueryHandler*) new Query1Handler(m_DB, m_nthreads, t_clientSocket, m_verbose);
        else if (m_DB->getPirVersion() == 2)
            queryHandler = (QueryHandler*) new Query2Handler(m_DB, m_nthreads, t_clientSocket, m_verbose);
        else if (m_DB->getPirVersion() == 3)
            queryHandler = (QueryHandler*) new Query3Handler(m_DB, m_nthreads, t_clientSocket, m_verbose);
        else if (m_DB->getPirVersion() == 4)
            queryHandler = (QueryHandler*) new Query4Handler(m_DB, m_nthreads, t_clientSocket, m_verbose);
//        else if (m_DB->getPirVersion() == 5)
//            queryHandler = (QueryHandler*) new Query5Handler(m_DB, m_nthreads, t_clientSocket, m_verbose);
        else 
            error("ServerOps::serveNewClient: Incorrect PIR version number");
        receiveInfo(infoBuffer, INFO_BUFFER_SIZE, t_clientSocket);
        if (m_verbose) cout << "ServerOps::serveNewClient: ->received from client (" << t_clientSocket << "): " << infoBuffer << endl;
        string reply((char *)infoBuffer);
        if (reply == DBINFO_REQUEST) {
            cout << "ServerOps::serveNewClient: <-Sending DB info to client (" << t_clientSocket << ")..." << endl;
            sendDBInfoToClient(t_clientSocket);
            if (m_verbose) cout << "ServerOps::serveNewClient: <-DB info sent to client (" << t_clientSocket << ") successfully" << endl;
        }
        else if (reply == CATALOG_REQUEST) {
            cout << "ServerOps::serveNewClient: <-Sending catalog to client (" << t_clientSocket << ")..." << endl;
            sendDBCatalogToClient(t_clientSocket);
            if (m_verbose) cout << "ServerOps::serveNewClient: <-Catalog sent to client (" << t_clientSocket << ") successfully" << endl;
        }
        else if (reply == QUERY_REQUEST) {
            cout << "ServerOps::serveNewClient: ->Getting query request from client (" << t_clientSocket << ")..." << endl;
            serveQuery(t_clientSocket, queryHandler);
            if (m_verbose) cout << "ServerOps::serveNewClient: <-Query of client (" << t_clientSocket << ") served successfully" << endl;
        }
        else if (reply == CLOSE_CONNECTION) {
            close(t_clientSocket);
            cout << "ServerOps::serveNewClient: ->Connection closed by client (" << t_clientSocket << ")." << endl;
            return 0;
        }
        else {
            cout << "ServerOps::serveNewClientERROR: ->received [" << reply << "] couldn't be treated" << endl;
            close(t_clientSocket);
            cout << "ServerOps::serveNewClient: <-Connection closed with client (" << t_clientSocket << ")." << endl;
        }
    }
    return 0;
}

int ServerOps::serveQuery(int t_clientSocket, QueryHandler* queryHandler) {
    chrono::time_point<chrono::system_clock> start, end;
    chrono::duration<double> queryProcessingTime = start-start;
    unsigned char* query = (unsigned char *) malloc (queryHandler->getQuerySize());
    receiveData(query, queryHandler->getQuerySize(), t_clientSocket);
    cout << "ServerOps::serveQuery: query received" << endl;
    start = chrono::system_clock::now();
    queryHandler->processOneQuery(query);
    end = chrono::system_clock::now();
    queryProcessingTime = end - start;
    if (m_verbose) cout << "ServerOps::serveQuery: queryProcessingTime: " <<  queryProcessingTime.count() << "s" << endl;
    cout << "ServerOps::serveQuery: query processed" << endl;
    sendData(queryHandler->getReply(), queryHandler->getReplySize(), t_clientSocket);
    cout << "ServerOps::serveQuery: reply sent" << endl;
    ofstream ofs;
    ofs.open("ServerStats.csv", ofstream::out | ofstream::app);
    ofs << m_DB->getPirVersion() << ",";
    ofs << m_DB->getM() << ",";
    ofs << m_DB->getN() << ",";
    ofs << m_DB->getN2() << ",";
    if (m_DB->getPirVersion() == 3 || m_DB->getPirVersion() == 5) // DB size in GB
        ofs << m_DB->getN2()*m_DB->getN()*m_DB->getM()/1024/1024/1024 << ",";
    else
        ofs << m_DB->getN()*m_DB->getM()/1024/1024/1024 << ",";
    ofs << queryProcessingTime.count() << endl;
    ofs.close();
    return 0;
}

int ServerOps::sendDBInfoToClient(int t_clientSocket) { // M, N, N2, PIRversion
    unsigned char *infoBuffer = (unsigned char *) malloc ((3*sizeof(size_t)+sizeof(int)) * sizeof (unsigned char));
    size_t M = m_DB->getM(), N = m_DB->getN(), N2 = m_DB->getN2(); int pirVersion = m_DB->getPirVersion();
    memcpy(infoBuffer, &M, sizeof(size_t));
    memcpy(infoBuffer+sizeof(size_t), &N, sizeof(size_t));
    memcpy(infoBuffer+2*sizeof(size_t), &N2, sizeof(size_t));
    memcpy(infoBuffer+3*sizeof(size_t), &pirVersion, sizeof(int));
    sendInfo(infoBuffer, 3*sizeof(size_t)+sizeof(int), t_clientSocket);
    return 0;
}

int ServerOps::sendDBCatalogToClient(int t_clientSocket) {  // TODO to be redone. send every thing in one DATA buffer
    size_t totalSize = (m_DB->getPirVersion() == 3 || m_DB->getPirVersion() == 5) ? 
        m_DB->getN()*m_DB->getN2()*(CATALOG_NAME_SIZE+sizeof(size_t)) : 
        m_DB->getN()*(CATALOG_NAME_SIZE+sizeof(size_t));
    unsigned char *dataBuffer = (unsigned char *) calloc (totalSize, sizeof (unsigned char));
    unsigned char *parser = dataBuffer;
    for(size_t i=0; i<m_DB->getDBfilesNamesList()->size(); i++) {    //TODO remove the boolean test. for loop(read write, read write)
        memcpy(parser, m_DB->getDBfilesNamesList()->at(i).c_str(), m_DB->getDBfilesNamesList()->at(i).length()+1);
        memcpy(parser+CATALOG_NAME_SIZE, &(m_DB->getDBfilesSizesList()->at(i)), sizeof(size_t));
        parser += CATALOG_NAME_SIZE + sizeof(size_t);
    }
    for (; parser<dataBuffer+totalSize; parser++)
        parser[0] = '\0';
    sendData(dataBuffer, totalSize, t_clientSocket);
    return 0;
}

int ServerOps::setServerPort(int t_serverPort) { m_serverPort = t_serverPort; }
    
ServerOps::~ServerOps() {}

