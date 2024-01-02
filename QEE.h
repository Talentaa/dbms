/*
Author:
Knute Ortiz - QEE.h

This file contains all of the primary Query Evaluation Engine functions and
additional helper functions.
*/

#pragma once

#include <iostream>
#include <string>

using namespace std;
#include "BufferManager.h"
#include "FA.h"
#include "Gvariables.h"
#include "tokenizer.h"
// #include "Parser.h"

/* itoa() implementation under linux */
char *itoa(int a, char *str, int size_str) {
  sprintf(str, "%d", a);
  return str;
}

/*Function used to return true if the database exists in the metadata otherwise
false functions also provides the position of the database in the metadata
*/
bool isDB(string db, int &i) {
  for (i = 0; i < MAXDBS; i++) // loop to search for the database name
  {
    if (strcmp(mdata.dbases[i].name, db.c_str()) == 0) {
      //
      return true;
    }
  }
  i = -1; // not found
  return false;
}

/*Function used to return true if the index for this database exists in the
metadata otherwise false
functions also provides the position of the index in the metadata
*/
bool isInd(string db, string ind, int &i) {
  for (i = 0; i < MAXINDEXES; i++) // loop to search for the database name
  {
    if (strcmp(mdata.indexes[i].DBName, db.c_str()) == 0 &&
        strcmp(mdata.indexes[i].name, ind.c_str()) == 0) {
      return true;
    }
  }
  i = -1; // not found
  return false;
}

/*Function used to return true if the table (relation) for this database exists
in the metadata otherwise false functions also provides the position of the
table (relation) in the metadata
*/
bool isTable(string db, string tab, int &i) {
  for (i = 0; i < MAXRELS; i++) // loop to search for the database name
  {
    if (strcmp(mdata.relations[i].DBName, db.c_str()) == 0 &&
        strcmp(mdata.relations[i].name, tab.c_str()) == 0) {
      return true;
    }
  }
  i = -1; // not found
  return false;
}

/*Function used to return true if the column for this table, this database
exists in the metadata otherwise false functions also provides the position of
the column in the metadata
*/
bool isCol(string db, string tab, string col, int &i) {
  for (i = 0; i < MAXATTRS; i++) // loop to search for the database name
  {
    if (strcmp(mdata.attrs[i].DBName, db.c_str()) == 0 &&
        strcmp(mdata.attrs[i].Relname, tab.c_str()) == 0 &&
        strcmp(mdata.attrs[i].name, col.c_str()) == 0) {
      return true;
    }
  }
  i = -1; // not found
  return false;
}

/*Function used to count the number of columns in the table, database
 */
int numcols(string DB, string table) {
  int count = 0;
  for (unsigned int i = 0; i < MAXATTRS; i++) {
    if (!strcmp(mdata.attrs[i].DBName, DB.c_str()) &&
        !strcmp(mdata.attrs[i].Relname, table.c_str()) &&
        strcmp(mdata.attrs[i].name, ""))
      count++;
  }
  return count;
}

/*Function used to return true if the value given is unique in the array
 */
bool isUnique(int con[100], int num) {
  for (int i = 0; i < 100; i++) // loop to search for the database name
  {
    if (con[i] == num)
      return false;
  }
  return true;
}

/*Function used to calculate the location of an attribute in the metadata
 */
int indexOfAtt(int pos, string DB, string tabl) {
  int i;
  int index = 0;
  for (i = 0; i < MAXATTRS; i++) {
    if (!strcmp(mdata.attrs[i].DBName, DB.c_str()) &&
        !strcmp(mdata.attrs[i].Relname, tabl.c_str())) {
      if (mdata.attrs[i].position == pos) {
        index = i;
        return index;
      }
    }
  }
  return index;
}

/*Function used find the type(int, float, char) for a attribute in the metadata
  This function uses the attr name
*/
int getType(string &DB, string &rel, string att) {
  int type = 0;
  for (int i = 0; i < MAXATTRS; i++) {
    if (!strcmp(mdata.attrs[i].DBName, DB.c_str()) &&
        !strcmp(mdata.attrs[i].Relname, rel.c_str()) &&
        !strcmp(mdata.attrs[i].name, att.c_str())) {
      if (!strcmp(mdata.attrs[i].TypeName, "int") ||
          !strcmp(mdata.attrs[i].TypeName, "INT")) {
        return 0;
      }
      if (!strcmp(mdata.attrs[i].TypeName, "float") ||
          !strcmp(mdata.attrs[i].TypeName, "FLOAT")) {
        return 1;
      }
      if (!strcmp(mdata.attrs[i].TypeName, "char") ||
          !strcmp(mdata.attrs[i].TypeName, "CHAR")) {
        return 2;
      }
    }
  }
  return type;
}

/*Function used to find the type(int, float, char) for a relation in the
  metadata This function uses the position in the relation
*/
int getType(string &DB, string &rel, int pos) {
  int type = 0;
  for (int i = 0; i < MAXATTRS; i++) {
    if (!strcmp(mdata.attrs[i].DBName, DB.c_str()) &&
        !strcmp(mdata.attrs[i].Relname, rel.c_str()) &&
        mdata.attrs[i].position == pos) {
      if (!strcmp(mdata.attrs[i].TypeName, "int") ||
          !strcmp(mdata.attrs[i].TypeName, "INT")) {
        return 0;
      }
      if (!strcmp(mdata.attrs[i].TypeName, "float") ||
          !strcmp(mdata.attrs[i].TypeName, "FLOAT")) {
        return 1;
      }
      if (!strcmp(mdata.attrs[i].TypeName, "char") ||
          !strcmp(mdata.attrs[i].TypeName, "CHAR")) {
        return 2;
      }
    }
  }
  return type;
}

