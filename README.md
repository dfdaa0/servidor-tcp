## Proposta
Este repositório visa implementar um servidor e um cliente que são capazes de se comunicar enviando arquivos através de uma conexão TCP. O servidor pode especificar se utilizará IPv4 ou IPv6, bem como especificar a porta (nesse caso, settada para 51511 para fins de avaliação).

## Funcionamento de uma conexão entre servidor e cliente TCP
Inicialmente, um servidor é inicializado. Ele cria um socket utilizando uma porta, e usa a função bind() para utilizar o socket criado em suas conexões. O servidor, então, usa a função listen(), e espera algum cliente requerer uma conexão.  
O cliente então, é inicializado, referencia o IP e a porta que o servidor está utilizando e usa a função connect() para se conectar ao servidor.  
O servidor usa a função accept() para aceitar a conexão. A partir desse momento, cliente e servidor possuem uma via de comunicação.  
O cliente pode selecionar um arquivo que esteja em seu diretório e enviar para o servidor. Isso é feito com uma divisão do arquivo em vários buffers de tamanho definido, que são enviados após o envio do nome do arquivo em questão.  
Os terminais do servidor e do cliente são abertos separadamente, um processo é diferente do outro.

## Especificações do projeto
O usuário liga o servidor com o comando no formato "./server v4 51511". Nesse caso, ele utiliza o IPv4 na porta 51511. Como servidor e cliente estão numa mesma máquina, eles se conectam usando o endereço IP de loopback (referencia a própria máquina), sendo 127.0.0.1 no caso do IPv4, e ::1 no caso do IPv6.  
O usuário liga o cliente com o comando no formato "./client 127.0.0.1 51511", também utilizando IPv4 na porta 51511. O cliente pode, a partir desse momento, usar os seguintes comandos:  
- select file nomedoarquivo.extensao --> seleciona um arquivo, sendo sua existência e sua extensão checadas (apenas extensões 'tex', 'java', 'cpp', 'c', 'py' e 'txt' são permitidas por enquanto);
- send file --> envia o arquivo selecionado;
- exit --> fecha a conexão.
