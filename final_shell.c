/*
handle quotes and space perfectly
command implemented:
cd      : change the working directory
ls      : list files and directories with inode number
mkdir   : makes directories
cp      : copies files
cpdir   : copy dir (use / at end to source to cp only content and do not use / to copy folder itself)
touch   : creates a file if doesnt exist
cat     : display the content of file
writer  : Sort of editor (append or overwrite)
echo    : print something
run     : run some executable
estatus : exit statues of last command
exit    : exit shell
pwd     : prints the working directory
*/
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<dirent.h>

#define BUFFER_SIZE 1024

char *getCommand();
char** splitCmd(char* cmd);         //delim : ' ' ''' '"' and \to ignore delim
int execCommand(char** args);
int ls(char* path);
int cd(char* path);
int touch(char* path);
int cat(char* path);
int writer(char* path);
int makeDir(char* path);
int cp(char* source,char *destin);
int cpdir(char* source,char* destin);
int cpdir_helper(char* source,char* destin);
int echo(char **args);
int run(char** args);
int estatus(int status);
char* relativeToAbsolute(char* path);

char pwd[BUFFER_SIZE];
int noOfargs;  

int eq(const char* st1, const char* st2){
    if(strcmp(st1,st2)==0)
        return 1;
    else 
        return 0;
}

char* mygetline(char* str){
    stdin->_IO_read_ptr = stdin->_IO_read_end;
    if(str==NULL){
        str=(char*)malloc(BUFFER_SIZE*sizeof(char));
    }
    int n=BUFFER_SIZE;
    int idx=0;
    char ch=' ';
    while(1){
        scanf("%c",&ch);
        if(ch!='\n'){
            str[idx++]=ch;
            if(idx==n){
                str=(char*)realloc(str,2*n);
                n*=2;
            }
        }else{
            break;
        }
    }
    str[idx++]='\0';
    return str;
}

int main(){
    umask(0);
    getcwd(pwd,BUFFER_SIZE);
    while(1){
        printf("%s$ ",pwd);
        char* cmd = getCommand();
        char** args = splitCmd(cmd);
        int status = execCommand(args);
        if(status==0)break;
    }
    return 0;
}

char* getCommand(){
    char *cmd = (char*)malloc(BUFFER_SIZE*sizeof(char));
    cmd=mygetline(cmd);
    return cmd;
}

char** splitCmd(char* cmd){
    char delim=' ';
    char* word=(char*)malloc(100*sizeof(char));
    char** args=(char**)malloc(100*sizeof(char*));
    int noOfWords=0;
    int wordidx=0;
    for(int i=0;i<strlen(cmd);i++){
        if(i>0&&(cmd[i]=='\''||cmd[i]=='\"'||cmd[i]==' ')&&cmd[i-1]=='\\'){
            word[--wordidx]=cmd[i];
            wordidx++;
            continue;
        }
        if(cmd[i]=='\''&&delim==' '){
            delim='\'';
            continue;
        }else if(cmd[i]=='"'&&delim==' '){
            delim='"';
            continue;
        }
        if(cmd[i]==delim){
            word[wordidx++]='\0';
            args[noOfWords]=word;
            noOfWords++;
            wordidx=0;
            word=(char*)malloc(100*sizeof(char));
            if(delim!=' ')delim=' ';
            continue;
        }
        word[wordidx++]=cmd[i];
    }
    word[wordidx++]='\0';
    args[noOfWords++]=word;
    noOfargs=noOfWords;
    return args;
}

int execCommand(char** args){
    char *cmd=args[0];
    int exit_status;
    if(eq(cmd,"exit")||eq(cmd,"quit")){
        exit(0);
    }else if(eq(cmd,"clear")){
        system(cmd);
        exit_status=1;
    }else if(eq(cmd,"ls")){
        exit_status=ls(args[1]);
    }else if(eq(cmd,"cd")){
        exit_status=cd(args[1]);
    }else if(eq(cmd,"pwd")){
        printf("%s\n",pwd);
        exit_status=1;
    }else if(eq(cmd,"mkdir")){
        for(int i=1;i<noOfargs;i++)
            exit_status=makeDir(args[i]);
    }else if(eq(cmd,"touch")){
        for(int i=1;i<noOfargs;i++)
            exit_status=touch(args[i]);
    }else if(eq(cmd,"cat")){
        exit_status=cat(args[1]);
    }else if(eq(cmd,"writer")){
        exit_status=writer(args[1]);
    }else if(eq(cmd,"cp")){
        exit_status=cp(args[1],args[2]);
    }else if(eq(cmd,"cpdir")){
        exit_status=cpdir(args[1],args[2]);
    }else if(eq(cmd,"echo")){
        exit_status=echo(args);
    }else if(eq(cmd,"run")){
        exit_status=run(args);
    }else if(eq(cmd,"estatus")){
        estatus(exit_status);
    }
    else{
        printf("msh: %s command not found!\n",cmd);
    }
    return 1;
}