/*Function used to calculate a new offset for retrieving tuples from the buffer
Function used when inserting tuples into the buffer
function takes a database name, relation name, attribute name, and an offset
*/
int getNewOff(string &DB, string &rel, string att, int offset) {
  int num = numcols(DB, rel);
  for (int i = 0; i < num; i++) // get all columns for the table
  {
    if (!strcmp(mdata.attrs[indexOfAtt(i, DB, rel)].name, att.c_str()))
      return offset;

    int typeSize = mdata.attrs[indexOfAtt(i, DB, rel)].length;
    int t = indexOfAtt(i, DB, rel);
    char type[100];
    strcpy(
        type,
        mdata.attrs[indexOfAtt(i, DB, rel)].TypeName); // type of the ith column
    if (!strcmp(type, "int") || !strcmp(type, "INT")) {
      offset += sizeof(int);
    } else if (!strcmp(type, "float") || !strcmp(type, "FLOAT")) {
      offset += sizeof(float);
    } else if (!strcmp(type, "char") || !strcmp(type, "CHAR")) {
      offset += sizeof(char) * typeSize;
    }
  }
  return offset;
}

/*Function used to calculate a new offset for retrieving tuples from the buffer
Function used when inserting tuples into the buffer
function takes a database name, relation name, index in metadata, and an offset
*/
int getNewIOff(string &DB, string &rel, int ind, int offset) {
  int num = numcols(DB, rel);
  for (int i = 0; i < num; i++) // get all columns for the table
  {
    if (mdata.attrs[indexOfAtt(i, DB, rel)].position == ind)
      return offset;

    int typeSize = mdata.attrs[indexOfAtt(i, DB, rel)].length;
    int t = indexOfAtt(i, DB, rel);
    char type[100];
    strcpy(
        type,
        mdata.attrs[indexOfAtt(i, DB, rel)].TypeName); // type of the ith column
    if (!strcmp(type, "int") || !strcmp(type, "INT")) {
      offset += sizeof(int);
    } else if (!strcmp(type, "float") || !strcmp(type, "FLOAT")) {
      offset += sizeof(float);
    } else if (!strcmp(type, "char") || !strcmp(type, "CHAR")) {
      offset += sizeof(char) * typeSize;
    }
  }
  return offset;
}

/*Function used to return true if the string contains all numeric values
 * otherwise false
 */
bool isNum(string val) {
  for (unsigned int i = 0; i < val.size(); i++) {
    if (isalpha(val.at(i))) {
      return false;
    }
  }
  return true;
}

/* function to count the number of characters in a character array
 */
int strSize(char str[100]) {
  int ctr = 0;
  for (unsigned int i = 0; i < 100; i++) {
    if (str[i] == '\0' || str[i] == ' ')
      return ctr;
    ctr++;
  }
  return ctr;
}

#include <fstream>

/*Function used to display the tuples in a table in the buffer
 */
void displayTable(string DB, string table) {
  ofstream file("so-tired");
  extern int g_flag;

  cout << endl;
  file << endl;

  int z = 0;
  if (!isTable(DB.c_str(), table.c_str(),
               z)) // make sure table exists in metadata
  {
    cerr << "Can not display an empty table!" << endl;
    return;
  }

  vector<int> colsize;
  for (unsigned int j = 0; j < MAXATTRS;
       j++) // loop to print the column headings
  {
    if (strcmp(mdata.attrs[j].Relname, table.c_str()) == 0) {
      int colnamesize = strSize(mdata.attrs[j].name);
      if (colnamesize > mdata.attrs[j].length) {
        if (g_flag)
          cout << left << setw(colnamesize) << mdata.attrs[j].name << " ";
        file << left << setw(colnamesize) << mdata.attrs[j].name << " ";
        colsize.push_back(colnamesize);
      } else if (mdata.attrs[j].name[0] > 47 &&
                 mdata.attrs[j].name[0] <
                     123) // if bogus column exixsts it's not printed
      {
        if (g_flag)
          cout << left << setw(mdata.attrs[j].length) << mdata.attrs[j].name
               << " ";
        file << left << setw(mdata.attrs[j].length) << mdata.attrs[j].name
             << " ";
        colsize.push_back(mdata.attrs[j].length);
      }
    }
  }
  if (g_flag)
    cout << endl;
  file << endl;

  for (unsigned int k = 0; k < colsize.size();
       k++) // loops to print the seperator line(s)
  {
    for (int m = 0; m < colsize.at(k); m++) {
      if (g_flag)
        cout << "-";
      file << "-";
    }
    if (g_flag)
      cout << " ";
    file << " ";
  }
  if (g_flag)
    cout << endl;
  file << endl;

  Frame f;
  f = Tb.Get_first_record(DB.c_str(), table.c_str());

  while (f.frame_id != -1) // stop when at end of table
  {
    int counter = 0;
    int temp = numcols(DB, table);
    int offset = f.offset;
    for (int i = 0; i < temp; i++) {
      int typeSize = mdata.attrs[indexOfAtt(i, DB, table)].length;
      int testvalue;
      float flval;
      char type[100];
      char val[100];
      strcpy(type, mdata.attrs[indexOfAtt(i, DB, table)]
                       .TypeName); // type of the ith column

      if (!strcmp(type, "int") || !strcmp(type, "INT")) {
        memcpy(&testvalue, &buf[f.frame_id].field[offset], sizeof(int));
        if (g_flag)
          cout << left << setw(colsize.at(counter)) << testvalue << " ";
        file << left << setw(colsize.at(counter)) << testvalue << " ";
        offset += sizeof(int);
      } else if (!strcmp(type, "float") || !strcmp(type, "FLOAT")) {
        memcpy(&flval, &buf[f.frame_id].field[offset], sizeof(float));
        if (g_flag)
          cout << left << setw(colsize.at(counter)) << flval << " ";
        file << left << setw(colsize.at(counter)) << flval << " ";
        offset += sizeof(float);
      } else if (!strcmp(type, "char") || !strcmp(type, "CHAR")) {
        strcpy(val, &buf[f.frame_id].field[offset]);
        if (g_flag)
          cout << left << setw(colsize.at(counter)) << val << " ";
        file << left << setw(colsize.at(counter)) << val << " ";
        offset += sizeof(char) * typeSize;
      }
      counter++;
    }
    f = Tb.Get_next_record(DB.c_str(), table.c_str());

    if (g_flag)
      cout << endl;
    file << endl;
  }
  if (g_flag)
    cout << endl;
  file << endl;
}

