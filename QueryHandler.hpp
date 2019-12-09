/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   QueryHandler.hpp
 * Author: mahdi
 *
 * Created on November 14, 2019, 5:32 PM
 */

#ifndef QUERYHANDLER_HPP
#define QUERYHANDLER_HPP

#include <string>

class QueryHandler {
public:
    QueryHandler();
    virtual std::size_t getQuerySize() {}
    virtual int processOneQuery(unsigned char* t_query) {}
    virtual unsigned char* getReply() {}
    virtual std::size_t getReplySize() {}
    QueryHandler(const QueryHandler& orig);
    virtual ~QueryHandler();
};

#endif /* QUERYHANDLER_HPP */

