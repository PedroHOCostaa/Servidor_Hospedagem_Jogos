#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[1024];
    
    // Criação do socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        printf("Erro ao criar o socket\n");
        return 1;
    }

    // Configuração do endereço do servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Conectar ao servidor
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("Erro ao conectar ao servidor\n");
        return 1;
    }

    // Envia dados ao servidor
    char *message = "Olá, servidor!";
    send(client_socket, message, strlen(message), 0);

    // Recebe a resposta do servidor
    int n = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (n < 0) {
        printf("Erro ao receber dados\n");
        return 1;
    }

    buffer[n] = '\0';
    printf("Recebido do servidor: %s\n", buffer);

    // Fecha o socket
    close(client_socket);

    return 0;
}