/*performs "projection" computations
function contains 2 primary steps:
  1 remove unwanted attributes
  2 eliminate duplicates
*/
string Select(string &DB, string &srel1, SQLInfo *query) {
  // step 1
  Frame f;
  string srel2 = "SelRel";
  string selDistArr[1024];
  int cntselDistArr = 0;
  int bAdd = 0;
  CMdata.Create_table(DB.c_str(), srel2.c_str());

  for (unsigned int q = 0; q < query->selcnt; q++) // for all queries
  {
    for (unsigned int i = 0; i < MAXATTRS; i++) // get all columns for the table
    {
      if (strcmp(mdata.attrs[i].Relname, srel1.c_str()) ==
              0 && // if in the corect table
          strcmp(mdata.attrs[i].name, query->select[q].colName.c_str()) ==
              0) {                                // if in the correct attribute
        CMdata.Add_attribute(DB.c_str(),          // DB
                             srel2.c_str(),       // table name
                             mdata.attrs[i].name, // attribute name
                             mdata.attrs[i].TypeName,  // attribute type
                             mdata.attrs[i].length,    // int, column width
                             mdata.attrs[i].precision, // precision
                             q,                        // position
                             mdata.attrs[i].IsPrimary, // bool, is primary
                             0,                        // bool, is not null
                             0,                        // bool, is foreign
                             0                         // bool, is unique
        );
      }
    }
  }
  f = Tb.Get_first_record(DB.c_str(), srel1.c_str());
  for (unsigned int i = 0; f.frame_id != -1 && f.offset != 4095;
       i++) // each record in relation
  {
    char values[FRAMESIZE];
    strcpy(values, "\0");
    int off = f.offset;
    for (unsigned int m = 0; m < query->selcnt;
         m++) // each attribute in the query statement
    {
      bAdd = 0;
      int k = 0;
      isCol(DB, query->tbl[0], query->select[m].colName, k);
      int j = mdata.attrs[k].position;

      off = getNewIOff(DB, srel1, j, f.offset);
      if (getType(DB, srel1, j) == 0) {
        int temp = 0;
        memcpy(&temp, &buf[f.frame_id].field[off], sizeof(int));
        char *tmp = (char *)malloc(sizeof(int) + 1);
        strcat(values, itoa(temp, tmp, 10));
      } else if (getType(DB, srel1, j) == 1) {
        float temp = 0.0;
        memcpy(&temp, &buf[f.frame_id].field[off], sizeof(int));
      } else {
        strcat(values, &buf[f.frame_id].field[off]);
      }
      if (j != (numcols(DB, srel1) - 1)) {
        strcat(values, ",");
      }
    }

    if (query->distinct) // distinct == "true")
    {
      for (int zz = 0; zz < cntselDistArr; zz++) {
        if (strcmp(values, selDistArr[zz].c_str()) == 0) {
          bAdd = -1;
          break;
        }
      }
    }
    if (bAdd != -1) {
      selDistArr[cntselDistArr] = values;
      cntselDistArr++;
      Tb.Insert_record(DB.c_str(), srel2.c_str(), values);
    }
    f = Tb.Get_next_record(DB.c_str(), srel1.c_str());
  }

  // step 2
  // NO SORT Implemented Yet!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  /*
    if(query->distinct)//if distinct
    {
      bool duplicate = false;
      Frame f2;
      string srel22 = "SelRel22";
      CMdata.Create_table(DB.c_str(), srel22.c_str());
      int numValsInPag = 0;//used for slot directory

      for(unsigned int q = 0; q < query->selcnt; q++)
      {
        for(unsigned int i = 0; i < MAXATTRS; i++)//get all columns for the
  table
        {
          if(strcmp(mdata.attrs[i].Relname, srel1.c_str()) == 0 && // if in the
  corect table strcmp(mdata.attrs[i].name, query->select[q].colName.c_str()) ==
  0) {     //if in the correct attribute CMdata.Add_attribute( DB.c_str(), //DB
              srel22.c_str(), //table name
              mdata.attrs[i].name, //attribute name
              mdata.attrs[i].TypeName, //attribute type
              mdata.attrs[i].length, //int, column width
              mdata.attrs[i].precision,//precision
              q,//position
              mdata.attrs[i].IsPrimary,//bool, is primary
              0,//bool, is not null
              0,//bool, is foreign
              0//bool, is unique
              );
          }
        }
      }

      f = Tb.Get_first_record(DB.c_str(), srel1.c_str());
      for(unsigned int i = 0; f.frame_id != -1 && f.offset != 4095; i++)//each
  record in relation
      {
        char values[FRAMESIZE];
        strcpy(values,"\0");
        int off = f.offset;
        for(unsigned int m = 0; m < query->selcnt; m++)//each attribute in the
  query statement
        {
          int k = 0;
          isCol(DB,query->tbl[0],query->select[m].colName,k);
          int j = mdata.attrs[k].position;

          off = getNewIOff(DB,srel1,j,f.offset);
          if(getType(DB,srel1,j) == 0)
          {
            int temp = 0;
            memcpy(&temp,&buf[f.frame_id].field[off],sizeof(int));
            char * tmp = (char *)malloc(sizeof(int)+1);
  s
  s
  s
            strcat(values,itoa(temp,tmp,10));
          }
          else if(getType(DB,srel1,j) == 1)
          {
            float temp = 0.0;
            memcpy(&temp,&buf[f.frame_id].field[off],sizeof(int));
          }
          else
          {
            strcat(values, &buf[f.frame_id].field[off]);
          }
          if(j != query->selcnt)
          {
            strcat(values, ",");
          }
        }//end loop for each value in srel1

        if(i==0)
          Tb.Insert_record(DB.c_str(), srel22.c_str(), values);//insert first
  record into srel22

        f2 = Tb.Get_first_record(DB.c_str(), srel22.c_str());
        for(unsigned int ii = 0; f2.frame_id != -1 && f2.offset != 4095;
  ii++)//each record in relation
        {
          char values2[FRAMESIZE];
          strcpy(values2,"\0");
          int off2 = f2.offset;
          for(unsigned int m2 = 0; m2 < query->selcnt; m2++)//each attribute in
  the query statement
          {
            int k2 = 0;
            isCol(DB,srel22,query->select[m2].colName,k2);
            int j2 = mdata.attrs[k2].position;

            off2 = getNewIOff(DB,srel22,j2,f2.offset);
            if(getType(DB,srel22,j2) == 0)
            {
              int temp2 = 0;
              memcpy(&temp2,&buf[f2.frame_id].field[off2],sizeof(int));
              char * tmp2 = (char *)malloc(sizeof(int)+1);
              strcat(values2,itoa(temp2,tmp2,10));
            }
            if(getType(DB,srel22,j2) == 1)
            {
              float temp2 = 0.0;
              memcpy(&temp2,&buf[f2.frame_id].field[off2],sizeof(int));
            }
            else
            {
              strcat(values2, &buf[f2.frame_id].field[off2]);
            }
            if(j2 != (numcols(DB, srel22)-1))
            {
              strcat(values2, ",");
            }
          }//end loop for each value in srel22

          if(strcmp(values, values2) == 0)//compare values with values2
          {
            duplicate = true;
          }

          f2 = Tb.Get_next_record(DB.c_str(), srel22.c_str());
        }//end loop for each record in srel22


        if(!duplicate)
        {
          Tb.Insert_record(DB.c_str(), srel22.c_str(), values);
        }
        duplicate = false;

        f = Tb.Get_next_record(DB.c_str(), srel1.c_str());
      }//end loop for each record in srel1

      CMdata.Drop_table(DB.c_str(), srel2.c_str());
      return srel22;
    }//distinct
    */
  return srel2;
}

