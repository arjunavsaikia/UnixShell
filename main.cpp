#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <sys/wait.h>
#include <cstring>
#include <fcntl.h>
#include <dirent.h>
#include <termios.h>
#include<bits/stdc++.h>


using namespace std;




bool is_executable(const string & path)
{
  return access(path.c_str(),X_OK)==0;
}
string find_in_path(const string &cmd)
{
  char* p=getenv("PATH");
  if(!p) return "";
  stringstream ss(p);
  string dir;

  while(getline(ss,dir,':')){
    string full_path = dir +"/" +cmd;
    if(is_executable(full_path)){
        return full_path;
    }
  }
  return "";
}
bool startsWith(const string& s, const string& prefix) {return s.rfind(prefix,0)==0;}

struct termios orig;

void enableRawMode(){
  tcgetattr(STDIN_FILENO,& orig);
  struct termios raw = orig;
  raw.c_lflag &= ~(ICANON | ECHO); 
  tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}
void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig);
}
string longestCommonPrefix(const vector<string>& matches) {
    if (matches.empty()) return "";
   
     string lcp = matches[0];

    for (int i = 1; i < matches.size(); i++) {
        int j = 0;
        while (j < lcp.size() && j < matches[i].size() &&
               lcp[j] == matches[i][j]) {
            j++;
        }
        lcp = lcp.substr(0, j);
        if (lcp.empty()) break;
    }
    return lcp;
}

vector<string> completeCommand(const string& buffer) {
    vector<string> matches;

    if (buffer.find(' ') != string::npos) return matches;

    char* pathEnv = getenv("PATH");
    if (!pathEnv) return matches;

    stringstream ss(pathEnv);
    string dir;

    while (getline(ss, dir, ':')) {
        DIR* d = opendir(dir.c_str());
        if (!d) continue;

        dirent* entry;
        while ((entry = readdir(d)) != nullptr) {
            string name = entry->d_name;
            if (!startsWith(name, buffer)) continue;

            string fullPath = dir + "/" + name;
            if (access(fullPath.c_str(), X_OK) == 0) {
                matches.push_back(name);
            }
        }
        closedir(d);
    }

    sort(matches.begin(), matches.end());
    return matches;
}

void run_builtin(vector<string>& arg){

  string target="";
    for(int i=1;i<(int)arg.size();i++){
      target+=arg[i];
      if(i!=(int)arg.size()-1)
      target+=" ";}
      
    if(arg[0]=="exit")
    return ;
    else if(arg[0]=="type")
    {
      
      if((target=="type")||(target=="echo")||(target=="exit")||(target=="pwd")||(target=="cd"))
      {
        cout<<target<<" is a shell builtin\n";
   
      }
      else{
      string result = find_in_path(target);
      if(result.empty()){
        cout<<target<<": not found\n";
      }
      else{
        cout<<target<<" is "<<result<<"\n";
      }
    }
    }
    else if(arg[0]=="echo")
    {
      cout<<target<<"\n";
    }
    else if(arg[0]=="pwd")
    {
      char cwd[256];
      if(getcwd(cwd,sizeof(cwd))!=nullptr){
        cout<<cwd<<"\n";
      }
      else 
      {
        perror("getcwd error");
      }
    }
    else if(arg[0]=="cd")
    {
     
         if(target=="~")
         {
          chdir(getenv("HOME"));
         }
         else if(chdir(target.c_str())!=0)
         {
            perror(("cd: "+target).c_str());
         }
    }}

struct Redir{
  string out,err;
  bool append_out = false;
  bool append_err = false;
};
pair<vector<string>, Redir> simplify(const vector<string>& arg) {
    vector<string> clean;
    Redir r;

    for (int i = 0; i < (int)arg.size(); i++) {
        if ((arg[i] == ">" || arg[i] == "1>") && i+1 < arg.size()) {
            r.out = arg[++i];
            r.append_out = false;
        }
        else if ((arg[i] == ">>" || arg[i] == "1>>") && i+1 < arg.size()) {
            r.out = arg[++i];
            r.append_out = true;
        }
        else if (arg[i] == "2>" && i+1 < arg.size()) {
            r.err = arg[++i];
            r.append_err = false;
        }
        else if (arg[i] == "2>>" && i+1 < arg.size()) {
            r.err = arg[++i];
            r.append_err = true;
        }
        else {
            clean.push_back(arg[i]);
        }
    }
    return {clean, r};
}


