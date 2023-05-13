/*
./client 127.0.0.1 51511

Cliente que envia arquivos para um servidor
Requisitos:
(a) Selecionar o nome do arquivo - "select file nome.extensão"
(b) Enviar o arquivo selecionado - "send file"
(c) Terminar a conexão           - "exit"

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_PALAVRAS 3
#define MAX_BYTES 500
#define BUFFER_SIZE 500
#define PORT 51511

char *ip, input[MAX_BYTES], *comando1, *comando2, *nomeArquivo, *arquivoSelecionado, **palavras;


char* extensaoArquivo(const char* arquivo) {
    char* extensao = strrchr(arquivo, '.');
    if (extensao != NULL) {
        extensao++;
    }
    return extensao;
}

char** splitString(const char* input) {
    char** palavras = (char**)malloc(MAX_PALAVRAS * sizeof(char));
    for (int i = 0; i < MAX_PALAVRAS; i++) {
      palavras[i] = (char*)malloc(MAX_BYTES * sizeof(char));
      palavras[i][0] = '\0';
    }

    int posicao = 0;
    const char* delimitador = " ";
    char* token = strtok((char*)input, delimitador);

    while (token != NULL && posicao < MAX_PALAVRAS) {
      strcpy(palavras[posicao], token);
      posicao++;
      token = strtok(NULL, delimitador);
    }
    while (posicao < MAX_PALAVRAS) {
      strcpy(palavras[posicao], "");
      posicao++;
    }

    return palavras;
}

int checaVersaoIp(const char* ip) {
  struct sockaddr_in sa;
  struct sockaddr_in6 sa6;

  if (inet_pton(AF_INET, ip, &(sa.sin_addr)) == 1) {
    return 4;
  }

  if (inet_pton(AF_INET6, ip, &(sa6.sin6_addr)) == 1) {
    return 6; 
  }

  return 0; // Não é válido
}

int arquivoExiste(const char* nome){
	return access(nome, F_OK) != -1;
}

int extensaoValida(const char* extensao) {
  const char* extensoesValidas[] = {"txt", "py", "tex", "c", "cpp", "java"};
  int valida = 0;
  for (int i = 0; i < 6; i++) {
    if (strcmp(extensao, extensoesValidas[i]) == 0) {
      valida = 1;
      break;
    }
  }
  return valida;
}

int main(int argc, char* argv[]) {
  int versaoIp = checaVersaoIp(argv[1]);
  
  if (argc != 3) {
      return -1;
  }
  
  int clientSocket;
  
  char buffer[BUFFER_SIZE];
  
  if (versaoIp == 4) {
    // socket()
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      exit(EXIT_FAILURE);
    }
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = inet_addr(argv[1]);
    // connect()
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
      exit(EXIT_FAILURE);
    }
    
  }else if (versaoIp == 6) {
    // socket()
    if ((clientSocket = socket(AF_INET6, SOCK_STREAM, 0)) == -1) {
      exit(EXIT_FAILURE);
    }
    struct sockaddr_in6 serverAddress;
    serverAddress.sin6_family = AF_INET6;
    serverAddress.sin6_port = htons(PORT);
    inet_pton(AF_INET6, argv[1], &(serverAddress.sin6_addr));
    // connect()
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
      exit(EXIT_FAILURE);
    }
  }

  while (1) {
    fgets(input, sizeof(input), stdin);
    palavras = splitString(input);
    comando1 = palavras[0];
    comando2 = palavras[1];
    nomeArquivo = palavras[2];

    if (strcmp(comando1, "select") == 0) { // select file
      if (strcmp(comando2, "file") == 0) {
        if (arquivoExiste(nomeArquivo) == 1) {
            if (extensaoValida(extensaoArquivo(nomeArquivo)) == 1) {
              FILE* file = fopen(nomeArquivo, "rb");
              if (file == NULL) {
                exit(1);
              }
              arquivoSelecionado = nomeArquivo;
              printf("%s selected\n", nomeArquivo);
              } else {
                printf("%s not valid\n", nomeArquivo);
              }
          } else {
            printf("%s does not exist\n", nomeArquivo);
          }
      }
    } else if (strcmp(comando1, "send") == 0) { // send file
      if (strcmp(comando2, "file") == 0) {
        if (strcmp(nomeArquivo, "") == 0) {
          FILE* file = fopen(arquivoSelecionado, "rb");
          if (file == NULL) {
            exit(1);
          }

          if (send(clientSocket, nomeArquivo, strlen(nomeArquivo), 0) == -1) {
            exit(1); // Envia o nome do arquivo antes
          }
          
          // Lê e envia os buffers
          char buffer[BUFFER_SIZE];
          size_t bytesRead;
          while ((bytesRead = fread(buffer, sizeof(char), BUFFER_SIZE, file)) > 0) {
            if (send(clientSocket, buffer, bytesRead, 0) == -1) {
              exit(1);
            }
          }
                
          // Envio do marcador de fim de arquivo
          const char* endMarker = "\\end";
          if (send(clientSocket, endMarker, strlen(endMarker), 0) == -1) {
            exit(1);
          }
                
          fclose(file);
        }
      }
    }else if (strcmp(comando1, "exit") == 0) { // exit
      if (strcmp(comando2, "") == 0) {
          if (strcmp(nomeArquivo, "") == 0) {
              close(clientSocket);
              break;
            }
        }
    }
  }

  return 0;
}