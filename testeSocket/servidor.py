import socket
import struct

# Criação do socket TCP
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Bind do socket ao IP e porta
server_socket.bind(('localhost', 5000))

# Coloca o socket em modo de escuta
server_socket.listen(1)
print("Aguardando conexão...")
while 1:
    # Aceita a conexão de um cliente
    client_socket, address = server_socket.accept()
    print(f"Conectado a {address}")

    # Recebe dados do cliente (binário)
    op, port, error, size = struct.unpack('!IIII', client_socket.recv(16))  # 4 inteiros (op, port, error, ip_size)
    
    # Desempacota os dados binários (big-endian)
    ip = client_socket.recv(size).decode('utf-8')    # Decodificar o IP de acordo com o tamanho
    
    print(f"Operação: {op}, Porta: {port}, Erro: {error}, IP: {ip}")
    if op == 0:
        if error == -1:
            print("erro na tua mae")
        else:
            print("erro na tua tia")
        print(f"operação 0 -> {error}")
    # Envia resposta para o cliente
    response = "Minha tia não!"
    client_socket.send(response.encode('utf-8'))
# Fecha a conexão
client_socket.close()
server_socket.close()
