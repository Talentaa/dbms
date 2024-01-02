/*
Author:
Knute Ortiz - Tokenizer.cpp

This file contains all of the function definitions and helper functions for the
Tokenizer module.
*/

// #pragma once
#include <iostream>
#include <stdio.h>
#include <string>

using namespace std;

#include "tokenizer.h"

// constructor
SQLInfo::SQLInfo() {
  rename = false;
  dropDB = false;
  dropTabl = false;
  dropAtt = false;
  selcnt = 0;
  tblcnt = 0;
  colcnt = 0;
  whrcnt = 0;
  whrcntset = 0;
  setOp = 0;
  distinct = false;
}

/*function used to convert a string to UPPER case
 */
string strToUpper(string &str) {
  for (unsigned int i = 0; i < str.length(); i++)
    str[i] = toupper(str[i]);
  return str;
}

/*function used to convert a string to lower case
 */
string strToLower(string &str) {
  for (unsigned int i = 0; i < str.length(); i++)
    str[i] = tolower(str[i]);
  return str;
}

/*function used to remove all of the white spaces from a string
 */
void removeAllSpaces(string &sql) {
  string temp;
  for (unsigned int i = 0; i < sql.size(); i++)
    if (sql.at(i) != ' ')
      temp += sql.at(i);
    else if (i != 0 && sql.at(i - 1) != ',') // only remove spaces after comma
      temp += sql.at(i);
  sql = temp;
}

/*function used to remove multiple white spaces that are next to each other and
 * ';' from a string
 */
void clean(string &str) {
  string tmp = "";
  unsigned int i;
  for (i = 1; i < str.size(); i++) {
    if (!(str.at(i) == ' ' && str.at(i - 1) == ' '))
      tmp += str.at(i - 1);
  }

  size_t semi = tmp.find(";");
  if (semi > -1)
    tmp.erase(semi, semi + 1);

  if (!(str.at(i - 1) == ' ' || str.at(i - 1) == ';'))
    tmp += str.at(i - 1); // add last character back

  if (tmp.at(0) == ' ') // if blank space at beginning of string
    tmp.erase(0, 1);    // remove it

  str = tmp; //.substr(0, tmp.size()-1);// remove ';' from end of string
}

/*function used to remove ALL quotes " ' " from a string
 */
void removeAllQuotes(string &str) {
  string temp;
  for (unsigned int i = 0; i < str.size(); i++)
    if (str.at(i) != '\'')
      temp += str.at(i);
  str = temp;
  //  cout << sql << endl;
  //  system("pause");
}

/*function used to remove ALL parentheses "()" from a string
 */
void removeAllParens(string &sql) {
  string temp;
  for (unsigned int i = 0; i < sql.size(); i++)
    if (!(sql.at(i) == '(' || sql.at(i) == ')'))
      temp += sql.at(i);
  sql = temp;
  //  cout << sql << endl;
  //  system("pause");
}

/*function used to remove ALL periods "." from a string
 */
void removeDot(string &sql, string &table, string &col) {
  size_t cutMe = sql.find(".");
  if (cutMe == -1) {
    col = sql;
    removeAllQuotes(col);
    return; // no '.' to cut
  }

  table = sql.substr(0, cutMe);
  col = sql.substr(cutMe + 1, sql.size());
  removeAllQuotes(col);
}

/*function used to convert a string set operation code to an integer set
 * operation code
 */
void SQLInfo::convertOp() {
  for (unsigned int i = 0; i < whrcnt; i++) {
    if (where[i].strop == "=" && !where[i].bNot)
      where[i].op = OPNEQ;
    else if (where[i].strop == ">")
      where[i].op = OPGT;
    else if (where[i].strop == "<")
      where[i].op = OPLT;
    else if (where[i].strop == "=")
      where[i].op = OPEQ;
    else if (where[i].strop == ">=")
      where[i].op = OPGTEQ;
    else if (where[i].strop == "<=")
      where[i].op = OPLTEQ;
  }
  for (unsigned int i = 0; i < whrcntset; i++) {
    if (whereset[i].strop == "=" && !whereset[i].bNot)
      whereset[i].op = OPNEQ;
    else if (whereset[i].strop == ">")
      whereset[i].op = OPGT;
    else if (whereset[i].strop == "<")
      whereset[i].op = OPLT;
    else if (whereset[i].strop == "=")
      whereset[i].op = OPEQ;
    else if (whereset[i].strop == ">=")
      whereset[i].op = OPGTEQ;
    else if (whereset[i].strop == "<=")
      whereset[i].op = OPLTEQ;
  }
}

