// ./server v4 51511

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 51511
#define BUFFER_SIZE 500

int main(int argc, char* argv[]) {
  if (argc != 3) {
    return -1;
  }
  int versaoIp, *port;
  versaoIp = atoi(&argv[1][1]);
  int serverSocket;
  if (versaoIp == 4) { // IPv4
    // socket()
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
      exit(EXIT_FAILURE);
    }

    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &(serverAddress.sin_addr));
    
    // bind()
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) { 
        exit(EXIT_FAILURE);
    }  
  }
    
  else if (versaoIp == 6) { // IPv6
    // socket()
    serverSocket = socket(AF_INET6, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in6 serverAddress6;
    memset(&serverAddress6, 0, sizeof(serverAddress6));
    serverAddress6.sin6_family = AF_INET6;
    serverAddress6.sin6_port = htons(PORT);
    inet_pton(AF_INET6, "::1", &(serverAddress6.sin6_addr));
    
    // bind()
    if (bind(serverSocket, (struct sockaddr*)&serverAddress6, sizeof(serverAddress6)) == -1) {
      exit(EXIT_FAILURE);
    }
  }
  else {
      exit(EXIT_FAILURE);
  }

  // listen()
  if (listen(serverSocket, 5) == -1) {
      exit(EXIT_FAILURE);
  }

  // Servidor aguardando...
  
  struct sockaddr_storage clientAddress;
  socklen_t clientAddressLength = sizeof(clientAddress);

  int connectionSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
  if (connectionSocket == -1) {
      exit(EXIT_FAILURE);
  }
  
  char nomeArquivo[100]; // Tamanho adequado para o nome do arquivo
  
  if (recv(connectionSocket, nomeArquivo, sizeof(nomeArquivo), 0) == -1) {
    exit(EXIT_FAILURE);
  }
  
  if (access(nomeArquivo, F_OK) == 0) {
    printf("%s overwritten\n", nomeArquivo);
  } else {
    
    memset(nomeArquivo, 0, sizeof(nomeArquivo));
    
    // Abre o arquivo para escrita
    FILE* file = fopen(nomeArquivo, "wb");
    if (file == NULL) {
      exit(EXIT_FAILURE);
    }
    
    // Recebe e escreve os buffers no arquivo
    char buffer[BUFFER_SIZE];
    size_t bytesRead;
    while ((bytesRead = recv(connectionSocket, buffer, sizeof(buffer), 0)) > 0) {
      if (fwrite(buffer, sizeof(char), bytesRead, file) != bytesRead) {
        exit(EXIT_FAILURE);
      }
      if (strstr(buffer, "\\end") != NULL) {
        break;
      }
    }
    
    fclose(file);
  }
  
  return 0;
}