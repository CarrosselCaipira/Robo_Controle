#ifndef RADIO_H
#define RADIO_H

#include <vector>
#include <thread>
#include <QSerialPort>
#include <QString>
#include "robo.hpp"

/** @class Radio radio.h "radio.hpp"
 *  @brief Classe para a realizacao do envio dos dados para o arduino com o radio que manda as instrucoes para os robos.
 *
 *  Esta classe faz usodo da biblioteca QT5, mais especificamente, do módulo serial presente na biblioteca. Certifique-se
 *  de possuir o QT5 instalado.
 *
 *  Esta classe faz a configuração e a comunicação pela porta Serial do PC com um arduino conectado na USB.
 *  Ela irá criar uma nova thread para fazer esta comunicação, já que a comunicação é feita de maneira bloqueante onde
 *  onde esperamos o arduino confirmar o recebimento para enviar mais informações, enquanto isso não ocorre, a thread dorme.
 *  Foram implementados métodos de recebimento, mas não estão sendo utilizados no momento. Para habilitar é necessário alterar
 *  o construtor da classe para leitura e escrita (no momento está apenas para escrita) e fazer a modicações que vir ser necessárias
 *  no método Radio::recebeDados().
 *  Funcões de pausa e termino da comunicação também estão disponíveis.
 *
 */
class Radio {
    QSerialPort serial;

    const unsigned char caractere_inicial = 0x80; /**< envia 0x80 como primeiro byte (header) que sera interpretado pelos robos */
    const unsigned char caractere_recebido_okay = 0x0d; /**< recebe 0x0d como primeiro byte (header) vindo do arduino conectado à serial  (momento setado para capturar o ACK da serial do arduino mas não está sendo usado no momento). */
    const QString caminho_dispositivo = "/dev/ttyUSB0"; /**< caminho para a porta a ser aberta para comunicao serial */
    const int THREAD_SLEEP_TIME = 33; /**< tempo que a thread irá dormir para checar se ainda está pausada. Este tempo está em millisegundos. */

    std::vector<Robo>& vector_robos; /**< referencia para o vector que contem os robos  */
    int num_bytes_enviados; /**< Numero de bytes a serem enviados pela serial. Dois bytes para cada robo (um para cada roda) mais um, inicial, que indica o inicio de uma nova sequência. */
    char* dados_envio = nullptr; /**< array a ser enviado para os robos, é preparado na função enviaDados a partir dos dados dos robos. */

    /* CONFIIGURAÇÃO DA THREAD */
    bool comunicacao_pronta = false; /**< flag que indica se é a primera chamada da função startComunicacao(). Na primeira chamada ela prepara a infraestrutura para a execução da thread de comunicação e executa a ação em si, a partir daí apenas execurá a ação. */
    bool comunicacao_terminada = false; /**< indica se a comunicação foi terminada ou não (thread deve continuar existindo (false) deve parar (true). */
    bool comunicacao_pausada = true; /**< indica se a comunicação está em pausa ou não (thread de envio esta enviando dados (false) ou esta em pausa (existe mais não esta enviando nada) (true). */
    std::thread *thread_envio = nullptr; /**< thread temporizada que fará o envio dos dados à porta serial executando a  funcao Radio->enviaDados() */

public:

    static const int VELOCIDADE_SERIAL = 115200;  /**< frequencia de input e output da porta serial. Bit rate padrao de 115200. Disponíveis: 115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200. */

    /** @fn Radio(std::vector<Robo>& v)
    *  @brief Faz a configuracao da porta serial para efetuar a comunicao e a 'acoplagem' do vetor de robos.
    *  @param std::vector<Robo>& v Vetor contendo os robos do time.
    */
    Radio(std::vector<Robo>& v);

    /** @fn ~Radio()
     *  @brief Fecha a porta serial e termina a comunicao.
     */
    ~Radio();

    /**
     * @brief Prepara a thread de comunicação e inicializa a comunicação em uma thread separada.
     */
    void comecaComunicacao();


    /**
     * @brief Termina (fecha) a thread de comunicação, não são mais enviados dados ao rádio conectado à serial. Para reinicializar a comunicação após esta chamada é necessário fazer a chamada para a função comecaComunicacao() novamente.
     */
    void terminaComunicacao();

    /**
     * @brief Pausa o envio de dados para a serial. A thread continua executando, apenas não faz a escrita na serial do arduino. Para reativar a comunicação, deve ser feita a chamada para a função recomecaComunicacao(). A thread do rádio irá dormir por Radio::THREAD_SLEEP_TIME antes de checar se não está mais em pausa.
     */
    void pausaComunicacao();

    /**
     *
     */
    void recomecaComunicacao();

private:
    /** @fn enviaDados()
     *  @brief Faz o envio dos valores de velocidade de estadoAtualRobo de cada robo em campo
     */
    void enviaDados();


    /**
     * Quando o rádio é aberto para leitura e escrita no construtor, faz a leitura da serial. Este metodo não está sendo usado e pode ser necessário reescreve-lo. Foi mantido com o principio de ser utilizado como base,
     */
    void recebeDados();


};


#endif /* RADIO_H */
