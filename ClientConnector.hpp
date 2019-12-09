/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ClientConnector.hpp
 * Author: mahdi
 *
 * Created on August 23, 2017, 11:43 PM
 */

#ifndef CLIENTCONNECTOR_HPP
#define CLIENTCONNECTOR_HPP

#include <string>
#include <vector>

#include "DBDescriptor.hpp"

class ClientConnector {
    static const int INFO_BUFFER_SIZE = 1024;
    static const int DATA_BUFFER_SIZE = 16384;
    static const int CATALOG_NAME_SIZE = 20;    // max file name is 20 chars
    static const std::string DBINFO_REQUEST;
    static const std::string CATALOG_REQUEST;
    static const std::string QUERY_REQUEST;
    static const std::string CLOSE_CONNECTION;
    
public:
    ClientConnector(const int t_verbose);
    ClientConnector(const std::string t_ipAddr, const int t_portNumber, const int t_verbose);
    ClientConnector(const ClientConnector& orig);
    virtual ~ClientConnector();
    int setDataBase(const DBDescriptor* t_DB);
    int initiateConnection();
    int getDBInfoFromServer();
    int getCatalogFromServer();
    int getClientSocket();
    int closeConnection();
    int sendQueryToServer(unsigned char* t_query, const size_t t_querySize);
    unsigned char* getReplyFromServer(const size_t t_replySize);
    std::string getServerIP();
    int getServerPort();
    DBDescriptor* getDB();
private:
    int m_sockfd;
    DBDescriptor* m_DB;
    std::string m_serverIP;
    int m_serverPort;
    int m_verbose = 0;
};

#endif /* CLIENTCONNECTOR_HPP */

