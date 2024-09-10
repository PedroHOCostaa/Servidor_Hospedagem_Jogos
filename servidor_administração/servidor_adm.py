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

### Servidor de processamento -> Servidor de salas

# =================================================================== #
# | op (int 4 bytes)| port (int 4 bytes)|     error (int 4 bytes)   | #
# | size (int 4 bytes) |            ip (string size bytes)          | #
# =================================================================== #



class Sala:
    def __init__(self, ip, port, id):
        self.ip = ip
        self.port = port
        self.estado = 0
        self.id = id

def thread_handle_sala(conn, addr):             
    id, port = struct.unpack('>II', conn.recv(8))   ### Ponto 2 sala
    
    
    sala_atual = Sala(addr[0], port, id)
    semaforo1.acquire()
    lista_salas.append(sala_atual)
    semaforo1.release()
    
    while True:
        tipo = struct.unpack('>I', conn.recv(4))[0]
        semaforo1.acquire()
        if tipo == 10:              ### Jogo acabou
            sala_atual.estado = 0
        semaforo1.release()
        
def thread_salas():
    socket_salas = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    socket_salas.bind(('localhost', 5000))
    socket_salas.listen()
    while True:
        conn, addr = socket_salas.accept()  ### Ponto 1 sala
        thread_handle = threading.Thread(target=thread_handle_sala, args=(conn, addr))
        thread_handle.start()
        lista_thrads_servidor.append(thread_handle)

def procura_sala_vaga():
    for sala in lista_salas:
        if sala.estado == 1:    ### Procura sala que possuem um jogador em espera
            return sala
    for sala in lista_salas:
        if sala.estado == 0:    ### Procura sala vazias
            return sala
    return None     ### Retorna None explicitamente quando não encontrar uma sala disponível

def thread_handle_cliente(conn, addr):
    semaforo1.acquire()
    sala = procura_sala_vaga()
    if sala is None:
        semaforo1.release()
        conn.send(struct.pack('>II', 0, 0)) ### Envia 0 para indicar que não há sala disponível
        return
    sala.estado = sala.estado + 1
    semaforo1.release()
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