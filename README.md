# Projeto de Sistemas Distribuídos - Jogo de Batalha Naval

Este projeto é um sistema distribuído para a hospedagem de salas de jogos, especificamente para o jogo de Batalha Naval. O sistema é composto por três componentes principais: o Servidor de Processamento, o Servidor Mestre (ou Servidor de Administração) e o Cliente. A comunicação entre esses componentes é realizada através de sockets e mensagens estruturadas.

## Autores

- **Marcos Rampaso** - Desenvolvedor 
- **Pedro Costa** - Desenvolvedor

## Professor Orientador

- **Rodrigo Campiolo** - Professor Orientador

## Arquitetura do Projeto

A arquitetura do projeto foi definida com base na Imagem 1.

![Ilustração da arquitetura](Arquitetura_e_fluxo_da_comunicação_SD.drawio.png)


## Componentes do Sistema e Tecnologias

O sistema é composto por diversos componentes desenvolvidos utilizando diferentes tecnologias. As principais tecnologias usadas são:

- **Python**: Utilizado para o desenvolvimento do cliente e do servidor de administração.
- **Linguagem C**: Utilizada para o desenvolvimento do servidor de processamento.
  
### Servidor de Administração (Python)

O servidor de administração gerencia as solicitações dos clientes e coordena a criação e o estado das salas de jogos. Ele se comunica com o servidor de processamento e com os clientes para fornecer informações sobre salas disponíveis e para iniciar e finalizar jogos.

### Servidor de Processamento (C)

O servidor de processamento lida com a criação e a gestão das salas de jogos. Ele aguarda conexões de clientes e gerencia o fluxo do jogo de batalha naval. Quando dois clientes se conectam a uma sala, o servidor de processamento inicia o jogo e gerencia a comunicação entre os jogadores.

### Cliente (Python)

O cliente se conecta ao servidor de administração para solicitar uma sala e iniciar o jogo. Após receber o endereço da sala, o cliente se conecta ao servidor de processamento correspondente e participa do jogo de batalha naval.

## Funcionamento dos componentes e comunicação

### Servidor de Hospedagem de Jogos

O sistema distribuído para hospedagem de salas de jogos é projetado para gerenciar a criação e o gerenciamento de salas de jogos. O fluxo de comunicação é o seguinte:

1. **Cliente**: Solicita ao Servidor Mestre o endereço de uma sala para jogar.
2. **Servidor Mestre**: Envia o endereço da sala para o cliente.
3. **Cliente**: Conecta-se ao endereço fornecido para jogar Batalha Naval (o único jogo implementado para este projeto).

O sistema é composto pelos seguintes processos principais:

### Servidor de Processamento

- **Inicialização**: O servidor de processamento solicita um número de porta inicial `X` pelo terminal.
- **Criação de Salas**: Inicializa `N` threads para salas, cada uma associada a uma porta no intervalo de `X` a `X + N - 1`.
- **Conexão com o Servidor Mestre**: Cada sala se conecta ao servidor mestre e notifica o endereço e a porta que está aguardando conexões.
- **Gerenciamento do Jogo**: Quando dois clientes se conectam a uma sala, o jogo é iniciado. Os clientes trocam mensagens com a sala até que o jogo termine.
- **Finalização**: Após o término do jogo, a sala informa o servidor mestre sobre o resultado e indica que está disponível para novos clientes.

### Cliente

- **Solicitação de Sala**: O cliente se conecta ao servidor mestre e solicita uma sala.
- **Conexão com a Sala**: Após receber o endereço da sala, o cliente se desconecta do servidor mestre e se conecta ao socket da sala para iniciar o jogo.
- **Participação no Jogo**: O cliente escolhe onde posicionar seus navios e onde disparar. Após o término do jogo, o processo é encerrado.

### Servidor Mestre ou Servidor de Administração

- **Conexão com Salas**: O servidor mestre possui um socket que aguarda conexões de salas. Para cada nova conexão, uma thread é criada para lidar com a sala:
  - **Inicialização da Sala**: Inicializa a sala antes do primeiro jogo, salva seu endereço em um objeto sala e o coloca em uma fila de salas.
  - **Atualização do Estado**: Após o término de cada jogo, o estado da sala é atualizado.

