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

### Sala de jogo -> Cliente

# ========================================= #
# |op (int 4 bytes)| mensagem(int 4 bytes)| #
# |    mapa jogador (100 int 4 bytes)     | #
# |  mapa adversario (100 int 4 bytes)    | #       
# ========================================= #
            #        #
            #   ||   #
            #   ||   #
            #  \||/  #
            #   \/   #
            #        #
### Cliente -> Sala de jogo

# ================================================ #
# | op (int 4 bytes)|    coluna (int 4 bytes)    | #
# | linha (int 4 bytes)| orientacao (int 4 bytes)| #
# | size (int 4 bytes) |   nome (string size)    | #
# ================================================ #

def printa_mapa(mapa):
    print(" \t0\t1\t2\t3\t4\t5\t6\t7\t8\t9")
    for i in range(10):
        print(i, end="")
        for j in range(10):
            print("\t",mapa[i*10 + j], end="")
        print()



def main():
        ### Conexão com o servidor de administração ###
    socket_servidor_adm = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    socket_servidor_adm.connect(endereco_servidor_adm)
    print("Conectado ao servidor de administração\n")


    # Escolhe o nome e o jogo que deseja jogar e envia para o servidor de administração um pedido de sala para este jogo
        # ====================================================== #
        # | op (int 4 bytes)| jogo selecionado (int 4 bytes)   | #
        # | size (int 4 bytes) | nome (string utf-8 size bytes)| #
        # ====================================================== #

    nome = input("Escolha seu nome: ").encode()                                 
    jogo = int(input("Selecione o jogo que quer jogar\n\tDigite 1 para batalha naval: "))
    socket_servidor_adm.send(struct.pack('!III', 1, jogo, len(nome)))           ### Envia 1(pedido de jogo), o jogo selecionado e o tamanho do nome
    socket_servidor_adm.send(nome)                                              ### Envia o nome do jogador


    # Recebe a resposta do servidor de administração
        # ============================================================ #
        # | op (int 4 bytes)| port (int 4 bytes)| error (int 4 bytes)| #
        # | size (int 4 bytes) |        ip (string size bytes)       | #
        # ============================================================ #

    op, port, error, size = struct.unpack('!IIII', socket_servidor_adm.recv(16))    ### Recebe a operação, a porta, o erro e tamanho da mensgaem
    mensagem = socket_servidor_adm.recv(size).decode()                              ### Recebe a mensagem

    if(op == 0):                                ### Conexão com a sala não foi possível 
        print("Erro: ", error, " - ", mensagem) 
        socket_servidor_adm.close()             ### Fecha o socket de comunicação com o servidor de administração

    else:                                       ### Ip e porta da sala adquiridos com sucesso 
        socket_servidor_adm.close()             ### Fecha o socket de comunicação com o servidor de administração


            # Conexão com o socket da sala, por meio do ip e porta adquiridos pelo servidor de adminitração
        print("Conectando a sala de ip: ", mensagem, " e porta: ", port)
        socket_sala = socket.socket(socket.AF_INET, socket.SOCK_STREAM)     ### Cria o socket da sala de jogo e o configura
        socket_sala.connect((mensagem, port))                               ### Conecta a sala de jogo
        print("Conectado a sala de jogo")
        print("Aguarde o inicio do jogo")

        while True:

                # ========================================= #
                # |op (int 4 bytes)| mensagem(int 4 bytes)| #           
                # |    mapa jogador (100 int 4 bytes)     | #           # Mensagem recebida da sala de jogo #
                # |  mapa adversario (100 int 4 bytes)    | #           
                # ========================================= #
            print("Esperando mensagem da sala")
            dados = socket_sala.recv(808)

            # Recebe os dados do socket
            valores = struct.unpack('!II100I100I', dados)

            # Atribui os valores às variáveis apropriadas
            op = valores[0]
            mensagem = valores[1]
            mapa_jogador = valores[2:102]
            mapa_adversario = valores[102:202]
            print("Pacote recebido da sala, op: ", op, " mensagem: ", mensagem)

            if(op == 1):              ### Jogador escolhe aonde irá colocar o Navio ###
                print("Mapa jogador")
                printa_mapa(mapa_jogador)
                print("Navio de tipo: ", mensagem)  ### Mensgem que indica o tipo do navio que o jogador deve colocar
                orientacao = int(input("Digite 0 para orientação original e 1 para invertida: "))
                coluna = int(input("Digite a posição na coluna: "))
                linha = int(input("Digite a posição na linha: "))
                cabecalho = struct.pack('!IIIII', 1, coluna, linha, orientacao, len(nome))

            elif(op == 2):              ### Jogador realiza um disparo ###
                print("Mapa jogador")
                printa_mapa(mapa_jogador)
                print("Mapa adversario")
                printa_mapa(mapa_adversario)
                coluna = int(input("Digite a posição na coluna para realizar um disparo: "))
                linha = int(input("Digite a posição na linha para realizar um disparo: "))
                cabecalho = struct.pack('!IIIII', 2, coluna, linha, 0, len(nome))

            elif(op == 3):              ### Jogo acabou ###
                if mensagem == 1:
                    print("O jogo acabou, você ganhou")
                elif mensagem == 2:
                    print("O jogo acabou, você perdeu")
                else:
                    print("O jogo acabou, por motivos de erro, erro numero: ", mensagem)
                printa_mapa(mapa_jogador)
                printa_mapa(mapa_adversario)
                break

            elif(op == 4):              ### Jogador decide qual será o tipo do jogo ###
                tipo_do_jogo = int(input("Escolha qual o tipo do jogo que deseja jogar\n\tDigite 1 para batalhar com: 1 navio de cada tipo\n\tDigite 2 para batalhar com 2 navios do tipo um e dois e 1 navio do tipo 3 e 4"))
                cabecalho = struct.pack('!IIIII', 4, 0, 0, tipo_do_jogo, len(nome))
            

                # ================================================ #        # ======================================================================== #
                # | op (int 4 bytes)|    coluna (int 4 bytes)    | #        # op indica qual operação será realizada, coluna e linha indicam a posição #
                # | linha (int 4 bytes)| orientacao (int 4 bytes)| #        # se op == 1 orientacao indica a orientação do navio                       #
                # | size (int 4 bytes) |   nome (string size)    | #        # se op == 4 orientacao indica o tipo do jogo que será jogado              #
                # ================================================ #        # ======================================================================== #

            socket_sala.send(cabecalho + nome)      ### Envia a mensagem para a sala de jogo ###
        socket_sala.close()                         ### Fecha o socket de comunicação com a sala de jogo

main()