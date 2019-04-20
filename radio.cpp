#include "radio.hpp"

/* o trecho: vector_robos(v) serve para acoplar o vector com os robos a classe de transmissao */
Radio::Radio(std::vector<Robo>& v) : vector_robos(v) {

    /* abrindo a porta serial para leitura e escrita (O_RDWR), não irá  se tornar o terminal controlador do processo (O_NOCTTY), e faz uso de I/O não bloqueante (O_NDELAY) */
//    this->USB = open(this->caminho_dispositivo, O_RDWR| O_NOCTTY | O_NDELAY);
    this->USB = open(this->caminho_dispositivo, O_RDWR| O_NOCTTY);

    /* setando todos os campos de dispositivo_tty com 0 (vamos evitar surpresas...) */
    memset(&this->dispositivo_tty, 0, sizeof(this->dispositivo_tty));

    /* Tratamento de Erros */
    /* Testado se foi possivel abrir a porta serial */
    if(this->USB < 0) {
        std::cerr << "Erro " << errno << " @Radio->open " << this->caminho_dispositivo << ": " << std::strerror(errno) << std::endl;
        exit(1); /**< Nao permite que o programa rode se nao foi possivel configurar a porta serial */
    }

    /* Testando se a porta serial aberta esta apontando para um dispositivo TTY (nosso radio eh TTY) */
    if(isatty(this->USB) < 0) {
        std::cerr << "Erro " << errno << " @Radio->isatty: " << std::strerror(errno) << std::endl;
        exit(1); /**< Nao permite que o programa rode se nao foi possivel configurar a porta serial */
    }
    /* Testando se a atual configuracao da porta serial pode ser lida */
    if(tcgetattr( this->USB, &this->dispositivo_tty ) < 0 ) {
        std::cerr << "Erro " << errno << " @Radio->tcgetattr: " << std::strerror(errno) << std::endl;
        exit(1); /**< Nao permite que o programa rode se nao foi possivel configurar a porta serial */
    }
    /* Setando a frequencia de input e output do radio */
    if(cfsetspeed(&this->dispositivo_tty, Radio::VELOCIDADE_SERIAL) < 0){
        std::cerr << "Erro " << errno << " @Radio->cfsetospeed: " << std::strerror(errno) << std::endl;
        exit(1); /**< Nao permite que o programa rode se nao foi possivel configurar a porta serial */
    }

    /* Setando outras configuracoes da porta (no momento: aceite que funciona)*/
    this->dispositivo_tty.c_iflag = IGNBRK | IGNPAR;
    this->dispositivo_tty.c_oflag = 0;
    this->dispositivo_tty.c_lflag = 0;
    this->dispositivo_tty.c_cflag = Radio::VELOCIDADE_SERIAL | CS8;

    /* Setagem para realizar a 'leitura contada', a leitura da porta serial eh efetuada apenas se o buffer de recebimento possui VMIN bytes(chars). Detalhes mais aprofundados podem ser encontrados em: VSS/Docs/Comunicacao_Serial/Understanding UNIX termios VMIN and VTIME.pdf na ultima pagina no topico VMIN > 0 and VTIME = 0 */
    this->dispositivo_tty.c_cc[VMIN] = 1; /**< aguarda que seja recebido 1 byte (char) para efetuar a leitura. Senao tiver chego, bloqueia o processo. */
    this->dispositivo_tty.c_cc[VTIME] = 0; /**< nao ha timing para fazer a leitura da porta */

    /* descarta dos dados atualmente na porta ('lixo' de memoria)*/
    tcflush(this->USB, TCIOFLUSH);
    /* Aplicando as configuracoes.
    * TCSANOW = aplica instantaneamente as configuracoes.
    */
    if (tcsetattr(this->USB, TCSANOW, &this->dispositivo_tty ) < 0) {
        std::cerr << "Error " << errno << " @Radio->tcsetattr " << std::strerror(errno) << std::endl;
        exit(1); /**< Nao permite que o programa rode se nao foi possivel configurar a porta serial */
    }
    this->num_bytes_enviados = 2 * this->vector_robos.size() + 1; /**< Dois bytes para cada robo (um para cada roda) mais um, inicial, que indica o inicio de uma nova sequência. */

    Radio::calculaTempoSleep();
}

