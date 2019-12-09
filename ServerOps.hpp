/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ServerOps.hpp
 * Author: mahdi
 *
 * Created on January 7, 2018, 10:22 AM
 */

#ifndef SERVEROPS_HPP
#define SERVEROPS_HPP

#include <string>
#include <vector>

#include "DBManager.hpp"
#include "QueryHandler.hpp"

class ServerOps {
    static const int INFO_BUFFER_SIZE = 1024;
    static const int DATA_BUFFER_SIZE = 16384;
    static const int CATALOG_NAME_SIZE = 20;    // max file name is 20 chars
    static const std::string DBINFO_REQUEST;
    static const std::string CATALOG_REQUEST;
    static const std::string QUERY_REQUEST;
    static const std::string CLOSE_CONNECTION;

public:
    ServerOps();
    ServerOps(DBManager* t_DB, const int t_serverPort, const int t_nthreads, const int t_verbose);
    int openConnection();
    ServerOps(const ServerOps& orig);
    int setServerPort(const int t_serverPort);
    virtual ~ServerOps();
    
private:
    int m_serverPort = 12345;
    int MAX_CLIENTS = 1000;
    DBManager* m_DB;
    int m_verbose = 0;
    int m_nthreads = 0;
    
    int serveNewClient(int t_clientSocket);
    int waitForClients();
    int serveQueuedClients();
    int sendQueriesToClients();
    int getNumberOfCatalogChunks();
    int sendDBInfoToClient(int t_clientSocket);
    int sendDBCatalogToClient(int t_clientSocket);
    int serveQuery(int t_clientSocket, QueryHandler* queryHandler);
};

#endif /* SERVEROPS_HPP */