bool isFrameAgain(vector<Frame> vec, Frame f) {
  for (unsigned int i = 0; i < vec.size(); i++) // get all columns for the table
  {
    if (vec.at(i).frame_id == f.frame_id && vec.at(i).offset == f.offset)
      return true;
  }
  return false;
}

/*Function used to fill an array with unique random values
Note: used in determination of multiple SQL: where clause order of evaluation
*/
void randSelect(int con[100], int num) {
  int numvals = 0;
  while (numvals < num) {
    int r = rand() % num;
    if (isUnique(con, r)) {
      con[numvals] = r;
      numvals++;
    }
  }
}

/*performs "select" computations
Precondition - all where clauses have been typed in conjunctive normal form
*/
string Where(string &DB, string &rel1, SQLInfo *query) {
  int z = 0;
  string Rel2Arr[1024];
  int cntRel2Arr = 0;
  if (!isTable(DB.c_str(), rel1.c_str(),
               z)) // make sure table exists in metadata
  {
    cerr << "HUGE Where problem!!!!!!!!!!" << endl;
    return "    ";
  }

  char val[100]; // stores one buffer attribute value;

  string rel2 = "WhereRel";
  Frame f;    // used in accessing records stored in the buffer
  Record rid; // used in removing records from the temporary relation

  int con[100]; // array to hold the order of conjuncts to be evaluated
  for (int z = 0; z < 100; z++)
    con[z] = -1;
  randSelect(con, query->whrcnt);

  int pos = 0;

  int operand1, operand2;
  int posInMet1 = 0, posInMet2 = 0;
  if (isCol(DB, rel1, query->where[con[pos]].oprnd1, posInMet1)) {
    operand1 = OPERANDATT;
  } else {
    cerr << "Operand 1 must be an attribute!";
    return rel2;
  }
  if (isCol(DB, rel1, query->where[con[pos]].oprnd2, posInMet2)) {
    operand2 = OPERANDATT;
  } else {
    operand2 = OPERANDVAL;
  }

  CMdata.Create_table(DB.c_str(), rel2.c_str());
  for (unsigned int i = 0; i < MAXATTRS; i++) // get all columns for the table
  {
    if (strcmp(mdata.attrs[i].Relname, rel1.c_str()) ==
        0) // if the table is in metadata
    {
      CMdata.Add_attribute(DB.c_str(),               // DB
                           rel2.c_str(),             // table name
                           mdata.attrs[i].name,      // attribute name
                           mdata.attrs[i].TypeName,  // attribute type
                           mdata.attrs[i].length,    // int, column width
                           mdata.attrs[i].precision, // precision
                           mdata.attrs[i].position,  // position
                           mdata.attrs[i].IsPrimary, // bool, is primary
                           0,                        // bool, is not null
                           0,                        // bool, is foreign
                           0                         // bool, is unique
      );
    }
  }

  int operat = query->where[con[pos]].op;
  // displayTable(DB.c_str(),rel2.c_str());

  //  cout << "first operand: " << query->where[con[pos]].op << endl;

  f = Tb.Get_first_record(DB.c_str(), rel1.c_str());
  for (unsigned int i = 0; f.frame_id != -1;
       i++) // num tuples in relation load tuples into main memory (rel1)
  {
    bool add = false, isNumber = false;
    double qu, bu;
    int off =
        getNewOff(DB, rel1, query->where[con[pos]].oprnd1, f.offset); // att
    if (getType(DB, rel1, query->where[con[pos]].oprnd1) ==
        2) // if buffer value is a string
    {
      strcpy(val, &buf[f.frame_id].field[off]);
    } else if (getType(DB, rel1, query->where[con[pos]].oprnd1) ==
               0) // if buffer valus is a number
    {
      int temp = 0;
      memcpy(&temp, &buf[f.frame_id].field[off], sizeof(int));
      itoa(temp, val, 10);
    } else // if float, not used with current implementation
    {
      cout << "float " << val << endl;
      float temp = 0.0;
    }

    if (isNum(query->where[con[pos]].oprnd2)) // if the conditions are numbers
    {
      qu = atof(query->where[con[pos]].oprnd2.c_str());
      bu = atof(val); // convert them to floats
      isNumber = true;
    }
    switch (operand2) {
    case OPERANDATT: {
      switch (operat) {
      case OPEQ: {
        cout << "operand 1 att, operand 2 att " << operat << endl;
        break;
      }; // endl case OPEQ
      case OPNEQ: {
        cout << "operand 1 att, operand 2 att " << operat << endl;
        break;
      }; // endl case OPNEQ
      default:
        cout << "operator " << operat << " not supported." << endl;
      } // end switch operat
      break;
    }; // end case OPERANDATT

    case OPERANDVAL: {
      switch (operat) {
      case OPGT: {
        if (isNumber) {
          if (bu > qu)
            add = true;
        } else if (strcmp(query->where[con[pos]].oprnd2.c_str(), val) <
                   0) // val
        {
          add = true;
        }
        break;
      }; // endl case OPGT
      case OPLT: {
        if (isNumber) {
          if (bu < qu)
            add = true;
        } else if (strcmp(query->where[con[pos]].oprnd2.c_str(), val) >
                   0) // val
        {
          add = true;
        }
        break;
      }; // endl case OPLT
      case OPGTEQ: {
        if (isNumber) {
          if (bu >= qu)
            add = true;
        } else if (strcmp(query->where[con[pos]].oprnd2.c_str(), val) == 0 ||
                   strcmp(query->where[con[pos]].oprnd2.c_str(), val) <
                       0) // val
        {
          add = true;
        }
        break;
      }; // endl case OPGTEQ
      case OPLTEQ: {
        if (isNumber) {
          if (bu <= qu)
            add = true;
        } else if (strcmp(query->where[con[pos]].oprnd2.c_str(), val) == 0 ||
                   strcmp(query->where[con[pos]].oprnd2.c_str(), val) >
                       0) // val
        {
          add = true;
        }
        break;
      }; // endl case OPLTEQ
      case OPEQ: {
        if (strcmp(query->where[con[pos]].oprnd2.c_str(), val) == 0) // val
        {
          add = true;
        }
        break;
      }; // endl case OPEQ

      case OPNEQ: {
        if (strcmp(query->where[con[pos]].oprnd2.c_str(), val) == -1 || // val
            strcmp(query->where[con[pos]].oprnd2.c_str(), val) == 1) {
          add = true;
        }
        break;
      }; // endl case OPNEQ
      }  // end switch operat
      break;
    }; // end case OPERANDVAL
    }  // end switch operand2

    if (add) // need position retrieved from metadata
    {
      char values[FRAMESIZE];
      strcpy(values, "\0");
      int off = f.offset;
      for (int j = 0; j < numcols(DB, rel1);
           j++) // for all values for this temptable
      {
        off = getNewIOff(DB, rel1, j, f.offset);
        if (getType(DB, rel1, j) == 0) {
          int temp = 0;
          memcpy(&temp, &buf[f.frame_id].field[off], sizeof(int));
          char *tmp = (char *)malloc(sizeof(int) + 1);
          strcat(values, itoa(temp, tmp, 10));
        } else if (getType(DB, rel1, j) == 1) {
          float temp = 0.0;
          memcpy(&temp, &buf[f.frame_id].field[off], sizeof(int));
        } else {
          strcat(values, &buf[f.frame_id].field[off]);
        }
        if (j != (numcols(DB, rel1) - 1)) {
          strcat(values, ",");
        }
      }
      Rel2Arr[cntRel2Arr] = values;
      cntRel2Arr++;

      Tb.Insert_record(DB.c_str(), rel2.c_str(), values);
      // displayTable(DB.c_str(),rel2.c_str()); //for displaying temp whereRel
      // after inserting 1 record
    }
    add = false; // reset add for next tuple

    f = Tb.Get_next_record(DB.c_str(), rel1.c_str());
  } // for num tuples in rel1
  pos++;

  vector<Frame> deadFrames;
  //  Perform remaining ti comparisons on tuples in rel2
  for (unsigned int i = 1; i < query->whrcnt;
       i++) // for number of remaining where clauses
  {
    if (query->where[i - 1].logConnect == "and") {

      operat = query->where[con[i]].op; // get next operator
      //      cout << "next operand: " << query->where[con[i]].op << endl;

      if (isCol(DB, rel2, query->where[con[i]].oprnd1, posInMet1)) {
        operand1 = OPERANDATT;
      } else {
        cerr << "Operand 1 must be an attribute!";
        return rel2;
      }
      if (isCol(DB, rel2, query->where[con[i]].oprnd2, posInMet2)) {
        operand2 = OPERANDATT;
      } else {
        operand2 = OPERANDVAL;
      }

      int numValsInPag = 0; // used for slot directory
      f = Tb.Get_first_record(DB.c_str(), rel2.c_str());

      //    displayTable(DB,rel2);

      for (unsigned int m = 0; f.frame_id != -1; m++) // num tuples in relation
      {
        //      cout << "num tuples " << m << "frame " << f.frame_id << " " <<
        //      f.offset << endl;
        bool kill = true, isNumber = false;
        double qu, bu;
        int off =
            getNewOff(DB, rel2, query->where[con[i]].oprnd1, f.offset); // att
        if (getType(DB, rel2, query->where[con[i]].oprnd1) ==
            2) // if buffer value is a string
        {
          strcpy(val, &buf[f.frame_id].field[off]);
        } else if (getType(DB, rel2, query->where[con[i]].oprnd1) ==
                   0) // if buffer valus is a number
        {
          int temp = 0;
          memcpy(&temp, &buf[f.frame_id].field[off], sizeof(int));
          itoa(temp, val, 10);
        } else // if float, not used with current implementation
        {
          cout << "float " << val << endl;
          float temp = 0.0;
        }

        if (isNum(query->where[con[i]].oprnd2)) // if the conditions are numbers
        {
          qu = atof(query->where[con[i]].oprnd2.c_str());
          bu = atof(val); // convert them to floats
          isNumber = true;
        }

        switch (operand2) {
        case OPERANDATT: {
          switch (operat) {
          case OPEQ: // remaining ti comparisons
          {
            cout << "operand 1 att, operand 2 att " << operat << endl;
            break;
          };          // endl case OPEQ
          case OPNEQ: // remaining ti comparisons
          {
            cout << "operand 1 att, operand 2 att " << operat << endl;
            break;
          }; // endl case OPNEQ
          default:
            cout << "operator " << operat << " not supported." << endl;
          } // end switch operat
          break;
        }; // end case OPERANDATT

        case OPERANDVAL: {
          switch (operat) {
          case OPGT: // remaining ti comparisons
          {
            if (isNumber) // if the conditions are numbers
            {
              if (bu > qu) {
                kill = false;
              }
            } else if (strcmp(query->where[con[i]].oprnd2.c_str(), val) <
                       0) // val
            {
              kill = false;
            }
            break;
          };         // endl case OPGT
          case OPLT: // remaining ti comparisons
          {
            if (isNumber) // if the conditions are numbers
            {
              if (bu < qu) {
                kill = false;
              }
            } else if (strcmp(query->where[con[i]].oprnd2.c_str(), val) >
                       0) // val
            {
              kill = false;
            }
            //
            break;
          };           // endl case OPLT
          case OPGTEQ: // remaining ti comparisons
          {
            if (isNumber) // if the conditions are numbers
            {
              if (bu >= qu) {
                kill = false;
              }
            } else if (strcmp(query->where[con[i]].oprnd2.c_str(), val) == 0 ||
                       strcmp(query->where[con[i]].oprnd2.c_str(), val) <
                           0) // val
            {
              kill = false;
            }
            //
            break;

          };           // endl case OPGTEQ
          case OPLTEQ: // remaining ti comparisons
          {
            if (isNumber) // if the conditions are numbers
            {
              if (bu <= qu) {
                kill = false;
              }
            } else if (strcmp(query->where[con[i]].oprnd2.c_str(), val) == 0 ||
                       strcmp(query->where[con[i]].oprnd2.c_str(), val) >
                           0) // val
            {
              kill = false;
            }
            break;
          };         // endl case OPLTEQ
          case OPEQ: // remaining ti comparisons
          {
            if (strcmp(query->where[con[i]].oprnd2.c_str(), val) == 0) // val
            {
              kill = false;
            }
            break;
          }; // endl case OPEQ

          case OPNEQ: // remaining ti comparisons
          {
            if (strcmp(query->where[con[i]].oprnd2.c_str(), val) == -1 || // val
                strcmp(query->where[con[i]].oprnd2.c_str(), val) == 1) {
              kill = false;
            }
            break;
          }; // endl case OPNEQ
          }  // end switch operat
          break;
        }; // end case OPERANDVAL
        }  // end switch operand2

        numValsInPag = buf[f.frame_id].field[4092];
        if (rid.slot_num == numValsInPag) // at end of page
        {
          rid.slot_num = 0; // reset slot
          rid.page_id++;    // next page
        }
        if (isFrameAgain(deadFrames, f)) {
          cout << "\nframe: " << f.frame_id << " " << f.offset
               << " already been deleted" << endl;
          //        rid.slot_num++;
        }
        if (kill) // not satisfy any conjunct
        {
          //        cout << "call Delete_record() for: " << val << endl;
          Tb.Delete_record(DB.c_str(), rel2.c_str(), rid, 1);
          // displayTable(DB.c_str(),rel2.c_str()); //whereRel check
          deadFrames.push_back(f);
        }
        rid.slot_num++;

        f = Tb.Get_next_record(DB.c_str(), rel2.c_str());
      } // for number of tuples in rel2

      rid.slot_num = 0; // reset slot
      rid.page_id = 0;  // reset page
    }                   // if logConnect = "and"
    else if (query->where[i - 1].logConnect == "or") {
      operat = query->where[con[i]].op; // get next operator
      //      cout << "next operand: " << query->where[con[i]].op << endl;

      if (isCol(DB, rel2, query->where[con[i]].oprnd1, posInMet1)) {
        operand1 = OPERANDATT;
      } else {
        cerr << "Operand 1 must be an attribute!";
        return rel2;
      }
      if (isCol(DB, rel2, query->where[con[i]].oprnd2, posInMet2)) {
        operand2 = OPERANDATT;
      } else {
        operand2 = OPERANDVAL;
      }

      int numValsInPag = 0; // used for slot directory
      f = Tb.Get_first_record(DB.c_str(), rel1.c_str());

      for (unsigned int i = 0; f.frame_id != -1;
           i++) // num tuples in relation load tuples into main memory (rel1)
      {
        bool add = false, isNumber = false;
        double qu, bu;
        int off =
            getNewOff(DB, rel1, query->where[con[pos]].oprnd1, f.offset); // att
        if (getType(DB, rel1, query->where[con[pos]].oprnd1) ==
            2) // if buffer value is a string
        {
          strcpy(val, &buf[f.frame_id].field[off]);
        } else if (getType(DB, rel1, query->where[con[pos]].oprnd1) ==
                   0) // if buffer valus is a number
        {
          int temp = 0;
          memcpy(&temp, &buf[f.frame_id].field[off], sizeof(int));
          itoa(temp, val, 10);
        } else // if float, not used with current implementation
        {
          cout << "float " << val << endl;
          float temp = 0.0;
        }

        if (isNum(
                query->where[con[pos]].oprnd2)) // if the conditions are numbers
        {
          qu = atof(query->where[con[pos]].oprnd2.c_str());
          bu = atof(val); // convert them to floats
          isNumber = true;
        }
        switch (operand2) {
        case OPERANDATT: {
          switch (operat) {
          case OPEQ: {
            cout << "operand 1 att, operand 2 att " << operat << endl;
            break;
          }; // endl case OPEQ
          case OPNEQ: {
            cout << "operand 1 att, operand 2 att " << operat << endl;
            break;
          }; // endl case OPNEQ
          default:
            cout << "operator " << operat << " not supported." << endl;
          } // end switch operat
          break;
        }; // end case OPERANDATT

        case OPERANDVAL: {
          switch (operat) {
          case OPGT: {
            if (isNumber) {
              if (bu > qu)
                add = true;
            } else if (strcmp(query->where[con[pos]].oprnd2.c_str(), val) <
                       0) // val
            {
              add = true;
            }
            break;
          }; // endl case OPGT
          case OPLT: {
            if (isNumber) {
              if (bu < qu)
                add = true;
            } else if (strcmp(query->where[con[pos]].oprnd2.c_str(), val) >
                       0) // val
            {
              add = true;
            }
            break;
          }; // endl case OPLT
          case OPGTEQ: {
            if (isNumber) {
              if (bu >= qu)
                add = true;
            } else if (strcmp(query->where[con[pos]].oprnd2.c_str(), val) ==
                           0 ||
                       strcmp(query->where[con[pos]].oprnd2.c_str(), val) <
                           0) // val
            {
              add = true;
            }
            break;
          }; // endl case OPGTEQ
          case OPLTEQ: {
            if (isNumber) {
              if (bu <= qu)
                add = true;
            } else if (strcmp(query->where[con[pos]].oprnd2.c_str(), val) ==
                           0 ||
                       strcmp(query->where[con[pos]].oprnd2.c_str(), val) >
                           0) // val
            {
              add = true;
            }
            break;
          }; // endl case OPLTEQ
          case OPEQ: {
            if (strcmp(query->where[con[pos]].oprnd2.c_str(), val) == 0) // val
            {
              add = true;
            }
            break;
          }; // endl case OPEQ

          case OPNEQ: {
            if (strcmp(query->where[con[pos]].oprnd2.c_str(), val) ==
                    -1 || // val
                strcmp(query->where[con[pos]].oprnd2.c_str(), val) == 1) {
              add = true;
            }
            break;
          }; // endl case OPNEQ
          }  // end switch operat
          break;
        }; // end case OPERANDVAL
        }  // end switch operand2

        if (add) // need position retrieved from metadata
        {
          char values[FRAMESIZE];
          strcpy(values, "\0");
          int off = f.offset;

          for (int j = 0; j < numcols(DB, rel1);
               j++) // for all values for this temptable
          {
            off = getNewIOff(DB, rel1, j, f.offset);
            if (getType(DB, rel1, j) == 0) {
              int temp = 0;
              memcpy(&temp, &buf[f.frame_id].field[off], sizeof(int));
              char *tmp = (char *)malloc(sizeof(int) + 1);
              rid.slot_num = 0; // reset slot
              rid.page_id = 0;  // reset page
              strcat(values, itoa(temp, tmp, 10));
            } else if (getType(DB, rel1, j) == 1) {
              float temp = 0.0;
              memcpy(&temp, &buf[f.frame_id].field[off], sizeof(int));
            } else {
              strcat(values, &buf[f.frame_id].field[off]);
            }
            if (j != (numcols(DB, rel1) - 1)) {
              strcat(values, ",");
            }
          }

          int bAdd = 1;
          // if (query->distinct)
          //{
          for (int zz = 0; zz < cntRel2Arr; zz++) {
            if (strcmp(values, Rel2Arr[zz].c_str()) == 0) {
              bAdd = -1;
              break;
            }
          }
          //}
          if (bAdd == 1) {
            Tb.Insert_record(DB.c_str(), rel2.c_str(), values);
            Rel2Arr[cntRel2Arr] = values;
            cntRel2Arr++;
          }
          // displayTable(DB.c_str(),rel2.c_str());
        }
        add = false; // reset add for next tuple

        f = Tb.Get_next_record(DB.c_str(), rel1.c_str());
      }                 // for num tuples in rel1
      rid.slot_num = 0; // reset slot
      rid.page_id = 0;  // reset page
    }                   // else if logConnect = "or"

  } // for number of remaining where clauses
  return rel2;
}

