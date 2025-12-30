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

using namespace std;

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

void run_external(char* argv[]){
  pid_t pid= fork();
  if(pid==0){
    execvp(argv[0],argv);
    for(int i=0;argv[i]!=NULL;i++)
    {if(argv[i+1]==NULL)
      {
        cout<<argv[i];break;
      }
      cout<<argv[i]<<" ";
    }
      cout<<": command not found\n";
    
    exit(1);
  }
  else
  {
    wait(NULL);
  }
}
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
bool startsWith(const string& s, const string& prefix){
  return s.rfind(prefix,0)==0;
}
string completeCommand(const string& buffer){
  if(buffer.find(' ')!=string::npos) return buffer;
  char* pathEnv=getenv("PATH");
  if(!pathEnv) return buffer;
  vector<string> matches;
  string prefix = buffer;
  string path(pathEnv);

  stringstream ss(path);
  string dir;
  while(getline(ss,dir,':')){
    DIR* d= opendir(dir.c_str());
    if(!d) continue;
    dirent* entry;
    while((entry=readdir(d))!=nullptr){
      string name = entry->d_name;
      if(!startsWith(name,prefix))continue;
      string fullPath = dir+"/"+name;
      if(access(fullPath.c_str(),X_OK)==0){
        matches.push_back(name);
      }
    }
    closedir(d);
  }
  if(matches.size()==1){
    cout<<"\r$ "<<matches[0]<<" ";
    cout.flush();
    return matches[0]+" ";
  }
     return buffer;
}
int main() {
  
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  
  

  string command;
  int saved_stdout=dup(STDOUT_FILENO);
  int saved_error=dup(STDERR_FILENO);


  while(true)
  {
     dup2(saved_stdout,STDOUT_FILENO);
     
     dup2(saved_error,STDERR_FILENO);
  
   cout << "$ " << flush;
      string buffer;
char c;
enableRawMode();


while (read(STDIN_FILENO, &c, 1) == 1) {

    if (c == '\n') {
        cout << "\n";
        break;
    }

    
    if (c == '\t') {
        if (buffer == "ech") {
            cout << "\r$ echo "<<flush ;
            buffer = "echo ";
        } else if (buffer == "exi") {
            cout << "\r$ exit " << flush;
            buffer = "exit ";
        }
        else if(buffer!=completeCommand(buffer))
        {
          buffer=completeCommand(buffer);
        }
        else
        cout<<'\a'<<flush;
        
        continue;
    }

    buffer.push_back(c);
    cout << c << flush;
}

disableRawMode();

    command=buffer;
   string current="";
   vector<string> arg;
   bool isq=false;
   bool isq1=false;
   for(int i=0;i<(int)command.size();i++){
    char c=command[i];
    if(c=='\"')
    {
      if(isq==false)
       isq1=!isq1;
       else
       {
        current+=c;
       }
    }
    else if(c=='\''){
       if(isq1==false)
       {
        isq=!isq;
       }
       else
       {
         current+=c;
       }
    }
    else if(c=='\\'&&(!isq1&&!isq))
      {
        if(i+1<(int)command.size())
        {
          current+=command[i+1];
          i++;
          continue;
        }
      }
      else if(c=='\\'&&isq1)
      {
        if(i+1<(int)command.size()&&(command[i+1]=='\\'||command[i+1]=='\"'))
        {
          current+=command[i+1];
          i++;
          continue;
        }
        else
        {
          current+=command[i];
          continue;
        }
      }
    else if((c==' '||c=='\t')&&(!isq&&!isq1)){
      if(!current.empty()){
              
      arg.push_back(current);
        current="";
      }
    }
    else{
      current+=c;
    }
   }
   if(!current.empty()){
  arg.push_back(current);
   }
  
    string file_out;
    string file_error;
    string file_append;
    vector<string> arg_cpy;
    for(int i=0;i<(int)arg.size();i++){
      if(arg[i]==">>"||arg[i]=="1>>")
      {
           if(i+1<(int)arg.size())
           {
              file_append=arg[i+1];i++;
              int fd= open(file_append.c_str(),O_WRONLY|O_CREAT|O_APPEND,0644);
              dup2(fd,STDOUT_FILENO);
              close(fd);
              continue;
           }
      }
      else if(arg[i]=="2>>")
      {
        
              file_append=arg[i+1];i++;
              int fd= open(file_append.c_str(),O_WRONLY|O_CREAT|O_APPEND,0644);
              dup2(fd,STDERR_FILENO);
              close(fd);
              continue;
      }
      else if(arg[i]=="2>")
      {
        if(i+1<(int)arg.size()){
          file_error=arg[i+1];i++;
          int fd = open(file_error.c_str(),O_WRONLY|O_CREAT|O_TRUNC,0644);
          dup2(fd,STDERR_FILENO);
          close(fd);
          continue;
        }
      }
      else if(arg[i]==">"||arg[i]=="1>"){
        if(i+1<(int)arg.size()){
           file_out= arg[i+1];i++;
           int fd = open(file_out.c_str(),O_WRONLY|O_CREAT|O_TRUNC,0644);
           dup2(fd,STDOUT_FILENO);
           close(fd);
          continue;
        }else{
          cout<<"bash: syntax error near unexpected token newline"<<"\n";
          continue;
        }

      }
      arg_cpy.push_back(arg[i]);
    }
    swap(arg,arg_cpy);
   
  


  string target="";
    for(int i=1;i<(int)arg.size();i++)
    {
      target+=arg[i];
      if(i!=(int)arg.size()-1)
      target+=" ";
    }
    if(arg[0]=="exit")
    break;
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
    }
    else 
    {
      char *argv[32];
      for(int i=0;i<(int)arg.size();i++)
      {
        argv[i]=const_cast<char*>(arg[i].c_str());
      }
      argv[(int)arg.size()]=NULL;
      run_external(argv);
    }
   
}
return 0;
}
