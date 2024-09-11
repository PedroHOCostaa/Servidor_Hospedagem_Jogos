#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "estruturas.h"

#define PORT "65430"
#define HOST "127.0.0.1"       // IP do servidor admin
#define SALAS 5
#define MEU_IP "192.168.18.23" // IP do servidor 

int jogadoresNaSalas[SALAS];

int realizarTiroJogador(struct mapa* mapaAdversario)
{
    int colunaAlvo, linhaAlvo;
    printf("Digite a linha em que o tiro será disparado: ");
    scanf("%d", &colunaAlvo);
    printf("Digite a coluna em que o tiro será disparado: ");
    scanf("%d", &linhaAlvo);
    int resultado = realizarTiro(mapaAdversario, colunaAlvo, linhaAlvo);
    
    return resultado;
}


void jogo()
{

    //Inicialização do mapa do jogo//

    struct mapa* mapaJogadorUm = (struct mapa*)malloc(sizeof(struct mapa));;
    inicializaMapa(mapaJogadorUm);
    struct mapa* mapaJogadorDois = (struct mapa*)malloc(sizeof(struct mapa));;
    inicializaMapa(mapaJogadorDois);

    
    int tipoJogo;
    //Escolha do tipo de jogo//
    printf("Digite o tipo do jogo\n");
    scanf("%d", &tipoJogo);
    

    int qtdNavios[4], qtdNaviosJ1[4], qtdNaviosJ2[4], qtdTiros;
    if(tipoJogo == 1)
    {
        qtdNaviosJ2[0] = qtdNaviosJ1[0] = qtdNavios[0] = 1;
        qtdNaviosJ2[1] = qtdNaviosJ1[1] = qtdNavios[1] = 0;
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
    

    int orientacao, ancoraColuna, ancoraLinha;
    struct navio* novoNavio;
    int criado = 0;
    
    //Jogador 1 posiciona seus navios//
    for(int i = 0; i < 4; i++)
    {
        printf("Navio do tipo %d\n", i);
        while(qtdNaviosJ1[i] > 0)
        {
                /// Jogador seleciona posição do navio, está secção será substituida por uma operação de socket para receber estes dados
                printf("%d navios do tipo %d a serem posicionados\n", qtdNaviosJ1[i], i);
                printf("Digite a orientação do navio: ");
                scanf("%d", &orientacao);
                printf("Digite a ancora da coluna do navio: ");
                scanf("%d", &ancoraColuna);
                printf("Digite a ancora da linha do navio: ");
                scanf("%d", &ancoraLinha);
            if(criado == 0)
            {
                novoNavio = (struct navio*)malloc(sizeof(struct navio));
                inicializaNavio(novoNavio, i, orientacao);
                criado = 1;
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
            //imprimirMapaAdversario(mapaJogadorUm);
        }
    }
    printf("Jogador 1 pronto\n");
    //Jogador 2 posiciona seus navios//
    for(int i = 0; i < 4; i++)
    {
        printf("Navio do tipo %d\n", i);
        while(qtdNaviosJ2[i] > 0)
        {
                /// Jogador seleciona posição do navio, está secção será substituida por uma operação de socket para receber estes dados
                printf("%d navios restantes\n", qtdNaviosJ2[i]);
                printf("Digite a orientação do navio: ");
                scanf("%d", &orientacao);
                printf("Digite a ancora da coluna do navio: ");
                scanf("%d", &ancoraColuna);
                printf("Digite a ancora da linha do navio: ");
                scanf("%d", &ancoraLinha);
            if(criado == 0)
            {
                novoNavio = (struct navio*)malloc(sizeof(struct navio));
                inicializaNavio(novoNavio, i, orientacao);
                criado = 1;
                printf("Navio criado\n");
            }
            printf("Navio do tipo %d orientação %d na posicao %d %d\n", novoNavio->tipo, novoNavio->orientacao, ancoraColuna, ancoraLinha);
            if(insereNavio(mapaJogadorDois, novoNavio, ancoraColuna, ancoraLinha) == 0)
            {
                qtdNaviosJ2[i]--;
                criado = 0;
                    /// Servidor envia uma mensagem sobre o resultado para o servidor de comunicação
                    printf("Navio inserido\n");
            }else
            {
                    /// Servidor envia uma mensagem sobre o resultado para o servidor de comunicação
                    printf("Navio não inserido\n"); 
            }
            imprimirMeuMapa(mapaJogadorDois);
            //imprimirMapaAdversario(mapaJogadorDois);
        }
    }
    printf("Jogador 2 pronto\n");
    int vencedor = 0;
    int alvoColuna, alvoLinha;
    while(vencedor == 0)
    {
        printf("passou\n");
        if(vencedor == 0)
        {
            for(int i = 0; i < qtdTiros; i++)
            {
                    /// Jogador 1 seleciona posição do tiro, está secção será substituida por uma operação de socket para receber estes dados
                    printf("Jogador 1 selecione a região que irá realizar o tiro\n");
                    scanf("%d %d", &alvoColuna, &alvoLinha);
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
                        printf("Jogador 1 venceu\n");
                        vencedor = 1;
                        i = qtdTiros;
                    }
                }
                imprimirMapaAdversario(mapaJogadorDois);
            }
        }
        if(vencedor == 0)
        {
            for(int i = 0; i < qtdTiros; i++)
            {
                    /// Jogador 2 seleciona posição do tiro, está secção será substituida por uma operação de socket para receber estes dados
                    printf("Jogador 2 selecione a região que irá realizar o tiro\n");
                    scanf("%d %d", &alvoColuna, &alvoLinha);
                int resultado = realizarTiro(mapaJogadorUm, alvoColuna, alvoLinha);
                if(resultado == 0)
                    printf("Tiro na agua!\n");
                if(resultado == 1)
                    printf("Tiro acertou um navio\n");
                if(resultado == 2)
                {
                    printf("Tiro acertou um navio e o afundou\n");
                    if(verificaPerdedor(mapaJogadorUm) == 1)
                    {
                        printf("Jogador 1 venceu\n");
                        vencedor = 2;
                        i = qtdTiros;
                    }
                }
                imprimirMapaAdversario(mapaJogadorUm);
            }        
        }
    }
    printf("Fim de jogo\n Jogador %d venceu!!!/n", vencedor);
}