/*
function to performe SQL: "from" computations
*/
string Join(string &DB, string &R, string &S, string &jrel2, bool first) {
  Frame r, s;
  //  string jrel2 = newTab;

  CMdata.Create_table(DB.c_str(), jrel2.c_str());

  int p = 0;
  for (unsigned int i = 0; i < MAXATTRS; i++) // get all columns for the table
  {
    if (strcmp(mdata.attrs[i].Relname, R.c_str()) ==
        0) // if the table is in metadata
    {
      char x[100];
      //      if(first)
      //      {
      strcpy(x, R.c_str());
      strcat(x, ".");
      strcat(x, mdata.attrs[i].name);
      //      }
      //      strcpy(x, mdata.attrs[i].name);

      //    cout << endl << x << endl << endl;
      CMdata.Add_attribute(DB.c_str(),               // DB
                           jrel2.c_str(),            // table name
                           x,                        // attribute name
                           mdata.attrs[i].TypeName,  // attribute type
                           mdata.attrs[i].length,    // int, column width
                           mdata.attrs[i].precision, // precision
                           p, // mdata.attrs[i].position,//position
                           mdata.attrs[i].IsPrimary, // bool, is primary
                           0,                        // bool, is not null
                           0,                        // bool, is foreign
                           0                         // bool, is unique
      );
      p++;
    }
  }
  for (unsigned int i = 0; i < MAXATTRS; i++) // get all columns for the table
  {
    if (strcmp(mdata.attrs[i].Relname, S.c_str()) ==
        0) // if the table is in metadata
    {
      char x[100];
      strcpy(x, S.c_str());
      strcat(x, ".");
      strcat(x, mdata.attrs[i].name);
      //      strcpy(x, mdata.attrs[i].name);
      //      cout << endl << x << endl << endl;
      CMdata.Add_attribute(DB.c_str(),               // DB{
                           jrel2.c_str(),            // tarble name
                           x,                        // attribute name
                           mdata.attrs[i].TypeName,  // attribute type
                           mdata.attrs[i].length,    // int, column width
                           mdata.attrs[i].precision, // precision
                           p, // mdata.attrs[i].position,//position
                           mdata.attrs[i].IsPrimary, // bool, is primary
                           0,                        // bool, is not null
                           0,                        // bool, is foreign
                           0                         // bool, is unique
      );
      p++;
    }
  }
  int off;
  r = Tb.Get_first_record(DB.c_str(), R.c_str());
  while (r.frame_id != -1) {
    char values[FRAMESIZE];
    char values1[FRAMESIZE];
    strcpy(values, "\0");
    strcpy(values1, "\0");
    if (r.frame_id != -1) // one tuple in R
    {
      off = r.offset;
      for (int j = 0; j < numcols(DB, R);
           j++) // for all values for this temptable
      {
        off = getNewIOff(DB, R, j, r.offset);
        if (getType(DB, R, j) == 0) {
          int temp = 0;
          memcpy(&temp, &buf[r.frame_id].field[off], sizeof(int));
          char *tmp = (char *)malloc(sizeof(int) + 1);
          strcat(values, itoa(temp, tmp, 10));
        } else if (getType(DB, R, j) == 1) {
          float temp = 0.0;
          memcpy(&temp, &buf[r.frame_id].field[off], sizeof(int));
        } else {
          strcat(values, &buf[r.frame_id].field[off]);
        }
        if (j != (numcols(DB, R) - 1)) {
          strcat(values, ",");
        }
      }
    }
    if (r.frame_id == -1) {
      for (int k = 0; k < numcols(DB, R); k++)
        strcat(values, "-,");
    } else
      strcat(values, ",");
    strcpy(values1, values);
    s = Tb.Get_first_record(DB.c_str(), S.c_str());
    while (s.frame_id != -1) {
      strcpy(values, "\0");
      strcpy(values, values1);
      if (s.frame_id != -1) // one tuple in S
      {
        off = s.offset;
        for (int j = 0; j < numcols(DB, S);
             j++) // for all values for this temptable
        {
          off = getNewIOff(DB, S, j, s.offset);
          if (getType(DB, S, j) == 0) {
            int temp = 0;
            memcpy(&temp, &buf[s.frame_id].field[off], sizeof(int));
            char *tmp = (char *)malloc(sizeof(int) + 1);
            strcat(values, itoa(temp, tmp, 10));
          } else if (getType(DB, S, j) == 1) {
            float temp = 0.0;
            memcpy(&temp, &buf[s.frame_id].field[off], sizeof(int));
          } else {
            strcat(values, &buf[s.frame_id].field[off]);
          }
          if (j != (numcols(DB, S) - 1)) {
            strcat(values, ",");
          }
        }
      } else if (s.frame_id == -1) {
        for (int k = 0; k < numcols(DB, S); k++)
          strcat(values, "-,");
      }
      //              cout << values << endl;
      Tb.Insert_record(DB.c_str(), jrel2.c_str(), values);
      s = Tb.Get_next_record(DB.c_str(), S.c_str());
    }
    r = Tb.Get_next_record(DB.c_str(), R.c_str());
  }

  //      displayTable(DB,jrel2);
  return jrel2;
}

