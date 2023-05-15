/*
Formato: ./client 127.0.0.1 51511

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
// Settei para usar a 51511 sempre, mas pode ser trocado

char *ip, input[MAX_BYTES], *comando1, *comando2, *nomeArquivo, *arquivoSelecionado, **palavras;


char* extensaoArquivo(const char* arquivo) { // funcionando
    char* extensao = strrchr(arquivo, '.');
    if (extensao != NULL) {
        extensao++;
    }
    return extensao;
}

char** splitString(const char* input) { // funcionando
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

int checaVersaoIp(const char* ip) { // funcionando
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

int arquivoExiste(const char *nomeArquivo) {
    if (access(nomeArquivo, F_OK) == 0) {
        return 1; // O arquivo existe
    } else {
        return 0; // O arquivo não existe
    }
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
	if (valida == 1) printf("Valid extension: %s\n", extensao);
	return valida;
}

int criaSocketIPv4(char* endereco) {
	printf("Started IPv4 socket creation\n");
	int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == -1) {
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in serverAddress;
	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(PORT);
	serverAddress.sin_addr.s_addr = inet_addr(endereco);

	if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
		exit(EXIT_FAILURE);
	} else printf("Connected to IPv4\n");

	return clientSocket;
	}

int criaSocketIPv6(char* endereco) {
	printf("Started IPv6 socket creation\n");
	int clientSocket = socket(AF_INET6, SOCK_STREAM, 0);
	if (clientSocket == -1) {
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in6 serverAddress6;
	memset(&serverAddress6, 0, sizeof(serverAddress6));
	serverAddress6.sin6_family = AF_INET6;
	serverAddress6.sin6_port = htons(PORT);
	inet_pton(AF_INET6, endereco, &(serverAddress6.sin6_addr));

	if (connect(clientSocket, (struct sockaddr*)&serverAddress6, sizeof(serverAddress6)) == -1) {
		exit(EXIT_FAILURE);
	} else printf("Connected to IPv6\n");

	return clientSocket;
}

void selecionaArquivo(char* nomeArquivo) {
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

void enviaArquivo(int clientSocket, char* nomeArquivo) {
	FILE* file = fopen(nomeArquivo, "rb");
	if (file == NULL) {
		printf("Error opening the archive\n");
		perror("fopen");
		exit(1);
	}

	// Envia o nome do arquivo
	size_t nomeArquivoSize = strlen(nomeArquivo) + 1; // Tamanho do nome do arquivo, incluindo o caractere nulo
	if (send(clientSocket, nomeArquivo, nomeArquivoSize, 0) == -1) {
		printf("Error trying to send file name\n");
		exit(1);
	}
	
	// Lê e envia os buffers
	char buffer[BUFFER_SIZE];
	size_t bytesRead;
	while ((bytesRead = fread(buffer, sizeof(char), BUFFER_SIZE, file)) > 0) {
		if (send(clientSocket, buffer, bytesRead, 0) == -1) {
			printf("Error trying to send buffer\n");
			exit(1);
		}
		printf("Buffer sent\n");
	}
	// Envio do marcador de fim de arquivo
	const char* endMarker = "\\end";
	if (send(clientSocket, endMarker, strlen(endMarker) + 1, 0) == -1) {
		printf("Error trying to send end marker\n");
		exit(1);
	}
	printf("Last buffer sent\n");
	fclose(file);
}


int main(int argc, char* argv[]) {
	if (argc != 3) {
		return -1;
	}

	int versaoIp = checaVersaoIp(argv[1]);
	int clientSocket;

	if (versaoIp == 4) {
		clientSocket = criaSocketIPv4(argv[1]);
	} else if (versaoIp == 6) {
		clientSocket = criaSocketIPv6(argv[1]);
	} else {
		exit(EXIT_FAILURE);
	}

	while (1) {
		char input[BUFFER_SIZE];
		fgets(input, sizeof(input), stdin);

		// Remover o caractere de nova linha
		input[strcspn(input, "\n")] = '\0';

		char** palavras = splitString(input);
		char* comando1 = palavras[0];
		char* comando2 = palavras[1];
		char* nomeArquivo = palavras[2];

		// Verifica os comandos
		if (strcmp(comando1, "select") == 0) {
			if (strcmp(comando2, "file") == 0) {
				selecionaArquivo(nomeArquivo);
			}
		} else if (strcmp(comando1, "send") == 0) {
			if (strcmp(comando2, "file") == 0) {
				printf("Sending file %s\n", arquivoSelecionado);
				enviaArquivo(clientSocket, arquivoSelecionado);
			}
		} else if (strcmp(comando1, "exit") == 0) {
			if (strcmp(comando2, "") == 0 && strcmp(nomeArquivo, "") == 0) {
				printf("Closing socket\n");
				close(clientSocket);
				break;
			}
		}
	}

	return 0;
}