void* sala(void* arg)
{
 struct sockaddr_in endereco_servidor_admin;
    int porta_sala = *((int*)arg); // Porta da sala
    char* ip_sala = MEU_IP; // IP da sala
    free(arg);

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
    data->op = 1;                       // Operação de criação de sala
    data->port = porta_sala;            // Porta da sala (exemplo)
    data->error = 0;                    // Código de erro
    data->ip = strdup(ip_sala);         // IP (UTF-8)
    data->ip_size = strlen(data->ip);   // Tamanho do IP
    data->admin_socket = admin_socket;  // Socket de conexão com o servidor de administração
    

    /// Cria um socket para esperar conexões do servidor de comunicação
    /// Envia para a thread do servidor de administração o id desta sala e a porta que irá esperar uma conexão de cliente
    /// Ponto 2
    comunicar_com_admin(data);

    while(1)    
    {
        /// Ponto 3 primeiro jogador
        /// Aceita uma conexão do servidor de comunicação
        /// Avisa que a sala está em espera de um jogador

        /// Ponto 3 segundo jogador
        /// Aceita uma segunda conexão com o servidor de comunicaçãp

        jogo();

        /// avisa para o servidor de administração que a sala está vazia e retorna quem ganhou ou se houve erro
        /// Ponto 4
    }

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

    // Converter os dados para big-endian
    int net_op = htonl(data->op); 
    int net_port = htonl(data->port);
    int net_error = htonl(data->error);
    int net_ip_size = htonl(data->ip_size);

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
           data->op, data->port, data->error, data->ip);

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