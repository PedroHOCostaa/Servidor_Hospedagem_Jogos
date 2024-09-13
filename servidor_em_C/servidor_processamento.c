#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "estruturas.h"

#define PORT "5000"
#define HOST "127.0.0.1"       // IP do servidor admin
#define SALAS 5
#define MEU_IP "127.0.0.1" // IP do servidor 

int jogadoresNaSalas[SALAS];

//# ================================================ #
//# | op (int 4 bytes)|    coluna (int 4 bytes)    | #
//# | linha (int 4 bytes)| orientacao (int 4 bytes)| #
//# | size (int 4 bytes) |   nome (string size)    | #
//# ================================================ #


void receberDoCliente(int socket, int* op, int* coluna, int* linha, int* orientacao, char* nome) {
    char buffer[20]; // Buffer para os primeiros 20 bytes
    int bytes_recebidos = recv(socket, buffer, sizeof(buffer), 0);
    if (bytes_recebidos < 0) {
        perror("Erro ao receber dados do cliente");
        return;
    }

    // Converte os dados do buffer para as variáveis
    int offset = 0;
    int temp;
    
    memcpy(&temp, buffer + offset, sizeof(int));
    *op = ntohl(temp);
    offset += sizeof(int);
    
    memcpy(&temp, buffer + offset, sizeof(int));
    *coluna = ntohl(temp);
    offset += sizeof(int);
    
    memcpy(&temp, buffer + offset, sizeof(int));
    *linha = ntohl(temp);
    offset += sizeof(int);
    
    memcpy(&temp, buffer + offset, sizeof(int));
    *orientacao = ntohl(temp);
    offset += sizeof(int);
    
    memcpy(&temp, buffer + offset, sizeof(int));
    int size = ntohl(temp);

    // Recebe os bytes restantes iguais a size
    char* nome_buffer = (char*)malloc(size + 1); // +1 para o caractere nulo
    if (nome_buffer == NULL) {
        perror("Erro ao alocar memória para o nome");
        return;
    }

    bytes_recebidos = recv(socket, nome_buffer, size, 0);
    if (bytes_recebidos < 0) {
        perror("Erro ao receber o nome do cliente");
        free(nome_buffer);
        return;
    }

    // Adiciona o caractere nulo ao final da string
    nome_buffer[size] = '\0';

    // Copia o nome para a variável nome
    strcpy(nome, nome_buffer);

    // Libera a memória alocada para o nome_buffer
    free(nome_buffer);
}

void carregarMapa(struct mapa* mapaJogador, struct mapa* mapaAdversario, int * mapa_jogador, int * mapa_adversario)
{
    for(int i = 0; i < 10; i++)
    {
        for(int j = 0; j < 10; j++)
        {
            /// Salva os valores respectivos do mapa do jogador no vetor
            if(mapaJogador->tabuleiro[i][j].indicador == 0) // indicador == 0: Água
            {
                mapa_jogador[i*10 + j] = 20;         // mapa recebe = 20: Água
            }else                                   // indicador == 1: Navio
            {                                                                  // barco 30 = desconhecido, 40 = tipo 0, 50 = tipo 1, 60 = tipo 2, 70 = tipo 3
                struct navio* navio = mapaJogador->tabuleiro[i][j].superficie;
                if ((*(navio)).estado == 0)           // 0: Navio destruido
                {                                                              
                    mapa_jogador[i*10 + j] = ((navio->tipo + 4) * 10) + 5;     // mapa recebe casa da dezena do codigo do barco + 5: Navio destruido
                }else
                {
                    int ancoraColuna = navio->ancoraColuna;
                    int ancoraLinha = navio->ancoraLinha;
                    if ((*(navio)).estrutura[i - ancoraColuna][j - ancoraLinha] == 1)
                    {   /// Parte do navio atingida
                        mapa_jogador[i*10 + j] = ((navio->tipo + 4) * 10) + 1;      // mapa recebe dezena do codigo do barco + 1: Parte do navio destruida
                    }else
                    {
                        mapa_jogador[i*10 + j] = (navio->tipo + 4) * 10;           // mapa recebe dezena do codigo do barco: Parte do navio integral
                    }
                }
            }

            /// Salva os valores respectivos do mapa descoberto até o momento do adversario no vetor
            if(mapaAdversario->mapaAdversario[i][j] == 'A')
            {
                mapa_adversario[i*10 + j] = 20;
            }else if(mapaAdversario->mapaAdversario[i][j] == 'N')
            {
                mapa_adversario[i*10 + j] = 31;
            }else if(mapaAdversario->mapaAdversario[i][j] == '0')
            {
                mapa_adversario[i*10 + j] = 45;
            }else if(mapaAdversario->mapaAdversario[i][j] == '1')
            {
                mapa_adversario[i*10 + j] = 55;
            }else if(mapaAdversario->mapaAdversario[i][j] == '2')
            {
                mapa_adversario[i*10 + j] = 65;
            }else if(mapaAdversario->mapaAdversario[i][j] == '3')
            {
                mapa_adversario[i*10 + j] = 75;
            }else{              /// Se não a conhecimento doque tem na posição
                mapa_adversario[i*10 + j] = 10;
            }
        }
    }
}

