#ifndef RADIO_H
#define RADIO_H

#include <vector>
#include <iostream>
#include <thread>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>   // Para uso da rotina memset
#include <cerrno> // Para o uso da rotina de indicacao de erro strerror() e da constante errno

#include <QSerialPort>
#include <QString>
#include <QByteArray>

#include "robo.hpp"

/* funcionamento:
 *  este programa espera o ardunio (radio) enviar um caractere_inicial (1 byte), geralmente 0x80 para iniciar a comunicação
 *  apos recebida este sinal / byte ele envia ao ardunio na serial (radio) o array contendo a velocidade dos motores, com o mesmo byte inicial na posicao 0 do array (header)
 *  espera o ardunio (radio) enviar um caractere_inicial, geralmente 0x80, para iniciar a comunicação novamente e o ciclo se repete.
 * */


/** @class Radio radio.h "radio.hpp"
 *  @brief Classe para a realizacao do envio dos dados para o arduino com o radio que manda as instrucoes para os robos.
 *
 *  A classe faz a configuracao da porta serial, em seguida faz a leitura dos valores de velocidade atual das rodas de todos
 *  os robos do time em campo, desloca os valores de velocidade 4 bits para a esquerda para que possam ser interpretados
 *  corretamente pelos robos e grava esses dados porta serial onde esta o arduino com o radio para que ele faca o envio dessas
 *  informacoes aos robos.
 *  @todo Futuramente tambem implementaremos o recebimento.
 */
class Radio {
    QSerialPort serial;

    const unsigned char caractere_inicial = 0x80; /**< envia 0x80 como primeiro byte (header) que sera interpretado pelos robos */
    const QString caminho_dispositivo = "/dev/ttyUSB0"; /**< caminho para a porta a ser aberta para comunicao serial */

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
     *
     */
    void comecaComunicacao();

    void terminaComunicacao();

    void pausaComunicacao();

    void recomecaComunicacao();

private:
    /** @fn enviaDados()
     *  @brief Faz o envio dos valores de velocidade de estadoAtualRobo de cada robo em campo
     */
    void enviaDados();

    void recebeDados();


};


#endif /* RADIO_H */