/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Connector.cpp
 * Author: mahdi
 * 
 * Created on August 24, 2017, 12:11 AM
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

#include "Connector.hpp"
#include "DBManager.hpp"
#include "QueryHandler.hpp"
#include "lib.hpp"

using namespace std;

const string Connector::DBINFO_REQUEST = "DBInfo request";
const string Connector::DBINFO_ACK = "DBInfo ACK";
const string Connector::XDBFILE_REQUEST = "Sending XDB request";
const string Connector::XDBFILE_REQUEST_ACK = "Sending XDB request ACK";
const string Connector::QUERY_REQUEST = "Query request";
const string Connector::QUERY_REQUEST_ACK = "Query ACK";
const string Connector::CLOSE_CONNECTION = "Close Connection";

Connector::Connector() {
}

Connector::Connector(const Connector& orig) {
}

int Connector::openConnection(DBManager& t_DB) {
    int sockfd, newsockfd, pid;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(m_serverPort);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR on binding");
    listen(sockfd, MAX_CLIENTS);
    clilen = sizeof(cli_addr);
    
    cout << "db loaded to memory" << endl;
    cout << "Waiting for clients..." << endl;
    
    //////////////////////////////////
    vector<int> *socketsQueue;
    //////////////////////////////////
    
    
    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) error("ERROR on accept");
        //thread clientThread = thread(&Connector::serveNewClient, this, newsockfd, t_DB, dbFileBuffer);
        pid = fork();
        if (pid < 0) error("ERROR on fork");
        if (pid == 0) {
            close(sockfd);
            serveNewClient(newsockfd, t_DB);
            exit(0);
        }
        else close(newsockfd);
    }
    close(sockfd);
    
    return 0;
}

int Connector::serveNewClient(int t_clientSocket, DBManager &t_DB) {
    char *infoBuffer;
    infoBuffer = (char *) malloc (INFO_BUFFER_SIZE * sizeof (char));
    int sockErr;
    while (1) {
        bzero(infoBuffer, INFO_BUFFER_SIZE);
        sockErr = read(t_clientSocket, infoBuffer, INFO_BUFFER_SIZE);
        cout << "received from client_: " << infoBuffer << endl;
        if (sockErr < 0) error("ERROR reading from socket");
        string reply(infoBuffer);
        if (reply == DBINFO_REQUEST) {
            cout << "Sending DB info to the client..." << endl;
            sendDBFileInfoToClient(t_clientSocket, t_DB);
        } else if (reply == XDBFILE_REQUEST) {
            cout << "Sending XDB file to the client..." << endl;
            t_DB.getX()->getXDB()->sendOutputFileToClient(t_clientSocket);
        } else if (reply == QUERY_REQUEST ){
            cout << "=============================================" << endl;
            QueryHandler qh(&t_DB, t_clientSocket, 0);
            cout << "Getting S from the client..." << endl;
            //qh.getSFromClient();
            
            
            
            
            
            /*
            cout << "Loading db to memory..." << endl;
            if (!t_DB.getSystemParm()->getSplitDBintoChunks())  //DB can fit entirely into the RAM
            {
            
                cout << "Calculating SDB..." << endl;
                qh.calculateSdb(t_dbFileBuffer);
            
            }
            else {
                cout << "DB is larger than the available RAM, DB should be splitted." << endl;
                cout << "DB size: " << t_DB.getM()*t_DB.getN() << " Total available memory: " << t_DB.getSystemParm()->getTotalAvailRAM() <<
                        " Max chunk size: " << t_DB.getSystemParm()->getChunkSize() << endl;

                do {

                }


            }
            
            
            */
            
            
            
            
            
            cout << "Sending SDB to the client..." << endl;
            //qh.sendBackSdbToClient();
        } else if (reply == CLOSE_CONNECTION) {
            close(t_clientSocket);
            cout << "Connection closed by the client" << endl;
        }
        else 
            cout << "ERROR: " << reply << " couldn't be treated" << endl;
    }
    return 0;
}

int Connector::sendDBFileInfoToClient(int t_clientSocket, DBManager& t_DB) {
    cout << "sending db info to the client: " << t_DB.getDBName() << endl;
    ifstream DBInfoFileStream(t_DB.getDBFileInfo(),ifstream::in);
    if (DBInfoFileStream.is_open()) {
        int sockErr;
        char *infoBuffer;
        infoBuffer = (char *) malloc (INFO_BUFFER_SIZE * sizeof (char));
        int i = 0;  //Counter used to not send the XDB filename
        for(string line; getline(DBInfoFileStream, line);) {
            if (i!=6) {
                bzero(infoBuffer, INFO_BUFFER_SIZE);
                strcpy (infoBuffer, line.c_str());
                sockErr = write(t_clientSocket, infoBuffer, INFO_BUFFER_SIZE);
                if (sockErr < 0) error("ERROR sending db name from the db file");
            }
            i++;
        }
        DBInfoFileStream.close();
        bzero(infoBuffer, INFO_BUFFER_SIZE);
        sockErr = read(t_clientSocket, infoBuffer, INFO_BUFFER_SIZE);
        if (sockErr < 0) error("ERROR while sending DB info");
        string reply(infoBuffer);
        if (reply == DBINFO_ACK)
            cout << "DB File sent successfully. Received: " << reply << endl;
        else
            cout << "ERROR: " << DBINFO_ACK << " not received" << endl;
    }
    else
        cout << "Unable to open the db file: " << t_DB.getDBFileInfo() << endl;
    return 0;
}

Connector::~Connector() {
}

