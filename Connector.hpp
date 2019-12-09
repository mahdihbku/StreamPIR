/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Connector.hpp
 * Author: mahdi
 *
 * Created on August 24, 2017, 12:11 AM
 */

#ifndef CONNECTOR_HPP
#define CONNECTOR_HPP

#include <string>

#include "DBManager.hpp"

class Connector {
    static const int m_serverPort = 12343;
    static const int MAX_CLIENTS = 10;
    static const int MAX_PARALLEL_CLIENTS = 5;
    static const int INFO_BUFFER_SIZE = 4096;
    static const int DATA_BUFFER_SIZE = 16384;
    static const std::string DBINFO_REQUEST;
    static const std::string QUERY_REQUEST;
    static const std::string CLOSE_CONNECTION;
public:
    Connector();
    Connector(const Connector& orig);
    int openConnection(DBManager& t_DB);
    virtual ~Connector();
private:
    int serveNewClient(int t_clientSocket, DBManager& t_DB);
    int sendDBFileInfoToClient(int t_clientSocket, DBManager& t_DB);
};

#endif /* CONNECTOR_HPP */