int jogo(int jogador1, int jogador2, struct admin_data* data)
{
    struct mapa* mapaJogadorUm = (struct mapa*)malloc(sizeof(struct mapa));;
    inicializaMapa(mapaJogadorUm);
    struct mapa* mapaJogadorDois = (struct mapa*)malloc(sizeof(struct mapa));;
    inicializaMapa(mapaJogadorDois);
    int mapaVazio[100] = {10}; // Inicializa o vetor com 100 posições vazias
    
    int tipoJogo = 0;
    
    // Envia a solicitação para o cliente escolher o tipo de jogo
    int op = 4; // Indica operação de escolha do tipo de jogo
    int mensagem = 0; // Exemplo de mensagem inicial
    int orientacao;
    int decisao;
    int ancoraColuna, ancoraLinha;
    char nomeJogador1[256];
    char nomeJogador2[256];

    
    // Exemplo de uso da função
    int mapa_jogador[100] = {10};  // Inicializa com 100 posições valendo 10
    int mapa_adversario[100] = {10}; // Inicializa com 100 posições valendo 10
    
    enviarParaCliente(jogador1, op, mensagem, mapaVazio, mapaVazio);
    
    receberDoCliente(jogador1, &op, &ancoraColuna, &ancoraLinha, &decisao, nomeJogador1);
    
    printf("Jogador 1 escolheu o tipo de jogo: %d\n", decisao);
    tipoJogo = decisao;
    int qtdNavios[4], qtdNaviosJ1[4], qtdNaviosJ2[4], qtdTiros;
    if(tipoJogo == 1)
    {
        qtdNaviosJ2[0] = qtdNaviosJ1[0] = qtdNavios[0] = 1;
        qtdNaviosJ2[1] = qtdNaviosJ1[1] = qtdNavios[1] = 1;
        qtdNaviosJ2[2] = qtdNaviosJ1[2] = qtdNavios[2] = 0;
        qtdNaviosJ2[3] = qtdNaviosJ1[3] = qtdNavios[3] = 0;
        qtdTiros = 1;
    }
    if(tipoJogo == 2)
    {
        qtdNaviosJ2[0] = qtdNaviosJ1[0] = qtdNavios[0] = 2;
        qtdNaviosJ2[1] = qtdNaviosJ1[1] = qtdNavios[1] = 2;
        qtdNaviosJ2[2] = qtdNaviosJ1[2] = qtdNavios[2] = 1;
        qtdNaviosJ2[3] = qtdNaviosJ1[3] = qtdNavios[3] = 1;
        qtdTiros = 3;
    }
    

    struct navio* novoNavio;
    int criado = 0;
    
    //Jogador 1 posiciona seus navios//
    for(int i = 0; i < 4; i++)
    {
        printf("Navio do tipo %d\n", i);
        while(qtdNaviosJ1[i] > 0)
        {
            // Envia solicitação para o cliente posicionar o navio
            op = 1; // Indica operação de posicionar navio
            mensagem = i; // Exemplo de mensagem inicial
            carregarMapa(mapaJogadorUm, mapaJogadorDois, mapa_jogador, mapa_adversario); // Carrega os mapas
            enviarParaCliente(jogador1, op, mensagem, mapa_jogador, mapa_adversario);
            // Recebe do cliente a posição do navio que tentará ser inserido no mapa
            receberDoCliente(jogador1, &op, &ancoraColuna, &ancoraLinha, &orientacao, nomeJogador1);


            if(criado == 0) // Verifica se o navio foi inicializado, se ja foi não ira o inicializar novamente
            {
                novoNavio = (struct navio*)malloc(sizeof(struct navio));    // Aloca memória para o novo navio
                inicializaNavio(novoNavio, i, orientacao);                  // Inicializa o navio, com o tipo e orientação e cria a estrutura dele
                criado = 1;                                                 // Marca que o navio foi criado
                printf("Navio criado\n");
            }
            printf("Navio do tipo %d orientação %d na posicao %d %d\n", novoNavio->tipo, novoNavio->orientacao, ancoraColuna, ancoraLinha);
            if(insereNavio(mapaJogadorUm, novoNavio, ancoraColuna, ancoraLinha) == 0)
            {
                qtdNaviosJ1[i]--;
                criado = 0;
                printf("Navio inserido\n");
            }else
            {
                printf("Navio não inserido\n");
            }
            imprimirMeuMapa(mapaJogadorUm);
        }
    }
    printf("Jogador 1 pronto\n");
    //Jogador 2 posiciona seus navios//
    for(int i = 0; i < 4; i++)
    {
        printf("Navio do tipo %d\n", i);
        while(qtdNaviosJ2[i] > 0)
        {
            // Envia solicitação para o cliente posicionar o navio
            op = 1; // Indica operação de posicionar navio
            mensagem = i; // Exemplo de mensagem inicial
            carregarMapa(mapaJogadorDois, mapaJogadorUm, mapa_jogador, mapa_adversario); // Carrega os mapas
            enviarParaCliente(jogador2, op, mensagem, mapa_jogador, mapa_adversario);
            
            // Recebe do cliente a posição do navio que tentará ser inserido no mapa
            receberDoCliente(jogador2, &op, &ancoraColuna, &ancoraLinha, &orientacao, nomeJogador2);

            if(criado == 0)
            {
                novoNavio = (struct navio*)malloc(sizeof(struct navio));    // Aloca memória para o novo navio
                inicializaNavio(novoNavio, i, orientacao);                  // Inicializa o navio, com o tipo e orientação e cria a estrutura dele
                criado = 1;                                                 // Marca que o navio foi criado
                printf("Navio criado\n");
            }
            printf("Navio do tipo %d orientação %d na posicao %d %d\n", novoNavio->tipo, novoNavio->orientacao, ancoraColuna, ancoraLinha);
            if(insereNavio(mapaJogadorDois, novoNavio, ancoraColuna, ancoraLinha) == 0)
            {
                qtdNaviosJ2[i]--;
                criado = 0;
                    printf("Navio inserido\n");
            }else
            {
                    printf("Navio não inserido\n"); 
            }
            imprimirMeuMapa(mapaJogadorDois);
        }
    }
    printf("Jogador 2 pronto\n");


    int vencedor = 0;
    int alvoColuna, alvoLinha;
    while(1)
    {
        printf("passou\n");
        if(vencedor == 0)
        
        for(int i = 0; i < qtdTiros; i++)
        {
            
            // Jogador1 recebe uma solicitação para realizar um disparo e devolve a sala a posição do disparo
            printf("Jogador 1 selecione a região que irá realizar o tiro\n");
            // Envia solicitação para o cliente posicionar o navio
            op = 2; // Indica operação de realizar um disparo
            carregarMapa(mapaJogadorUm, mapaJogadorDois, mapa_jogador, mapa_adversario); // Carrega os mapas
            enviarParaCliente(jogador1, op, mensagem, mapa_jogador, mapa_adversario);
            
            // Recebe do cliente a posição do navio que tentará ser inserido no mapa
            receberDoCliente(jogador1, &op, &alvoColuna, &alvoLinha, &orientacao, nomeJogador1);


            int resultado = realizarTiro(mapaJogadorDois, alvoColuna, alvoLinha);
            if(resultado == 0)
                printf("Tiro na agua!\n");
            if(resultado == 1)
                printf("Tiro acertou um navio\n");
            if(resultado == 2)
            {
                printf("Tiro acertou um navio e o afundou\n");
                if(verificaPerdedor(mapaJogadorDois) == 1)
                {
                    // Envia um aviso para os clientes que a partida acabou
                    op = 3; // Indica que o jogo foi terminado
                    mensagem = 1; // Indica que o jogador 1 venceu
                    carregarMapa(mapaJogadorUm, mapaJogadorDois, mapa_jogador, mapa_adversario); // Carrega os mapas
                    enviarParaCliente(jogador1, op, mensagem, mapaVazio, mapaVazio);
                    mensagem = 2; // Indica que o jogador 2 perdeu
                    carregarMapa(mapaJogadorDois, mapaJogadorUm, mapa_jogador, mapa_adversario); // Carrega os mapas
                    enviarParaCliente(jogador2, op, mensagem, mapaVazio, mapaVazio);

                    // Envia para o servidor de administração o resultado do jogo e avisa que a sala está vazia
                    data->op = htonl(2);// indica que é uma atualização de estado da sala                //# ============================================================ #
                    data->port = htonl(0);                                                               //# | op (int 4 bytes)| port (int 4 bytes)| error (int 4 bytes)| #
                    data->error = htonl(0);                                                              //# | size (int 4 bytes) |        ip (string size bytes)       | #
                    data->ip_size = strlen(nomeJogador1);       // Tamanho do nome                       //# ============================================================ #
                    strcpy(data->ip, nomeJogador1);             // Copiar o nome para a estrutura
                    data->error = 0;                            // Código de erro
                    comunicar_com_admin(data);
                    return 1;
                }
            }
            imprimirMapaAdversario(mapaJogadorDois);
        }
        
        for(int i = 0; i < qtdTiros; i++)
        {
            // Jogador2 recebe uma solicitação para realizar um disparo e devolve a posição do disparo
            op = 2; // Indica operação de realizar um disparo
            carregarMapa(mapaJogadorDois, mapaJogadorUm, mapa_jogador, mapa_adversario); // Carrega os mapas
            enviarParaCliente(jogador2, op, mensagem, mapa_jogador, mapa_adversario);
            
            // Recebe do cliente a posição do navio que tentará ser inserido no mapa
            receberDoCliente(jogador2, &op, &alvoColuna, &alvoLinha, &orientacao, nomeJogador1);


            int resultado = realizarTiro(mapaJogadorUm, alvoColuna, alvoLinha);
            if(resultado == 2)
            {
                printf("Tiro acertou um navio e o afundou\n");
                if(verificaPerdedor(mapaJogadorUm) == 1)
                {
                    // Envia um aviso para os clientes que a partida acabou
                    op = 3; // Indica que o jogo foi terminado
                    mensagem = 2; // Indica que o jogador 1 perdeu
                    carregarMapa(mapaJogadorUm, mapaJogadorDois, mapa_jogador, mapa_adversario); // Carrega os mapas
                    enviarParaCliente(jogador1, op, mensagem, mapaVazio, mapaVazio);
                    mensagem = 1; // Indica que o jogador 2 venceu
                    carregarMapa(mapaJogadorDois, mapaJogadorUm, mapa_jogador, mapa_adversario); // Carrega os mapas
                    enviarParaCliente(jogador2, op, mensagem, mapaVazio, mapaVazio);

                    // Envia para o servidor de administração o resultado do jogo e avisa que a sala está vazia
                    data->op = htonl(2);// indica que é uma atualização de estado da sala                //# ============================================================ #
                    data->port = htonl(0);                                                               //# | op (int 4 bytes)| port (int 4 bytes)| error (int 4 bytes)| #
                    data->error = htonl(0);                                                              //# | size (int 4 bytes) |        ip (string size bytes)       | #
                    data->ip_size = strlen(nomeJogador2);       // Tamanho do nome                       //# ============================================================ #
                    strcpy(data->ip, nomeJogador2);             // Copiar o nome para a estrutura
                    data->error = 0;                            // Código de erro
                    comunicar_com_admin(data);
                    return 1;
                }
            }
            imprimirMapaAdversario(mapaJogadorUm);
        }
    }
}