/*function used to tokenize an SQL: where clause
 */
size_t SQLInfo::tokenizeWhere(string sql, int isSetOp) {
  string tempWhere = sql;
  strToLower(tempWhere);
  size_t wi = tempWhere.find("where", 0);
  if (wi > 100000)
    wi = tempWhere.find("WHERE", 0);
  if (wi > 100000)
    return wi;
  else
    wi += 6;

  string str = sql.substr(wi, sql.size());
  string temp = "";
  bool quote = false;
  int operand = 1;
  if (isSetOp == 0) {
    for (int i = 0; i < str.size(); i++) {
      if (str.at(i) == '\'') {
        if (quote == false)
          quote = true;
        else
          quote = false;
      }
      if (str.at(i) == ' ' && !quote || !quote && i == str.size() - 1) {
        if (i == str.size() - 1) // so we don't loose las char
          temp += str.at(str.size() - 1);

        //    cout << temp << endl;

        if (temp == "and" || temp == "or") {
          where[whrcnt].logConnect = temp;
          whrcnt++; // end of one conjunct
        } else if (temp == "not")
          where[whrcnt].bNot = false;
        else if (temp == "=")
          where[whrcnt].strop = temp;
        else if (temp == "<")
          where[whrcnt].strop = temp;
        else if (temp == ">")
          where[whrcnt].strop = temp;
        else if (temp == "<=")
          where[whrcnt].strop = temp;
        else if (temp == ">=")
          where[whrcnt].strop = temp;
        else if (temp.at(0) == '\'' && operand == 1) // value
        {
          removeAllQuotes(temp);
          where[whrcnt].oprnd1 = temp;
          operand = 2;
        } else if (temp.at(0) == '\'' && operand == 2) // value
        {
          removeAllQuotes(temp);
          where[whrcnt].oprnd2 = temp;
          operand = 1;
        } else if (operand == 1) // attribute
        {
          string table, col;
          removeDot(temp, table, col);
          where[whrcnt].oprnd1 = col;
          where[whrcnt].tableOp1 = table;
          operand = 2;
        } else if (operand == 2) // attribute
        {
          string table, col;
          removeDot(temp, table, col);
          where[whrcnt].oprnd2 = col;
          where[whrcnt].tableOp2 = table;
          operand = 1;
        }
        temp.clear();
      } else {
        temp += str.at(i);
      }
    }
    whrcnt++;
  } else {
    if (str.at(str.size() - 1) == ')')
      str.erase(str.size() - 1, str.size());

    for (int i = 0; i < str.size(); i++) {
      if (str.at(i) == '\'') {
        if (quote == false)
          quote = true;
        else
          quote = false;
      }
      if (str.at(i) == ' ' && !quote || !quote && i == str.size() - 1) {
        if (i == str.size() - 1) // so we don't loose last char
          temp += str.at(str.size() - 1);

        //      cout << temp << endl;

        if (temp == "and" || temp == "or") {
          whereset[whrcntset].logConnect = temp;
          whrcntset++; // end of one conjunct
        } else if (temp == "not")
          whereset[whrcntset].bNot = false;
        else if (temp == "=")
          whereset[whrcntset].strop = temp;
        else if (temp == "<")
          whereset[whrcntset].strop = temp;
        else if (temp == ">")
          whereset[whrcntset].strop = temp;
        else if (temp == "<=")
          whereset[whrcntset].strop = temp;
        else if (temp == ">=")
          whereset[whrcntset].strop = temp;
        else if (temp.at(0) == '\'' && operand == 1) // value
        {
          removeAllQuotes(temp);
          whereset[whrcntset].oprnd1 = temp;
          operand = 2;
        } else if (temp.at(0) == '\'' && operand == 2) // value
        {
          removeAllQuotes(temp);
          whereset[whrcntset].oprnd2 = temp;
          operand = 1;
        } else if (operand == 1) // attribute
        {
          string table, col;
          removeDot(temp, table, col);
          whereset[whrcntset].oprnd1 = col;
          whereset[whrcntset].tableOp1 = table;
          operand = 2;
        } else if (operand == 2) // attribute
        {
          string table, col;
          removeDot(temp, table, col);
          whereset[whrcntset].oprnd2 = col;
          whereset[whrcntset].tableOp2 = table;
          operand = 1;
        }
        temp.clear();
      } else {
        temp += str.at(i);
      }
    } // loop
    whrcntset++;
  }
  convertOp(); // convert the string operation to an int operation code
  return 1;
}

