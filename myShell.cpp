#include <dirent.h>
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
extern char ** environ;
void parseCommand(string str, vector<string> & myvector);
vector<string> parseEnviron(char * env);
map<string, string> storeVar(string key, string value, map<string, string> varMap);
string lookupMap(string varname, map<string, string> varMap);
string getVar(string varline, map<string, string> varMap);
bool isvalidVar(string var);
/*type_prompt() function:this function provide the interface of Myshell,
and it also show the current directory before the $*/
void type_prompt() {
  char cwd[1024];
  getcwd(cwd, sizeof(cwd));
  std::cout << "myShell:" << cwd << "$";
}
/*searchCommand takes in the envVec vector and the command vector,
if the command contain a '/'in it, just set bool value to ture and return the original argv vector
else, if the command is not complete, than open each directory according to the PATH envVector and 
find if the command is in it, if ture, than complete the command.*/
pair<bool, vector<string> > searchCommand(vector<string> dirname,
                                          vector<string> argvVec) {  //pair<bool,string>
  pair<bool, vector<string> > findArgv;
  findArgv.second = argvVec;
  struct dirent * entry;
  DIR * dp;
  string command = argvVec[0];
  if (dirname[0][0] != '/') {
    cerr << "can't find the path" << endl;
    findArgv.first = false;
    return findArgv;
  }
  for (vector<string>::iterator it = dirname.begin(); it != dirname.end(); it++) {
    //open each directory under the dirname vector
    dp = opendir((*it).c_str());
    if (dp == NULL) {
      std::cout << "Can't open " << *it << std::endl;
      findArgv.first = false;
      closedir(dp);
      return findArgv;
    }

    while ((entry = readdir(dp)) != NULL) {
      std::string file_name(entry->d_name);
      if (file_name == "." || file_name == "..") {
        continue;
      }
      //if its not complete path,go search
      if (strcmp(entry->d_name, command.c_str()) == 0) {
        command = *it + "/" + command;
        argvVec[0] = command;
        findArgv.first = true;
        (findArgv.second)[0] = command;
        closedir(dp);
        return findArgv;
      }
      //complete path which contain '/'
      else if (command.find('/') != string::npos) {
        findArgv.first = true;
        closedir(dp);
        return findArgv;
      }
    }
    closedir(dp);
  }
  findArgv.first = false;
  return findArgv;
}
/*vec2charp is a helper function to convert a vector to the char** type */
char ** vec2charp(vector<string> vec) {
  char ** charp = new char *[vec.size() + 1];
  for (unsigned i = 0; i < vec.size(); i++) {
    charp[i] = new char[vec[i].length() + 1];
    strcpy(charp[i], vec[i].c_str());
  }
  charp[vec.size()] = NULL;
  return charp;
}
/* runCommand function takes in the command argument and the environment parameter
it runs normal command other than built-in commands */
void runCommand(char ** args, char ** envp) {
  pid_t cpid, w;
  int wstatus;
  cpid = fork();
  if (cpid == -1) {
    perror("fork");
    exit(EXIT_FAILURE);
  }
  else if (cpid == 0) { /* Code executed by child */
    execve(args[0], args, envp);
    perror("execve");
  }
  else { /* Code executed by parent */
    do {
      w = waitpid(cpid, &wstatus, WUNTRACED | WCONTINUED);
      if (w == -1) {
        perror("waitpid");
        exit(EXIT_FAILURE);
      }
      if (WIFEXITED(wstatus)) {
        cout << "Program exited with status " << WTERMSIG(wstatus) << endl;
      }
      else {
        cout << "Program was killed by signal " << WEXITSTATUS(wstatus) << endl;
      }
    } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));
  }
}
/* runCdCommand takes in the argv Vector, it can change the current directory and
show the directory on the pormpt */
void runCdCommand(vector<string> argvVec) {
  string direct;
  //if cd takes one argument, return to its home path
  if (argvVec.size() == 1) {
    direct = getenv("HOME");
  }
  //cd can take at most one argument
  else if (argvVec.size() == 2) {
    direct = argvVec[1];
  }
  else if (argvVec.size() > 2) {
    cerr << "invalid number of argument for cd command!" << endl;
  }
  if (chdir(direct.c_str()) == -1) {
    cerr << argvVec[1] << "no such directory!" << endl;
  }
}

/*runSetCommand takes in the command vetor, the variable map and also the input string
it parse the input string if it contains "$", and then split the variable and let
the rest of the string be the value of this variable. Then, store the variable and its value
into the varMap.*/
void runSetCommand(vector<string> & argvVec, string input, map<string, string> & varMap) {
  if (argvVec.size() < 3) {
    cerr << "too few arguments for set command" << endl;
  }
  else if (!isvalidVar(argvVec[1])) {
    cerr << "wrong format of the variable" << endl;
  }
  else {
    //seperate the variable and the rest of string as its value
    size_t varpos = input.find(argvVec[1]);
    input = input.substr(varpos);
    size_t cut = input.find_first_of(' ');
    string value = input.substr(cut + 1);
    varMap = storeVar(argvVec[1], value, varMap);
  }
}

