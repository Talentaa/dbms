/*
Author:
Knute Ortiz - Tokenizer.h

This file contains all of the structures for the Tokenizer module.
*/

#pragma once

#include <time.h>

using namespace std;

#include "Gvariables.h"

// string strToUpper(string & str);
// string strToLower(string & str);
// void clean(string & str);
// SQLInfo::SQLInfo();

// structure to hold where clause information
struct WhereInfo {
  bool bNot;
  string oprnd1;   // left side of equation
  string tableOp1; // table for operand 1
  string oprnd2;   // right side of operation
  string tableOp2; // table for operand 2
  string strop;    // operator that connects operand1 and operand2
  int op;
  string logConnect; // and, or
  int numLogC;       // number of logical connectives

  WhereInfo() {
    bNot = true;
    oprnd1 = "-";
    tableOp1 = "";
    oprnd2 = "-";
    tableOp2 = "";
    numLogC = 0;
  }

  void display() {
    cout << boolalpha << bNot << "   " << tableOp1 << "   " << oprnd1 << "   "
         << strop << "   " << tableOp2 << "   " << oprnd2 << "   " << logConnect
         << "   " << op;
  }
};
// structure to hold column (attribute) data information
struct ColInfo {
  string colname;       // column name
  string aggrgte;       // agregate function (if any)
  string tabname;       // table this column belongs to
  string coltype;       // column's datatype
  string colvalue;      // value of this column (used in inerts)
  bool isNotNul;        // stores "not null"
  bool isPrimary;       // primary key
  bool isForeign;       // foreign key
  bool isUnique;        // Unique
  string refTable;      // table reference, for foreign key
  string refCol;        // table reference, for foreign key
  unsigned int colwdth; // how wide the column is (used when type is char)

  ColInfo() // constructor
  {
    colname = "";
    coltype = "";
    colvalue = "";
    refTable = "";
    refCol = "";
    tabname = "";
    colwdth = 0;
    isPrimary = false;
    isForeign = false;
    isNotNul = false;
    isUnique = false;
  }
  void display() {
    cout << boolalpha << "\ntabname    " << tabname << "\ncolname    "
         << colname << "\ncoltype    " << coltype << "\ncolvalue   " << colvalue
         << "\ncolwidth   " << colwdth << "\nisPrimary  " << isPrimary
         << "\nisForeign  " << isForeign << "\nnotnull    " << isNotNul
         << "\nisUnique   " << isUnique << "\nref table  " << refTable
         << "\nref Col    " << refCol << endl;
  }
};

// structure to hold Database information
struct DbInfo {
  string database_name;
  string database_owner;
  string creation_date;
  string modify_date;
  bool created;

  DbInfo() {
    created = false;
    database_name = "-";
    database_owner = "-";
    modify_date = "-";
    time_t ltime;
    time(&ltime);
    creation_date = ctime(&ltime);
  }

  DbInfo(string name, string owner) {
    database_name = name;
    database_owner = owner;
    time_t ltime;
    time(&ltime);
    modify_date = ctime(&ltime);
  }

  void tokenizeDatabase(string sql) {
    sql = sql.substr(16, sql.size()); // cut
    ssize_t space = sql.find_first_of(" ");
    database_name = sql.substr(0, space);
    database_owner = sql.substr(space + 1, sql.size());
    created = true;
  }

  void setModifyTime() {
    time_t ltime;
    time(&ltime);
    modify_date = ctime(&ltime);
  }

  void setCreatTime() {
    time_t ltime;
    time(&ltime);
    creation_date = ctime(&ltime);
  }

  void display() {
    cout << "Database Info:\n"
         << "\nDB Name: " << database_name << "\nOwner: " << database_owner
         << "\nCreated on: " << creation_date
         << "\nModified on: " << modify_date << endl
         << endl;
  }
};

// structure to hold SQL: select clause data information
struct Select {
  string colName;   // left side of equation
  string tableName; // table for operand 1

  Select() {
    colName = "-";
    tableName = "-";
  }

  void display() { cout << tableName << "   " << colName << endl; }
};

// structure to hold info about an sql statement
struct SQLInfo {
  string name;
  Select select[MAXTABLECOLS];      // array of select clauses
  string tbl[MAXTABLES];            // array of table names
  string rTbl[MAXTABLES];           // array of rename table names
  ColInfo col[MAXTABLECOLS];        // array of column names
  WhereInfo where[MAXTABLECOLS];    // array of where clauses
  WhereInfo whereset[MAXTABLECOLS]; // array of where clauses for set operations
  bool distinct;
  bool dropDB;
  bool dropTabl;
  bool dropAtt;
  bool rename;
  unsigned int selcnt;
  unsigned int tblcnt;
  unsigned int colcnt;
  unsigned int whrcnt;
  unsigned int whrcntset;
  unsigned int setOp;

  // function prototypes
  SQLInfo(); // constructor
  void convertOp();
  int tokenize(string sql, DbInfo *db);
  size_t tokenizeSelect(string sql, int isSetOp);
  size_t tokenizeTable(string sql);
  ssize_t tokenizeTable(string sql, string keyword);
  void fixRename();
  size_t tokenizeWhere(string sql, int isSetOp);
  int tokenizeDescribe(string &sql);
  int tokenizeDrop(string &sql);
  int tokenizeInsert(string sql);
  int tokenizeCreate(string sql, DbInfo *db);
  void tokenizeKeys(string sql, DbInfo *db);
  void processPrimary(string sql, DbInfo *db);
  void processForeign(string sql, DbInfo *db);
  void display(); // debugging
};