/*function used to change the user table rename statements to the actual table
 * names
 */
void SQLInfo::fixRename() {
  for (unsigned int i = 0; i < tblcnt; i++) // loop for all of the table entries
  {
    for (unsigned int j = 0; j < selcnt;
         j++) // loop for all of the select entries
    {
      if (select[j].tableName == rTbl[i]) // if rename
      {
        select[j].tableName = tbl[i];
      }
    }
    for (unsigned int j = 0; j < whrcnt;
         j++) // loop for all of the where entries
    {
      if (where[j].tableOp1 == rTbl[i]) // if operand 1 has rename
      {
        where[j].tableOp1 = tbl[i];
      }
      if (where[j].tableOp2 == rTbl[i]) // if operand 2 has rename
      {
        where[j].tableOp2 = tbl[i];
      }
    }
    for (unsigned int j = 0; j < whrcntset;
         j++) // loop for all of the where entries if there was a set op
    {
      if (whereset[j].tableOp1 == rTbl[i]) // if operand 1 has rename
      {
        whereset[j].tableOp1 = tbl[i];
      }
      if (whereset[j].tableOp2 == rTbl[i]) // if operand 2 has rename
      {
        whereset[j].tableOp2 = tbl[i];
      }
    }
  }
}

/*function used to tokenize an SQL: where clause for the table information
function takes a string and keyword
*/
size_t SQLInfo::tokenizeTable(string sql) {
  char lastchar = '\0', thischar = '\0';
  string tablename = "";
  string tempSql = sql;
  strToLower(tempSql);
  size_t tablestart = tempSql.find("from", 0);

  // size_t tablestart = sql.find("from",0);
  if (tablestart > 100000)
    tablestart = tempSql.find("FROM", 0);
  if (tablestart > 100000)
    return tablestart;
  tablestart += 5; // remove "from"

  // for select
  size_t wi = sql.find("where", 0);
  if (wi > 100000)
    wi = sql.find("WHERE", 0);
  if (wi > 100000)
    wi = sql.size(); // if there still was no where clause

  string str = sql.substr(tablestart, wi - tablestart);
  tbl[tblcnt].clear();

  bool end = false;
  size_t comma = str.find_first_of(",");
  string subStr = str.substr(0, comma);
  str = str.substr(comma + 1, str.size()); // cut

  while (!end) {
    if (subStr.at(0) == ' ') // if blank space at beginning of string
      subStr.erase(0, 1);    // remove it
    if (subStr.at(subStr.size() - 1) == ' ') // if blank space at end of string
      subStr.erase(subStr.size() - 1, subStr.size()); // remove it

    size_t space = subStr.find_first_of(" ");

    if (space > 0 && space < comma) // rename
    {
      tbl[tblcnt] = subStr.substr(0, space);
      rTbl[tblcnt] = subStr.substr(space + 1, subStr.size());
      tblcnt++;
      rename = true;
    } else // no rename
    {
      tbl[tblcnt] = subStr;
      rTbl[tblcnt] = subStr;
      tblcnt++;
    }

    if (comma == -1)
      end = true;
    else {
      comma = str.find_first_of(",");
      subStr = str.substr(0, comma);
      str = str.substr(comma + 1, str.size()); // cut
    }
  }
  return 0;
}

