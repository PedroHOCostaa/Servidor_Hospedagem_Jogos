#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "estruturas.h"

#define PORT "8080"
#define HOST "127.0.0.1"



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
        qtdNaviosJ2[3] = qtdNaviosJ1[3] = qtdNavios[3] = 1;
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
            imprimirMapaAdversario(mapaJogadorUm);
        }
    }
    print("Jogador 1 pronto\n");
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
            imprimirMapaAdversario(mapaJogadorDois);
        }
    }
    printf("Jogador 2 pronto\n");
    int vencedor = 0;
    int alvoColuna, alvoLinha;
    while(vencedor == 0)
    {
        printf("Jogador 1 selecione a região que irá realizar o tiro\n");
        scanf("%d %d", &alvoColuna, &alvoLinha);

    }
}

int main()
{
    jogo();
}