/*runExportCommand function takes in the command vector and variable map. first lookup the varmap map to find if the
variable has already been set, if not print out error. If it find in the map, set the variable as well as its vakue
to the environment */
void runExportCommand(vector<string> argvVec, map<string, string> varMap) {
  if (argvVec.size() != 2) {
    cerr << "wrong number of arguments! export can only take two arguments!" << endl;
    return;
  }
  string varname = argvVec[1];
  if (!isvalidVar(argvVec[1])) {
    cerr << "wrong format of the variable" << endl;
    return;
  }
  else {
    //if the value in the varMap is not set before
    string value = lookupMap(varname, varMap);
    if (!strcmp(value.c_str(), "")) {
      cerr << "can not export variable before set!" << endl;
      return;
    }
    if (setenv(varname.c_str(), value.c_str(), 1) != 0) {
      std::cerr << "fail to export environemnt variable" << endl;
    }
    cout << "export variable " << varname << " with value " << value << endl;
  }
}

/*is int is the helper function of "inc", which takes a string and determine if this string
consist all digit and base 10.if yes,return ture, else false. */
bool isint(string key) {
  //if it only contain a '-', then it's wrong
  if (key.length() == 1 && key[0] == '-') {
    return false;
  }
  for (unsigned i = 1; i < key.size(); i++) {
    if (isdigit(key[i])) {
      continue;
    }
    else {
      return false;
    }
  }
  //test negative number,which has a '-' at the start
  if (key[0] == '-') {
    return true;
  }
  //if the first sign is not '-', than it's false
  else {
    return false;
  }
}
/*runIncCommand takes in the command vector and the variable map. It first determine
if the variable is in the varMap, and then look up if the varible consist only number
If the variable in the command only contains number and is set before, then add the number by one,
if not, just initial it to 0,update its value to the varMap and then increase by one.*/
void runIncCommand(vector<string> argvVec, map<string, string> & varMap) {
  if (argvVec.size() != 2) {
    cerr << "wrong number of arguments! inc can only take two arguments!" << endl;
    return;
  }
  string varname = argvVec[1];
  string ans;
  if (!isvalidVar(argvVec[1])) {
    cerr << "wrong format of the variable" << endl;
    return;
  }
  //is valid varname
  else {
    //find the varname in map
    map<string, string>::iterator it = varMap.find(varname);
    int value = 0;
    //if the value of this variable valid number(base 10)
    if (it != varMap.end() && isint(it->second)) {
      //transfer string to int
      value = atoi((it->second).c_str());
      value += 1;
      stringstream s;
      s << value;
      ans = s.str();
      cout << ans << endl;
      //update the value of this variable
      varMap = storeVar(varname, ans, varMap);
    }
    else {  //value not exist or not base 10 number
      value += 1;
      stringstream ss;
      ss << value;
      ans = ss.str();
      varMap = storeVar(varname, ans, varMap);
      cout << ans << endl;
    }
  }
}

int main(void) {
  string input;
  vector<string> envVec;
  map<string, string> varMap;
  bool status = true;
  if (cin.fail()) {
    cerr << "invalid input";
  }
  while (status) {
    type_prompt();
    vector<string> argvVec;
    //initialize the ECE551PATH to the PATH in env
    setenv("ECE551PATH", getenv("PATH"), 1);
    char * env = getenv("ECE551PATH");
    envVec = parseEnviron(env);
    string input;
    getline(cin, input);
    if (cin.eof()) {
      return EXIT_SUCCESS;
    }
    parseCommand(input, argvVec);
    //if empty command
    if (argvVec.empty()) {
      continue;
    }
    //if "exit" command,exit the program
    if ((!strcmp(argvVec[0].c_str(), "exit"))) {
      return EXIT_SUCCESS;
    }
    //cd command
    if (!strcmp(argvVec[0].c_str(), "cd")) {
      runCdCommand(argvVec);
      continue;
    }
    //set command
    if (!strcmp(argvVec[0].c_str(), "set")) {
      runSetCommand(argvVec, input, varMap);
      continue;
    }
    //export command
    if (!strcmp(argvVec[0].c_str(), "export")) {
      runExportCommand(argvVec, varMap);
      continue;
    }
    //inc command
    if (!strcmp(argvVec[0].c_str(), "inc")) {
      runIncCommand(argvVec, varMap);
      continue;
    }
    //excute normal command
    else {
      //parse the variable with its value
      for (unsigned i = 0; i < argvVec.size(); i++) {
        argvVec[i] = getVar(argvVec[i], varMap);
      }
      pair<bool, vector<string> > findArgv;
      findArgv = searchCommand(envVec, argvVec);
      char ** argv = vec2charp(findArgv.second);
      //if the command exist
      if (findArgv.first) {
        runCommand(argv, environ);
      }
      //if the command does not exist
      else {
        cerr << "command " << argv[0] << " not found" << endl;
      }
      //delete char** argv after use
      for (unsigned i = 0; i < argvVec.size(); i++) {
        delete[] argv[i];
      }
      delete[] argv;
    }
  }
  return EXIT_SUCCESS;
}