- **Conexão com Clientes**: O servidor mestre possui um segundo socket que aguarda conexões de clientes:
  - **Atendimento ao Cliente**: Quando um cliente se conecta, uma thread é criada para atendê-lo. A thread busca uma sala disponível para o cliente usando a lista de salas.
  - **Envio de Endereço da Sala**: Após encontrar uma sala disponível, atualiza o estado da sala e envia seu endereço ao cliente.
  - **Encerramento**: Após o envio do endereço, a thread que lida com o cliente é encerrada e os sockets são fechados.

**Sincronização e Proteção de Dados**:
- As operações de escrita no estado da sala e a busca e atualização do valor na busca da sala para o cliente são protegidas com uma região crítica usando um semáforo. Isso garante a consistência dos dados.

## Cabeçalhos de Comunicação

### Salas -> Servidor Mestre

| Campo  | Tipo       | Descrição                                      |
|--------|------------|------------------------------------------------|
| op     | int (4 bytes) | Tipo de operação: 1 -> Criação de sala, 2 -> Atualização de sala após finalizar jogo |
| port   | int (4 bytes) | Porta que o socket da sala está esperando conexões de clientes. Se `op = 2`, indica o resultado do jogo. |
| error  | int (4 bytes) | Código do erro, se ocorrer algum erro.         |
| size   | int (4 bytes) | Tamanho em UTF-8 do IP.                        |
| ip     | string UTF-8 (size bytes) | IP da sala que está esperando conexões de clientes. |

**Descrição Adicional:**  
Se o servidor de Administração encontrar uma sala com estado `== 1`, essa é a sala selecionada para o jogo. Se não encontrar salas, busca salas com estado `== 0`. Ao selecionar uma sala, ele envia o endereço para o cliente e altera o estado da sala para indicar que mais um jogador se conectou nela. Se nenhuma sala for encontrada ou o tipo de jogo solicitado não estiver disponível, uma mensagem de erro é enviada ao cliente. Se uma sala for encontrada, o endereço é enviado ao cliente. As operações de leitura e escrita no objeto sala e na lista salas são protegidas com uma região crítica usando semáforo.

### Cabeçalho da Comunicação da Sala com o Cliente

| Campo                | Tipo                | Descrição                                                                                                 |
|----------------------|---------------------|-----------------------------------------------------------------------------------------------------------|
| op                   | int (4 bytes)       | Tipo de operação solicitada pela sala: <br> - 1 -> Solicitação de posicionamento de navio <br> - 2 -> Solicitação de disparo <br> - 3 -> Indica que o jogo acabou <br> - 4 -> Solicitação para escolha do tipo de jogo (quantidade de cada navio) |
| mensagem             | int (4 bytes)       | Dependendo de `op`: <br> - Quando `op = 1`: Tipo do navio a ser inserido <br> - Quando `op = 2`: Determina se foi uma vitória, derrota ou erro <br> - Quando `op = 4`: Indica qual jogo foi selecionado |
| mapa jogador         | int array (100x4 bytes) | Mapa do jogador, utilizado quando `op = 1`, `op = 2`, ou `op = 3`                                        |
| mapa adversario      | int array (100x4 bytes) | Mapa do adversário, utilizado quando `op = 2` ou `op = 3`                                                |

**Observação:**  
Após o jogador receber uma solicitação com `op` diferente de 3 (jogo finalizado), o jogador deve salvar os dados solicitados no cabeçalho e seu nome em UTF-8.

### Cliente -> Sala

| Campo         | Tipo                | Descrição                                                                                                 |
|---------------|---------------------|-----------------------------------------------------------------------------------------------------------|
| op            | int (4 bytes)       | Igual ao `op` da solicitação enviada para o cliente.                                                     |
| coluna        | int (4 bytes)       | Valor no eixo y onde o tiro ou o navio será posicionado.                                                 |
| linha         | int (4 bytes)       | Valor no eixo x onde o tiro ou o navio será posicionado.                                                 |
| orientação    | int (4 bytes)       | Utilizada quando `op = 1` (orientação do navio) ou `op = 3` (tipo de jogo)                              |
| size          | int (4 bytes)       | Tamanho do nome em UTF-8.                                                                               |
| nome          | string (size bytes) | Nome do cliente em UTF-8.                                                                               |

**Descrição Adicional:**  
Esta troca de mensagem se repete até que o jogo acabe. A sala manda uma solicitação para o cliente, e o cliente responde com suas escolhas.

## Comunicação Entre o Cliente e o Servidor de Administração

