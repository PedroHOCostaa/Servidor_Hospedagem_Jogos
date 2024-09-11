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
        

    