///////////////Below are the set operations that were not
///implemented////////////////////////////////

/*
function to perform set union operations
*/
// string SetUnion(string & DB, SQLInfo* query)
//{
//   if()// not R & S same schema
//   {
//
//     for(int i; i<????; i++)
//     {
//
//
//     }
//
//     return "ERROR";
//   }
//
//   string R = query->tbl[0];
//   string S = query->tbl[1];//does not consider if there was a "rename"
//
//   Frame f,s;
//
//   string tempTabl = "tempRel";
//   CMdata.Create_table(DB.c_str(), tempTabl.c_str());
//   for(unsigned int i; i<????; i++)//each wanted attribute in rel
//   {//can not type Rel.attType if rel is
//   string??????????????????????????????????????????????????????
//     CMdata.Add_attribute(DB.c_str(), tempTabl.c_str(), query->name,
//     query->name)
//   }
//   for(unsigned int i; i<????; i++)// all tuples in R
//   {
//     if()// first iteration
//     {
//       f = Get_first_record(DB.c_str(), R.c_str());
//     }
//     else
//     {
//       f = Get_next_record(DB.c_str(), R.c_str());
//     }
//     tab.Insert_record(DB.c_str(), tempTabl.c_str(), f);
//   for(unsigned int i; i<????; i++)// all tuples in S
//   {
//     if()// first iteration
//     {
//       f = Get_first_record(DB.c_str(), S.c_str());
//     }
//     else
//     {
//       f = Get_next_record(DB.c_str(), S.c_str());
//     }
//     tab.Insert_record(DB.c_str(), tempTabl.c_str(), s);
//   }
//
//   sort(tempTabl);//using n-way merge sort remove duplicates
//
//   return tempTabl;
// }

