import socket
import threading
import struct

semaforo1 = threading.Semaphore(1)

lista_salas = []
lista_thrads_servidor = []


class Sala:
    def __init__(self, ip, port):
        self.ip = ip
        self.port = port
        self.estado = 0


def thread_handle_sala(conn, addr):             
    id, estado, port = struct.unpack('>III', conn.recv(12))
    
    sala_atual = Sala(addr[0], port)
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
        conn, addr = socket_salas.accept()
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
    return ligar_servidor()     ### Se não encontrar nenhuma sala vazia ou com jogador em espera, ativa um novo servidor de processamento

def thread_handle_cliente(conn, addr):
    semaforo1.acquire()
    sala = procura_sala_vaga()
    sala.estado = sala.estado + 1
    semaforo1.release()
    conn.send(struct.pack('>II', sala.port, len(sala.ip)))
    conn.send(sala.ip.encode())

def thread_clientes():
    socket_clientes = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    socket_clientes.bind(('localhost', 5001))
    socket_clientes.listen()
    while True:
        conn, addr = socket_clientes.accept()
        thread_cli = threading.Thread(target=thread_handle_cliente, args=(conn, addr))

def ligar_servidor():
    pass