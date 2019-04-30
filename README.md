# Robo_Controle

Programa para o controle dos robôs do VSS através dos controles de PS2. Este código foi criado para funcionar especificamente com sistemas Linux.

Para compilar apenas execute o script `configure.sh` e execute o programa digitando `./Robo_Controle`
Este código fez uso da biblioteca joystick desenvolvida por [drewnoakes](https://github.com/drewnoakes/joystick).

----
## Docker

Este programa possui um ambiente de desenvolvimento e/ou teste docker. É possível adquiri-la a partir do dockerhub atavés do comando: 

`docker pull carrosselcaipira/vss_base:1.0`.

Detalhes sobre o que está contido nesta imagem podem ser encontrados na página do Carrossel Caipira no [dockerhub](https://hub.docker.com/r/carrosselcaipira/vss_base).

Caso queira abrir um terminal que tenha à disposição os componentes contidos no container, execute o comando:

`docker run --rm -it --device=/dev/ttyUSB0 -v $PWD/:/Robo_Controle/ -w /Robo_Controle vss_base:1.0`

Com este comando o container terá acesso aos arquivos da pasta que está com o terminal aberto, logo, certifique-se de que a pasta é a do repositório (Robo_Controle).

A partir deste momento você está com um terminal aberto dentro da imagem fornecida pelo Carrossel, agora só é necessário fazer a compilação da biblioteca através do comando `./configure` (em caso de dúvidas, vá para a sessão [Compilando](##Compilando)).

Com o programa compilado, é necessário apenas executá-lo através do comando `./Robo_Controle` seguindo as instruções da sessão [Rodando o programa](##Rodando o programa).
    
----
## Compilando

Para compilar o programa é necessário os seguintes pacotes instalados em seu sistema:
 - Um compilador C++ (`gcc-c++` ou `clang`); 
 - `cmake`
 - QT5 (>= 5.9) com o componente Desktop gcc 64bit; 

Com os pacotes instalados, é apenas necessário rodar o script: `./configure.sh`

>Obs.: Talves seja necessário tornar o script executável, para isso execute: `chmod +x configure.sh` e execute o comando acima novamente.

----
## Rodando o programa

Para que o programa funcione sem a necessidade de `root`, é necessário que usuário pertença necessáriamente ao grupo `dialout`. Existem casos em que é necessário pertencer **também** aos grupos `lock`, `uucp` e `tty`. Para isso, execute o seguinte comando:

`sudo usermod -a  $USER -G dialout,lock,tty,uucp`

Após isso efetue o logout de seu usuário ou reinicie o computador para que estas modificações tenham efeito.

>Observação: Caso ocorra um erro durante a inclusão de seu usuário a um grupo, tente rodar o programa assim mesmo, pois pode ser que pertencer ao grupo em questão não seja necessário em sua distribuição.

O programa agora possui um modo de configuração. Não é possível executá-lo sem informar o número de controles conectados e o modo de operação.

O programa deve ser executado da seguinte forma:
`./Robo_Controle NUM_JOYSTICKS MODO_OPERACAO`

Exemplo:

`./Robo_Controle 1 0`

Isso significa que usaremos um controle (logo, apenas um robô) e operaremos ele utlizando o botão analogico do controle (mais detalhes sobre modos de operação abaixo).

Onde:

* `NUM_JOYSTICKS`:  É o número de controles a serem usados pelo programa. São aceitos valores numericos de 1 a 3 (inclusivo para ambas extremidades [1 a 3]);
* `MODO_OPERACAO`: É o modo de operação do controle. São aceitos valores numéricos de 0 a 1.

  Quando:

  * `TOTAL_MODO_OPERACAO = 0`:  Usaremos o analogico para controlar o sentido do robo (esquerda direita) e o L2 e R2 para controlar a direção (Ré e Frente, respectivamente).
  * `TOTAL_MODO_OPERACAO = 1`: Usaremos apenas os botões L e R para o controle dos robos. Os botões L (L1 para trás L2 para frente) controla a roda esquerda e os botões R (R1 para trás R2 para frente) controlam a roda direita.

----
## Contribuindo

Para contribuir criando novos modos de operação dos robos é necessário alterar apenas o fonte `robos_joystick.cpp`.

 1. Adicinar ao enum `MODO_OPERACAO` o novo modo de operação que está adicionando. Lembre-se de fazer as alterações em README.md de acordo com as recomendações presentes na forma de comentários dentro da enumeração;

 2. Caso você adicione novos botões, será necessário adicioná-los à estrutura `botoesControle`. Para descobrir quais são esses valores, comente as linhas na função `main` que são referentes ao rádio e encontre a linha de `DEBUG` `std::cout << evento_joystick_i << '\n';` e descomente-a para receber informações sobre botões pressionados. Para carater de testes, talvez seja interessante reduzir o valor da constante `INTERVALO_TEMPO` para 100 para um feedback numérico mais rápido;

 3. Atualizar as 3 mensagens de erro e de explicação de uso na função `main` explicando como funciona seu modo de operação. Utilize as explicações dos demais modos de operação como base;

 4. Dentro do loop infinito definido pelo `while (true)`, encontre a secção que define o encontra o que está sendo pressionado, ela se encontra abaixo do comentário `/* DETECÇÃO DE EVENTOS DISPARADOS PELO JOYSTICK i */`. Observe o que já foi implementado como exemplo, adições provavelmente não serão muito diferentes.

 5. Na secção abaixo do comentário `/* SETANDO OS VALORES DE VELOCIDADE DOS ROBOS */` adicione seu modo de operação ao switch-case. observe que os case deverá ser o mesmo utilizado na enumeração adicionada no passo 1, siga os exemplos já implementados, provavelmente seu controle é semelhante.

 Caso tenha interesse em contribuir de outra forma entre em contato: carrosselcaipira@gmail.com
 

----
## Problemas

### Instalação do QT5 não sendo detectada pelo CMake: 

Caso tenha problemas com a detecção da instalação do QT5 pelo CMake, adicione ele à sua variável ambiente.

Para isso é necessário que você execute o comando: `PATH=caminho_absoluto_para_pasta_qt5/versao_do_qt/gcc_64/bin/:$PATH`.

Por exemplo, suponto que a instalação do QT 5.9.8 tenha sido feita na home do usuário danilown, teríamos o seguinte comando:

`PATH=/home/danilown/Qt/5.9.8/gcc_64/bin/:$PATH`

para não ser necessário rodar este comando sempre que abrir um novo terminal, pode-se adicioná-lo ao arquivo `~/.bash_profile`. 

Para isso, abra tal arquivo, e cole, em sua última linha: `export PATH=/home/danilown/Qt/5.9.8/gcc_64/bin/:$PATH` fazenddo as devidas alterações.
 
Explicações mais bem detalhadas podem ser encontradas em:

http://www.ntu.edu.sg/home/ehchua/programming/howto/environment_variables.html
