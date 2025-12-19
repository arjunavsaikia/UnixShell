#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/stat.h>
#include<sys/wait.h>
#include<cstring>


using namespace std;

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
int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  
  // TODO: Uncomment the code below to pass the first stage

  string command;
 
  while(true)
  {
    cout<<"$ ";
    getline(cin,command);
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
