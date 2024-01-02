
#pragma once

#include <time.h>

using namespace std;

#define IN
#define OUT
#define INOUT

// return values
#define RV_OK 0
#define RV_PASS 1
#define RV_FAIL -1
typedef int RV;

#define TRUE 1
#define FALSE 0

#define MAXQUERYLEN 80  // character length of biggest query
#define MAXTABLECOLS 10 // how many columns a table can have
#define MAXTABLES 10    // how many tables can be referenced by one query
#define MAXKEYS 10      // how many primary & foreign keys can be referenced

// query types

#define SELECT 101U // select query
#define INSERT 102U // insert query
#define UPDATE 103U // update query

#define OPENDB 201U // load DB
#define DROPDB 202U // drop database
#define DESCDB 203U // describe database
#define DESCTA 204U // describe table
#define DESCAT 205U // describe attribute
#define CREATD 206U // create database

#define CREATT 301U // create table
#define CREATI 302U // create index
#define DROPTA 303U // drop table
#define DROPAT 304U // drop attribute
#define DROPIN 305U // drop index

// comparison operators
#define OPGT 401U   // >
#define OPLT 402U   // <
#define OPEQ 403U   // ==
#define OPGTEQ 404U // >=
#define OPLTEQ 405U // <=
#define OPNEQ 406U  // !=
// conjuction operators
#define OPAND 407U // AND
#define OPOR 408U  // OR
// set operators
#define OPDIFF 409U  // DIFFERENCE
#define OPINTER 410U // INTERSECTION
#define OPUNION 411U // UNION

// operands
#define OPERANDVAL 501U
#define OPERANDATT 502U