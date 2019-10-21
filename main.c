/* CSC 360 - Assignemnt 1 - Omar AbdulAziz */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

char arrow[4] = " > ";//used for hostname display
char at_sign[2] = "@";//^
char colon[2] = ":";  //^
int bgP = 0; //total number of background processes

#define MAX_SIZE 255

struct node{
    pid_t pid;
    char command[1024];
    struct node* next;
}node;
struct node* head = NULL;

/* lists out all the background processes if there are any */
void background_list(){
    if(bgP>0){
        struct node* current = head;
        while(current!=NULL){
//            current = current->next;
            printf("%d '%s'\n", current->pid, current->command);
            current = current->next;
        }
        printf("Total number of background processes: %d\n", bgP);
    }else{
        printf("There are no processes running in the background\n");
    }
}

void background_check(){
    if(bgP > 0){
        pid_t to_terminate = waitpid(0, NULL, WNOHANG);
        while (to_terminate > 0){
            if(head->pid= to_terminate){
                printf("%d %s is being terminated\n" ,head->pid, head->command);
                bgP--;
                head = head->next;
            }else{
                struct node* current = head;
                while(current->pid!=to_terminate){
                    current = current->next;
                }
                printf("%d %s is being terminated\n",current->pid,current->command);
                bgP--;
                current = current->next;
            }
            to_terminate = waitpid(0,NULL,WNOHANG);
        }

    }
}

void background_process(char *tok[], int count){
    int i;
    char **new_token = malloc(sizeof(char*)*5);
    for(i =1; i<count;i++){
        new_token[i-1] = malloc(sizeof(char)*strlen(tok[i]+5));
        strcpy(new_token[i-1],tok[i]);
    }
        pid_t parent = getpid();
        pid_t child_id = fork();
        if(child_id ==-1){
            printf("Fork Failed\n");
        }else if(child_id>0){
            if(bgP==0){
                bgP++;
                head = malloc(sizeof(node)*3);
                head->pid = child_id;
                head->next = NULL;
                for(i=0;i<count-1;i++){
                strcat(head->command, new_token[i]);
                strcat(head->command, " ");
            }
        }else{
            bgP++;
            struct node* current = head;
            while(current->next!=NULL){
                current= current->next;
            }
            current->next = malloc(sizeof(node));
            current->next->pid = child_id;
            strcat(current->next->command, new_token[0]);
            for (i=1;i<count-1;i++){
                strcat(current->next->command, " ");
                strcat(current->next->command, new_token[i]);
            }
            current->next->next=NULL;
        }
            waitpid(child_id,NULL,WNOHANG);
        }else{
        if(execvp(new_token[0],new_token)==-1){
            printf("command not found/n");
            exit(1);
        }
    }
}

void change_directory(char *tok[], int count){
    if (count == 1){
        chdir(getenv("HOME"));
    }else if (count == 2){
        if (strcmp(tok[1],"~")==0){
            chdir(getenv("HOME")); 
        }else if(strcmp(tok[1],"..")==0){
            chdir("..");
        }else if(chdir(tok[1])==-1){
            printf("%s\n",strerror(errno));
        }else{
            (chdir(tok[1]));//changes directory based on arguments
        }
    }else{
        printf("Too many input arguments\n");
    }

}
void standard_exec(char *tok[], int count){
    pid_t parent = getpid();
    pid_t child_id = fork();//creates new process to hand tokenized command
    if (child_id <0){
        printf("Fork Failed");
    }else if (child_id >0){
        wait(NULL); //wait for process to complete before returning to SSI
    }else{
       if(execvp(tok[0],tok)==-1){ //invalid command sink
           printf("command not found\n");
           exit(1);
       }
    }
}

void parser(char *user_input){
    char *tokens = strtok(user_input, " "); //all the commands sent in by users
    int count =0;
    int i;
    char **token = malloc(sizeof(char*)); //i-th command in line
    while(tokens != NULL){
        token[count] = malloc(sizeof(char)*(strlen(tokens)+1));
        strcpy(token[count], tokens);
        tokens = strtok(NULL, " ");
        count++;
        if(token !=NULL){
            char **next_token = realloc(token, sizeof(char *)*(count+1)); //i+1th command in line
            if(next_token!=NULL){
                token = next_token;
            }
        }
    }
    token[count]=NULL;
    if(token[0]!=NULL){
        if(!strcmp(token[0], "cd\0")){ //first token compared to cd
            change_directory(token,count);
        }else if(!strcmp(token[0], "bg\0")){//first token compared bg
            background_process(token, count);
        }else if(!strcmp(token[0], "bglist\0")){//first tok comp to bglist, 0 negated
            background_list();
        }else{//all other commands handled here
            standard_exec(token, count);
        }
    }
}

void init(){
    char *cwd, *cmd; //current directory
    char host_name[1024];
    char user_name[1024];
    char cwdval[1024]; //string that stores user/host name and dir
    char ssi_line[2024] = "SSI: ";
    getcwd(cwdval, sizeof(cwdval)); //retrieve current directory
    gethostname(host_name, sizeof(host_name));
    getlogin_r(user_name, sizeof(user_name));
    strcat(user_name, "@");
    strcat(user_name, host_name);
    strcat(user_name, ":");
    strcat(user_name, cwdval);
    strcat(user_name, " > ");
    strcat(ssi_line, user_name); //concatenate directory and host to ssi string 
    cmd =  ssi_line;
   cwd = readline(ssi_line);
    if (strlen(ssi_line) > 0&&strcmp(cmd,cwd)){
        parser(cwd);
        free(cwd);
    }
}


int main(){
    while(1){
        init();
        background_check();
    }
    return(EXIT_SUCCESS);
}