Radio::~Radio() {
    /* indicando que a thread deve parar sua execução */
    this->comunicacao_terminada = true;

    /* checando se é possivel unir a thread de comunicacao à principal. */
    if (this->thread_envio->joinable())
        Radio::terminaComunicacao();

    close(this->USB);
}

/* enviando um byte para cada roda. Futuramente armazenaremos em um byte os valores de velocidade de ambas as rodas >> necessario corrigir robo.hpp robo.cpp tipoEstruturas.hpp e os codigos do arduino. */
void Radio::enviaDados() {
    /* aparentemente desnecessario, descomentar em caso de problemas */
    /* descarta dos dados atualmente na porta ('lixo' de memoria) */
    tcflush(this->USB, TCIOFLUSH);

    /* o primeiro (dados[0]) eh o caractere inicial para delimitar o inicio de uma nova sequencia de comandos */
    /* o segundo (dados[1]) eh o valor de velocidade do motor da roda esquerda do primeiro robo */
    /* o terceiro (dados[2]) eh o valor de velocidade do motor da roda direita do primeiro robo */
    /* o quarto (dados[3]) eh o valor de velocidade do motor da roda esquerda do segundo robo */
    /* o quinto (dados[4]) eh o valor de velocidade do motor da roda direita do segundo robo */
    /* e assim por diante para todos os robos do time em campo */
    /* os ultimos dois elementos do vetor(bytes) correspondem ao caractere zero */

    unsigned char dados[this->num_bytes_enviados];
    dados[0] = this->caractere_inicial;
    /* preenchendo o vetor de dados das rodas com os valores de velocidade dos robos */
    for(int i = 0; i < this->vector_robos.size(); i++) {
        /* deslocamos para esquerda os ultimos 4 bits para que eles sejam os primeiros do byte. Para os robos que receberao, apenas os primeiros 4 bytes sao importantes para os robos. */
        dados[2 * i + 1] = this->vector_robos[i].getVelocidadeAtualRobo().rodaEsq;
        dados[2 * i + 2] = this->vector_robos[i].getVelocidadeAtualRobo().rodaDir;
    }

    /* faz o envio das velocidades de cada roda para a porta serial */
    if(write(this->USB, dados, this->num_bytes_enviados) < 0)
        std::cerr << "Error " << errno << " @Radio::enviaDados->write " << std::strerror(errno) << std::endl;
}

void Radio::comecaComunicacao() {
    /* se é a primeira chamada desta função, prepara os parametros para a execução da thread. */
    if(!this->comunicacao_pronta) {
        this->comunicacao_pausada = false; /* indicando que a thread agora esta ativa. */
        this->comunicacao_pronta = true; /* indicando que já ajustamos os parametros que precisavamos. (não vamos mais executar esse trecho de novo) */
        this->thread_envio = new std::thread(&Radio::comecaComunicacao, this); /* instanciando a thread. */

        return; /* saimos pois já fizemos a configuração, a thread agora está rodando até que solicitemos sua parada. */
    }

    /* enquanto a comunicacao não é terminanda, faz o envio dos dados para a serial*/
    while(!this->comunicacao_terminada) {
        /* se a comunicação não está em pausa, pode fazer o envio dos dados, senão apenas espera. */
        if(!this->comunicacao_pausada)
            Radio::enviaDados();

        /* espera para executar novamente e manter a sincronia com o rádio */
        std::this_thread::sleep_for(std::chrono::microseconds(this->THREAD_SLEEP_TIME));
    }
}

void Radio::terminaComunicacao() {
    /* indicando que a thread deve terminar sua execução */
    this->comunicacao_terminada = true;

    this->thread_envio->join();
}

void Radio::pausaComunicacao() {
    /* indicando que a thread deve parar de enviar os dados durante sua execução */
    this->comunicacao_pausada = true;
}

void Radio::recomecaComunicacao() {
    /* indicando que a thread deve voltar a enviar os dados durante sua execução  */
    this->comunicacao_pausada = false;

}

void Radio::calculaTempoSleep() {
    /* calculando o numero total de bits que irão ser enviados pela porta serial */
    double total_bits = this->num_bytes_enviados * this->BITS_PER_BYTE;

    /* calculando quanto tempo (em microsegundos) vamos precisar para enviar o numero total de bits (total_bits) */
    this->THREAD_SLEEP_TIME = int((total_bits / this->VELOCIDADE_SERIAL_INT) * 1000000);
}