/*function used to tokenize an SQL: where clause for the table information
function takes a string and keyword
*/
ssize_t SQLInfo::tokenizeTable(string sql, string keyword) {
  size_t i = 0; // index
  size_t tablestart = 0;
  char lastchar = '\0', thischar = '\0';
  string tablename = "";

  tablestart = sql.find(keyword, 0);
  if (tablestart > 100000)
    tablestart = sql.find(strToLower(keyword).c_str(), 0);
  if (tablestart > 100000)
    return 999999;

  tbl[tblcnt].clear();

  // parse table(s) from sql query string
  for (i = tablestart + keyword.size() + 1; i < sql.length(); i++) {
    lastchar = thischar;
    thischar = sql[i];
    if ((thischar == ' ' && lastchar != ',') || (thischar == ';') ||
        (thischar == '(')) {
      tbl[tblcnt++] = tablename;
      return tblcnt; // return the number of tables
    } else if (thischar == ',') {
      tbl[tblcnt] = tablename;
      tablename = "";
      tbl[++tblcnt].clear(); // else prep for a new table name
    } else if (thischar != ' ')
      tablename += thischar;
  }
  return -1; // if we got here, something went wrong, return an error
}

/*function used to tokenize an SQL: select clauses
function takes a string and isSetOp variable to process the statement properly
if there was a set operation entered into the query
*/
size_t SQLInfo::tokenizeSelect(string sql, int isSetOp) {
  if (isSetOp == 0)
    tokenizeTable(sql); // parse out the table name
  // strToLower(sql);
  tokenizeWhere(sql, isSetOp); // parse out the where clause

  size_t fro = sql.find("from", 0);
  if (fro > 100000)
    fro = sql.find("FROM", 0);

  string selClause = sql.substr(7, fro - 8);

  if (selClause.find("distinct ") < 100000) {
    distinct = true;
    selClause = selClause.substr(9, selClause.size()); // cut "distinct"
  }

  if (isSetOp == 0) // performed only the first time if there is a set operation
  {
    int dot, comma = selClause.find_first_of(',');
    while (comma > -1) // multiple columns
    {
      dot = selClause.find_first_of('.');
      if (dot > -1) // if exists table name
      {
        select[selcnt].tableName = selClause.substr(0, dot);
        select[selcnt].colName = selClause.substr(dot + 1, comma - dot - 1);
        selcnt++;
      } else // no table name
      {
        select[selcnt].colName = selClause.substr(0, comma);
        selcnt++;
      }
      selClause = selClause.substr(comma + 1, selClause.size()); // cut
      comma = selClause.find_first_of(',');
    }
    if (comma == -1) // only one select item, or last select item
    {
      dot = selClause.find_first_of('.');
      if (dot > -1) // if exists table name
      {
        select[selcnt].tableName = selClause.substr(0, dot);
        select[selcnt].colName = selClause.substr(dot + 1, selClause.size());
        selcnt++;
      } else // no table name
      {
        select[selcnt].colName = selClause.substr(0, selClause.size());
        selcnt++;
      }
    }
  }

  if (rename)
    fixRename();

  return SELECT;
}

/*function used to tokenize foreign key statements
function used by tokenizeKeys()
*/
void SQLInfo::processForeign(string sql, DbInfo *db) {
  size_t space = sql.find_first_of(" ");
  string proxCol = sql.substr(0, space);   // proximal column
  sql = sql.substr(space + 1, sql.size()); // cut
  space = sql.find_first_of(" ");
  string ref = sql.substr(0, space);       // references
  sql = sql.substr(space + 1, sql.size()); // cut
  space = sql.find_first_of(" ");
  string disTab = sql.substr(0, space);    // distal table
  sql = sql.substr(space + 1, sql.size()); // cut
  space = sql.find_first_of(" ");
  string disCol = sql.substr(0, space); // distal column

  bool found = false; // stop when proper column is found
  for (unsigned int i = 0; i < colcnt && !found;
       i++) // loop through all cols to set "foreign" key data
  {
    if (col[i].colname == proxCol) {
      col[i].isForeign = true;
      col[i].refTable = disTab;
      col[i].refCol = disCol;
      found = true;
    }
  }
}

/*function used to tokenize primary key statements
function used by tokenizeKeys()
*/
void SQLInfo::processPrimary(string sql, DbInfo *db) {
  int space = 0;
  while (space > -1) {
    space = sql.find_first_of(' ');
    string st = sql.substr(0, space);
    for (unsigned int i = 0; i < colcnt;
         i++) // loop through all cols to set "not null"
    {
      if (col[i].colname == st) {
        col[i].isPrimary = true;
        col[i].isNotNul = true;
      }
    }
    sql = sql.substr(space + 1, sql.size()); // cut
  }
}

/*function used to tokenize key statements
 */
