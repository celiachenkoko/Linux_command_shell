#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
using namespace std;
/*parseCommand function:this function use to deal with multiple commands,
indent multiple white space and then split it with white space,but if the white
space is escaped with a \ ,it's included*/
void parseCommand(string str, vector<string> &myvector) {
  string::iterator it = str.begin();
  string::iterator it_end = str.end();
  string word;
  while (it != it_end) {
    if (*it == ' ') {
      ++it;
    }
    else {
      if ((*it == '\\') && (*(it + 1) == ' ')) {
       //if the '\\'is followed by a white space, remain
        word.push_back(' ');
        //skip the '\\' and ' '
        it += 2;
        //ignore the following white space, add the word back to vector
        if ((*it == ' ') || (*it == '\0')) {
          myvector.push_back(word);
          word.clear();
        }
        continue;
      }
      //normal words need to split
      else {
        word.push_back(*it);
        ++it;
        if ((*it == ' ') || (*it == '\0')) {
          myvector.push_back(word);
          word.clear();
        }
      }
    }
  }
  //return myvector;
}
/* parseEnviron takes in a char*, which represent the path of the environment
split it with ":" and store them into envVec vector */
vector<string> parseEnviron(char * env) {
  vector<string> myvector;
  string word;
  stringstream ss;
  ss << env;
  while (!ss.eof()) {
    getline(ss, word, ':');
    //cout << "word is:" << word << endl;
    myvector.push_back(word);
  }

  return myvector;
}