void* sala(void* arg)
{
    struct sockaddr_in endereco_servidor_admin;
    int porta_sala = *((int*)arg); // Porta da sala
    char* ip_sala = MEU_IP; // IP da sala
    free(arg);
    printf("Porta da sala: %d\n", porta_sala);
    // Ponto 1: Criação do socket para o servidor de administração
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
    data->op = htonl(1);                           // Operação de criação de sala
    data->port = htonl(porta_sala);                // Porta da sala (exemplo)
    data->error = 0;                        // Código de erro
    data->ip_size = strlen(ip_sala);        // Tamanho do IP
    data->ip = malloc(strlen(ip_sala) + 1); // Alocar memória para o IP
    strcpy(data->ip, ip_sala);              // Copiar o IP para a estrutura de dados
    data->admin_socket = admin_socket;      // Socket de conexão com o servidor de administração
    

    /// Cria um socket para esperar conexões do servidor de comunicação
    int communication_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (communication_socket < 0) {
        perror("Erro ao criar o socket para o servidor de comunicação");
        close(data->admin_socket);
        pthread_exit(NULL);
    }

    // Configuração do endereço do servidor de comunicação
    struct sockaddr_in communication_address;
    communication_address.sin_family = AF_INET;
    communication_address.sin_port = htons(porta_sala);
    communication_address.sin_addr.s_addr = inet_addr(ip_sala); // Conversão correta do IP

    // Vincula o socket à porta especificada
    if (bind(communication_socket, (struct sockaddr*)&communication_address, sizeof(communication_address)) < 0) {
        perror("Erro ao vincular o socket à porta");
        close(communication_socket);
        close(data->admin_socket);
        pthread_exit(NULL);
    }

    // Aguarda conexões de clientes
    if (listen(communication_socket, 1) < 0) {
        perror("Erro ao aguardar conexões de clientes");
        close(communication_socket);
        close(data->admin_socket);
        pthread_exit(NULL);
    }
    comunicar_com_admin(data); // Comunicação com o servidor de administração


    // Fecha o socket de comunicação
    while(1)    
    {
        printf("Aguardando conexões de clientes na porta %d\n", ntohl(data->port));

        // Aceita uma conexão do servidor de comunicação
        int client_socket = accept(communication_socket, NULL, NULL);
        if (client_socket < 0) {
            perror("Erro ao aceitar conexão do servidor de comunicação");
            close(communication_socket);
            close(data->admin_socket);
            pthread_exit(NULL);
        }
        printf("Conexão estabelecida com o jogador 1.\n");

        int client_socket2 = accept(communication_socket, NULL, NULL);
        if (client_socket2 < 0) {
            perror("Erro ao aceitar conexão do servidor de comunicação");
            close(communication_socket);
            close(data->admin_socket);
            pthread_exit(NULL);
        }

        printf("Conexão estabelecida com o jogador 2.\n");
        
        jogo(client_socket, client_socket2, data); // Função do jogo 

        /// avisa para o servidor de administração que a sala está vazia e retorna quem ganhou ou se houve erro
    }

    close(communication_socket);
}