void SQLInfo::tokenizeKeys(string sql, DbInfo *db) {
  string str, pKey, fKey[10];
  bool forKey = false, primKey = false;
  int end = -1;
  for (unsigned int i = 0; i < sql.size(); i++) {
    str += sql.at(i);

    if (primKey && sql.at(i) != '(' && sql.at(i) != ')' && sql.at(i) != ',')
      pKey += sql.at(i);
    else if (primKey)
      pKey += " ";

    if (forKey && sql.at(i) != '(' && sql.at(i) != ')' && sql.at(i) != ',')
      fKey[end] += sql.at(i);
    else if (forKey)
      fKey[end] += " ";

    if (sql.at(i) == ' ') {
      str.clear();
    } else if (str == "primary") {
      primKey = true;
      str.clear();
    } else if (str == "foreign") {
      forKey = true;
      primKey = false;
      str.clear();
      end++;
    }
  }

  int mark;
  if (pKey.size() > 0) {
    clean(pKey);
    pKey.erase(0, 4);
    mark = pKey.find("foreign");
    if (mark > -1)
      pKey.erase(mark - 1, 8);
    processPrimary(pKey, db);
  }
  for (int i = 0; i <= end; i++) {
    clean(fKey[i]);
    mark = fKey[i].find("foreign");
    if (mark > -1)
      fKey[i].erase(mark - 1, 8);
    fKey[i].erase(0, 4);
    processForeign(fKey[i], db);
  }
}

/*function used to tokenize SQL: create statements
 */
int SQLInfo::tokenizeCreate(string sql, DbInfo *db) {
  size_t found;
  string sub = sql.substr(7, 3);

  strToLower(sub);

  // create a new database
  if (sub == "dat") {
    db->tokenizeDatabase(sql);
    return CREATD;
  }

  // create new index
  if (sub == "ind") {
    tokenizeTable(sql, "ON");         // table
    sql = sql.substr(13, sql.size()); // cut

    //  cout << sql << endl;//debugging
    //  system("pause");//debugging

    size_t space = sql.find_first_of(' ');
    string nam = sql.substr(0, space);
    if (nam != "on") {
      name = nam;
      sql = sql.substr(space + 4, sql.size()); // cut "on "
    } else
      sql = sql.substr(3, sql.size()); // cut "on "

    size_t openParens = sql.find_first_of('(');
    col[colcnt].tabname = sql.substr(0, openParens);
    sql.erase(sql.size() - 1, 1); // erase ");"
    col[colcnt].colname = sql.substr(openParens + 1, sql.size());

    removeAllSpaces(col[colcnt].colname); // remove spaces

    colcnt++;
    return CREATI;
  }

  // create a new table
  else if (sub == "tab") {
    tokenizeTable(sql, "TABLE");
    string colname, coltype, colwdth, key;
    int which = 0; // what are we processing now?
                   // 0 column name
                   // 1 column type
                   // 2 column width
                   // 3 key
    colcnt = 0;

    bool isInt = false;
    for (size_t i = sql.find("(", 0) + 1; i < sql.rfind(")", sql.length() + 1);
         i++) {
      if (sql[i] == ' ') {
        if (colname.empty())
          which = 0;
        else if (coltype.empty())
          which = 1;
        else if (colwdth.empty() && !isInt) // no width for integers
          which = 2;
        else if (key.empty())
          which = 3;
      } else if (sql[i] == '(' && !isInt) { /* beginning a column width */
        which = 2;
      } else if (sql[i] == ',') { /* ending a column def */
        int ncolwidth = atoi(colwdth.c_str());
        col[colcnt].tabname = tbl[tblcnt - 1];
        col[colcnt].colname = colname;
        col[colcnt].coltype = coltype;
        if (coltype == "int") {
          ncolwidth = 1;
        } else if (coltype == "INT") {
          ncolwidth = 1;
        }
        if (coltype == "float") {
          ncolwidth = 1;
        } else if (coltype == "FLOAT") {
          ncolwidth = 1;
        }
        if (ncolwidth == 0)
          ncolwidth = 1;

        col[colcnt].colwdth = ncolwidth;

        found = key.find("notnull");
        if (found < 100000) {
          col[colcnt].isNotNul = true;
        }
        found = key.find("unique");
        if (found < 100000) {
          col[colcnt].isUnique = true;
        }

        colcnt++;

        which = 0;
        colname.clear();
        coltype.clear();
        colwdth.clear();
        key.clear();
        isInt = false;
      } else if (sql[i] == ')') { /* ending column width def */
      } else {                    /* add character to a string */
        switch (which) {
        case 0: // column name
        {
          colname += sql[i];
          if (colname == "primary" || colname == "foreign") {
            tokenizeKeys(colname + sql.substr(i + 1, sql.size()), db);
            return CREATT;
          }
          break;
        }
        case 1: // column type
        {
          coltype += sql[i];
          if (coltype == "int")
            isInt = true;
          break;
        }
        case 2: // column width
        {
          colwdth += sql[i];
          break;
        }
        case 3: // key
        {
          key += sql[i];
          break;
        }
        }
      }
    }
    // load ending column
    int ncolwidth = atoi(colwdth.c_str());
    col[colcnt].tabname = tbl[tblcnt - 1];
    col[colcnt].colname = colname;
    col[colcnt].coltype = coltype;
    if (coltype == "int") {
      ncolwidth = 1;
    } else if (coltype == "INT") {
      ncolwidth = 1;
    }
    if (coltype == "float") {
      ncolwidth = 1;
    } else if (coltype == "FLOAT") {
      ncolwidth = 1;
    }
    col[colcnt].colwdth = ncolwidth;

    found = key.find("notnull");
    if (found < 100000)
      col[colcnt].isNotNul = true;

    found = key.find("unique");
    if (found < 100000)
      col[colcnt].isUnique = true;

    colcnt++;

    return CREATT;
  }
  return 99999; // error
}

