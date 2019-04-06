/* Texto usado como base para este codigo disponivel em: https://en.wikibooks.org/wiki/Serial_Programming/termios
 * e em https://stackoverflow.com/questions/18108932/linux-c-serial-port-reading-writing
 *
 * Algumas informacoes uteis:
 * O_RDONLY: abrindo porta para apenas leitura.
 * O_NOCTTY: A porta nunca se torna o terminal de controle do processo.
 * O_NDELAY: utilizando comunicao nao bloqueante.
 */

#ifndef RADIO_H
#define RADIO_H

#include <stdio.h>
#include <stdlib.h>
#include <cstring>   // Para uso da rotina memset
#include <unistd.h> // Definicoes de funcao padrao do UNIX
#include <fcntl.h> // Para o uso das constantes utilizadas em 'open(...)' ex.: O_RDWR, O_RDONLY, etc. Para mais informacoes, va para Docs/tcntl.pdf
#include <cerrno> // Para o uso da rotina de indicacao de erro strerror() e da constante errno
#include <termios.h> // Definicoes de controle de terminal POSIX
#include <vector>
#include <iostream>
#include <thread>
#include <chrono>

#include "robo.hpp"


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
  const unsigned char caractere_inicial = 0x80; /* envia 0x80 como primeiro sinal (sera interpretado pelos robos) */	
  int num_bytes_enviados = 0; /**< Numeoro de bytes a serem enviados pela serial. Dois bytes para cada robo (um para cada roda) mais um, inicial, que indica o inicio de uma nova sequência. */
  std::vector<Robo>& vector_robos; /**< referencia para o vector que contem os robos  */
  struct termios dispositivo_tty; /**< estrutura de configuracao da porta serial */
  int USB; /**< descritor da porta serial a ser lida */
  const char* caminho_dispositivo = "/dev/ttyUSB0"; /**< caminho para a porta a ser aberta para comunicao serial */

  bool comunicacao_terminada = false; /**< indica se a comunicação foi terminada ou não (thread deve continuar existindo (false) deve parar (true). */
  bool comunicacao_pausada = true; /**< indica se a comunicação está em pausa ou não (thread de envio esta enviando dados (false) ou esta em pausa (existe mais não esta enviando nada) (true). */
  /* flag que indica se é a primera chamada da função startComunicacao(). Na primeira chamada ela prepara a infraestrutura para
   * a execução da thread de comunicação e executa a ação em si, a partir daí apenas execurá a ação. */
  bool comunicacao_pronta = false;
  std::thread *thread_envio = nullptr; /**< thread temporizada que fará o envio dos dados à porta serial executando a  funcao Radio->enviaDados() */

  public:
    /* frequencia de input e output da porta serial. Bit rate padrao de 115200. Disponíveis: 115200, 9600. Mantida
     * publica para ser usada por outras classes que fazem uso do radio e também na variável Radio::VELOCIDADE_SERIAL_INT*/
    static const speed_t VELOCIDADE_SERIAL = B115200;

    /* bit rate convertido para inteiro (usado na temporização da thread). Deve ser igual a Radio::VELOCIDADE_SERIAL */
    const unsigned int VELOCIDADE_SERIAL_INT = 115200;

    /* número de bits que são enviados à porta serial para concluir o envio de 1 byte (8 bits).
     * Isso ocorre pois existem mais bits sendo enviados além dos que estamos enviando. Logo, caso queiramos mandar 8 bits,
     * e o canal que possui 1 bit de start, 1 bit de stop e não possui bit de paridade teremos:
     * 1(start) + 1(stop) + 0(paridade) + 8(nossos dados) = 10 bits. Este valor pode mudar de canal para canal, logo precisa ser
     * pesquisado antes de ser alterado. (não trocar e testar para ver o que acontece Sherlock).
     * Essa variável é usada para calcular o tempo que a thread de startComuncacao. */
    const unsigned int BITS_PER_BYTE = 10;

    unsigned int THREAD_SLEEP_TIME; /* tempo que a thread dormirá esperando a proxima execução (em microsegundos) */

    /** @fn Radio(std::vector<Robo>& v)
    *  @brief Faz a configuracao da porta serial para efetuar a comunicao e a 'acoplagem' do vetor de robos.
    *  @param std::vector<Robo>& v Vetor contendo os robos do time.
    */
    Radio(std::vector<Robo>& v);

   /** @fn ~Radio()
    *  @brief Fecha a porta serial e termina a comunicao.
    */
    ~Radio();

    /** @fn enviaDados()
     *  @brief Faz o envio dos valores de velocidade de estadoAtualRobo de cada robo em campo
     */
    void enviaDados();

    /**
     *
     */
    void comecaComunicacao();

    void terminaComunicacao();

    void pausaComunicacao();

    void recomecaComunicacao();

    /** @fn calculaTempoSleep()
     *  @brief Calcula o tempo que thread de envio de dados precisá dormir para manter a sincronia com o rádio
     */
    void calculaTempoSleep();

};

#endif /* RADIO_H */
