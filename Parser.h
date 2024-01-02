/*
Author:
Knute Ortiz - Parser.h

This file contains all of the helper functions for the Query Evaluation Engine.
*/

#pragma once

#include <iostream>
#include <string>
#include <vector>

using namespace std;

#include "Gvariables.h"
#include "QEE.h"

/*Function to set the modify time when the database is modified
 */
void setModTime(DbInfo *DB) {
  int i = 0;
  if (isDB(DB->database_name, i))
    strcpy(mdata.dbases[i].date_modified, DB->modify_date.c_str());
}

/*Function to perform SQL create a database
 */
void parseCreateDB(DbInfo *DB) {
  CMdata.Create_db(DB->database_name.c_str(), DB->database_owner.c_str(),
                   DB->modify_date.c_str(), DB->creation_date.c_str());

  int i = 0; // make sure it got in to the metadata
  isDB(DB->database_name.c_str(), i);
  cout << endl << mdata.dbases[i].name << " created." << endl << endl;
  //  List_db();//debugging
  //  DB->display();//debugging
}

/*Function used to create a table in an existing database and add the
 * appropriate attributes
 */
void parseCreateTabl(DbInfo *DB, SQLInfo *query) {
  CMdata.Create_table(DB->database_name.c_str(), query->tbl[0].c_str());

  // loop to add all of the attributes to this table
  for (unsigned int i = 0; i < query->colcnt; i++) {
    CMdata.Add_attribute(DB->database_name.c_str(),     // DB
                         query->col[i].tabname.c_str(), // table name
                         query->col[i].colname.c_str(), // attribute name
                         query->col[i].coltype.c_str(), // attribute type
                         query->col[i].colwdth,         // int, column width
                         0,                             // precision
                         i,                             // position
                         query->col[i].isPrimary,       // bool, is primary
                         query->col[i].isNotNul,        // bool, is not null
                         query->col[i].isForeign,       // bool, is foreign
                         query->col[i].isUnique         // bool, is unique
    );
  }
  int j = 0; // make sure it got in to the metadata
  isTable(DB->database_name.c_str(), query->col[0].tabname.c_str(), j);
  cout << endl
       << mdata.relations[j].DBName << " " << mdata.relations[j].name
       << " created." << endl
       << endl;

  setModTime(DB);
}

/*Function to create an Index in an existing database
 */
void parseCreatIndex(DbInfo *DB, SQLInfo *query) {
  Ind.Create_index(DB->database_name.c_str(), // DB name
                   query->tbl[0].c_str(),     // table name
                   query->name.c_str(), // index name !!!!will be empty if index
                                        // name not entered!!!!!!!!
                   query->col[0].colname.c_str()); // column name(s)

  int j = 0; // make sure it got in to the metadata
  isInd(DB->database_name.c_str(), query->name.c_str(), j);
  cout << mdata.indexes[j].DBName << " " << mdata.indexes[j].name << " created."
       << endl;

  setModTime(DB);
}

/*Function to perform SQL: drop operations
 */
void parseDrop(DbInfo *DB, int type, SQLInfo *query) {
  int i = 0, rel;
  isTable(DB->database_name.c_str(), query->name.c_str(),
          rel); // used to check if relation has tuples

  if (type == DROPDB) {
    if (!isDB(query->name.c_str(),
              i)) // if database name does not exist in the metadata
    {
      cerr << "\nDatabase: \"" << query->name << "\" does not exist!\n";
      return;
    }
    cout << mdata.dbases[i].name << " will now be deleted." << endl;
    CMdata.Drop_db(DB->database_name.c_str());
    setModTime(DB);
    return;
  }
  if (type == DROPTA) {
    // if table name does not exist in the metadata
    if (!isTable(DB->database_name.c_str(), query->name.c_str(), i)) {
      cerr << "\ntable: \"" << query->name
           << "\" does not exist in Database: \"" << DB->database_name << "\"!"
           << endl;
      return;
    }
    cout << "In Database: \"" << mdata.relations[i].DBName << "\", Table: \""
         << mdata.relations[i].name << "\" will now be deleted." << endl;

    CMdata.Drop_table(DB->database_name.c_str(), query->name.c_str());

    // List_table();//debugging
    setModTime(DB);
    return;
  }
  if (type == DROPAT) {
    if (!isCol(DB->database_name.c_str(), query->tbl[0].c_str(),
               query->name.c_str(),
               i)) // if attribute name does not exist in the metadata
    {
      cerr << "\nattribute \"" << query->name
           << "\" does not exist in Table: \"" << query->tbl[0]
           << "\", Database: \"" << DB->database_name << "\"\n";
      return;
    }
    if (mdata.relations[rel].size == 0) {
      cout << "Table \"" << mdata.attrs[i].Relname << "\" attribute \""
           << mdata.attrs[i].name
           << "\" was not deleted because there are tuples in the relation."
           << endl;
      return;
    }
    cout << "In Table \"" << mdata.attrs[i].Relname << "\" attribute \""
         << mdata.attrs[i].name << "\" will now be deleted." << endl;

    CMdata.Drop_attribute(DB->database_name.c_str(), // database
                          query->tbl[0].c_str(),     // table
                          query->name.c_str());      // attribute
    setModTime(DB);
    return;
  }
  if (type == DROPIN) { // if index name does not exist in the metadata
    if (!isInd(DB->database_name.c_str(), query->name.c_str(), i)) {
      cerr << "\nindex \"" << query->name.c_str()
           << "\" does not exist in Database: \"" << DB->database_name
           << "\"\n";
      return;
    }
    cout << "In Database: \"" << mdata.indexes[i].DBName << "\" "
         << mdata.indexes[i].name << "\" will now be deleted" << endl;

    Ind.Drop_index(
        DB->database_name.c_str(), // database
        mdata.indexes[i]
            .name, // table name????WHY metadata does not support table???
        query->name.c_str()); // index name
    setModTime(DB);
    return;
  }
  return; // error
}

