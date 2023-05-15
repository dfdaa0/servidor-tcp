// ./server v4 51511

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 51511
#define BUFFER_SIZE 500

void criaServerSocket(int versaoIp, int* serverSocket) {
  if (versaoIp == 4) { // IPv4
    *serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (*serverSocket == -1) {
      exit(EXIT_FAILURE);
    }

    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &(serverAddress.sin_addr));

    if (bind(*serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
      exit(EXIT_FAILURE);
    }
  } else if (versaoIp == 6) { // IPv6
    *serverSocket = socket(AF_INET6, SOCK_STREAM, 0);
    if (*serverSocket == -1) {
      exit(EXIT_FAILURE);
    }
    

    struct sockaddr_in6 serverAddress6;
    memset(&serverAddress6, 0, sizeof(serverAddress6));
    serverAddress6.sin6_family = AF_INET6;
    serverAddress6.sin6_port = htons(PORT);
    inet_pton(AF_INET6, "::1", &(serverAddress6.sin6_addr));
    
    if (bind(*serverSocket, (struct sockaddr*)&serverAddress6, sizeof(serverAddress6)) == -1) {
      exit(EXIT_FAILURE);
    }
  } else {
    exit(EXIT_FAILURE);
  }
}

void aguardaConexao(int serverSocket, int* connectionSocket, char* nomeArquivo) {
  if (listen(serverSocket, 5) == -1) {
    exit(EXIT_FAILURE);
  }

  struct sockaddr_storage clientAddress;
  socklen_t clientAddressLength = sizeof(clientAddress);

  *connectionSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
  printf("Accepted socket connection\n");
  if (*connectionSocket == -1) {
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Recebe o nome do arquivo
    size_t nomeArquivoSize = sizeof(nomeArquivo); // Defina o tamanho máximo adequado
    if (recv(*connectionSocket, nomeArquivo, nomeArquivoSize - 1, 0) == -1) {
      exit(EXIT_FAILURE);
    }
    nomeArquivo[nomeArquivoSize - 1] = '\0'; // Adiciona o caractere nulo manualmente
    printf("Archive name received: %s\n", nomeArquivo);
    size_t bytesRead;

    if (access(nomeArquivo, F_OK) == 0) {
      printf("%s overwritten\n", nomeArquivo);
    } else {
      FILE* file = fopen(nomeArquivo, "wb");
      if (file == NULL) {
        exit(EXIT_FAILURE);
      }
      printf("Created archive in the server location\n");

      char buffer[BUFFER_SIZE];

      while ((bytesRead = recv(*connectionSocket, buffer, sizeof(buffer), 0)) > 0) {
        if (fwrite(buffer, sizeof(char), bytesRead, file) != bytesRead) {
          exit(EXIT_FAILURE);
        }
        printf("Buffer received\n");
        if (strstr(buffer, "\\end") != NULL) {
          printf("Last buffer received\n");
          break;
        }
      }

      fclose(file);

      printf("%s received\n", nomeArquivo);
    }

    // Verifica se a conexão foi fechada pelo cliente
    if (bytesRead == 0) {
      printf("Connection closed by client\n");
      break; // Finaliza o programa
    }
  }

  // Fecha a conexão com o cliente
  close(*connectionSocket);
}




int main(int argc, char* argv[]) {
  if (argc != 3) {
    return -1;
  }

  int versaoIp = atoi(&argv[1][1]);
  int serverSocket;
  int connectionSocket;
  char nomeArquivo[100];

  criaServerSocket(versaoIp, &serverSocket);
  aguardaConexao(serverSocket, &connectionSocket, nomeArquivo);

  return 0;
}
