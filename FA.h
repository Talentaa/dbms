
/*****************************************************************************************
Program : File and Access Methods Management
Author : Hyunju Nam
Begin Date : Mar/28/2005
Last Update : May/2/2005
Comments : Every DB , table or column name is case sensitive
       Most important function is the Insert_record function and it is
complicate.
*****************************************************************************************/

#pragma once

#include "BufferManager.h"
#include <iostream>

#include <math.h>
#include <string>

using namespace std;

//////////////////////////////////////////////////////////////////////////////
// File & Access Global structs & variables

const int SPECIALVALUE =
    FRAMESIZE -
    1; // This is the special value for initializing of offsets. : "4095"
const int DELETED = -FRAMESIZE + 1;   // the offset value of deleted data
int numPid = FRAMESIZE / sizeof(int); // page numbers of the Index file
const int ExceptM = 2; // this is for protecting the slot of M value (passing
                       // the first record of slot_num is 0 therefore 2)

struct Frame {
  Frame() {
    frame_id = 0;
    offset = 0;
  }

  int frame_id;
  int offset;
};

struct Record {
  Record() {
    page_id = 0;
    slot_num = 0;
  }
  int page_id;
  int slot_num;
};

struct Rtype {
  char type[100];
  int length;
};

Record current_rid[MAXRELS]; // This is for current record, it has the Global
                             // page_id value.

//////////////////////////////////////////////////////////////////////////////
// Classes

class ControlMetadata {
public:
  ControlMetadata();
  int Create_db(const char *DB, const char *Owner, const char *Mdate,
                const char *Cdate);
  int Drop_db(const char *DB);
  int Create_table(const char *DB, const char *Table);
  int Drop_table(const char *DB, const char *Table);
  int Add_attribute(const char *DB, const char *Table, const char *AttName,
                    const char *AttType, int leng, int prec, int position,
                    int IsPrimary, int IsNotNull, int IsForeign, int IsUnique);
  int Drop_attribute(const char *DB, const char *Table, const char *AttName);

private:
};

class Tables {
public:
  Tables();
  Frame Get_record(const char *DB, const char *Table, Record rid, int Islock);
  int Delete_record(const char *DB, const char *Table, Record rid, int Islock);
  int Insert_record(const char *DB, const char *Table,
                    const char *Record); // actual record
  int Insert_join_record(const char *DB, const char *Table, const char *Lrecord,
                         const char *Rrecord);
  Frame Get_first_record(const char *DB, const char *Table);
  Frame Get_next_record(const char *DB, const char *Table);
  Frame Get_prev_record(const char *DB, const char *Table);

private:
};

class Indexes {
public:
  Indexes();
  int Create_index(const char *DB, const char *Table, const char *IndName,
                   const char *key);
  int Drop_index(const char *DB, const char *Table, const char *IndName);
  Record Fetch(const char *DB, const char *Table, const char *IndName,
               const char *key);
  Record Get_next(const char *DB, const char *Table, const char *IndName);
  Record Get_prev(const char *DB, const char *Table, const char *IndName);
  int Ind_insert(const char *DB, const char *Table, const char *IndName,
                 const char *key, Record rid);
  int Ind_delete(const char *DB, const char *Table, const char *IndName,
                 const char *key);

private:
};

// constructor
ControlMetadata::ControlMetadata() {}
Tables::Tables() {}
Indexes::Indexes() {}

ControlMetadata CMdata;
Tables Tb;
Indexes Ind;

//////////////////////////////////////////////////////////////////////////////
// Implementing Meta Functions

int ControlMetadata::Create_db(const char *DB, const char *Owner,
                               const char *Mdate, const char *Cdate) {
  int result1, result2; // result1 : compare DB name (0 : same), result2 = find
                        // empty space
  int ret = 0;

  for (int i = 0; i < MAXDBS; i++) {
    result1 = strcmp(mdata.dbases[i].name, DB);

    if (result1 == 0)
      break;
  }

  if (result1 != 0) {
    for (int j = 0; j < MAXDBS; j++) {
      result2 = strcmp(mdata.dbases[j].name, "");
      if (result2 == 0) {
        strcpy(mdata.dbases[j].name, DB);
        strcpy(mdata.dbases[j].owner, Owner);
        strcpy(mdata.dbases[j].date_modified, Mdate);
        strcpy(mdata.dbases[j].create_date, Cdate);
        break;
      }
    }
  } else {
    ret = 1; // DB name duplicated
    cout << "DB name duplicated"
         << "\n";
  }
  return ret;
}

int ControlMetadata::Drop_db(const char *DB) {
  int result; // compare DB name ( 0 : same)
  int ret = 0;
  int i, j, k;

  for (i = 0; i < MAXDBS; i++) {
    result = strcmp(mdata.dbases[i].name, DB);

    if (result == 0)
      break;
  }

  if (result == 0) {
    strcpy(mdata.dbases[i].name, "\0");
  } else {
    ret = 1; // DB name does not exist.
    cout << "DB name does not exist."
         << "\n";
  }

  return ret;
}

