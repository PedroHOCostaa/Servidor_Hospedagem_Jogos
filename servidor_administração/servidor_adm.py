import socket
import threading
import struct

semaforo1 = threading.Semaphore(1)  ### Semaforo utilizado na região critica para leitura e escrita da lista de salas

lista_salas = []
lista_thrads_servidor = []



### ========================= ###
### Protocolos de comunicação ###
### ========================= ###

### Servidor de administração -> Cliente

# ============================================================ #
# | op (int 4 bytes)| port (int 4 bytes)| error (int 4 bytes)| #
# | size (int 4 bytes) |        ip (string size bytes)       | #
# ============================================================ #

### Cliente -> Servidor de administração

# ====================================================== #
# | op (int 4 bytes)| jogo selecionado (int 4 bytes)  |  #
# | size (int 4 bytes) | nome (string utf-8 size bytes)| #
# ====================================================== #

### Cliente -> Sala de jogo

# ================================================ #
# | op (int 4 bytes)|    coluna (int 4 bytes)    | #
# | linha (int 4 bytes)| orientacao (int 4 bytes)| #
# | size (int 4 bytes) |   nome (string size)    | #
# ================================================ #



### Sala de jogo -> Cliente

# ========================================= #
# |op (int 4 bytes)| mensagem(int 4 bytes)| #
# |    mapa jogador (100 int 4 bytes)     | #
# |  mapa adversario (100 int 4 bytes)    | #       
# ========================================= #




class Sala:
    def __init__(self, ip, port, id):
        self.ip = ip
        self.port = port
        self.estado = 0
        self.id = id
        
    ### Sala do servidor de processamento -> Servidor de administração

    # ============================================================ #
    # | op (int 4 bytes)| port (int 4 bytes)| error (int 4 bytes)| #    ### Nota: Pode-se reparar que as salas enviam mensagens para o servidor de administração
    # | size (int 4 bytes) |    ip (string utf-8 size bytes)     | #    ### mas o servidor de administração não envia mensagens para as salas
    # ============================================================ #

def thread_handle_sala(conn, addr):     ### Thread é iniciada quando uma sala se conecta ao socket que esperá conexão do servidor de processamento

    op, port, error, size = struct.unpack('!IIII', conn.recv(16))   ### Recebe e desempacota a mensagem da sala
                                                                    ### Indicando a porta que está esperando conexões de clientes
    ip = conn.recv(size).decode()                                   ### Recebe o ip do socket que aguarda conexões de clientes
    print("Sala de ip: ", ip, " e porta: ", port, " conectada\n, operação: ", op, "\n")

    
    sala_atual = Sala(ip, port, port)   # Cria o objeto sala da sala que acabou de se conectar                                     
    semaforo1.acquire()             # ================região critica================== #
    lista_salas.append(sala_atual)  #   # Adiciona o objeto sala na lista de salas #   #
    semaforo1.release()             # ================região critica================== #    
    
    ### Esta thread agora ficará esperando atualizações da sala
    while True:

        ### Recebe uma mensagem da sala indicando que o jogo acabou
        op, resultado, erro, size = struct.unpack('!IIII', conn.recv(16))   ### Recebe e desempacota a mensagem enviada pela sala
        nome = conn.recv(size).decode()                                     ### Recebe o nome do jogador que venceu a partida


        ### Utilizado op para verificar se o jogo acabou por qual motivo, se foi uma vitória ou erro
        ### Utiliza o endereço da sala e nome do jogador vencedor para salvar no log e fazer uma leaderboard
        if resultado == 1:  ### Indica que o jogo acabou por vitória
            print("Jogo acabou, o jogador ", nome, " venceu a partida na sala ", sala_atual.id,"\n")
        else:               ### Indica que o jogo acabou por erro
            print("Jogo da sala: ", sala_atual.id, "encerrou por motivo de erros, codigo do erro:",erro,"\n")


        ### Marca a sala como disponível para um novo jogo
        semaforo1.acquire()         # ================região critica================== #
        sala_atual.estado = 0       #           # marca a sala como vazia #            #
        semaforo1.release()         # ================região critica================== #
        
def thread_salas():                     ### Thread que espera conexões de salas do servidor de processamento
    ### Cria o socket que espera as salas do servidor de processamento se conectarem
    socket_salas = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    socket_salas.bind(('127.0.0.1', 5000))
    socket_salas.listen()
    
    while True:
        
        conn, addr = socket_salas.accept()  ### Um cliente sala conseguiu se consectar ao socket, agora chama um thread para lidar com ele
        thread_handle = threading.Thread(target=thread_handle_sala, args=(conn, addr))
        thread_handle.start()               ### Inicia a thread que irá lidar com a sala
        lista_thrads_servidor.append(thread_handle)


