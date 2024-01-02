#include "Gvariables.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

static void remove_space(string &INOUT s) {
  while (1) {
    int i = s.find(" ");
    if (i == string::npos)
      break;
    s.erase(i, strlen(" "));
  }
}

RV query_check(string IN query) {
  if (query.find("select") != query.npos && query.find("from") != query.npos &&
      query.find("where") != query.npos && query.find(".") != query.npos)
    return TRUE;
  else
    return FALSE;
}

RV get_token_pos(string IN line, string IN c1_name, string IN c2_name,
                 int &OUT c1_pos, int &OUT c2_pos) {
  RV rv = RV_FAIL;
  try {
    const char *delimiters = " \t";
    vector<string> wordlist;
    // Capture individual words:
    char *s = strtok((char *)line.c_str(), delimiters);
    while (s) {
      // Automatic type conversion:
      wordlist.push_back(s);
      s = strtok(0, delimiters);
    }
    int pos = 0;
    vector<string>::iterator i;
    for (i = wordlist.begin(); i != wordlist.end(); ++i) {
      if (c1_name == *i)
        c1_pos = pos;
      if (c2_name == *i)
        c2_pos = pos;
      pos++;
    }
    rv = RV_OK;
  } catch (int err) {
    cerr << "get_token_pos:" << err << endl;
  } catch (char *msg) {
    cerr << "get_token_pos:" << msg << endl;
  } catch (...) {
    cerr << "get_token_pos" << endl;
  }
  return rv;
}

RV get_tokens(string IN line, string &OUT t1, string &OUT t2, int IN c1_pos,
              int IN c2_pos) {
  RV rv = RV_FAIL;
  try {
    const char *delimiters = " \t";
    vector<string> wordlist;
    char *s = strtok((char *)line.c_str(), delimiters);
    while (s) {
      wordlist.push_back(s);
      s = strtok(0, delimiters);
    }
    int pos = 0;
    vector<string>::iterator i;
    for (i = wordlist.begin(); i != wordlist.end(); ++i) {
      if (c1_pos == pos)
        t1 = *i;
      if (c2_pos == pos)
        t2 = *i;
      pos++;
    }
    rv = RV_OK;
  } catch (int err) {
    cerr << "get_tokens:" << err << endl;
  } catch (char *msg) {
    cerr << "get_tokens:" << msg << endl;
  } catch (...) {
    cerr << "get_tokens" << endl;
  }
  return rv;
}

RV get_token_names(string IN in, string &OUT c1, string &OUT c2, int &OUT op) {
  RV rv = RV_FAIL;
  try {
    int i;
#if 1
    i = in.find("!=");
    if (i != in.npos) {
      string tmp1(in, 0, i);
      string tmp2(in, i + 2, in.size());
      remove_space(tmp1);
      remove_space(tmp2);
      c1 = tmp1;
      c2 = tmp2;
      op = OPNEQ;
      return rv = RV_OK;
    }

    i = in.find(">=");
    if (i != in.npos) {
      string tmp1(in, 0, i);
      string tmp2(in, i + 2, in.size());
      remove_space(tmp1);
      remove_space(tmp2);
      c1 = tmp1;
      c2 = tmp2;
      op = OPGTEQ;
      return rv = RV_OK;
    }

    i = in.find("<=");
    if (i != in.npos) {
      string tmp1(in, 0, i);
      string tmp2(in, i + 2, in.size());
      remove_space(tmp1);
      remove_space(tmp2);
      c1 = tmp1;
      c2 = tmp2;
      op = OPLTEQ;
      return rv = RV_OK;
    }
#endif

    i = in.find("=");
    if (i != in.npos) {
      string tmp1(in, 0, i);
      string tmp2(in, i + 1, in.size());
      remove_space(tmp1);
      remove_space(tmp2);
      c1 = tmp1;
      c2 = tmp2;
      op = OPEQ;
      return rv = RV_OK;
    }

    i = in.find(">");
    if (i != in.npos) {
      string tmp1(in, 0, i);
      string tmp2(in, i + 1, in.size());
      remove_space(tmp1);
      remove_space(tmp2);
      c1 = tmp1;
      c2 = tmp2;
      op = OPGT;
      return rv = RV_OK;
    }

    i = in.find("<");
    if (i != in.npos) {
      string tmp1(in, 0, i);
      string tmp2(in, i + 1, in.size());
      remove_space(tmp1);
      remove_space(tmp2);
      c1 = tmp1;
      c2 = tmp2;
      op = OPLT;
      return rv = RV_OK;
    }
  } catch (int err) {
    cerr << "get_token_names:" << err << endl;
  } catch (char *msg) {
    cerr << "get_token_names:" << msg << endl;
  } catch (...) {
    cerr << "get_token_names" << endl;
  }
  return rv;
}