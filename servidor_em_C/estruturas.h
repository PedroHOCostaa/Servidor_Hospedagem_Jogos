#include <stdio.h>
#include <stdlib.h>

struct navio;
struct celula;
struct mapa;
struct no;
struct admin_data;

void inserirFinalLista(struct no* lista, struct navio* novonavio);
int inicializaNavio(struct navio* novoNavio, int tipo, int orientacao);
void inicializaMapa(struct mapa* novoMapa);
int insereNavio(struct mapa* meuMapa, struct navio* novoNavio, int ancoraColuna, int ancoraLinha);
int atira(struct mapa* meuMapa, int coluna, int linha);
void acertaNavio(struct navio* navio, int coluna, int linha);
int realizarTiro(struct mapa* meuMapa, int coluna, int linha);
int verificarEstrtura(struct navio* navio);
void pintaestrutura(struct mapa* meuMapa,struct navio* navio);
void imprimirMeuMapa(struct mapa* meuMapa);
void imprimirMapaAdversario(struct mapa* meuMapa);
void imprimeNavio(struct navio* navio);
int verificaPerdedor(struct mapa* meuMapa);
int vefificaPerdedorAux(struct no* lista);
void comunicar_com_admin(struct admin_data* data);
void enviarParaCliente(int socket, int op, int mensagem, int mapa_jogador[100], int mapa_adversario[100]);


/// @brief      Estrutura de lista encadeada
struct no {
    struct navio* navio;
    struct no* prox;
};

/// @brief      Estrutura de um navio
struct navio {
    int ancoraColuna, ancoraLinha, tamanhoColuna, tamanhoLinha;         // exemplo1: para um barco {{1, 1}} possui tamanhoColuna 1 e tamanhoLinha 2 e se ancora estiver em {0,0} 
    int orientacao;                                                     // o barco estará em {0,0} e {0,1}
    int tipo;                                                           // exemplo2: para um barco {{1,1,1},{1,1,1}} possui tamanhoColuna 2 e tamanhoLinha 3 e se ancora estiver em {0,0}
    int** estrutura;                                                    // o barco estará em {0,0}, {0,1}, {0,2}, {1,0}, {1,1}, {1,2}
    int estado;                                                         
};

struct celula {
    int indicador;
    struct navio* superficie;
};

struct mapa {
    struct no* listaBarcos;
    int numBarcos;
    struct celula tabuleiro[10][10];
    char mapaAdversario[10][10];
};
// Definição da estrutura admin_data
struct admin_data {
    int op;             // Operação
    int port;           // Porta
    int error;          // Código de erro
    int ip_size;        // Tamanho do IP
    char* ip;           // IP em formato UTF-8
    int admin_socket;   // Socket de conexão com o servidor de administração
};
// Definição da estrutura cliente_para_servidor
// Nessa estrutura o cliente envia dados à sala do servidor de processamento
struct cliente_para_servidor {
    int op;             // Operação
    int coluna;         // Coluna
    int linha;          // Linha
    int orientacao;     // Orientação
    int size_nome;      // Tamanho do nome do jogador
    char nome_jogador[256]; // Nome do jogador (tamanho fixo para simplificar)
};

// Definição da estrutura servidor_para_cliente
// Nessa estrutura o servidor envia dados ao cliente
struct servidor_para_cliente {
    int op;                     // Operação
    int mensagem;               // Resultado da operação
    int mapa_jogador[100];      // Mapa do jogador 
    int mapa_adversario[100];   // Mapa do adversário
};