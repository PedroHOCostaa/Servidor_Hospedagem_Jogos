import socket

# Criação do socket TCP
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Bind do socket ao IP e porta
server_socket.bind(('localhost', 12345))

# Coloca o socket em modo de escuta
server_socket.listen(1)
print("Aguardando conexão...")
while 1:
    # Aceita a conexão de um cliente
    client_socket, address = server_socket.accept()
    print(f"Conectado a {address}")

    # Recebe dados do cliente
    data = client_socket.recv(1024).decode('utf-8')
    print(f"Recebido do cliente: {data}")
    
    # Envia resposta para o cliente
    response = "Olá, cliente!"
    client_socket.send(response.encode('utf-8'))

# Fecha a conexão
client_socket.close()
server_socket.close()