/// @brief      Função para comunicação com o servidor de administração
/// @param data Estrutura com os dados para comunicação
/// @return     void
void comunicar_com_admin(struct admin_data* data) {


    // Montar o cabeçalho seguindo o formato 
    int buffer_size = 4 * sizeof(int) + data->ip_size;
    char* buffer = malloc(buffer_size); // Aloca o buffer dinamicamente

    if (buffer == NULL) {
        perror("Erro ao alocar memória para o buffer");
        pthread_exit(NULL);
    }



    memcpy(buffer, &data->op, sizeof(int));                         // Copiar operação
    memcpy(buffer + sizeof(int), &data->port, sizeof(int));         // Copiar porta
    memcpy(buffer + 2 * sizeof(int), &data->error, sizeof(int));    // Copiar código de erro
    memcpy(buffer + 3 * sizeof(int), &data->ip_size, sizeof(int));  // Copiar o tamanho do IP
    memcpy(buffer + 4 * sizeof(int), data->ip, data->ip_size);      // Copiar o IP

    // Enviar os dados para o servidor de administração
    if (send(data->admin_socket, buffer, buffer_size, 0) < 0) {
        perror("Erro ao enviar dados ao servidor de administração");
        free(buffer); // Libera o buffer antes de sair
        close(data->admin_socket);
        pthread_exit(NULL);
    }

    printf("Dados enviados ao servidor de administração: op=%d, port=%d, error=%d, ip=%s\n", 
           ntohl(data->op), ntohl(data->port), ntohl(data->error), data->ip);

    // Libera a memória alocada para o buffer
    free(buffer);

}