int ControlMetadata::Create_table(const char *DB, const char *Table) {
  int result1, result22, result2,
      result3; // result1 : compare db name (0 : same), result2 = compare table
               // name (0 : same), result3: find empty space
  int ret = 0;
  NewPage newPage;

  int i, j, k;
  for (i = 0; i < MAXDBS; i++) {
    result1 = strcmp(mdata.dbases[i].name, DB);

    if (result1 == 0)
      break;
  } // find the DB

  if (result1 == 0) {
    for (j = 0; j < MAXRELS; j++) {
      result22 = strcmp(mdata.relations[j].DBName, DB);
      result2 = strcmp(mdata.relations[j].name, Table);

      if (result2 == 0)
        break;
    } // find Table
    if (result22 != 0 || result2 != 0) {
      for (k = 0; k < MAXRELS; k++) {
        result3 = strcmp(mdata.relations[k].name, "");

        if (result3 == 0) {
          strcpy(mdata.relations[k].DBName, DB);
          strcpy(mdata.relations[k].name, Table);

          ////////////////////////////////////////////////////////////////////////////////
          // It will be corrected the above.
          // When the table is created (Create_Table function)
          // do
          // 1. relations[k].first_page = FixNewPage(); First_page is for an
          // index page of the relation.
          // 2. Every Gpid (size:sizeof(int)) in the index page sould be
          // initilized 0

          newPage = bufMan.FixNewPage();
          mdata.relations[k].first_page =
              newPage.page_id; // the Firstpage of File Directory.
          // mdata.relations[k].first_page = 0;//initially 0
          // initialize Gpid = "\0"
          numPid = FRAMESIZE / sizeof(int);
          int zero = 0;
          for (int m = 0; m < numPid; m++) {
            memcpy(&buf[newPage.frame_id].field[sizeof(int) * m], &zero,
                   sizeof(int));
          } // Gpids of the index page initialize = 0
          mdata.relations[k].last_page = 0;
          break;
        }
      }
    } else {
      ret = 2; // Table name duplicated
      cout << "Table name duplicated"
           << "\n";
    }
  } else {
    ret = 1; // DB name does not exist.
    cout << "DB name does not exist."
         << "\n";
  }
  return ret;
}
int ControlMetadata::Drop_table(const char *DB, const char *Table) {
  int result1, result22, result2; // result1 : compare DB name ( 0 : same),
                                  // result2 : compare Table name
  int ret = 0;
  int i, j, k;
  for (i = 0; i < MAXDBS; i++) {
    result1 = strcmp(mdata.dbases[i].name, DB);

    if (result1 == 0)
      break;
  }

  if (result1 == 0) {
    for (j = 0; j < MAXRELS; j++) {
      result22 = strcmp(mdata.relations[j].DBName, DB);
      result2 = strcmp(mdata.relations[j].name, Table);

      if (result22 == 0 && result2 == 0)
        break;
    } // find Table
    if (result22 == 0 && result2 == 0) {
      strcpy(mdata.relations[j].name, "\0");
      mdata.relations[j].first_page = -1; // initialize
      mdata.relations[j].last_page = -1;  // initialize
      for (k = 0; k < MAXATTRS; k++) {
        if (!strcmp(mdata.attrs[k].DBName, DB) &&
            !strcmp(mdata.attrs[k].Relname, Table)) {
          strcpy(mdata.attrs[k].name, "\0");
        }
      }
    } else {
      ret = 2; // Table name does not exist.
      cout << "Table name does not exist."
           << "\n";
    }

  } else {
    ret = 1; // DB name does not exist.
    cout << "DB name does not exist."
         << "\n";
  }

  return ret;
}

int ControlMetadata::Add_attribute(const char *DB, const char *Table,
                                   const char *AttName, const char *AttType,
                                   int leng, int prec, int position,
                                   int IsPrimary, int IsNotNull, int IsForeign,
                                   int IsUnique) {
  int result, result1, result2, result3, result4, ret1, ret2;
  // result1 : compare db name (0 : same), result2 = compare table name (0 :
  // same), result3: compare attribute name (0 : same),
  int ret = 0;
  int i, j, k;
  int mPage, Gpid;
  Frame dir;

  // Type check first!
  for (i = 0; i < MAXTYPES; i++) {
    result = strcmp(mdata.types[i].name, AttType);

    if (result == 0)
      break;
  }

  if (result == 0) { // Mete Type exists.
    for (j = 0; j < MAXRELS; j++) {
      result1 = strcmp(mdata.relations[j].DBName, DB);
      result2 = strcmp(mdata.relations[j].name, Table);

      // Gpid check - if it is not zero, then Add_attribute and Drop_attribute
      // can not work.
      mPage = mdata.relations[j].first_page;   // metadata's first page
      dir.frame_id = bufMan.FixPage(mPage, 1); // lock
      memcpy(&Gpid, &buf[dir.frame_id].field[0],
             sizeof(int)); // the first data page

      if (result1 == 0 && result2 == 0)
        break;
    } // find the DB and the Table

    if (result1 == 0 && result2 == 0 && Gpid == 0) {
      for (k = 0; k < MAXATTRS; k++) {
        ret1 = strcmp(mdata.attrs[k].DBName, DB);
        ret2 = strcmp(mdata.attrs[k].Relname, Table);
        result3 = strcmp(mdata.attrs[k].name, AttName); // Attribute name
                                                        // compare

        if (ret1 == 0 && ret2 == 0 && result3 == 0) {
          ret = 3; // Attribute name duplicated
          cout << "Attribute name duplicated."
               << "\n";
          break;
        } else {
          if (ret1 != 0 || ret2 != 0 || result3 != 0) {
            result4 = strcmp(mdata.attrs[k].name, "");

            if (result4 == 0) {
              strcpy(mdata.attrs[k].DBName, DB);
              strcpy(mdata.attrs[k].Relname, Table);
              strcpy(mdata.attrs[k].name, AttName);
              mdata.attrs[k].length = leng;
              mdata.attrs[k].precision = prec;
              mdata.attrs[k].position = position;
              mdata.attrs[k].IsPrimary = IsPrimary;
              strcpy(mdata.attrs[k].TypeName, AttType);
              break;
            }
          }
        }
      }
    } else if (result1 == 0 && result2 == 0 && Gpid != 0) {
      cout << "This table already have some data, so you can't modify the "
              "attribute."
           << "\n";
    } else {
      ret = 2; // Table no exist.
      cout << "The DB name or Table name no exis."
           << "\n";
    }
  } else {
    ret = 1; // Meta Type no exist.
    cout << "Meta Type no exist."
         << "\n";
  }
  return ret;
}

