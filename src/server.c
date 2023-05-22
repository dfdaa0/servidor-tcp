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
        // Recebe o arquivo completo
        char arquivoCompleto[BUFFER_SIZE];
        size_t bytesRead = recv(*connectionSocket, arquivoCompleto, sizeof(arquivoCompleto) - 1, 0);
        if (bytesRead == -1) {
            exit(EXIT_FAILURE);
        }
        arquivoCompleto[bytesRead] = '\0'; // Adiciona o caractere nulo manualmente

        // Remove a sequência "\end" caso esteja presente
        size_t arquivoCompletoSize = strlen(arquivoCompleto);
        if (arquivoCompletoSize >= 4 && strcmp(arquivoCompleto + arquivoCompletoSize - 4, "\\end") == 0) {
            arquivoCompleto[arquivoCompletoSize - 4] = '\0';  // Remove a sequência "\end"
        }

        // Extrai o nome do arquivo
        char* conteudoArquivo = strchr(arquivoCompleto, ':');
        *conteudoArquivo = '\0'; // Substitui o ':' por '\0' para separar o nome do conteúdo
        nomeArquivo = arquivoCompleto;

        // Ignora o conteúdo antes do primeiro "::"
        conteudoArquivo += 2; // Avança para o início do conteúdo

        printf("Archive name received: %s\n", nomeArquivo);

        if (access(nomeArquivo, F_OK) == 0) {
            printf("%s overwritten\n", nomeArquivo);
        } else {
            FILE* file = fopen(nomeArquivo, "wb");
            if (file == NULL) {
              exit(EXIT_FAILURE);
            }
            printf("Created archive in the server location\n");

            size_t conteudoArquivoSize = strlen(conteudoArquivo);

            // Escreve o conteúdo no arquivo
            if (fwrite(conteudoArquivo, sizeof(char), conteudoArquivoSize, file) != conteudoArquivoSize) {
                exit(EXIT_FAILURE);
            }

            fclose(file);

            printf("%s received\n", nomeArquivo);
        }

        // Adiciona o caractere nulo ao final do conteúdo do arquivo
        size_t conteudoArquivoSize = strlen(conteudoArquivo);
        conteudoArquivo[conteudoArquivoSize] = '\0';  // Adiciona o caractere nulo ao final do conteúdo

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
