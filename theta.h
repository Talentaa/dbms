#ifndef ___THETA_JOIN__
#define ___THETA_JOIN__

#include <deque>
#include <fstream>
#include <iostream>
#include <set>
#include <vector>
using namespace std;

RV query_check(string IN query);

RV get_token_pos(string IN in, string IN c1_name, string IN c2_name,
                 int &OUT c1_pos, int &OUT c2_pos);

RV get_tokens(string IN in, string &OUT t1, string &OUT t2, int IN t1_pos,
              int IN p2_pos);

RV get_token_names(string IN in, string &OUT c1, string &OUT c2, int &OUT op);

#endif