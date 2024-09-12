### Servidor_Hospedagem_Jogos
Sistema distribuido para hospedagem de salas de jogos, cliente solicita para um servidor mestre o endereço de uma sala para ele jogar, este servidor o envia o endereço
e então ele se conecta neste endereço para jogar batalha naval(Jogo implementado para o Projeto), o sistema funciona utilizando os seguintes processos principais.

# Servidor de processamento
O servidor de processamento solicita o numero de uma porta inicial X pelo terminal, inicia N threads salas, cada sala se conecta ao servidor mestre, as salas tentarão inciar um socket que espera conexões de clientes nas portas X até X+N-1, cada sala então avisa ao servidor Mestre o endereço vinculado a porta que está aguardando conexões de clientes, quando dois clientes se conectarem na sala, o jogo é iniciado, clientes trocam mensagems com a sala até o jogo finalizar. Após o jogo finalizar a sala avisa o servidor Mestre o resultado do jogo e tambem indica que está livre para clientes se conectarem.

# Cliente
Cliente se conecta ao servidor Mestre e solicita uma sala, ao receber o endereço da sala ele encerra a conexão com o servidor mestre, o cliente então se conecta no socket da sala e espera/começa o jogo, clietes escolhem aonde querem inserir os navios e aonde irão atirar, após acabar o jogo o processo acaba.

# Servidor Mestre ou Servidor de Adminitração
O servidor mestre possui dois sockets que espeream conexões, um que espera conexões de salas e cria uma thread para lidar com ela, inicializando a sala antes do primeiro jogo e salvando seu endereço em um objeto sala e o colocando em uma fila salas, e após cada jogo acabar atualiza o estado do objeto sala. O segundo socket que espera conexões do servidor de Adminitração lida com os clientes, quando um cliente se conecta uma thread é iniciada para lidar com ele, a thread busca uma sala livre para este cliente se conectar, utilizando a lista salas, após encontrar uma sala livre, atualiza o estado da sala e envia seu endereço para o cliente, após isso a thread que lida com este cliente é finalizada e os sockets são fechados. Tanto a escrita no estado da sala em sua inicialização quanto a busca e atualização do valor na busca da sala para o cliente, são protegidas em uma região critica usando um semaforo, mantendo assim consistencia dos dados.

### Fluxos de comunicação:

# Salas -> Servidor Mestre
 ============================================================ 
 | op (int 4 bytes)| port (int 4 bytes)| error (int 4 bytes)|            op = 1 -> Criação de sala                           
 | size (int 4 bytes) |    ip (string utf-8 size bytes)     |               = 2 -> Atualização de sala após finalizar jogo   
 ============================================================ 
port = Port que o socket da sala está esperando conexões de clientes, se op = 2, então esse campo indica o resultado
error = Se algum erro ocorrer, seu codigo estará na variavel erro
size = Tamanho em utf-8 do ip em que o socket da sala está esperando conexões
ip = Ip da sala que está esperando conexões de clientes

Salas do Servidor de Processamento se conectam com o socket do servidor de Adminitração que espera conexçoes de sala. A sala então envia para o servidor Mestre o endereço do socket que está esperando conexões de clientes, após o jogo acabar a sala envia para o servidor de Administração as seguintes informações: op, resultado, erro, size e nome, onde resultado é utilizado para determinar se o jogo terminou por uma vitória ou algum erro, nome indica o nome do jogador que venceu a partida, em caso de erro(resultado != 1) erro indica o codigo do erro recebido.

### Sala & Cliente
# Sala -> Cliente
 ========================================= 
 |op (int 4 bytes)| mensagem(int 4 bytes)|          op = indica o tipo de operação solicitada pela sala: op = 1 -> solicitação de posicionamento de navio, mensagem o 
 |    mapa jogador (100 int 4 bytes)     |          tipo do navio que será inserido, op = 2 -> solicitação de disparo para o jogador, op = 3 -> indica que o jogo acabou
 |  mapa adversario (100 int 4 bytes)    |          mensagem é utilizada para determinar se foi uma vitória uma derrota ou se algum erro encerrou a partida
 =========================================          op = 4 -> solicita que um jogador escolha o tipo do jogo que será realizado(quantidade de cada navio)
mapa da do jogador utilizado quando op = 1 | 2 | 3, mapa do adversário utilizado quando op = 2 | 3

Após o jogador receber uma solicitação diferente de op = 3(jogo finalizado), o jogador salva os dados solicitados no cabeçalho e seu nome em nome em modo utf-8
# Cliente -> Sala
 ================================================ 
 | op (int 4 bytes)|    coluna (int 4 bytes)    |   op: igual ao op da solicitação enviada para o cliente, coluna indica qual o valor no eixo y que o tiro ou o navio
 | linha (int 4 bytes)| orientacao (int 4 bytes)|   será posicionado, linha indica qual o valor no eixo x que o tiro ou o navio será posicionado, orientacao é utilizado
 | size (int 4 bytes) |   nome (string size)    |   quando op = 1 como a orientação do navio, quando op = 3 ele indica qual jogo foi selecionado. 
 ================================================ 
size = tamanho do nome do cliente em utf-8
nome = string do nome em utf-8

Esta troca de mensagem se repete nestá ordem até que o jogo acabe. Sala manda uma solicitação para o cliente, o cliente responde a solicitação com suas escolhas.

### Cliente & Servidor de Admistração
# Cliente -> Servidor de Admistração
 ====================================================== #
 | op (int 4 bytes)| jogo selecionado (int 4 bytes)  |  #       op = 1 solicitação de sala, jogo selecionado(no momento somente jogo 1 implementado, batalha naval)
 | size (int 4 bytes) | nome (string utf-8 size bytes)| #       tamanho do nome do cliente e o nome do cliente em utf-8
 ====================================================== #
Quando o cliente inicia ele adquire jogo alvo e nome do cliente, 
Envia uma solicitação para o servidor de administração solicitando uma sala para iniciar um jogo


# Servidor de administração -> Cliente
 ============================================================ #
 | op (int 4 bytes)| port (int 4 bytes)| error (int 4 bytes)| #     op = 0 se algum erro ocorreu e = 1 se um sala foi encontrada, error indica o codigo do erro
 | size (int 4 bytes) |        ip (string size bytes)       | #     size é utilizado para receber ip, se op = 1 então ip e port indicam o endereço da sala adquirida
 ============================================================ #     se op = 0 ip é a mensagem de erro recebida
 Se o servidor de Admistração encontrar uma sala com estado == 1 então essa é a sala selecionada para o jogo, se não encontrar salas comum jogador esperando
 busca salas que estão com estado == 0, ao selecionar uma sala ele envia para o cliente o endereço da respectiva sala e altera o estado da sala para indicar que mais 
 um jogador se conectou nela, se nenhuma sala foi encontrada ou o tipo de jogo solicitado pelo cliente não está disponivel evia uma mensagem de erro para o cliente, se não envia o endereço para o cliente.
 As operações de leitura e escrita no objeto sala e na lista salas são protegidas com uma região critica usando semaforo




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