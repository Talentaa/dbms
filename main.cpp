/*
Authors:
Knute Ortiz - Tokenizer, Parser, Query Evaluation Engine (QEE).
Todd Bolinger - Buffer Manager
Hyunju Nam - File & Access Module

Description:
This if a simple Database Management System that can create databases, create
tables, drop databases, drop relations, drop tables, drop attributes, insert
tuples and then perform various queries on the open database relations.

Known Issues:
1.  Index structure not implemented.
2.  QEE external sort function not implemented.
3.  Query optimization not implemented other than where conjuncts are evaluated
at random.
4.  Queries must be entered in conjunctive normal form(CNF).
5.  Primary key values are evaluated only for containing some data, the quality
of the data is not verified,
6.  user is required to insert correct data for primary keys.
7.  Set operations are not utilized in the QEE

*/

#include <iostream>
#include <string>

using namespace std;

#include "BufferManager.h"
#include "FA.h"
#include "Gvariables.h"
#include "Parser.h"
#include "QEE.h"
#include "theta.h"
#include "tokenizer.h"

int g_flag = TRUE;

RV theta_join(DbInfo *IN DB, string IN query) {
  int rv = RV_FAIL;
  try {
    if (query_check(query) == FALSE)
      return rv = RV_PASS;

    // split the given query into two subqueries.
    // example:
    // query   = "select * from t1, t2 where t1.a = t2.b;"
    // subquery1 = "select * from t1, t2;"
    // subquery2 = "t1.a = t2.b"
    int i = query.find("where");
    string subquery1(query, 0, i - 1);
    subquery1 += ";";
    string subquery2(query, i + 6, query.size());

    // get t1.a, t2.b, and =
    string c1_name, c2_name;
    int op;
    rv = get_token_names(subquery2, c1_name, c2_name, op);
    if (rv != RV_OK)
      throw "get_token_names";

    // get the Cartesian join result
    g_flag = FALSE;
    SQLInfo Q;
    Q.tokenize(subquery1, DB);
    parseSelect(DB, &Q);
    g_flag = TRUE;
    ifstream in("so-tired");

    string line;
    int linenum = 0;
    int c1_pos, c2_pos;

    while (getline(in, line)) {
      linenum++;
      if (linenum == 1 || linenum == 3)
        cout << line << endl;
      else if (linenum == 2) {
        cout << line << endl;
        get_token_pos(line, c1_name, c2_name, c1_pos, c2_pos);
      } else {
        string token1, token2;
        char buf[1024];
        strcpy(buf, line.c_str());
        get_tokens(line, token1, token2, c1_pos, c2_pos);
        if ((op == OPNEQ && token1 != token2) ||
            (op == OPGTEQ && token1 >= token2) ||
            (op == OPLTEQ && token1 <= token2) ||
            (op == OPEQ && token1 == token2) ||
            (op == OPGT && token1 > token2) || (op == OPLT && token1 < token2))
          cout << buf << endl;
      }
    }
  } catch (int err) {
    cerr << "theta_join:" << err << endl;
  } catch (char *msg) {
    cerr << "theta_join:" << msg << endl;
  } catch (...) {
    cerr << "theta_join" << endl;
  }
  return rv;
}

void clean(string &str);
string strToLower(string &str);

int main() {
  srand((unsigned)time(
      NULL)); // initialize the randomizer for QEE index selection

  mdata.InitMetadata(); // Initialize the metadata

  DbInfo DB; // need to create new database or load one

  cout << "enter \"exit;\" to quit\n" << endl;

  while (true) {
    string sqlStatement;
    char chr = ' ';

    cout << "SQL> ";
    int c = 2;
    while (chr != ';') {
      chr = getchar();
      if (chr == '\n') {
        sqlStatement += ' ';
        // cout << setw(3) << c << "  ";
        setw(3);
        c++;
      } else
        sqlStatement += chr;
    }
    cin.sync(); // clean up trailing newline
    clean(sqlStatement);

    string tempExit = sqlStatement;
    strToLower(tempExit);
    if (tempExit == "exit") {
      bufMan.writeDirtys();
      mdata.WriteMetadata();
      cout << "Saved buffer pages & metadata to HDD." << endl;
      return 1;
    }

    SQLInfo query;
    unsigned int qrytype =
        query.tokenize(sqlStatement, &DB); // tokenizes the query statement

    if (theta_join(&DB, sqlStatement) == RV_OK)
      continue;

    // Process querys here
    switch (qrytype) // calls appropriate QEE functions
    {
    case SELECT: {
      parseSelect(&DB, &query);
      break;
    };
    case INSERT: {
      DB.setModifyTime(); // set mnodify time
      parseInsert(&DB, &query);
      break;
    };
    case CREATI: {
      DB.setModifyTime(); // set mnodify time
      parseCreatIndex(&DB, &query);
      break;
    };
    case CREATT: // table
    {
      DB.setModifyTime(); // set mnodify time
      parseCreateTabl(&DB, &query);
      break;
    };
    case CREATD: {
      DB.setCreatTime();
      parseCreateDB(&DB);
      break;
    };
    case DROPDB: {
      DB.setModifyTime(); // set mnodify time
      parseDrop(&DB, qrytype, &query);
      break;
    };
    case DROPTA: {
      DB.setModifyTime(); // set mnodify time
      parseDrop(&DB, qrytype, &query);
      break;
    };
    case DROPAT: {
      DB.setModifyTime(); // set mnodify time
      parseDrop(&DB, qrytype, &query);
      break;
    };
    case DROPIN: {
      DB.setModifyTime(); // set mnodify time
      parseDrop(&DB, qrytype, &query);
      break;
    };
    case UPDATE: {
      DB.setModifyTime(); // set mnodify time
      cout << "Not Supported yet!" << endl;
      break;
    };
    case DESCDB: {
      DB.setModifyTime(); // set mnodify time
      parseDescribe(&DB, qrytype, &query);
      break;
    };
    case DESCTA: {
      DB.setModifyTime(); // set mnodify time
      parseDescribe(&DB, qrytype, &query);
      break;
    };
    case OPENDB: {
      parseOpen(&DB);
      break;
    };
    default:
      cout << "\nstatement not supported\n\n\n";
      query.display();
      qrytype = 0;
    }

    //    DB.display();
    //    query.display();
  }
  system("pause");
  return 0;
}