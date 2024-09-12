### Servidor_Hospedagem_Jogos
Sistema distribuido para hospedagem de salas de jogos, cliente solicita para um servidor mestre o endereço de uma sala para ele jogar, este servidor o envia o endereço
e então ele se conecta neste endereço para jogar batalha naval(Jogo implementado para o Projeto), o sistema funciona utilizando os seguintes processos principais.

# Servidor de processamento
O servidor de processamento solicita o numero de uma porta inicial X pelo terminal, inicia N threads salas, cada sala se conecta ao servidor mestre, as salas tentarão inciar um socket que espera conexões de clientes nas portas X até X+N-1, cada sala então avisa ao servidor Mestre o endereço vinculado a porta que está aguardando conexões de clientes, quando dois clientes se conectarem na sala, o jogo é iniciado, clientes trocam mensagems com a sala até o jogo finalizar. Após o jogo finalizar a sala avisa o servidor Mestre o resultado do jogo e tambem indica que está livre para clientes se conectarem.

# Cliente
Cliente se conecta ao servidor Mestre e solicita uma sala, ao receber o endereço da sala ele encerra a conexão com o servidor mestre, o cliente então se conecta no socket da sala e espera/começa o jogo, clietes escolhem aonde querem inserir os navios e aonde irão atirar, após acabar o jogo o processo acaba.

# Servidor Mestre ou Servidor de Adminitração
O servidor mestre possui dois sockets que espeream conexões, um que espera conexões de salas e cria uma thread para lidar com ela, inicializando a sala antes do primeiro jogo e salvando seu endereço em um objeto sala e o colocando em uma fila salas, e após cada jogo acabar atualiza o estado do objeto sala. O segundo socket que espera conexões do servidor de Adminitração lida com os clientes, quando um cliente se conecta uma thread é iniciada para lidar com ele, a thread busca uma sala livre para este cliente se conectar, utilizando a lista salas, após encontrar uma sala livre, atualiza o estado da sala e envia seu endereço para o cliente, após isso a thread que lida com este cliente é finalizada e os sockets são fechados. Tanto a escrita no estado da sala em sua inicialização quanto a busca e atualização do valor na busca da sala para o cliente, são protegidas em uma região critica usando um semaforo, mantendo assim consistencia dos dados.

