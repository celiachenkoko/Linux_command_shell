#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
using namespace std;
/* isvalidVar is a helper function to "set", which determines if the inputis a 
valid variable which only is the combination of letters,underscores and numbers */
bool isvalidVar(string var) {
  string::iterator it;
  for (it = var.begin(); it != var.end(); ++it) {
    if (isalnum(*it) || (*it == '_')) {
      continue;
    }
    else {
      return false;
    }
  }
  return true;
}
/* storeVar function is a helper function of "set" and "inc", it takes in the variable and its key and 
also the map container to store the <key,value>,or update the existed value in the variable map. */
map<string, string> storeVar(string key, string value, map<string, string> varMap) {
  map<string, string>::iterator it = varMap.find(key);
  //if key already exist,update its value
  if (it != varMap.end()) {
    it->second = value;
  }
  else {
    varMap.insert(std::pair<std::string, std::string>(key, value));
  }
  return varMap;
}
/*lookupMap function takes in a variable name,and the variable map to find the corresponding 
value of this variable.is the variable is not find, then add it into the map and set its value
to "" */
string lookupMap(string varname, map<string, string> varMap) {
  map<string, string>::iterator it = varMap.find(varname);
  string value;
  if (it != varMap.end()) {
    value = it->second;
  }
  else {
    value = "";
  }
  return value;
}
/*getVar takes in a line which may contain "$" and replace the potential variable to its value
while remain both the front and the behind stirng of the variable£¬recursive this proess until 
all the "$" in the line has been transfer to value. */
string getVar(string varline, map<string, string> varMap) {
  if (varline.find('$') == string::npos) {
    return varline;
  }
  string::iterator var = varline.find('$') + 1 + varline.begin();
  string::iterator varEnd = var;
  //track the valid variable's character until reach the illegal virable character
  while ((isalnum(*varEnd) || (*varEnd) == '_') && (varEnd != varline.end())) {
    varEnd++;
  }
  string varname;
  //assign the variable from the start(inclusive) to the end(exclusive)
  varname.assign(var, varEnd);
  string varFront, varBehind;
  if (!varname.empty()) {
    varFront.assign(varline.begin(), var - 1);
    varBehind.assign(varEnd, varline.end());
    string value;
    value = lookupMap(varname, varMap);
    //add the front and behind part of the variable name and replace the variable with its value
    varline = varFront + value + varBehind;
  }
  //if cannot search a legal character as variable name, then print out the rest of line
  else {
    cerr << "not valild variable name !" << endl;
    varline = varFront + varBehind;
  }
  return getVar(varline, varMap);
}