def procura_sala_vaga():        ### Toda essa função ocorre dentro de uma região critica que é protegida por um semaforo, procura uma sala vaga para um cliente
    for sala in lista_salas:    
        if sala.estado == 1:    ### Procura sala que possuem um jogador em espera
            return sala
    for sala in lista_salas:
        if sala.estado == 0:    ### Procura sala vazias
            return sala
    return None     ### Retorna None quando não encontrar uma sala disponível

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
                                            ### Thread que é iniciada quando um cliente se conecta ao servidor de administração solicitando uma sala
def thread_handle_cliente(conn, addr):      ### Está thread então envia para o cliente a sala encontrada ou uma mensagem de erro
    
    ### Recebe a mensagem do cliente
        # ====================================================== #
        # | op (int 4 bytes)| jogo selecionado (int 4 bytes)  |  #
        # | size (int 4 bytes) | nome (string utf-8 size bytes)| #
        # ====================================================== #

    op, jogo_selecionado, size = struct.unpack('!III', conn.recv(12)) ### Recebe do cliente a operação, qual jogo ele quer jogar e size de nome
    nome = conn.recv(size).decode()                                   ### Recebe o nome do jogador


    if(jogo_selecionado == 1):  ### Indica que o jogo selecionado é batalho naval

        ### Entra na região critica para procurar uma sala vaga para o cliente
        semaforo1.acquire()             # ================região critica================== # Inicio
        
        ### Primeiramente procura salas com um jogador esperando, depois procura salas vazias
        sala = procura_sala_vaga()

        if sala is None:            ### Se não encontrar nenhuma sala disponível
            semaforo1.release()         # ================região critica================== # Final se não encontrar sala

            mensagem =  "Não há salas disponíveis no momento".encode()  ### Salva a mensagem de erro em mensagem
                                                                        ### 0 para indicar que algum erro ocorreu, e codigo 44 = Não há salas disponíveis
            cabecalho = struct.pack('!IIII', 0, 0, 44, len(mensagem))   ### Salva os dados em cabecalho

        else:                       ### Se encontrar uma sala disponível
            sala.estado = sala.estado + 1   ### Aumenta o estado da sala em 1, mostrando que um jogador está ocupando uma vaga
            semaforo1.release()             # ================região critica================== # Final se encontrar sala

            mensagem = sala.ip.encode()                                         ### Salva o ip da sala em mensagem
            cabecalho = struct.pack('!IIII', 1, sala.port, 0, len(sala.ip))     ### Envia 1 para indicar que há sala disponível, a porta da sala, erro = 0
            print("Jogador ", nome, " irá se conectar na sala de ip: ", sala.ip, "e porta: ", sala.port, "\n")      ### Pode ser substituido por um log  
            ### Ponto 3 sala
    
    
    else:       ### Jogo selecionado não está disponível, então envia uma mensagem de erro para o cliente
        mensagem =  "Jogo não disponível".encode()                  ### Salva a mensagem de erro em mensagem
                                                                    ### 0 para indicar que algum erro ocorreu, e codigo 45 = Jogo selecionado não está disponível
        cabecalho = struct.pack('!IIII', 0, 0, 45, len(mensagem))   ### Salva os dados no cabecalho


    ### Envia a mensagem para o cliente
        # ============================================================ #
        # | op (int 4 bytes)| port (int 4 bytes)| error (int 4 bytes)| #
        # | size (int 4 bytes) |        ip (string size bytes)       | #
        # ============================================================ #
    conn.send(cabecalho)        ### Envia o cabeçalho para o cliente, indicando o ip e a porta da sala solicitada
    conn.send(mensagem)         ### Envia a mensagem para o cliente, que pode ser um ip ou uma mensagem de erro
    conn.close()

def thread_clientes():                      ### Thread que espera conexões de clientes solicitando uma sala
    socket_clientes = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    socket_clientes.bind(('localhost', 5001))                   
    socket_clientes.listen()                                ### Espera clientes conectarem para pedir o endereço de uma sala vaga
    while True:
        conn, addr = socket_clientes.accept()               ### Cliente conectou
        thread_cli = threading.Thread(target=thread_handle_cliente, args=(conn, addr))
        thread_cli.start()                                  ### Inicia uma thread para lidar com ele


def ligar_servidor():
    thread_sala_espera = threading.Thread(target=thread_salas)
    thread_sala_espera.start()                                          ### Inicia a thread que espera as salas do servidor de processamento se conectarem
    thread_espera_clientes = threading.Thread(target=thread_clientes)
    thread_espera_clientes.start()                                      ### Inicia a thread que espera os clientes se conectarem solocitando uma sala
    while True:
        pass

ligar_servidor()