int ls(char* path){
    path=relativeToAbsolute(path);
    DIR* dir;       //directory table
    struct dirent* dptr;    //ptr to table entry
    dir=opendir(path);
    if(dir==NULL){
        printf("msh: location not found!\n");
        return 0;
    }
    dptr=readdir(dir);
    while(dptr!=NULL){
        char type[5];
        if(dptr->d_type==4){
            strcpy(type,"dir");
        }else{
            strcpy(type,"file");
        }
        printf("%ld\t%s\t%s\n",dptr->d_ino,type,dptr->d_name);
        dptr=readdir(dir);
    }
    printf("\n");
    closedir(dir);
    return 1;
}

int cd(char *path){
    if(path==NULL){
        return 1;
    }
    path=relativeToAbsolute(path);
    DIR* dir;
    dir=opendir(path);      //check if dir exist or not
    if(dir==NULL){
        printf("msh: location not found!\n");
        return 0;
    }
    closedir(dir);
    strcpy(pwd,path);
    return 1;
}

int touch(char* path){
    if(path==NULL||noOfargs==1){
        printf("msh: Enter the file name!\n");
        return 0;
    }
    path=relativeToAbsolute(path);
    FILE* f;
    f=fopen(path,"r");
    if(f!=NULL){
        fclose(f);
        return 1;
    }
    f=fopen(path,"w");
    if(f==NULL){
        printf("msh: Cannot create file!\n");
        return 0;
    }
    fclose(f);
    return 1;
}

int cat(char* path){
    if(path==NULL){
        printf("msh: Please enter the filename!\n");
        return 0;
    }
    path=relativeToAbsolute(path);
    FILE* fr;
    fr = fopen(path,"rb");
    if(fr==NULL){
        printf("msh: File does not exist!\n");
        return 0;
    }
    int c=fgetc(fr);
    while(c!=EOF){
        printf("%c",(char)c);
        c=fgetc(fr);
    }
    printf("\n");
    fclose(fr);
    return 1;
}

int writer(char* path){
    if(path==NULL){
        printf("msh: Please enter the file name!\n");
        return 0;
    }
    path=relativeToAbsolute(path);
    FILE* fr;
    fr=fopen(path,"r");
    char choice='o';
    if(fr!=NULL){
        printf("File already exist.\n");
        printf("The content of given file are: \n");
        cat(path);
        printf("\nDo you want to append or overwrite?(a/o) ");
        scanf("%c",&choice);
        fclose(fr);
    }

    FILE* fw;
    if(choice=='a'){  
        fw=fopen(path,"a");
    }else{
        fw=fopen(path,"w");
    }
    if(fw==NULL){
        printf("msh: Can not open writer!\n");
        return 0;
    }
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    printf("\nWRITER: Press ^X to EXIT\n");
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    while(1){
        char *str=(char*)malloc(BUFFER_SIZE*sizeof(char));
        str=mygetline(str);
        if(eq(str,"^X")||str[0]==24){
            break;
        }
        fputs(str,fw);
        fputc('\n',fw);
        free(str);
    }
    fclose(fw);
    return 1;
}

int makeDir(char* path){
    if(path==NULL){
        printf("msh: Specify directory name!\n");
        return 1;
    }
    path=relativeToAbsolute(path);
    
    mkdir(path,S_IRWXU|S_IRWXG|S_IRWXO);
    DIR* dircheck;
    dircheck=opendir(path);
    if(dircheck==NULL){
        printf("msh: Unable to create directory!\n");
        return 0;
    }
    closedir(dircheck);
    return 1;
}

int cp(char* source,char *destin){
    if(source==NULL){
        printf("msh: Enter the file to be copied!");
        return 0;
    }
    if(destin==NULL){
        printf("Enter the destination value!\n");
        return 0;
    }
    source=relativeToAbsolute(source);
    destin=relativeToAbsolute(destin);
    FILE* fr;
    FILE* fw;
    fr=fopen(destin,"r");
    if(fr!=NULL){
        char ch;
        printf("msh: File already exists!\n");
        printf("The content of the file are:\n");
        cat(destin);
        printf("Do you want to overwrite the file?(y/n) ");
        stdin->_IO_read_ptr=stdin->_IO_read_end;
        scanf("%c",&ch);
        fclose(fr);
        if(ch!='y')return 0;
    }
    fr=fopen(source,"rb");
    if(fr==NULL){
        printf("msh: Can not open source file!\n");
        return 0;
    }
    fw=fopen(destin,"wb");
    if(fw==NULL){
        printf("msh: Can not open write file!\n");
        return 0;
    }
    int ch;
    ch=fgetc(fr);
    while(ch!=EOF){
        fputc(ch,fw);
        ch=fgetc(fr);
    }
    fclose(fr);
    fclose(fw);
    return 1;
}

