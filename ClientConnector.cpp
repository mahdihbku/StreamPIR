/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ClientConnector.cpp
 * Author: mahdi
 * 
 * Created on August 23, 2019, 11:43 PM
 */

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

#include "ClientConnector.hpp"
#include "DBDescriptor.hpp"
#include "lib.hpp"
#include "sha512.hpp"

using namespace std;

const string ClientConnector::DBINFO_REQUEST = "DBInfo request";
const string ClientConnector::CATALOG_REQUEST = "Catalog request";
const string ClientConnector::QUERY_REQUEST = "Query request";
const string ClientConnector::CLOSE_CONNECTION = "Close Connection";

ClientConnector::ClientConnector(int t_verbose) : 
    m_serverIP("0.0.0.0"), m_serverPort(0), m_verbose(t_verbose) {}

ClientConnector::ClientConnector(const string t_ipAddr, const int t_portNumber, const int t_verbose) : 
    m_serverIP(t_ipAddr), m_serverPort(t_portNumber), m_verbose(t_verbose) {}

int ClientConnector::setDataBase(const DBDescriptor* t_DB) { m_DB = (DBDescriptor*)t_DB; }

int ClientConnector::initiateConnection() {
    struct sockaddr_in serv_addr;
    struct in_addr addr = { 0 };
    struct hostent *server;
    m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_sockfd < 0) error("ClientConnector::initiateConnection: ERROR opening socket");
    
    int yes = 1;
    //setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    setsockopt(m_sockfd, IPPROTO_TCP, TCP_NODELAY, (void*) &yes, sizeof(yes));
    
    addr.s_addr = inet_addr(m_serverIP.c_str());
    if (addr.s_addr == INADDR_NONE)
        error ("ClientConnector::initiateConnection: The IPv4 address entered must be a legal address");
    //server = gethostbyaddr((char *) &addr, 4, AF_INET);
    server = gethostbyname(m_serverIP.c_str());
    if (server == nullptr)
        error("ClientConnector::initiateConnection: Error, no such host");
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(m_serverPort);
    if (connect(m_sockfd, (struct sockaddr *) & serv_addr, sizeof(serv_addr)) < 0) 
        error("ClientConnector::initiateConnection: Error connecting");
    return 0;
}

int ClientConnector::getDBInfoFromServer() {
    sendInfo(DBINFO_REQUEST, m_sockfd);
    int dbInfoBufferSize = 3*sizeof(size_t)+sizeof(int);
    unsigned char *infoBuffer = (unsigned char *) malloc (dbInfoBufferSize * sizeof (unsigned char));
    receiveInfo(infoBuffer, dbInfoBufferSize, m_sockfd);
    size_t M = *((size_t *)infoBuffer);
    size_t N = *((size_t *)(infoBuffer+sizeof(size_t)));
    size_t N2 = *((size_t *)(infoBuffer+2*sizeof(size_t)));
    int pirVersion = *((int *)(infoBuffer+3*sizeof(size_t)));
    m_DB = new DBDescriptor(N, N2, M, m_verbose, pirVersion, m_serverIP, m_serverPort);
}

int ClientConnector::getCatalogFromServer() {
    sendInfo(CATALOG_REQUEST, m_sockfd);
    size_t totalSize = (m_DB->getPirVersion() == 3) ? m_DB->getN()*m_DB->getN2()*(CATALOG_NAME_SIZE+sizeof(size_t)) : m_DB->getN()*(CATALOG_NAME_SIZE+sizeof(size_t));
    unsigned char *dataBuffer = (unsigned char *) malloc(totalSize * sizeof (unsigned char));
    receiveData(dataBuffer, totalSize, m_sockfd);
    m_DB->loadFilesNamesFromBuffer(dataBuffer);
    return 0;
}

int ClientConnector::sendQueryToServer(unsigned char* t_query, const size_t t_querySize) {
    sendInfo(QUERY_REQUEST, m_sockfd);
    sendData(t_query, t_querySize, m_sockfd);
    return 0;
}

unsigned char* ClientConnector::getReplyFromServer(const size_t t_replySize) {
    unsigned char* reply = (unsigned char *) calloc(t_replySize, sizeof (unsigned char));
    receiveData(reply, t_replySize, m_sockfd);
    return reply;
}

int ClientConnector::closeConnection() {
    sendInfo(CLOSE_CONNECTION, m_sockfd);
    return 0;
}

int ClientConnector::getClientSocket() { return m_sockfd; }
DBDescriptor* ClientConnector::getDB() { return m_DB; }
string ClientConnector::getServerIP() { return m_serverIP; }
int ClientConnector::getServerPort() { return m_serverPort; }

ClientConnector::ClientConnector(const ClientConnector& orig) {}

ClientConnector::~ClientConnector() {}

