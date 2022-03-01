#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>
#include<errno.h>

#define BUFF_SIZE 255
#define RED "\x1b[31m"
#define GREEN "\x1b[92m"
#define BLUE "\x1b[34m"
#define COLOR_RESET "\x1b[0m"

struct paths
{
    char prev[BUFF_SIZE];
    char current[BUFF_SIZE];
};
struct history
{
    char tab[BUFF_SIZE][BUFF_SIZE];
    int size;
};
struct history history;

void my_exit(){
    exit(0);
}
void help(char **command){
    if(command[1]==NULL){
        printf(BLUE"Projekt:"COLOR_RESET" microshell\n");
        printf(BLUE"Język programowania:"COLOR_RESET" c\n");
        printf(BLUE"Autor:"COLOR_RESET" Miłosz Rolewski\n");
        printf(BLUE"Obsługiwane komendy:"COLOR_RESET" cd, cp, exit, help, his\n");
    }
    else if(!strcmp(command[1],"cd")){
        printf("cd - Zmiana katalogu roboczego\n");
        printf(BLUE"$cd <katalog docelowy>\n"COLOR_RESET);
    }
    else if(!strcmp(command[1],"cp")){
        printf("cp - kopiuje plik zrodlowy do pliku docelowego\n");
        printf(BLUE"$cd <plik zrodlowy> <plik docelowy>\n"COLOR_RESET);
    }
    else if(!strcmp(command[1],"exit")){
        printf("exit - wyjscie z programu microshell\n");
        printf(BLUE"$exit\n"COLOR_RESET);
    }
    else if(!strcmp(command[1],"help")){
        printf("help - wyswietla informacje o programie i jego dzialaniu\n");
        printf(BLUE"$help\n"COLOR_RESET);
    }
    else if(!strcmp(command[1],"his")){
        printf("his [-clear] - wyswietla historie polecen wprowadzanych do programu\n");
        printf(" -clear -czyszczenie historii\n");
        printf(BLUE"$his\n"COLOR_RESET);
    }
    else{
        printf("Brak opisu dla polecenia"RED"[ $ %s ]\n"COLOR_RESET,command[1]);
    }
}
void clear_history(){
    int i=0;
    while(i<=history.size){
        strcpy(history.tab[i],"");
        i++;
    }
    history.size=0;
}
void to_history(char *new_line){
    if(history.size==BUFF_SIZE){
        clear_history();
    }
    else{
        strcpy(history.tab[history.size],new_line);
        history.size++;
    }
}
void print_history(char **command){
    int i=0;
    while(strcmp(history.tab[i],"")){
        printf("[%d]:%s",i+1,history.tab[i]);
        i++;
    }
}
void read_command(char *input, char **command){
    int argc=0;
    while (command[argc]!=NULL){
        command[argc]=NULL;
        argc++;
    }
    argc=0;
    char signs[]=" \n\t";
    fgets(input,BUFF_SIZE,stdin);

    if(strcmp(input,"his\n")){
        to_history(input);
    }
    
    char *tmp=strtok(input,signs);
    while(tmp!=NULL){
        command[argc]=tmp;
        tmp=strtok(NULL,signs);
        argc++;
    }
}
void print_path_and_login(){
    char path[BUFF_SIZE];
    getcwd(path,sizeof(path));
    printf(GREEN"[%s]"COLOR_RESET":"GREEN"[%s]"COLOR_RESET" $ ",getenv("USER"),path);
}
void change_directory(char **command, struct paths *path){
    if(command[2]!=NULL){
        printf(RED"Za duzo argumentow\n"COLOR_RESET);
    }
    else if(command[1]==NULL||!strcmp(command[1],"~")){
        getcwd(path->current,sizeof(path->current));
        chdir(getenv("HOME"));
        strcpy(path->prev,path->current);
    }
    else if(!strcmp(command[1],"-")){
        getcwd(path->current,sizeof(path->current));
        chdir(path->prev);
        strcpy(path->prev,path->current);
    }
    else if(!strcmp(command[1],".")){
        chdir(command[1]);
    }
    else{
        getcwd(path->current,sizeof(path->current));
        if(chdir(command[1])){
            printf(RED"Nie znaleziono katalogu\n"COLOR_RESET);
        }
        else{
            strcpy(path->prev,path->current);
        }
    }
}
void copy(char **command){
    FILE *from_f, *to_f;
    if(command[1]==NULL || command[2]==NULL){
        printf(RED"Nie podano pliku wejscia/wyjscia\n"COLOR_RESET);
    }
    else if((from_f=fopen(command[1],"r"))==NULL){
        printf(RED"Nie odnaleziono pliku wejscia\n"COLOR_RESET);
    }
    else{
        to_f=fopen(command[2],"w");
        fseek(from_f,0,SEEK_END);
        int lenght=ftell(from_f);
        fseek(from_f,0,SEEK_SET);
        while(lenght>0){
            fputc(fgetc(from_f),to_f);
            lenght--;
        }
        fclose(from_f);
        fclose(to_f);
    }
}
int main(){
    char input[BUFF_SIZE];
    char *command[BUFF_SIZE];
    struct paths *path=(struct paths*)malloc(sizeof(struct paths));
    history.size=0;
    while(1){
        print_path_and_login();
        read_command(input, command);
        if(*command!=NULL){
            if(!strcmp(*command,"his")){
                if(command[1]==NULL){
                    print_history(command);
                }
                else if(!strcmp(command[1],"clear")||!strcmp(command[1],"-clear")){
                    clear_history();
                }
            }
            else if(!strcmp(*command,"help")){
                help(command);
            }
            else if(!strcmp(*command,"exit")){
                my_exit();
            }
            else if(!strcmp(*command,"cd")){
                change_directory(command,path);
            }
            else if(!strcmp(*command,"cp")){
                copy(command);
            }
            else {
                switch (fork())
                {
                case -1:
                    printf("Fork error\n");
                    break;

                case 0:
                    execvp(*command,command);
                    if(execvp(*command,command)==-1){
                        printf(RED"ERROR[%d]: %s\n"COLOR_RESET,errno,strerror(errno));
                        exit(errno);
                    }
                    break;

                default:
                    wait(NULL);
                    break;
                }    
            }
        }
    }
}