int cpdir(char* source, char* destin){
    if(noOfargs<3||source==NULL||destin==NULL){
        printf("msh: Enter source and destination both!\n");
        return 0;
    }
    int makedir=source[strlen(source)-1]=='/'?0:1;
    source=relativeToAbsolute(source);
    destin=relativeToAbsolute(destin);
    DIR* sdir;
    DIR* ddir;
    sdir=opendir(source);
    ddir=opendir(destin);
    if(sdir==NULL||ddir==NULL){
        printf("msh: unable to open directory!\n");
        return 0;
    }
    closedir(sdir);
    closedir(ddir);

    if(makedir==1){
        char* dirName=(char*)malloc(BUFFER_SIZE*sizeof(char));
        int idx=0;
        for(int i=0;i<strlen(source);i++){
            if(source[i]=='/'){
                idx=0;
                continue;
            }
            dirName[idx++]=source[i];
        }
        dirName[idx]='\0';
        if(destin[strlen(destin)-1]!='/'){
            strcat(destin,"/");
        }
        strcat(destin,dirName);
        makeDir(destin);
        strcat(destin,"/");
        strcat(source,"/");
    }
    if(destin[strlen(destin)-1]!='/'){
        strcat(destin,"/");
    }
    return cpdir_helper(source,destin);
}

int cpdir_helper(char* source,char* destin){
    DIR* rdir;
    rdir=opendir(source);
    struct dirent* dptr;
    dptr=readdir(rdir);
    while(dptr!=NULL){
        if(eq(dptr->d_name,".")||eq(dptr->d_name,"..")){
            dptr=readdir(rdir);
            continue;
        }
        if(dptr->d_type==4){         //dir
            char temps[BUFFER_SIZE];
            char tempd[BUFFER_SIZE];
            strcpy(temps,source);
            strcpy(tempd,destin);
            strcat(temps,dptr->d_name);
            strcat(tempd,dptr->d_name);
            if(makeDir(tempd)==0){
                printf("msh: unable to copy directory!\n");
                return 0;
            }
            if(temps[strlen(temps)-1]!='/')
                strcat(temps,"/");
            if(tempd[strlen(tempd)-1]!='/'){
                strcat(tempd,"/");
            }
            cpdir_helper(temps,tempd);
        }else{
            char temps[BUFFER_SIZE];
            char tempd[BUFFER_SIZE];
            strcpy(temps,source);
            strcpy(tempd,destin);
            strcat(temps,dptr->d_name);
            strcat(tempd,dptr->d_name);
            if(cp(temps,tempd)==0){
                printf("msh: unable to copy directory %s   %s!\n",temps,tempd);
                return 0;
            }
        }
        dptr=readdir(rdir);
    }
    return 1;
}

int echo(char** args){
    for(int i=1;i<noOfargs;i++){
        printf("%s ",args[i]);
    }
    printf("\n");
}

int run(char** args){
    if(noOfargs==1|| args[1]==NULL){
        printf("msh:Enter the executable name!\n");
        return 0;
    }
    args[1]=relativeToAbsolute(args[1]);
    char ** args_cmd = (char**)malloc(10*(sizeof(char*)));
    for(int i=1;i<noOfargs;i++){
        args_cmd[i-1]=args[i];
    }
    int i=fork();
    if(i==0){
        execvp(args[1],args_cmd);
        printf("msh:Unable to run executable!\n");
        return 0;
    }
    wait(NULL);
    return 1;
}

int estatus(int exit){
    if(exit==0){
        printf("Last command failed!\n");
    }else{
        printf("Last command executed successfully!\n");
    }
}

char* relativeToAbsolute(char* path){
    if(path==NULL)return pwd;
    if(noOfargs==1)return pwd;
    if(strlen(path)==1&&path[0]=='.')return pwd;
    if(path[0]=='/'){
        return path;
    }
    char* abs_path=(char*)malloc(BUFFER_SIZE*sizeof(char));     
    strcpy(abs_path,pwd);
    if(abs_path[strlen(abs_path)-1]!='/')strcat(abs_path,"/");
    int iter=strlen(abs_path);
    
    int n=strlen(path);
    for(int i=0;i<n;i++){
        if(path[i]=='.'){
            if(path[i+1]=='.'){
                if(path[i+2]=='\0'||path[i+2]=='/'){
                    abs_path[iter-1]='\0';
                    while(abs_path[iter-1]!='/'){
                        abs_path[iter--]='\0';
                    }
                    i+=2;
                }else{
                    abs_path[iter++]=path[i];
                }
            }else if(path[i+1]=='/'){
                i++;
            }else if(path[i+1]=='\0'){
                break;
            }else{
                abs_path[iter++]=path[i];
            }
        }else{
            abs_path[iter++]=path[i];
        }
    }
    abs_path[iter++]='\0';
    return abs_path;
}