#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>

// Definição da estrutura admin_data
struct admin_data {
    int op;             // Operação
    int port;           // Porta
    int error;          // Código de erro
    int ip_size;        // Tamanho do IP
    char* ip;           // IP em formato UTF-8
    int admin_socket;   // Socket de conexão com o servidor de administração
};

// Função para comunicação com o servidor de administração
void comunicar_com_admin(struct admin_data* data) {
    

    // Montar o cabeçalho seguindo o formato 
    int buffer_size = 4 * sizeof(int) + data->ip_size;
    char* buffer = malloc(buffer_size); // Aloca o buffer dinamicamente

    if (buffer == NULL) {
        perror("Erro ao alocar memória para o buffer");
        pthread_exit(NULL);
    }
    data->op = 0; // Operação de criação de sala
    data->error = -2; // Código de erro
    // Converter os dados para big-endian
    int net_op = htonl(data->op); 
    int net_port = htonl(data->port);
    int net_error = htonl(data->error);
    int net_ip_size = htonl(data->ip_size);

    memcpy(buffer, &net_op, sizeof(int));                         // Copiar operação
    memcpy(buffer + sizeof(int), &net_port, sizeof(int));         // Copiar porta
    memcpy(buffer + 2 * sizeof(int), &net_error, sizeof(int));    // Copiar código de erro
    memcpy(buffer + 3 * sizeof(int), &net_ip_size, sizeof(int));  // Copiar o tamanho do IP
    memcpy(buffer + 4 * sizeof(int), data->ip, data->ip_size);    // Copiar o IP

    // Enviar os dados para o servidor de administração
    if (send(data->admin_socket, buffer, buffer_size, 0) < 0) {
        perror("Erro ao enviar dados ao servidor de administração");
        free(buffer); // Libera o buffer antes de sair
        close(data->admin_socket);
        pthread_exit(NULL);
    }

    printf("Dados enviados ao servidor de administração: op=%d, port=%d, error=%d, ip=%s\n", 
           data->op, data->port, data->error, data->ip);

    // Libera a memória alocada para o buffer
    free(buffer);
    recv(data->admin_socket, &data->error, sizeof(int), 0); // Receber o código de erro
    // Encerrar a conexão e a thread
    close(data->admin_socket);
    pthread_exit(NULL);
}

// Função da thread da sala
void* sala(void* arg) {
    struct sockaddr_in endereco_servidor_admin;
    int id_sala = *((int*)arg);
    free(arg);

    // Criação do socket para o servidor de administração
    int admin_socket = socket(AF_INET, SOCK_STREAM, 0); 
    if (admin_socket < 0) {
        perror("Erro ao criar o socket para o servidor de administração");
        return NULL;
    }

    // Configuração do endereço do servidor de administração
    endereco_servidor_admin.sin_family = AF_INET;
    endereco_servidor_admin.sin_port = htons(5000); // Porta do servidor de administração
    endereco_servidor_admin.sin_addr.s_addr = inet_addr("127.0.0.1"); // IP do servidor (localhost)

    // Conectar ao servidor de administração
    if (connect(admin_socket, (struct sockaddr*)&endereco_servidor_admin, sizeof(endereco_servidor_admin)) < 0) {
        perror("Erro ao conectar com o servidor de administração");
        close(admin_socket);
        return NULL;
    }

    printf("Conectado ao servidor de administração.\n");

    // Alocar e configurar os dados para a thread
    struct admin_data* data = malloc(sizeof(struct admin_data));
    data->op = 1; // Operação de criação de sala
    data->port = 12345; // Porta da sala (exemplo)
    data->error = 0; // Código de erro
    data->ip = strdup("127.0.1.1"); // IP (UTF-8)
    data->ip_size = strlen(data->ip); // Tamanho do IP
    data->admin_socket = admin_socket; // Socket de conexão com o servidor de administração
    
    comunicar_com_admin(data);  // Comunicação com o servidor de administração
    
    // Libera memória da estrutura
    free(data->ip);
    free(data);

    // Placeholder para lógica do jogo
    while(1) {
        // Ponto 3 e Ponto 4 (aceitar conexões e jogo)
    }

    pthread_exit(NULL);
}

// Função principal
int main() {
    pthread_t thread_id;
    int* id_sala = malloc(sizeof(int));
    *id_sala = 1;  // Exemplo de ID da sala

    // Criar uma nova thread para a sala
    if (pthread_create(&thread_id, NULL, sala, (void*)id_sala) != 0) {
        perror("Erro ao criar a thread da sala");
        free(id_sala);
        return 1;
    }

    // Esperar a thread terminar
    pthread_join(thread_id, NULL);

    return 0;
}
