import socket
import struct

endereco_servidor_adm = ('localhost', 5001)


### Cliente -> Servidor de administração

# ====================================================== #
# | op (int 4 bytes)| jogo selecionado (int 4 bytes)  |  #
# | size (int 4 bytes) | nome (string utf-8 size bytes)| #
# ====================================================== #
            #        #
            #   ||   #
            #   ||   #
            #  \||/  #
            #   \/   #
            #        #
### Servidor de administração -> Cliente

# ============================================================ #
# | op (int 4 bytes)| port (int 4 bytes)| error (int 4 bytes)| #
# | size (int 4 bytes) |        ip (string size bytes)       | #
# ============================================================ #

### Cliente -> Sala de jogo

# ================================================ #
# | op (int 4 bytes)|    coluna (int 4 bytes)    | #
# | linha (int 4 bytes)| orientacao (int 4 bytes)| #
# | size (int 4 bytes) |   nome (string size)    | #
# ================================================ #
            #        #
            #   ||   #
            #   ||   #
            #  \||/  #
            #   \/   #
            #        #
### Sala de jogo -> Cliente

# ======================================= #
# |op (int 4 bytes)| error (int 4 bytes)| #
# |    mapa jogador (100 int 4 bytes)   | #
# |  mapa adversario (100 int 4 bytes)  | #       
# ======================================= #


def printa_mapa_jogador(mapa):
    print("Mapa do jogador")
    print("  0 1 2 3 4 5 6 7 8 9")
    for i in range(10):
        print(i, end=" ")
        for j in range(10):
            print(mapa[i*10 + j], end=" ")
        print()


def printa_mapa_adversario(mapa):
    print("Mapa do adversário")
    print("  0 1 2 3 4 5 6 7 8 9")
    for i in range(10):
        print(i, end=" ")
        for j in range(10):
            print(mapa[i*10 + j], end=" ")
        print()


def main():
    
    socket_servidor_adm = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    socket_servidor_adm.connect(endereco_servidor_adm)
    print("Conectado ao servidor de administração\n")
    nome = input("Nome: ").encode()
    jogo = int(input("Selecione o jogo que quer jogar\n\tDigite 1 para batalha naval: "))
    socket_servidor_adm.send(struct.pack('!III', 1, jogo, len(nome))) ### Envia a operação, o jogo selecionado e o tamanho do nome
    socket_servidor_adm.send(nome)                                    ### Envia o nome do jogador
    op, port, error, size = struct.unpack('!IIII', socket_servidor_adm.recv(16)) ### Recebe a operação, a porta, o erro e o tamanho da mensgaem
    mensagem = socket_servidor_adm.recv(size).decode()                            ### Recebe a mensagem
    if(op == 0):
        print("Erro: ", error, " - ", mensagem) ### Conexão com a sala não foi possível
        socket_servidor_adm.close()             ### Fecha o socket de comunicação com o servidor de administração
    else:
        socket_servidor_adm.close()             ### Fecha o socket de comunicação com o servidor de administração
        print("Conectado a sala de ip: ", endereco_servidor_adm[0], " e porta: ", port)
        socket_sala = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        socket_sala.connect((mensagem, port))   ### Conecta a sala de jogo
        print("Conectado a sala de jogo")
        print("Aguarde o inicio do jogo")
        conexão = True
        while conexão:
            op, error, mapa_jogador, mapa_adversario = struct.unpack('!II100I100I', socket_sala.recv(808))
            if(op == -1):
                print("Erro: ", error)
                conexão = False
            elif(op == 1):
                printa_mapa_jogador(mapa_jogador)
                orientacao = int(input("Digite 0 para orientação original e 1 para invertida: "))
                coluna = int(input("Digite a posição na coluna: "))
                linha = int(input("Digite a posição na linha: "))
                socket_sala.send(struct.pack('!IIII', 1, coluna, linha, orientacao, len(nome), nome))
            elif(op == 2):
                printa_mapa_jogador(mapa_jogador)
                printa_mapa_adversario(mapa_adversario)
                coluna = int(input("Digite a posição na coluna para realizar um disparo: "))
                linha = int(input("Digite a posição na linha para realizar um disparo: "))
                socket_sala.send(struct.pack('!IIII', 2, coluna, linha, 0, len(nome), nome))
            elif(op == 3):
                printa_mapa_jogador(mapa_jogador)
                printa_mapa_adversario(mapa_adversario)
                print("O jogo acabou, você ganhou")
                conexão = False
            elif(op == 4):
                printa_mapa_jogador(mapa_jogador)
                printa_mapa_adversario(mapa_adversario)
                print("O jogo acabou, você perdeu")
                conexão = False