/*function used to tokenize insert statements
 */
int SQLInfo::tokenizeInsert(string sql) {
  bool done = false;
  string tempInsert = sql;
  strToLower(tempInsert);
  int intoLoc = tempInsert.find("into");
  int valLoc = tempInsert.find("values");
  name = sql.substr(intoLoc + 5, valLoc - 13);     // store the table name
  sql = sql.substr(sql.find("(") + 1, sql.size()); // cut

  col[colcnt].tabname = name; // store the table name
  removeAllQuotes(sql);       // remove all ' marks
  removeAllParens(sql);       // remove all '(' && ')'
  removeAllSpaces(sql);       // remove all white spaces
  col[colcnt].colvalue = sql; // store all values in one col
  colcnt++;

  ///////////////loop to place one "value" into seperate col
  ///slots////////////////////////
  // while(!done)
  //{
  //   col[colcnt].tabname = table;

  //  size_t comma = sql.find(',');
  //  if(comma == -1)
  //    done = true;
  //  string temp = sql.substr(0, comma);
  //  insertClean(temp);
  //  col[colcnt].colvalue = temp;
  //  sql = sql.substr(comma+1, sql.size());//cut

  //  colcnt++;
  //}
  return INSERT;
}

/*function used to determine the tpe of a set operation
 */
unsigned int isSetOps(string &sql) {
  if (sql.find(" intersect ") < 10000)
    return OPINTER;
  if (sql.find(" difference ") < 10000)
    return OPDIFF;
  if (sql.find(" union ") < 10000)
    return OPUNION;
  return 0;
}

/*function used to remove the set operation in an SQL statement
 */
string cutSetOps(string &sql, int which, unsigned int op) {
  if (which == 0) // cut string before set op
  {
    if (op == OPDIFF)
      return sql.substr(1, sql.find(" difference ") - 2);
    if (op == OPINTER)
      return sql.substr(1, sql.find(" intersect ") - 3);
    if (op == OPUNION)
      return sql.substr(1, sql.find(" union ") - 2);
  } else // cut string after set op
  {
    if (op == OPDIFF)
      // for (int i =0; i<sql.size(); i++)
      return sql.substr(sql.find(" difference ") + 13, sql.size() - 121);
    if (op == OPINTER)
      return sql.substr(sql.find(" intersect ") + 12, sql.size() - 120);
    if (op == OPUNION)
      return sql.substr(sql.find(" union ") + 8, sql.size() - 116);
  }
  return "error";
}

/*function used to tokenize SQL: drop statements
 */