/*Function used to describe a database or table in a database
 */
void parseDescribe(DbInfo *DB, int type, SQLInfo *query) {
  int i = 0;
  if (type == DESCDB) {
    if (isDB(query->name.c_str(), i)) {
      if (query->name != DB->database_name) {
        cerr << "Please open Database: \"" << query->name
             << "\" to describe its components!\n";
        return;
      }

      mdata.describeDatabase(query->name.c_str());
      return;
    }
    cerr << "Database: \"" << query->name << "\" does not exist!" << endl;
  }
  if (type == DESCTA) {
    if (isTable(DB->database_name.c_str(), query->name.c_str(), i)) {
      mdata.describeTable(DB->database_name.c_str(), query->name.c_str());
      return;
    }
    cerr << "Table: " << query->name
         << " in the database: " << DB->database_name << " does not exist!"
         << endl;
  }
}

/*Function used to open an existing database
 */
void parseOpen(DbInfo *DB) {
  bufMan.writeDirtys();
  cout << "Buffer memory saved to HDD." << endl;

  int i = 0;
  if (isDB(DB->database_name,
           i)) // if the requested database exists then load it
  {
    DB->database_owner = mdata.dbases[i].owner;
    DB->creation_date = mdata.dbases[i].create_date;
    DB->modify_date = mdata.dbases[i].date_modified;

    cout << "Database: \"" << DB->database_name << "\" loaded.\n\n";
    return;
  }
  cout << "Database \"" << DB->database_name << "\" was not found!" << endl;
  cout << "Would you like to create a new one?\nEnter \"y\" for yes, \"n\" for "
          "no."
       << endl;
  char choice = ' ';
  cin >> choice;
  if (choice == 'y' || choice == 'Y') {
    cout << "Please enter the owner name." << endl;
    cin >> DB->database_owner;
    DB->setCreatTime();
    DB->modify_date = "-";
    parseCreateDB(DB);
  }
  cin.clear(); // clears input stream in caase garbage was entered
  cin.sync();
  return;
}

/*Function used to find the primary key in a table stored in the metadata
 */
void getMetaPKey(string &DB, string &rel, vector<int> &vec) {
  for (unsigned int i = 0; i < MAXATTRS; i++) {
    if (strcmp(mdata.attrs[i].DBName, DB.c_str()) == 0 &&
        strcmp(mdata.attrs[i].Relname, rel.c_str()) == 0 &&
        mdata.attrs[i].IsPrimary) {
      vec.push_back(mdata.attrs[i].position);
    }
  }
}

/*Function used to check if the key value for a table is empty in the string
 */
bool isKeyNull(string &values, int pos) {
  int p = 0;
  string val = "";
  for (unsigned int i = 0; i < values.size(); i++) {
    if (values.at(i) == ',')
      p++;
    if (p == pos && values.at(i) != ',')
      val += values.at(i);
  }
  return val.size() == 0;
}

/*Function used to perform SQL Insert operations
 */
void parseInsert(DbInfo *DB, SQLInfo *query) {
  vector<int> vec;
  getMetaPKey(DB->database_name, query->name, vec);
  for (unsigned int i = 0; i < vec.size(); i++) {
    if (isKeyNull(query->col[0].colvalue, vec.at(i))) {
      cerr << "ERROR! Primary key value is empty." << endl;
      return;
    }
  }

  // loop will iterate only 1 time cause ALL tuple values are stored in the
  // first col[]
  for (unsigned int i = 0; i < query->colcnt; i++) {
    Tb.Insert_record(DB->database_name.c_str(),       // database name
                     query->name.c_str(),             // table name
                     query->col[i].colvalue.c_str()); // value(s)
  }
  setModTime(DB);
  cout << endl << "1 row created." << endl << endl;
}