int main() {
  
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  
  

  string command;
  int saved_stdout=dup(STDOUT_FILENO);
  int saved_stdin=dup(STDIN_FILENO);
  int saved_error=dup(STDERR_FILENO);

bool is_tab=false;
  while(true)
  {
     dup2(saved_stdout,STDOUT_FILENO);
     
     dup2(saved_error,STDERR_FILENO);
  
   cout << "$ " << flush;
      string buffer="";
char c;
enableRawMode();


while (read(STDIN_FILENO, &c, 1) == 1) {

    if (c == '\n') {
        cout << "\n";
        break;
    }

    
    if (c == '\t') {
      if(buffer=="ech")
      {
        buffer="echo ";
        cout<<"\r$ echo "<<flush;
        is_tab=false;
        continue;
      }
      else if(buffer=="exi")
      {
        buffer="exit ";
        cout<<"\r$ exit "<<flush;
        is_tab=false;
        continue;
      }
    auto matches = completeCommand(buffer);

    if (matches.empty()) {
        cout << '\a' << flush;
    }
    else if (matches.size() == 1) {
       
        cout << "\r$ " << matches[0] << " " << flush;
        buffer = matches[0] + " ";
     
    }
    else {
       
         string lcp = longestCommonPrefix(matches);

    if (lcp.size() > buffer.size()) {
       
        cout << "\r$ " << lcp << flush;
        buffer = lcp;
        is_tab=false;
    }
    else if(lcp.size()==buffer.size()){
        if(is_tab==false)
        {
        cout << '\a' << flush;
        is_tab=true;  
        }
      else
      {
        cout<<"\n";
        for(int i=0;i<(int)matches.size();i++){
          cout<<matches[i]<<"  ";
        }
        cout<<"\n$ "<<buffer;
        is_tab=false;
      }
    }

    }
    continue;
}
buffer+=c;
cout<<c<<flush;

}

disableRawMode();

    command=buffer;
   string current="";
   vector<string> arg;
   bool isq=false;
   bool isq1=false;
   for(int i=0;i<(int)command.size();i++){
    char c=command[i];
    if(c=='\"'){
      if(isq==false)isq1=!isq1;
      else current+=c;}

    else if(c=='\''){
       if(isq1==false)isq=!isq;
       else current+=c;}

    else if(c=='\\'&&(!isq1&&!isq)){
        if(i+1<(int)command.size()){
          current+=command[i+1];
          i++;
          continue;}}

      else if(c=='\\'&&isq1){
        if(i+1<(int)command.size()&&(command[i+1]=='\\'||command[i+1]=='\"')){
          current+=command[i+1];
          i++;
          continue;}
        else {
          current+=command[i];
          continue;}}

    else if((c==' '||c=='\t')&&(!isq&&!isq1)){
      if(!current.empty()){
      arg.push_back(current);
      current="";
      }}

    else current+=c;}

   if(!current.empty()) arg.push_back(current);
   else continue;
   
   
 
    vector<vector<string>> cmds; 
   vector<string> temp;
   for(int i=0;i<(int)arg.size();i++){
     if(arg[i]=="|"){
      cmds.push_back(temp);
      temp.clear();}

     else temp.push_back(arg[i]);}

   if(!temp.empty()) cmds.push_back(temp);
 
 int n= (int)cmds.size();

  
   
   vector<int> pipes(2*(n-1));

   for(int i=0;i<n-1;i++) pipe(&pipes[2*i]);
   
  bool done=false;
   
 
vector<pair<vector<string>,Redir>> v1(n);
   for(int i=0;i<n;i++){
    
    v1[i]=simplify(cmds[i]);

     Redir redir=v1[i].second;
     cmds[i]=v1[i].first;

    if(cmds[i][0]=="exit"){
    done=true;break;}

    if(cmds[i][0]=="exit"||cmds[i][0]=="type"||cmds[i][0]=="echo"||cmds[i][0]=="pwd"||cmds[i][0]=="cd"){
      
      if(i>0)dup2(pipes[(i-1)*2],STDIN_FILENO);
      if(i<n-1)dup2(pipes[i*2+1],STDOUT_FILENO);

        if (!redir.out.empty()) {
        int fd = open(redir.out.c_str(),
                O_WRONLY | O_CREAT | (redir.append_out ? O_APPEND : O_TRUNC),
                0644);
        dup2(fd, STDOUT_FILENO);
        close(fd);}

      if (!redir.err.empty()) {
         int fd = open(redir.err.c_str(),
             O_WRONLY | O_CREAT | (redir.append_err ? O_APPEND : O_TRUNC),
             0644);
        dup2(fd, STDERR_FILENO);
        close(fd);}

      run_builtin(cmds[i]);}

    else{
          pid_t pid = fork();
          if(pid==0){
            if(i>0)dup2(pipes[(i-1)*2],STDIN_FILENO);
            if(i<n-1)dup2(pipes[i*2+1],STDOUT_FILENO);
             if (!redir.out.empty()) {
                int fd = open(redir.out.c_str(),
                O_WRONLY | O_CREAT | (redir.append_out ? O_APPEND : O_TRUNC),
                0644);
               dup2(fd, STDOUT_FILENO);
               close(fd);}

            if (!redir.err.empty()){
             int fd = open(redir.err.c_str(),
             O_WRONLY | O_CREAT | (redir.append_err ? O_APPEND : O_TRUNC),
             0644);
           dup2(fd, STDERR_FILENO);
           close(fd);}

            for(int fd: pipes)close(fd);

            vector<char*> argv;
            for(auto &s:cmds[i])
            argv.push_back(const_cast<char*>(s.c_str()));
            argv.push_back(NULL);
            execvp(argv[0],argv.data());
            
             for(int i=0;argv[i]!=NULL;i++)
            {if(argv[i+1]==NULL)
            {
              cout<<argv[i];break;
            }
             cout<<argv[i]<<" ";
            }
             cerr<<": command not found\n";
            exit(1);
          }
        }
     dup2(saved_stdout, STDOUT_FILENO);
    dup2(saved_stdin, STDIN_FILENO);
    dup2(saved_error, STDERR_FILENO);
     
   }
  for(int fd:pipes)close(fd);
   while(wait(NULL)>0);
   
   if(done)break;
}
return 0;
}