int SQLInfo::tokenizeDrop(string &sql) {
  if (sql.find("database") < 100000) // if database
  {
    name = sql.substr(14, sql.size() - 1);
    dropDB = true;
    return DROPDB;
  }
  if (sql.find("table") < 100000) // if table
  {
    name = sql.substr(11, sql.size() - 1);
    dropTabl = true;
    return DROPTA;
  }
  if (sql.find("attribute") < 100000) // if table
  {
    sql = sql.substr(15, sql.size()); // cut "drop" & "attribute"
    name = sql.substr(0, sql.find_first_of(' '));
    sql = sql.substr(sql.find_first_of(' ') + 6,
                     sql.size()); // cut "????" & "from"
    tbl[colcnt] = sql;
    tblcnt++;
    dropAtt = true;
    return DROPAT;
  }
  sql = sql.substr(11, sql.size()); // cut
  name = sql;
  return DROPIN;
}

/*function used to tokenize SQL: describe statements
 */
int SQLInfo::tokenizeDescribe(string &sql) {
  if (sql.find("database") < 100000) // if database
  {
    name = sql.substr(18, sql.size() - 7);
    return DESCDB;
  }
  if (sql.find("table") < 100000) // if database
  {
    name = sql.substr(15, sql.size() - 7);
    return DESCTA;
  }
  return 999999; // error
}

/*function used for determening which sql statement is being entered by the
user and then calling the appropriate tokenize function(s)
*/
int SQLInfo::tokenize(string sql, DbInfo *db) {
  clean(sql);

  string sub;

  if (sql.at(0) == '(')
    sub = sql.substr(1, 3);
  else
    sub = sql.substr(0, 3);

  strToLower(sub);

  if (sub == "sel") {
    string str;
    unsigned int op = isSetOps(sql);
    if (op > 0) {
      setOp = op;
      str = cutSetOps(sql, 0, op);
      tokenizeSelect(str, 0);

      str = cutSetOps(sql, 1, op);
      tokenizeSelect(str, 1);
    } else
      tokenizeSelect(sql, 0);
    return SELECT;
  } else if (sub == "upd") {
    //    return tokenizeUpdate(sql, query);
    return 99999; // not yet supported
  } else if (sub == "cre") {
    return tokenizeCreate(sql, db);
  } else if (sub == "ins") {
    return tokenizeInsert(sql);
  } else if (sub == "del") {
    //    return tokenizeDelete(sql, query);
    return 99999; // not yet supported
  } else if (sub == "use") {
    //    return tokenizeDatabase(sql, "USE", query);
    return 99999; // not yet supported
  } else if (sub == "des") {
    return tokenizeDescribe(sql);
  } else if (sub == "dro") {
    return tokenizeDrop(sql);
  } else if (sub == "ope") {
    db->database_name = sql.substr(5, sql.size() - 1);
    return OPENDB;
  }
  return 99999; // error
}

/*function used to display all of the tokenized SQL statements
 */
void SQLInfo::display() // debugging
{
  cout << "\nSQLInfo:\n";
  if (name.size() > 0)
    cout << "\nname    " << name << endl;

  if (selcnt > 0) {
    cout << "distinct " << boolalpha << distinct << endl;
    cout << "\nSelect Info:\n";
    for (unsigned int i = 0; i < selcnt; i++) {
      select[i].display();
    }
  }

  if (tblcnt > 0) {
    cout << "\ntable Info:\n";
    for (unsigned int i = 0; i < tblcnt; i++) {
      cout << rTbl[i] << "     ";
      cout << tbl[i] << endl;
    }
  }
  if (colcnt > 0) {
    cout << "\n\nCollumn Info:\n";
    for (unsigned int i = 0; i < colcnt; i++) {
      col[i].display();
    }
  }

  if (whrcnt > 0) {
    cout << "\n\nWhere Info:\n";
    for (unsigned int i = 0; i < whrcnt; i++) {
      where[i].display();
      cout << endl;
    }
  }

  if (setOp > 0) {
    cout << "\n\nSet Operation " << setOp << endl;
    cout << "WhereInfo:\n";
    for (unsigned int i = 0; i < whrcntset; i++) {
      whereset[i].display();
      cout << endl;
    }
  }

  if (dropDB || dropTabl || dropAtt) {
    cout << "\nDrop Info:\n";
    cout << "DB:      " << boolalpha << dropDB << "\nTable:   " << dropTabl
         << "\nAtt:     " << dropAtt << endl;
  }
}