void enviarParaCliente(int socket, int op, int mensagem, int mapa_jogador[100], int mapa_adversario[100]) {
    struct servidor_para_cliente dados;
    
    // Preenche a estrutura com os dados
    dados.op = htonl(op);  // Converte para a ordem de bytes da rede
    dados.mensagem = htonl(mensagem);

    // Converte os mapas para a ordem de bytes da rede
    for (int i = 0; i < 100; ++i) {
        dados.mapa_jogador[i] = htonl(mapa_jogador[i]);
        dados.mapa_adversario[i] = htonl(mapa_adversario[i]);
    }

    // Calcula o tamanho total do buffer
    size_t tamanho_buffer = sizeof(dados.op) + sizeof(dados.mensagem) + sizeof(dados.mapa_jogador) + sizeof(dados.mapa_adversario);

    // Aloca memória para o buffer
    char *buffer = (char *)malloc(tamanho_buffer);
    if (buffer == NULL) {
        perror("Erro ao alocar memória para o buffer");
        return;
    }

    // Copia os dados para o buffer
    size_t offset = 0;
    memcpy(buffer + offset, &dados.op, sizeof(dados.op));
    offset += sizeof(dados.op);
    memcpy(buffer + offset, &dados.mensagem, sizeof(dados.mensagem));
    offset += sizeof(dados.mensagem);
    memcpy(buffer + offset, dados.mapa_jogador, sizeof(dados.mapa_jogador));
    offset += sizeof(dados.mapa_jogador);
    memcpy(buffer + offset, dados.mapa_adversario, sizeof(dados.mapa_adversario));

    // Envia o buffer de uma só vez
    ssize_t bytes_enviados = send(socket, buffer, tamanho_buffer, 0);
    if (bytes_enviados < 0) {
        perror("Erro ao enviar os dados para o cliente");
    } else {
        printf("Dados enviados para o cliente. Bytes enviados: %zd\n", bytes_enviados);
    }

    // Libera a memória alocada para o buffer
    free(buffer);
}

int main()
{
    int porta = 0;
    pthread_t threads[SALAS];
    printf("Digite a porta inicial do servidor: ");
    scanf("%d", &porta);
    for(int i = 0; i < SALAS; i++)
    {
        int* arg = malloc(sizeof(int));
        *arg = porta + i;
        printf("Criando sala %d\n", i);
        pthread_create(&threads[i], NULL, sala, (void*)arg);    // Cria uma thread para cada sala
        jogadoresNaSalas[i] = 0;
    }
    for(int i = 0; i < SALAS; i++)
    {
        pthread_join(threads[i], NULL);
    }
}