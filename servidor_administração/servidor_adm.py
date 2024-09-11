import socket
import threading
import struct

semaforo1 = threading.Semaphore(1)

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

# ===================================================== #
# | op (int 4 bytes)| jogo selecionado (int 4 bytes)  | #
# | size (int 4 bytes) |      nome (string size bytes)| #
# ===================================================== #

### Servidor de processamento -> Servidor de administração

# ============================================================ #
# | op (int 4 bytes)| port (int 4 bytes)| error (int 4 bytes)| #
# | size (int 4 bytes) |      ip (string size bytes)         | #
# ============================================================ #



class Sala:
    def __init__(self, ip, port, id):
        self.ip = ip
        self.port = port
        self.estado = 0
        self.id = id

def thread_handle_sala(conn, addr):             
    op, port, error, size = struct.unpack('>IIII', conn.recv(16))   ### Ponto 2 sala op == 1 pois indica que uma sala acabou de ser aberta
    ip = conn.recv(size).decode()       # Decodifica o ip para utf-8
    
    
    sala_atual = Sala(ip, port, port)   # Cria o objeto sala da sala que acabou de se conectar
                                                    
    semaforo1.acquire()             # ================região critica================== #
    lista_salas.append(sala_atual)  #   # Adiciona o objeto sala na lista de salas #   #
    semaforo1.release()             # ================região critica================== #    
    
    while True:
        ### Recebe uma mensagem da sala indicando que o jogo acabou op == 1 pois indica que jogo acabou
        op, resultado, erro, size = struct.unpack('>IIII', conn.recv(16))
        nome = conn.recv(size).decode()     ### Recebe o nome do jogador que venceu a partida


        ### Utilizado op para verificar se o jogo acabou por qual motivo, se foi uma vitória(op == 1) então utiliza sala_atual.id
        ### e nome do jogador vencedor para salvar no log e fazer uma leaderboard
        if resultado == 1:
            print("Jogo acabou, o jogador ", nome, " venceu a partida na sala ", sala_atual.id,"\n")
        else:
            print("Jogo da sala: ", sala_atual.id, "encerrou por motivo de erros, codigo do erro:",erro,"\n")

        ###

        ### Marca a sala como disponível 
        semaforo1.acquire()         # ================região critica================== #
        sala_atual.estado = 0       #           # marca a sala como vazia #            #
        semaforo1.release()         # ================região critica================== #
        
def thread_salas():
    socket_salas = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    socket_salas.bind(('localhost', 5000))
    socket_salas.listen()
    while True:
        conn, addr = socket_salas.accept()  ### Ponto 1 sala
        thread_handle = threading.Thread(target=thread_handle_sala, args=(conn, addr))
        thread_handle.start()
        lista_thrads_servidor.append(thread_handle)

def procura_sala_vaga():        # Toda essa função ocorre dentro de uma região critica # em um sistema poderia utizar varias listas de salas, na qual cada
                                # lista de salas teria um semaforo separado, assim evitando a ocorrencia de varios clientes ficarem esperando uma resposta
                                # de salas livres, aplicar uma lista circular que atualização a posição atual sempre que uma sala for completamente cheia
                                # para a proxima sala, que provavelmente estará vazia, pois a lista iria preencher linearmente as salas com jogadores.
    for sala in lista_salas:    
        if sala.estado == 1:    ### Procura sala que possuem um jogador em espera
            return sala
    for sala in lista_salas:
        if sala.estado == 0:    ### Procura sala vazias
            return sala
    return None     ### Retorna None explicitamente quando não encontrar uma sala disponível


### Servidor de administração -> Cliente

# ============================================================ #
# | op (int 4 bytes)| port (int 4 bytes)| error (int 4 bytes)| #
# | size (int 4 bytes) |        ip (string size bytes)       | #
# ============================================================ #


def thread_handle_cliente(conn, addr):
    semaforo1.acquire()             # ================região critica================== # Inicio
    sala = procura_sala_vaga()      
    if sala is None:
        semaforo1.release()         # ================região critica================== # Final se não encontrar sala
        mensagem =  "Não há salas disponíveis no momento".encode()
        conn.send(struct.pack('>II', 0, 0, 44, len(mensagem))) ### Envia 0 para indicar que não há sala disponível
        conn.send(mensagem)                           ### Envia a mensagem de erro para o cliente
        conn.close()
        return
    sala.estado = sala.estado + 1
    semaforo1.release()             # ================região critica================== # Final se encontrar sala
    conn.send(struct.pack('>II', sala.port, len(sala.ip)))
    conn.send(sala.ip.encode())                                 ### Envia a porta e o ip da sala para o cliente        
    ### Ponto 3 sala
    conn.close()

def thread_clientes():
    socket_clientes = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    socket_clientes.bind(('localhost', 5001))                   
    socket_clientes.listen()                                ### Espera clientes conectarem para pedir o endereço de uma sala vaga
    while True:
        conn, addr = socket_clientes.accept()               ### Cliente conectou, agora chama um thread para lidar com ele
        thread_cli = threading.Thread(target=thread_handle_cliente, args=(conn, addr))
        thread_cli.start()

def ligar_servidor():
    thread_sala_espera = threading.Thread(target=thread_salas)
    thread_sala_espera.start()
    thread_espera_clientes = threading.Thread(target=thread_clientes)
    thread_espera_clientes.start()
    while True:
        pass