int ControlMetadata::Drop_attribute(const char *DB, const char *Table,
                                    const char *AttName) {
  int result1, result2, result22, result3, result33,
      result333; // result1 : compare DB name ( 0 : same), result2 : compare
                 // Table name
  int ret = 0, mark = 0;

  int mPage, Gpid;
  Frame dir;
  int i, j, k;
  for (i = 0; i < MAXDBS; i++) {
    result1 = strcmp(mdata.dbases[i].name, DB);

    if (result1 == 0)
      break;
  }

  if (result1 == 0) {
    for (j = 0; j < MAXRELS; j++) {
      result22 = strcmp(mdata.relations[j].DBName, DB);
      result2 = strcmp(mdata.relations[j].name, Table);

      // Gpid check - if it is not zero, then Add_attribute and Drop_attribute
      // can not work.
      mPage = mdata.relations[i].first_page;   // metadata's first page
      dir.frame_id = bufMan.FixPage(mPage, 1); // lock
      memcpy(&Gpid, &buf[dir.frame_id].field[0],
             sizeof(int)); // the first data page

      if (result22 == 0 && result2 == 0)
        break;
    } // find Table
    if (result22 == 0 && result2 == 0 && Gpid == 0) {
      for (k = 0; k < MAXATTRS; k++) {
        result333 = strcmp(mdata.attrs[k].DBName, DB);
        result33 = strcmp(mdata.attrs[k].Relname, Table);
        result3 = strcmp(mdata.attrs[k].name, AttName);

        if (result333 == 0 && result33 == 0 && result3 == 0) {
          cout << "Attribute " << mdata.attrs[k].name << " was deleted"
               << "\n";
          strcpy(mdata.attrs[k].DBName, "\0");
          strcpy(mdata.attrs[k].Relname, "\0");
          strcpy(mdata.attrs[k].name, "\0");
          mdata.attrs[k].length = 0;
          mdata.attrs[k].precision = 0;
          mdata.attrs[k].position = 0;
          mdata.attrs[k].IsPrimary = 0;
          strcpy(mdata.attrs[k].TypeName, "\0");
          mark = 1;
        }
      }
      if (mark == 0) {
        ret = 3; // Attribute Name does not exist.
        cout << "Attribute " << AttName << " does not exist."
             << "\n";
      }
    } else if (result22 == 0 && result2 == 0 && Gpid != 0) {
      cout << "This table already have some data, so you can't modify the "
              "attribute."
           << "\n";
    } else {
      ret = 2; // Table name does not exist.
      cout << "Table name does not exist."
           << "\n";
    }

  } else {
    ret = 1; // DB name does not exist.
    cout << "DB name does not exist."
         << "\n";
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////////////
// For testing

void List_db() {
  cout << "\n"
       << "DB List : ";
  for (int i = 0; i < MAXDBS; i++) {
    cout << "\n"
         << mdata.dbases[i].name << "\t" << mdata.dbases[i].owner << "\t"
         << mdata.dbases[i].date_modified << "\t" << mdata.dbases[i].create_date
         << "\n";
  }
}

void List_table() {
  cout << "Table List :"
       << "\n";
  for (int i = 0; i < MAXRELS; i++) {
    cout << mdata.relations[i].DBName << "\n";
    cout << mdata.relations[i].name << "\n\n";
  }
}
void List_Attr(const char *DB, const char *Table) {
  int result1, result2;

  cout << "Attribute List :"
       << "\n";
  for (int i = 0; i < MAXATTRS; i++) {

    result1 = strcmp(mdata.attrs[i].DBName, DB);
    result2 = strcmp(mdata.attrs[i].Relname, Table);
    if (result1 == 0 && result2 == 0)
      cout << mdata.attrs[i].name << "\n";
  }
}

////////////////////////////////////////////////////////////////////////////////
// 1.Create GradDB
// 2.     Create Student, Course, and  Takes tables.
// 3.       Add attributes every table
// 4.         Insert data
// 5.       Test other functions
void TestFA() {
  mdata.InitMetadata();

  string x = "GradDB";
  CMdata.Create_db(x.c_str(), "hyunju", "04062005", "04062005"); // Create
                                                                 // GradDB
  CMdata.Create_db("WYO", "hyunju", "04062005", "04062005"); // Create GradDB
  // List_db();
  CMdata.Create_table("GradDB", "Student");
  CMdata.Create_table("GradDB", "Course");
  CMdata.Create_table("GradDB", "Takes");
  // CMdata.Create_table( "WYO", "Student" );
  // List_table();

  CMdata.Add_attribute("GradDB", "Student", "StudentID", "int", 1, 0, 0, 1, 1,
                       0, 1);
  CMdata.Add_attribute("GradDB", "Student", "FirstName", "char", 50, 0, 0, 0, 0,
                       0, 0);
  CMdata.Add_attribute("GradDB", "Student", "LastName", "char", 50, 0, 0, 0, 0,
                       0, 0);
  CMdata.Add_attribute("GradDB", "Student", "Major", "char", 2, 0, 0, 0, 0, 0,
                       0);
  cout << "Student"
       << "\n";
  List_Attr("GradDB", "Student");
  cout << "\n";

  CMdata.Add_attribute("GradDB", "Course", "CourseNum", "int", 1, 0, 0, 1, 1, 0,
                       1);
  CMdata.Add_attribute("GradDB", "Course", "CourseName", "char", 50, 0, 0, 0, 0,
                       0, 0);
  CMdata.Add_attribute("GradDB", "Course", "InstructorName", "char", 50, 0, 0,
                       0, 0, 0, 0);
  cout << "Course"
       << "\n";
  List_Attr("GradDB", "Course");
  cout << "\n";

  CMdata.Add_attribute("GradDB", "Takes", "StudentID", "int", 1, 0, 0, 1, 1, 1,
                       0);
  CMdata.Add_attribute("GradDB", "Takes", "CourseNum", "int", 1, 0, 0, 1, 1, 1,
                       0);
  cout << "Takes"
       << "\n";
  List_Attr("GradDB", "Takes");
  cout << "\n";
  // CMdata.Add_attribute( "GradDB", "Takes", "CourseNum2", "int", 1, 0, 0, 1,
  // 1, 1, 0 ); CMdata.Drop_attribute( "GradDB", "Takes", "CourseNum2");

  // RecordSize("GradDB", "Student");
  // RecordType("GradDB", "Student",1);

  Tb.Insert_record("GradDB", "Student", "01,John,Johnson,CS");
  Tb.Insert_record("GradDB", "Student", "02,Joe,Johnson,CS");
  Tb.Insert_record("GradDB", "Student", "03,Young,Son,EE");
  Tb.Insert_record("GradDB", "Student", "04,An,Jason,EE");
  Tb.Insert_record("GradDB", "Student", "05,Kim,Birly,PS");
  Tb.Insert_record("GradDB", "Student", "06,Lee,John,PS");
  Tb.Insert_record("GradDB", "Student", "07,Nataly,Anderson,ME");
  Tb.Insert_record("GradDB", "Student", "08,Senon,Dorothy,ME");
  Tb.Insert_record("GradDB", "Student", "09,Mick,Doson,CS");
  Tb.Insert_record("GradDB", "Student", "10,Don,Feona,CS");
  Tb.Insert_record("GradDB", "Student", "11,John,Johnson,CS");
  Tb.Insert_record("GradDB", "Student", "12,Joe,Johnson,CS");
  Tb.Insert_record("GradDB", "Student", "13,Young,Son,EE");
  Tb.Insert_record("GradDB", "Student", "14,An,Jason,EE");
  Tb.Insert_record("GradDB", "Student", "15,Kim,Birly,PS");
  Tb.Insert_record("GradDB", "Student", "16,Lee,John,PS");
  Tb.Insert_record("GradDB", "Student", "17,Nataly,Anderson,ME");
  Tb.Insert_record("GradDB", "Student", "18,Senon,Dorothy,ME");
  Tb.Insert_record("GradDB", "Student", "19,Mick,Doson,CS");
  Tb.Insert_record("GradDB", "Student", "20,Don,Feona,CS");
  Tb.Insert_record("GradDB", "Student", "21,John,Johnson,CS");
  Tb.Insert_record("GradDB", "Student", "22,Joe,Johnson,CS");
  Tb.Insert_record("GradDB", "Student", "23,Young,Son,EE");
  Tb.Insert_record("GradDB", "Student", "24,An,Jason,EE");
  Tb.Insert_record("GradDB", "Student", "25,Kim,Birly,PS");
  Tb.Insert_record("GradDB", "Student", "26,Lee,John,PS");
  Tb.Insert_record("GradDB", "Student", "27,Nataly,Anderson,ME");
  Tb.Insert_record("GradDB", "Student", "28,Senon,Dorothy,ME");
  Tb.Insert_record("GradDB", "Student", "29,Mick,Doson,CS");
  Tb.Insert_record("GradDB", "Student", "30,Don,Feona,CS");
  Tb.Insert_record("GradDB", "Student", "31,John,Johnson,CS");
  Tb.Insert_record("GradDB", "Student", "32,Joe,Johnson,CS");
  Tb.Insert_record("GradDB", "Student", "33,Young,Son,EE");
  Tb.Insert_record("GradDB", "Student", "34,An,Jason,EE");
  Tb.Insert_record("GradDB", "Student", "35,Kim,Birly,PS");
  Tb.Insert_record("GradDB", "Student", "36,Lee,John,PS");
  Tb.Insert_record("GradDB", "Student", "37,Nataly,Anderson,ME");
  Tb.Insert_record("GradDB", "Student",
                   "38,Senon,Dorothy,ME"); // new data page needed from here!
  Tb.Insert_record("GradDB", "Student", "39,Mick,Doson,CS");
  Tb.Insert_record("GradDB", "Student", "40,Don,Feona,CS");

  Tb.Insert_record("GradDB", "Course", "01,Found of Computing,Cowles00");
  Tb.Insert_record("GradDB", "Course", "02,Software Enginerring,Cowles032");
  Tb.Insert_record("GradDB", "Course", "03,Computing Theory,Wilson");
  Tb.Insert_record("GradDB", "Course", "04,Database System,Yu");
  Tb.Insert_record("GradDB", "Course",
                   "05,Algorithms and data structures,Peterson");
  Tb.Insert_record("GradDB", "Course", "11,Internship in Computing,Davidson");
  Tb.Insert_record("GradDB", "Course", "12,Funcional Programming,Davidson");
  Tb.Insert_record("GradDB", "Course", "13,Operating System,Yu");
  Tb.Insert_record(
      "GradDB", "Course",
      "14,Design & Implementation in Emerging Environments,Donovan");
  Tb.Insert_record("GradDB", "Course",
                   "21,Graphical Interface Programming,Olson");
  Tb.Insert_record("GradDB", "Course",
                   "22,Software Development Project,Peterson");
  Tb.Insert_record("GradDB", "Course",
                   "23,Programming Language Processors,Spears");
  Tb.Insert_record("GradDB", "Course", "24,Computer Networks,Davidson");

  Tb.Insert_record("GradDB", "Takes", "01,01");
  Tb.Insert_record("GradDB", "Takes", "01,02");
  Tb.Insert_record("GradDB", "Takes", "01,03");
  Tb.Insert_record("GradDB", "Takes", "01,04");
  Tb.Insert_record("GradDB", "Takes", "01,05");
  Tb.Insert_record("GradDB", "Takes", "02,11");
  Tb.Insert_record("GradDB", "Takes", "02,12");
  Tb.Insert_record("GradDB", "Takes", "02,13");
  Tb.Insert_record("GradDB", "Takes", "02,24");
  Tb.Insert_record("GradDB", "Takes", "03,12");
  Tb.Insert_record("GradDB", "Takes", "04,12");
  Tb.Insert_record("GradDB", "Takes", "04,13");
  Tb.Insert_record("GradDB", "Takes", "04,14");
  Tb.Insert_record("GradDB", "Takes", "04,24");
  Tb.Insert_record("GradDB", "Takes", "05,01");
  Tb.Insert_record("GradDB", "Takes", "05,02");
  Tb.Insert_record("GradDB", "Takes", "05,22");
  Tb.Insert_record("GradDB", "Takes", "06,01");
  Tb.Insert_record("GradDB", "Takes", "06,24");
  Tb.Insert_record("GradDB", "Takes", "07,03");
  Tb.Insert_record("GradDB", "Takes", "07,04");
  Tb.Insert_record("GradDB", "Takes", "07,13");
  Tb.Insert_record("GradDB", "Takes", "07,24");
  Tb.Insert_record("GradDB", "Takes", "08,11");
  Tb.Insert_record("GradDB", "Takes", "08,12");
  Tb.Insert_record("GradDB", "Takes", "09,24");
  Tb.Insert_record("GradDB", "Takes", "10,14");
  Tb.Insert_record("GradDB", "Takes", "10,23");

  // Record rec;
  // rec.page_id = 1;
  // rec.slot_num = 0;
  // Tb.Delete_record("GradDB", "Takes", rec, 1) ; //Tested!!!
  // Tb.Insert_record ("GradDB", "Takes", "10,24") ; //Tested :inserted this
  // data in the deleted record position. Tb.Get_record("GradDB", "Student",
  // rec, 1); //Tested!!! Tb.Delete_record("GradDB", "Student", rec, 1) ;
  // //Tested!!! Tb.Get_record("GradDB", "Student", rec, 1); //Tested!!!
  // Tb.Get_first_record("GradDB", "Student") ; //Tested!!!
  // Tb.Get_first_record("GradDB", "Course") ; //Tested!!!
  // Tb.Get_first_record("GradDB", "Takes") ; //Tested!!!
  // Tb.Delete_record("GradDB", "Student", rec, 1) ; //Tested!!!

  // Tb.Get_first_record("GradDB", "Student") ; //Tested!!!
  // Tb.Get_next_record("GradDB", "Student"); //Tested!!!
  // Tb.Get_next_record("GradDB", "Student"); //Tested!!!
  // Tb.Get_next_record("GradDB", "Student"); //Tested!!!
  // Tb.Get_prev_record("GradDB", "Student"); //Tested!!!
  // Tb.Get_prev_record("GradDB", "Student"); //Tested!!!
  // Tb.Get_prev_record("GradDB", "Student"); //Tested!!!
}

//////////////////////////////////////////////////////////////////////////////
// Implementing Tables Functions

// Calculate the record size
int RecordSize(const char *DB, const char *Table) {
  int result1, result2, result3, recordSize = 0;
  int i, j, k;

  //  cout << "the size of Record" << "\n";

  for (i = 0; i < MAXATTRS; i++) {
    result1 = strcmp(mdata.attrs[i].DBName, DB);
    result2 = strcmp(mdata.attrs[i].Relname, Table);

    if (result1 == 0 && result2 == 0) {
      for (j = 0; j < MAXTYPES; j++) {
        result3 = strcmp(mdata.attrs[i].TypeName, mdata.types[j].name);

        if (result3 == 0)
          break;
      }
      recordSize += (mdata.attrs[i].length * mdata.types[j].size);
    }
  }
  // cout << "recordSize = " << recordSize <<"\n";
  return recordSize;
}

// Find the record type
Rtype RecordType(const char *DB, const char *Table, int Position) {
  int result1, result2;
  Rtype recordType;
  int count = 1;
  int i, j, k;

  for (i = 0; i < MAXATTRS; i++) {
    result1 = strcmp(mdata.attrs[i].DBName, DB);
    result2 = strcmp(mdata.attrs[i].Relname, Table);
    if (result1 == 0 && result2 == 0) {

      strcpy(recordType.type, mdata.attrs[i].TypeName);
      recordType.length = mdata.attrs[i].length;
      if (count == Position)
        break;

      count++;

      // cout << "recordType = " << recordType.type << " recordLength = " <<
      // recordType.length <<"\n";
    }
  }
  return recordType;
}

// For Insert_record function
// do
// 1. FixPage(first_page) : This is looking for the index page of the relation.

// When the first record is inserted
// do
// 2. If every Gpid has 0 value then call FixNewPage() function for the first
// data page.
// 3. Put the Gpid of the first data page in the index page
// 3. For the data page, input M value first. (M value : M shows how many
// records you can have in the data page )
//     Calculate M value : valueM = ( FRAMESIZE - sizeof(int) ) / ( recordSize +
//     sizeof(int) ); Position of M value :
//     buf[frame_id].field[(FRAMESIZE-sizeof(int))] In this project, it would be
//     4092
// 3. Other offsets value in the slot directory sould be SPECIALVALUE(4095 : It
// means empty)
// 4. Next, the offset of the first record inserted in the slot directory 0 and
// it would be also 0.
// 5. Insert the record with checking the data type and size of metadata and
// attribute property.
//     In case the type is int or float, memcpy function should be used. (If you
//     use other methods such as strcpy, it has garbage value.)

// When other records are inserted
// do
// 2. Check the Gpid, if it is not 0 value, then loop the data page of the Gpid
// until M value
// 3. Check every offset value as looping, if it has SPECIALVALUE(4095) or
// DELETED(-4095) then put the new record in there.
// 4. If there is no empty space and no SPECIALVALUE(4095) or DELETED(-4095)
// value, call FixNewPage() for the next data page.
// 5. Repeate like the first record insert.

int Tables::Insert_record(const char *DB, const char *Table,
                          const char *Record) {

  //  cout << "Inside of InsertRecord " << Record << endl;
  int result1, result2,
      recordSize = 0; // result1 : compare db name (0 : same), result2 = compare
                      // table name (0 : same)
  int ret = 0, idxPage = 0, valueM, mPage, Gpid, M;
  NewPage newPage;
  Frame dir;
  Frame data;
  int svalue = SPECIALVALUE;
  int i, j, k;

  recordSize = RecordSize(DB, Table); // Calculate one recordsize

  for (i = 0; i < MAXRELS; i++) {
    result1 = strcmp(mdata.relations[i].DBName, DB);
    result2 = strcmp(mdata.relations[i].name, Table);

    if (result1 == 0 && result2 == 0)
      break;
  } // find DB & Table name

  if (result1 == 0 && result2 == 0) {
    mPage = mdata.relations[i].first_page;   // metadata's first page
    dir.frame_id = bufMan.FixPage(mPage, 1); // lock
    bufMan.setDirty(dir.frame_id);

    for (j = 0; j < numPid; j++) {
      memcpy(&Gpid, &buf[dir.frame_id].field[sizeof(int) * j],
             sizeof(int)); // Gpid

      if (Gpid == 0) {
        newPage = bufMan.FixNewPage();
        bufMan.setDirty(newPage.frame_id);
        // insert the new Gpid in the indexed page
        memcpy(&buf[dir.frame_id].field[sizeof(int) * j], &newPage.page_id,
               sizeof(int));

        // Calculate M value & insert M value in the first slot
        valueM = (FRAMESIZE - sizeof(int)) / (recordSize + sizeof(int));
        memcpy(&buf[newPage.frame_id].field[(FRAMESIZE - sizeof(int))], &valueM,
               sizeof(int)); // 4092 127
        ////Slot directory initialize = SPECIALVALUE
        for (k = 0; k < valueM; k++) {
          memcpy(
              &buf[newPage.frame_id].field[FRAMESIZE - (sizeof(int) * (k + 2))],
              &svalue, sizeof(int)); // 4095
        }

        // before inserting, call "void BMgr::setDirty(int frame_id)"
        bufMan.setDirty(newPage.frame_id);

        // insert record
        char *strData;
        strData = (char *)malloc(strlen(Record) + 1);
        strcpy(strData, Record);
        char *result = NULL;
        result = strtok(strData, ",");

        int n = 0, m = 1, value;
        float fvalue;

        // int testvalue; //for testing
        // char str[50]; //for testing
        // float testfvalue; //for testing

        while (result != NULL) {
          Rtype recordType;
          recordType = RecordType(DB, Table, m);
          m++;

          if (strcmp(recordType.type, "int") == 0) {
            value = atoi(result);
            memcpy(&buf[newPage.frame_id].field[n], &value, sizeof(int));

            ////for testing
            // memcpy(&testvalue, &buf[newPage.frame_id].field[n], sizeof(int));
            // cout << n <<" Value: " << testvalue << "\n" ;
            ////for testing

            n += sizeof(int);

          } else if (strcmp(recordType.type, "char") == 0) {
            strcpy(&buf[newPage.frame_id].field[n], result);
            //      cout << "result is " << result << endl;
            ////for testing
            // strcpy(str, &buf[newPage.frame_id].field[n]);
            // cout << n <<" Value: " << str << "\n" ;
            ////for testing

            n += sizeof(char) * recordType.length;
          } else if (strcmp(recordType.type, "float") == 0) {
            fvalue = (float)atof(result);
            memcpy(&buf[newPage.frame_id].field[n], &fvalue, sizeof(float));

            ////for testing
            // memcpy(&testfvalue, &buf[newPage.frame_id].field[n],
            // sizeof(float)); cout << n <<" Value: " << testfvalue << "\n" ;
            ////for testing

            n += sizeof(float);
          } else {
            cout << "It is not avaiable type!"
                 << "\0";
          }
          result = strtok(NULL, ",");
        } // test o.k!!!
        // insert 0 for the first offset value
        int offset = 0;
        memcpy(&buf[newPage.frame_id].field[FRAMESIZE - (sizeof(int) * 2)],
               &offset, sizeof(int)); // 4088
        // cout << "Data inserted" <<"\n";
        break;
      } else {
        data.frame_id = bufMan.FixPage(Gpid, 1);
        memcpy(&M, &buf[data.frame_id].field[FRAMESIZE - (sizeof(int))],
               sizeof(int)); // fine the M value in the first slot

        for (k = 1; k < M + 1; k++) {
          int value;
          memcpy(&value,
                 &buf[data.frame_id].field[FRAMESIZE - (sizeof(int) * (k + 1))],
                 sizeof(int)); // 4084
          data.offset = value;

          if (data.offset == SPECIALVALUE ||
              data.offset == DELETED) // SPECIALVALUE = 4095, DELETED = -4095
            break;
        }
        if (data.offset == SPECIALVALUE || data.offset == DELETED) {

          // before inserting, call "void BMgr::setDirty(int frame_id)"
          bufMan.setDirty(data.frame_id);

          // insert offset value in the slot directory
          data.offset = recordSize * (k - 1);
          memcpy(&buf[data.frame_id].field[FRAMESIZE - (sizeof(int) * (k + 1))],
                 &data.offset, sizeof(int));

          // insert
          char *strData;
          strData = (char *)malloc(strlen(Record) + 1);
          strcpy(strData, Record);
          char *result = NULL;
          result = strtok(strData, ",");
          int n = data.offset, m = 1, value;
          float fvalue;

          // int testvalue; //for testing
          // char str[50]; //for testing
          // float testfvalue; //for testing

          while (result != NULL) {
            Rtype recordType;
            recordType = RecordType(DB, Table, m);
            m++;
            if (strcmp(recordType.type, "int") == 0) {
              value = atoi(result);
              memcpy(&buf[data.frame_id].field[n], &value, sizeof(int));

              ////for testing
              // memcpy(&testvalue, &buf[data.frame_id].field[n], sizeof(int));
              // cout << n <<" Value: " << testvalue << "\n" ;
              ////for testing

              n += sizeof(int);
            } else if (strcmp(recordType.type, "char") == 0) {
              strcpy(&buf[data.frame_id].field[n], result);
              //          cout << "result is " << result << endl;
              ////for testing
              // strcpy(str, &buf[data.frame_id].field[n]);
              // cout << n <<" Value: " << str << "\n" ;
              ////for testing

              n += sizeof(char) * recordType.length;
            } else if (strcmp(recordType.type, "float") == 0) {
              fvalue = (float)atof(result);
              memcpy(&buf[data.frame_id].field[n], &fvalue, sizeof(float));

              ////for testing
              // memcpy(&testfvalue, &buf[data.frame_id].field[n],
              // sizeof(float)); cout << n <<" Value: " << testfvalue << "\n" ;
              ////for testing

              n += sizeof(float);
            } else {
              cout << "It is not an available type!"
                   << "\0";
            }
            result = strtok(NULL, ",");
          } // test o.k!!!
          break;
        } // if
      }
    }
  }
  return ret;
}

Frame Tables::Get_record(const char *DB, const char *Table, Record rid,
                         int Islock) {
  Frame temp;
  int mPage, Gpid; // mPage: the first_page in Metadata, Gpid : Global Page id

  int result1, result2, offset; // result1 : compare db name (0 : same), result2
                                // = compare table name (0 : same)
  int ret = 0;
  int i, j, k;
  for (i = 0; i < MAXRELS; i++) {
    result1 = strcmp(mdata.relations[i].DBName, DB);
    result2 = strcmp(mdata.relations[i].name, Table);

    if (result1 == 0 && result2 == 0)
      break;
  } // find DB & Table name

  if (result1 == 0 && result2 == 0) {
    mPage = mdata.relations[i].first_page; // metadata's first page
    temp.frame_id = bufMan.FixPage(mPage, Islock);
    // if frame_id value is negative then "There is no record."
    memcpy(&Gpid, &buf[temp.frame_id].field[sizeof(int) * rid.page_id],
           sizeof(int)); // Gpid

    temp.frame_id = bufMan.FixPage(Gpid, Islock);
    memcpy(&offset,
           &buf[temp.frame_id]
                .field[FRAMESIZE - (sizeof(int) * (rid.slot_num + ExceptM))],
           sizeof(int)); // Getting the offset value
    temp.offset = offset;
    if (temp.offset == SPECIALVALUE) {
      cout << "There is no record. Check it again!"
           << "\n";
    } else if (temp.offset == DELETED) {
      //      cout << "This data is deleted." <<"\n"; //for
      //      testing//////////////////////KNUTE////////////////////////////////////////
    } else {
      current_rid[i].page_id = Gpid;
      current_rid[i].slot_num = rid.slot_num;
      // cout << "temp.frame_id = " << temp.frame_id << " Current paige_id : "
      // << Gpid <<"\n"; cout << "temp.offset = " << temp.offset  <<" Current
      // slot_num : " << rid.slot_num  <<"\n";
    }
  }
  return temp;
}

// If data is deleted then the offset value will be -4095
int Tables::Delete_record(const char *DB, const char *Table, Record rid,
                          int Islock) {
  Frame temp;
  Frame check;
  int mPage, Gpid; // mPage: the first_page in Metadata, Gpid : Global Page id

  int result1, result2; // result1 : compare db name (0 : same), result2 =
                        // compare table name (0 : same)
  int ret = 0;

  // int testvalue; //for testing
  int i, j, k;

  for (i = 0; i < MAXRELS; i++) {
    result1 = strcmp(mdata.relations[i].DBName, DB);
    result2 = strcmp(mdata.relations[i].name, Table);

    if (result1 == 0 && result2 == 0)
      break;
  } // find Table

  if (result1 == 0 && result2 == 0) {
    mPage = mdata.relations[i].first_page; // metadata's first page
    temp.frame_id = bufMan.FixPage(mPage, Islock);
    memcpy(&Gpid, &buf[temp.frame_id].field[sizeof(int) * rid.page_id],
           sizeof(int)); // Gpid

    temp.frame_id = bufMan.FixPage(Gpid, Islock);

    // Check it first, there is avaiable data for deleting.
    check = Tb.Get_record(DB, Table, rid, 1);
    if (check.offset == SPECIALVALUE) {
      cout << "There is no data so you cannot delete it. "
           << "\n";
    } else {
      memcpy(&buf[temp.frame_id]
                  .field[FRAMESIZE - (sizeof(int) * (rid.slot_num + ExceptM))],
             &DELETED, sizeof(int)); // SPECIALVALUE=-4095

      ////for
      ///testing//////////////////////////////////////////////////////////////////////////////////////////
      // memcpy(&testvalue,
      // &buf[temp.frame_id].field[FRAMESIZE-(sizeof(int)*(rid.slot_num+ExceptM))],
      // sizeof(int)); cout << " Value: " << testvalue << "\n" ;
      ////for testing
    }
  } else {
    ret = 1;
    cout << "Cannot delete the record"
         << "\n";
  }

  return ret;
}

Frame Tables::Get_first_record(const char *DB, const char *Table) {
  Frame dir;
  Frame data;

  int mPage, Gpid,
      offset; // mPage: the first_page in Methadata, Gpid : Global Page id

  int result1, result2; // result1 : compare db name (0 : same), result2 =
                        // compare table name (0 : same)
  int ret = 0, CKpid = 0, M, k = 0;
  int i, j;

  for (i = 0; i < MAXRELS; i++) {
    result1 = strcmp(mdata.relations[i].DBName, DB);
    result2 = strcmp(mdata.relations[i].name, Table);

    if (result1 == 0 && result2 == 0)
      break;
  } // find Table

  if (result1 == 0 && result2 == 0) {
    mPage = mdata.relations[i].first_page;   // metadata's first page
    dir.frame_id = bufMan.FixPage(mPage, 1); // Lock the page

    // numPid = FRAMESIZE/sizeof(int) ; //the number of Page_id in the indexed
    // file.
    for (j = 0; j < numPid; j++) {
      memcpy(&Gpid, &buf[dir.frame_id].field[sizeof(int) * j],
             sizeof(int)); // Gpid

      if (Gpid != 0) {
        data.frame_id = bufMan.FixPage(Gpid, 1);
        memcpy(&M, &buf[data.frame_id].field[FRAMESIZE - sizeof(int)],
               sizeof(int)); // the slot address for M = 4092

        do {
          memcpy(&offset,
                 &buf[data.frame_id]
                      .field[FRAMESIZE - (sizeof(int) * (k + ExceptM))],
                 sizeof(int)); // Getting the first value
          data.offset = offset;
          k++;

        } while (data.offset == DELETED && k < M + 1); // loop M times
        break;
      }
    } // find the Global page_id which has the first record.
    current_rid[i].page_id = Gpid;
    current_rid[i].slot_num = k - 1;

    ////For testing
    // cout << "data.frame_id = " << data.frame_id << "Cuurent Page_id : " <<
    // Gpid << "\n"; cout << "data.offset = " << data.offset << "Current
    // slot_num : " << k-1 << "\n";
  }
  return data;
}

// This Get_next_record function returns frame_id and offset of the next record
// If there is no more record(meet the SPECIALVALUE), then it returns -1 both of
// frame_id and offset values.
Frame Tables::Get_next_record(const char *DB, const char *Table) {

  Frame data;
  Frame dir;
  int mPage, Gpid, Cpid; // mPage: the first_page in Metadata, Gpid : Global
                         // Page id, Cpid : Current Page id

  int result1, result2, offset; // result1 : compare db name (0 : same), result2
                                // = compare table name (0 : same)
  int ret = 0, M;

  int i, j, k;
  for (i = 0; i < MAXRELS; i++) {
    result1 = strcmp(mdata.relations[i].DBName, DB);
    result2 = strcmp(mdata.relations[i].name, Table);

    if (result1 == 0 && result2 == 0)
      break;
  } // find DB & Table name

  if (result1 == 0 && result2 == 0) {
    mPage = mdata.relations[i].first_page; // metadata's first page
    dir.frame_id = bufMan.FixPage(mPage, 1);

    Cpid = current_rid[i].page_id;
    data.frame_id = bufMan.FixPage(Cpid, 1);

    // check slot_num
    memcpy(&M, &buf[data.frame_id].field[FRAMESIZE - (sizeof(int))],
           sizeof(int)); // M value

    if (current_rid[i].slot_num == M - 1) {
      for (j = 0; j < numPid; j++) {
        memcpy(&Gpid, &buf[dir.frame_id].field[sizeof(int) * j],
               sizeof(int)); // Gpid

        if (Cpid == Gpid) {
          memcpy(&Cpid, &buf[dir.frame_id].field[sizeof(int) * (j + 1)],
                 sizeof(int)); // find the next datapage id
          data.frame_id = bufMan.FixPage(Cpid, 1);
          // slot check
          // check slot_num
          memcpy(&M, &buf[data.frame_id].field[FRAMESIZE - (sizeof(int))],
                 sizeof(int)); // M value

          for (k = 0; k < M; k++) {
            memcpy(&offset,
                   &buf[data.frame_id]
                        .field[FRAMESIZE - (sizeof(int) * (k + ExceptM))],
                   sizeof(int));
            current_rid[i].slot_num = k;
            if (offset == SPECIALVALUE) {
              offset = -1;        // No more record.
              data.frame_id = -1; // No more record.
              break;
            } else if (offset != DELETED) {
              break;
            }
          }
          break;
        }
      }
    } else {
      memcpy(
          &offset,
          &buf[data.frame_id]
               .field[FRAMESIZE - (sizeof(int) *
                                   ((current_rid[i].slot_num + 1) + ExceptM))],
          sizeof(int));
      current_rid[i].slot_num = current_rid[i].slot_num + 1;
      if (offset == SPECIALVALUE) {
        offset = -1;        // No more record.
        data.frame_id = -1; // No more record.
        current_rid[i].slot_num = current_rid[i].slot_num - 1;
      } else if (offset == DELETED) {
        for (int m = 0; m < M; m++) {
          memcpy(&offset,
                 &buf[data.frame_id]
                      .field[FRAMESIZE -
                             (sizeof(int) *
                              ((current_rid[i].slot_num + m) + ExceptM))],
                 sizeof(int));
          current_rid[i].slot_num = current_rid[i].slot_num + m;
          //        data.frame_id =
          //        -1;/////////////////////////////KNUTE/////////////////////////////////
          //        return
          //        data;/////////////////////////////////////KNUTE/////////////////////////////////
          if (offset != DELETED)
            break;
        }
      }
    }
    if (M == 0) {
      offset = -1;
      data.frame_id = -1;
    }
    data.offset = offset;
    current_rid[i].page_id = Cpid;

    ////for testing
    // cout << "data.frame_id = " << data.frame_id << " Current paige_id : " <<
    // Cpid <<"\n"; cout << "data.offset = " << data.offset  <<" Current
    // slot_num : " << current_rid[i].slot_num <<"\n";
  }
  return data;
}

Frame Tables::Get_prev_record(const char *DB, const char *Table) {

  Frame data;
  Frame dir;
  int mPage, Gpid, Cpid; // mPage: the first_page in Metadata, Gpid : Global
                         // Page id, Cpid : Current Page id

  int result1, result2, offset; // result1 : compare db name (0 : same), result2
                                // = compare table name (0 : same)
  int ret = 0, M;

  int i, j, k;
  for (i = 0; i < MAXRELS; i++) {
    result1 = strcmp(mdata.relations[i].DBName, DB);
    result2 = strcmp(mdata.relations[i].name, Table);

    if (result1 == 0 && result2 == 0)
      break;
  } // find DB & Table name

  if (result1 == 0 && result2 == 0) {
    mPage = mdata.relations[i].first_page; // metadata's first page
    dir.frame_id = bufMan.FixPage(mPage, 1);

    Cpid = current_rid[i].page_id;
    data.frame_id = bufMan.FixPage(Cpid, 1);

    // check slot_num
    memcpy(&M, &buf[data.frame_id].field[FRAMESIZE - (sizeof(int))],
           sizeof(int)); // M value

    if (current_rid[i].slot_num == 0) {
      memcpy(&Gpid, &buf[dir.frame_id].field[0],
             sizeof(int)); // the first data page
      if (Cpid == Gpid) {
        cout << "There is no previous record."
             << "\n";
        offset = 0;
      } else {
        for (j = 1; j < numPid; j++) {
          memcpy(&Gpid, &buf[dir.frame_id].field[sizeof(int) * j],
                 sizeof(int)); // Gpid

          if (Cpid == Gpid) {
            memcpy(&Cpid, &buf[dir.frame_id].field[sizeof(int) * (j - 1)],
                   sizeof(int)); // find the previous datapage id
            data.frame_id = bufMan.FixPage(Cpid, 1);
            // slot check
            // check slot_num
            memcpy(&M, &buf[data.frame_id].field[FRAMESIZE - (sizeof(int))],
                   sizeof(int)); // M value

            for (int k = M - 1; k > -1; k--) {
              memcpy(&offset,
                     &buf[data.frame_id]
                          .field[FRAMESIZE - (sizeof(int) * (k + ExceptM))],
                     sizeof(int));
              current_rid[i].slot_num = k;
              if (offset != DELETED)
                break;
            }
            break;
          }
        }
      }
    } else {
      memcpy(
          &offset,
          &buf[data.frame_id]
               .field[FRAMESIZE - (sizeof(int) *
                                   ((current_rid[i].slot_num - 1) + ExceptM))],
          sizeof(int));
      current_rid[i].slot_num = current_rid[i].slot_num - 1;
      if (offset == DELETED) {
        for (int m = 0; m < M; m++) {
          memcpy(&offset,
                 &buf[data.frame_id]
                      .field[FRAMESIZE -
                             (sizeof(int) *
                              ((current_rid[i].slot_num - m) + ExceptM))],
                 sizeof(int));
          current_rid[i].slot_num = current_rid[i].slot_num - m;
          if (offset != DELETED)
            break;
        }
      }
    }
    data.offset = offset;
    current_rid[i].page_id = Cpid;

    ////for testing
    // cout << "data.frame_id = " << data.frame_id << " Current paige_id : " <<
    // Cpid <<"\n"; cout << "data.offset = " << data.offset  <<" Current
    // slot_num : " << current_rid[i].slot_num <<"\n";
  }
  return data;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////////FUTURE WORK///////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int Tables::Insert_join_record(const char *DB, const char *Table,
                               const char *Lrecord, const char *Rrecord) {
  int ret = 0;
  return ret;
}

//////////////////////////////////////////////////////////////////////////////
// Implementing Indexes Functions

int Indexes::Create_index(const char *DB, const char *Table,
                          const char *IndName, const char *key) {
  int ret = 0;
  return ret;
}

int Indexes::Drop_index(const char *DB, const char *Table,
                        const char *IndName) {
  int ret = 0;
  return ret;
}

Record Indexes::Fetch(const char *DB, const char *Table, const char *IndName,
                      const char *key) {
  Record temp;
  temp.page_id = 0;
  temp.slot_num = 0;

  return temp;
}

Record Indexes::Get_next(const char *DB, const char *Table,
                         const char *IndName) {
  Record temp;
  temp.page_id = 0;
  temp.slot_num = 0;

  return temp;
}

Record Indexes::Get_prev(const char *DB, const char *Table,
                         const char *IndName) {
  Record temp;
  temp.page_id = 0;
  temp.slot_num = 0;

  return temp;
}

int Indexes::Ind_insert(const char *DB, const char *Table, const char *IndName,
                        const char *key, Record rid) {
  int ret = 0;
  return ret;
}

int Indexes::Ind_delete(const char *DB, const char *Table, const char *IndName,
                        const char *key) {
  int ret = 0;
  return ret;
}
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Main

// int main()
//{
//
//   //TestFA();
//   //cout <<"hello";
//   //char ch;
//   //cin >> ch;
//
// }