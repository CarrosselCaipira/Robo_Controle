#include "radio.hpp"

/* o trecho: vector_robos(v) serve para acoplar o vector com os robos a classe de transmissao */
Radio::Radio(std::vector<Robo>& v) : vector_robos(v) {

    /* Setando o baundrate de input e output do radio iguais */
    this->serial.setBaudRate(this->VELOCIDADE_SERIAL, QSerialPort::AllDirections);
    /* setando qual o caminho que se encontra / que foi montado o rádio */
    this->serial.setPortName(this->caminho_dispositivo);

    /* tentando abrir o dispositivo para leitura e escrita */
    if (!serial.open(QIODevice::ReadWrite)) {
        std::cerr << "Erro " << errno << " @Radio->open: " << std::strerror(errno) << std::endl;
        exit(1); /* Nao permite que o programa rode se nao foi possivel configurar a porta serial */
    }

    /* Dois bytes para cada robo (um para cada roda) mais um, inicial, que indica o inicio de uma nova sequência (header). */
    this->num_bytes_enviados = 2 * this->vector_robos.size() + 1;

    /* allocando o array que será enviado aos robos. sempre terá o primeiro byte fixo, logo, setamos ele aqui. */
    this->dados_envio = (char*) malloc(this->num_bytes_enviados);
    this->dados_envio[0] = this->caractere_inicial;
}

Radio::~Radio() {
    /* indicando que a thread deve parar sua execução */
    this->comunicacao_terminada = true;

    /* checando se é possivel unir a thread de comunicacao à principal. */
    if (this->thread_envio->joinable())
        Radio::terminaComunicacao();

    /* desalocando o array de dados enviados pela serial */
    free(this->dados_envio);

    /* fechando a serial */
    this->serial.close();
}

void Radio::enviaDados() {

    /* preenchendo o vetor de dados das rodas com os valores de velocidade dos robos */
    for(int i = 0; i < this->vector_robos.size(); i++) {
        /* deslocamos para esquerda os ultimos 4 bits para que eles sejam os primeiros do byte. Para os robos que receberao, apenas os primeiros 4 bytes sao importantes para os robos. */
        this->dados_envio[2 * i + 1] = this->vector_robos[i].getVelocidadeAtualRobo().rodaEsq;
        this->dados_envio[2 * i + 2] = this->vector_robos[i].getVelocidadeAtualRobo().rodaDir;
    }

    this->serial.write(QByteArray(this->dados_envio, this->num_bytes_enviados));

    /* espera o envio de, pelo menos, um byte para desbloquear a função. Caso não o faça (timeout/erro) retorná falso. */
    if(this->serial.waitForBytesWritten() == false) {
        std::cerr << "Error " << errno << " @Radio::enviaDados->write " << std::strerror(errno) << std::endl;
        return;
    }

}

void Radio::recebeDados() {
    bool okay_para_escrever = true; /* indica se já podemos passar para a fase de escrita na serial (true) ou não (false) */

    /* esperamos até receber o caracter this->caractere_inicial do arduino para prosseguir. */
    do {
        /* aguarda e indica se a serial pode (true) ou não ser lida (false)*/
        if (this->serial.waitForReadyRead() == false) {
            std::cerr << "Error " << errno << " @Radio::recebeDados->write " << "Possivel timeout da serial. "
                      << std::strerror(errno) << std::endl;
            okay_para_escrever = false;
        }

        /* faz a leitura da serial de 1 char / byte (que chega na forma de um QByteArray) */
        QByteArray char_validacao = this->serial.read(1);

        /* verificando se recebemos alguma informação e se recebemos a informação correta (um this->caractere_inicial char)*/
        if (char_validacao.isEmpty()) {
            std::cerr << "Error " << "Recebemos dados vazios durante a leitura da serial."
                      << " @Radio::recebeDados->write " << std::endl;

            okay_para_escrever = false;
        } else if ((char) char_validacao[0] != this->caractere_inicial) {
            std::cerr << "Error " << "Recebemos dados nao validos da serial." << "Esperado: "
                      << (int) this->caractere_inicial << " Recebido: " << (int) char_validacao[0]
                      << " @Radio::recebeDados->write " << std::endl;

            okay_para_escrever = false;
        }
    } while(!okay_para_escrever);
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
        if(!this->comunicacao_pausada) {
            // devemos receber os dados do arduino primeiro antes de enviar dados para ele
            Radio::recebeDados();
            // enviamos os dados ao ardunino (estamos certmos de que ele poderá recebe-los)
            Radio::enviaDados();
        }
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