/*
function to perform set difference operations
*/
// string setDiff(string & DB, SQLInfo* query)
//{
//   if()// not R & S same schema
//     return "ERROR";
//
//   string tR=R;//not change original tables
//   string tS=S;
//   sort(tR)//using n-way merge sort remove duplicates
//   sort(tS)//using n-way merge sort remove duplicates
//
//   string tempTabl = "tempRel";
//   CMdata.Create_table(DB.c_str(), tempTabl.c_str());
//   for(unsigned int i; i<????; i++)//each wanted attribute in rel
//   {//can not type Rel.attType if rel is
//   string???????????????????????????????????????????????
//     CMdata.Add_attribute(DB.c_str(), tempTabl.c_str(), query->name,
//     query->name);
//   }
//   for(unsigned int i; i<????; i++)// all tuples in R
//   {
//     if()// first iteration
//     {
//       f = Get_first_record(DB.c_str(), tR.c_str());
//     }
//     else
//     {
//       f = Get_next_record(DB.c_str(), tR.c_str());
//     }
//     tab.Insert_record(DB.c_str(), tempTabl.c_str(), f);
//     for(unsigned int i; i<????; i++)// all tuples in S
//     {
//       if()// first iteration
//       {
//         f = Get_first_record(DB.c_str(), tS.c_str());
//       }
//       else
//       {
//         f = Get_next_record(DB.c_str(), tS.c_str());
//       }
//       if()// r == s
//       {
//         break;//exit inner loop
//       }
//       else if()// end of inner loop
//       {
//         Insert_record(DB, tempTabl, r);
//       }
//     }
//   }
//   return tempTabl;
// }
