#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "parser.h"
#include <signal.h>
#define READ 0
#define WRITE 1

int file2;
int file;
char *nomeFile1;
char *nomeFile2;

/**
Função responsável por executar os comandos dados; Recebe como argumento a matriz produzida na função parse
*/
void executa (char** matriz){
  int i, g, status, a;
  int p[2];
  char buffer[1024];
  char *res = malloc(1024*sizeof(char));
  for(i = 0; matriz[i] != '\0'; i++);

  for(int k = 0; k < i; k++){
    char *token;
    char s[2] = " " ;
    token = strtok(matriz[k], s);
    char *args[30];
    for(g = 0; token != NULL; g++){
      args[g] = token;
      token = strtok(NULL, s);
    }
    args[g] = NULL;

    pipe(p);
    int f = fork();
    if(f == 0){

        dup2(p[WRITE], 1);

        close(p[WRITE]);
        close(p[READ]);

        execvp(args[0], args);
        perror("Erro no executável.");
        _exit(-1);
    }
    else{

      wait(&status);
      if (WIFEXITED(status))
        a = WEXITSTATUS(status);

      dup2(p[READ], 0);
      close(p[READ]);
      close(p[WRITE]);

      int n;
      int m = 0;
      while((n = read(0, &buffer, 1)) > 0){
          write(file2, &buffer, n );
          res[m] = buffer[0];
          m++;
      }
      res[m] = '\0';

      pipe(p);

        write(p[WRITE], res,  m);
        dup2(p[READ], 0);

        close(p[WRITE]);
        close(p[READ]);
    }
 }
}

/**
Função responsável por terminar os processos
*/
void aux_acaba(){
  int n;

  int status;
  char buffer2;
  wait(&status);
  if (WIFEXITED(status)){
    if(WEXITSTATUS(status) == 255){
      close(file2);
      close(file);

      file2 = open("ficheiroAux.nb", O_RDONLY, 0666);
      file = open(nomeFile1, O_WRONLY | O_TRUNC, 0666);
      if(file == -1){
        perror("Erro a abrir o ficheiro: file.");
        _exit(-1);
      }

      if(file2 == -1){
        perror("Erro a abrir o ficheiro: file 2.");
        _exit(-1);
      }
      while((n = read(file2,&buffer2, 1)) > 0)
        write(file, &buffer2, n);
        _exit(0);
  }
}
}

/**
Função parse
*/
int parse(int argc, char* argvs[]){
  signal(SIGCHLD, aux_acaba);
  if(argc < 1){
    perror("Erro no número de inputs.");
    exit(-1);
  }

  nomeFile1 = argvs[1];
  nomeFile2 = "ficheiroAux.nb";

  char * res = malloc(1024);
  int file = open(argvs[1], O_RDONLY, 0666);
  file2 = open("ficheiroAux.nb", O_WRONLY | O_CREAT | O_TRUNC, 0666);

  if (file == -1){
    perror("Erro a abrir o ficheiro: file.");
    _exit(-1);
  }
  if (file2 == -1){
    perror("Erro a abrir o ficheiro: file2.");
    _exit(-1);
  }

  int n = 1;
  int i = 0;
  int d = 0;
  int j = 0;
  char buffer[1024];
  char cmd[10];
  char* matriz[100][100];

  //ver se há >>>
  int contador = 0;
  int contadorNovo = 3;
  int imprime = 1;

  while(read(file, buffer, 1) > 0){
      if(imprime){
        if (buffer[0] != '>')
          write(file2, buffer, 1);

        if(buffer[0] == '>'){
          contador++;
          if(contador == 3) {
            imprime = 0;
            contador = 0;
          }
        }
      }

      if(buffer[0] == '<'){
        contadorNovo--;
        if(contadorNovo == 0) {
          imprime = 1;
          contadorNovo = 3;}
      }
  }

  close(file);
  close(file2);
  file2 = open(argvs[1], O_WRONLY | O_TRUNC, 0666);
  file = open("ficheiroAux.nb", O_RDONLY, 0666);
  if(file == -1){
    perror("Erro a abrir o ficheiro: file.");
    _exit(-1);
  }

  if(file2 == -1){
    perror("Erro a abrir o ficheiro: file2.");
    _exit(-1);
  }


  while ((n = read(file, buffer, 1)) > 0){
    i = 0;
    write(file2,buffer,1);

    if (buffer[0] == '$'){
        while ((n = read(file, buffer, 1)) > 0 && buffer[0] != '\n'){
          if (buffer[0] != '|'){
          cmd[i] = buffer[0]; //vai buscar o comando.
          i++;
          }
        }
        cmd[i] = '\0';

          write(file2,cmd,strlen(cmd));
          write(file2,"\n",1);
          write(file2,">>>\n",4);
        if(cmd[0] == ' ') {
          matriz[d++][0] = cmd;
          j = 1;
        }
        else{
          matriz[d][j++] = cmd;
        }

        int k = d > 0 ? d -1 : d;
        executa(matriz[k]);
        write(file2, "<<<\n", 4);
      }

  }
  close(file2);
  close(file);
  file2 = open("ficheiroAux.nb", O_WRONLY | O_TRUNC , 0666);
  file = open(argvs[1], O_RDONLY, 0666);

  if(file2 == -1){
    perror("Erro a abrir o ficheiro: file2.");
    _exit(-1);
  }

  if(file == -1){
    perror("Erro a abrir o ficheiro: file.");
    _exit(-1);
  }

  while(read(file, buffer, 1) > 0)
    write(file2, buffer, 1);
  return 0;
}