/*Function used to perform SQL: Select query operations
 */
void parseSelect(DbInfo *DB, SQLInfo *query) {
  //  query->display();
  //  system("pause");

  bool needToDrop = false;
  string joinTable = query->tbl[0];
  string wherTable = "";
  string selTable = "";
  string newT = "z";
  vector<string> tables;

  if (query->tblcnt > 1) // if need "Join", more than one table
  {
    joinTable =
        Join(DB->database_name, query->tbl[0], query->tbl[1], newT, true);
    tables.push_back(joinTable);
    for (unsigned int i = 2; i < query->tblcnt; i++) {
      newT += "a";
      joinTable =
          Join(DB->database_name, joinTable, query->tbl[i], newT, false);
      tables.push_back(joinTable);
    }
  }

  if (query->select[0].colName == "*" && query->whrcnt == 0) {
    displayTable(DB->database_name, joinTable);
  } else if (query->select[0].colName == "*" && query->whrcnt > 0) {
    wherTable = Where(DB->database_name, joinTable, query);
    displayTable(DB->database_name, wherTable);
    // delete temp relation(table) after done
    CMdata.Drop_table(DB->database_name.c_str(), wherTable.c_str());
  } else if (query->selcnt > 0 && query->whrcnt == 0) {
    selTable = Select(DB->database_name, joinTable, query);
    displayTable(DB->database_name, selTable);
    // delete temp relation(table) after done
    CMdata.Drop_table(DB->database_name.c_str(), selTable.c_str());
  } else if (query->selcnt > 0 && query->whrcnt > 0) {
    wherTable = Where(DB->database_name, joinTable, query);

    selTable = Select(DB->database_name, wherTable, query);

    displayTable(DB->database_name, selTable);
    // delete temp relation(table) after done
    CMdata.Drop_table(DB->database_name.c_str(), wherTable.c_str());
    CMdata.Drop_table(DB->database_name.c_str(), selTable.c_str());
  }
  // loop to delete all of the table names stored in the vector
  for (unsigned int k = 0; k < tables.size(); k++)
    CMdata.Drop_table(DB->database_name.c_str(), tables.at(k).c_str());
}

/* This function prints the Attributes referenced by index
 */
void Metadata::printAttr(int index) {
  cout << left << setw(36) << attrs[index].name << "      ";
  if (attrs[index].IsPrimary)
    cout << setw(9) << "not null";
  else
    cout << setw(9) << "null ok";

  string tn = attrs[index].TypeName;

  cout << attrs[index].TypeName << "(" << attrs[index].length << ")";
  cout << right << setw(15 - tn.size()) << attrs[index].IsPrimary;
  //  cout << setw(15) <<  "position" << right << setw(2) <<
  //  attrs[index].position;
}

/* This function is used when the user calls describe database
   the database is printed out
*/
void Metadata::describeDatabase(const char DBName[100]) {
  int i = 0;
  /* This loop prints out the database info */
  for (i = 0; i < MAXDBS; i++) {
    if (!strcmp(dbases[i].name, DBName)) {
      printDB(i);
      break;
    }
  }

  /* This loop prints out tables that are related to the database */
  for (i = 0; i < MAXRELS; i++) {
    if (!strcmp(relations[i].DBName, DBName) && strcmp(relations[i].name, "")) {
      cout << "Table: " << relations[i].name << endl;
      describeTable(DBName, relations[i].name);
    }
  }
  /* This loop prints out all related indexes */
  for (i = 0; i < MAXINDEXES; i++) {
    if (!strcmp(indexes[i].DBName, DBName)) {
      printIndex(i);
    }
  }
}

/* This function is used when the user calls describe table
   the table is printed out
*/
void Metadata::describeTable(const char dname[100], const char tname[100]) {
  int i = 0;
  /* This loop prints out the table */
  for (i = 0; i < MAXRELS; i++) {
    if (!strcmp(relations[i].DBName, dname) &&
        !strcmp(relations[i].name, tname)) {
      /*   printRel(i);*/
      break;
    }
  }
  cout << "Name                                      Null?    Type             "
          "PK FK"
       << endl;
  cout << "----------------------------------------- -------- ---------------- "
          "-- -----------"
       << endl;
  /* This loop prints out all related atts */
  for (i = 0; i < MAXATTRS; i++) {
    if (!strcmp(attrs[i].DBName, dname) && !strcmp(attrs[i].Relname, tname) &&
        strcmp(attrs[i].name, "")) {
      printAttr(i);
      cout << endl;
    }
  }
  cout << endl;

  /* This loop prints out all related ons */
  for (i = 0; i < MAXONS; i++) {
    if (!strcmp(ons[i].DBName1, dname) && !strcmp(ons[i].DBName2, dname) &&
        !strcmp(ons[i].Relname, tname)) {
      printOn(i);
    }
  }
  /* No need to print the types */
}