### Cabeçalho de Comunicação Entre o Cliente e o Servidor de Administração

| Campo                | Tipo                | Descrição                                                                                  |
|----------------------|---------------------|--------------------------------------------------------------------------------------------|
| op                   | int (4 bytes)       | Tipo de operação: <br> - 1 -> Solicitação de sala                                         |
| jogo selecionado     | int (4 bytes)       | Número do jogo selecionado (atualmente, somente o jogo 1, Batalha Naval, está implementado) |
| size                 | int (4 bytes)       | Tamanho do nome do cliente em UTF-8                                                         |
| nome                 | string (UTF-8 size) | Nome do cliente em UTF-8                                                                    |

**Observação:**  
Quando o cliente inicia, ele recebe o jogo alvo e o nome do cliente, e então envia uma solicitação para o servidor de administração solicitando uma sala para iniciar o jogo.

## Cabeçalho de Comunicação Entre o Servidor e o Cliente

| Campo                | Tipo                | Descrição                                                                                              |
|----------------------|---------------------|--------------------------------------------------------------------------------------------------------|
| op                   | int (4 bytes)       | Tipo de operação: <br> - 0 -> Se algum erro ocorreu <br> - 1 -> Sala encontrada                      |
| port                 | int (4 bytes)       | Porta da sala, se `op = 1`. Indica a porta onde a sala está aguardando conexões de clientes.           |
| error                | int (4 bytes)       | Código do erro, se `op = 0`. Se não houve erro, esse campo não é utilizado.                            |
| size                 | int (4 bytes)       | Tamanho do IP em bytes, usado para o endereço da sala se `op = 1`.                                      |
| ip                   | string (size bytes) | Endereço IP da sala, se `op = 1`. Se `op = 0`, o IP contém a mensagem de erro recebida.                |

**Observação:**  
O Servidor de Administração busca uma sala com estado `== 1` (livre para novos jogadores). Se não encontrar, procura por salas com estado `== 0` (em espera). Após selecionar uma sala, envia o endereço ao cliente e altera o estado da sala. Se não encontrar uma sala adequada ou se o tipo de jogo solicitado não estiver disponível, envia uma mensagem de erro para o cliente. As operações de leitura e escrita no objeto sala e na lista de salas são protegidas com uma região crítica usando semáforo.





### Funcionamento do jogo

# Abertura da sala

- Servidor Mestre ja ligado espera conexão de salas
- Servidor de processamento inicia e cria as salas
- Salas se conectam ao Mestre, criam um socket para esperar conexões de clientes, indica o endereço deste socket para o servidor Mestre
- Servidor de Administração recebe o endereço de cada sala e os salva em objetos sala na lista salas, ambos protegidos por semaforo

# Cliente inicia

- Cliente incia e faz um pedido de sala para o Servidor de Administração
- Servidor envia para um cliente a sala disponivel para ele se conectar, e atualiza o valor do objeto sala
- Cliente utiliza o endereço adquirido para se conectar a sala para iniciar o jogo
- Após dois jogadores se conectarem em uma sala o jogo será iniciado

# Inicio do jogo 

- Sala envia uma solicitação de seleção de tipo de jogo para o JogadorUm
- JogadorUm decide qual tipo do jogo(Interfere em quantidade de cada navio)         
- Sala envia uma solicitação de posicionamento de navio para o jogador 1        #  Se repete até todos os navios   #
- JogadorUm insere o navio                                                          # do jogadorUm serem posicionados  #
- Sala envia uma solicitação de posicionamento de navio para o jogador 2        #  Se repete até todos os navios     #
- JogadorUm insere o navio                                                          # do jogadorDois serem posicionados  #

# Meio do jogo

- Sala envia uma solicitação de disparo para o JogadorUm
- JogadorUm realiza disparo
- Sala verifica se JogadorDois foi derrotado, se sim então finaliza o jogo.              
- Sala envia uma solicitação de disparo para o JogadorDois
- JogadorDois realiza disparo
- Sala verifica se JogadorUm foi derrotado, se sim então finaliza o jogo.              
Este fluxo se repete até o jogo acabar por vitória ou motivos de erro

# Finalização do jogo

- Sala envia para cada jogador o resultado do jogo
- Sala encerra jogo
- Clientes encerram a conexão com a sala
- Sala avisa para o Servidor de Administração que o jogo foi finaizado e que a sala está livre
