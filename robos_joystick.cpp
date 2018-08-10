#include <unistd.h>
#include <stdlib.h>
#include "joystick.hh"
#include "radio.hpp"

enum MODO_OPERACAO {
  CONTROLE_ANALOGICO, /* Usaremos o analogico para controlar o sentido do robo (esquerda direita) e o L2 e R2 para controlar a direção (Ré e Frente, respectivamente). */
  CONTROLE_LR, /* Usaremos apenas os botões L e R para o controle dos robos. Os botões L (L1 para trás L2 para frente) controla a roda esquerda e os botões R (R1 para trás R2 para frente) controlam a roda direita. */

  /* ADICIONAR NOVOS MODOS DE OPERAÇÃO ANTES DE TOTAL_MODO_OPERACAO */
  TOTAL_MODO_OPERACAO
};

static const int INTERVALO_TEMPO = 115200; /**< Intervalo de tempo para a deteccao de eventos do joystick. Deve ser igual ao bit rate da porta serial para nao sobrecarega-la */
static const int NUM_ROBOS = 3; /**< LEGACY: MANTER EM 3 ENQUANTO O CODIGO DO ARDUINO NAO FOR CORRIGIDO - Deve ser igual ao esperado no código dos arduinos (pois o tamanho do buffer não variável) */
static const unsigned char MAX_VELOCIDADE_FRENTE = 7; /**< 0111(7). Vai para frente (bit mais significativo indica sentido da rotacao) com velocidade maxima */
static const unsigned char MAX_VELOCIDADE_TRAZ = 15; /**< 0111(7) or 1000(8) = 1111(15). Vai para tras (bit mais significativo indica sentido da rotacao) com velocidade maxima */

struct botoesControle {
  /* Nomeando os botões realcionados à struct JoystickEvent::number */
  /* Elementos relacionados à JoystickEvent::type = 1 == JS_EVENT_BUTTON */
  static const unsigned char BOTAO_L1 = 6;
  static const unsigned char BOTAO_R1 = 7;
  static const unsigned char BOTAO_L2 = 4;
  static const unsigned char BOTAO_R2 = 5;

  /* Elementos relacionados à JoystickEvent::type = 2 == JS_EVENT_AXIS */
  static const unsigned char AXIS_ESQ_H = 2; /* analogico esquerdo movintos horizontais (eixo X) */

	bool b_L1; /**< se o botao L1 estiver precionado b_L1 = 1, senao b_L1 = 0 */
	bool b_R1; /**< se o botao R1 estiver precionado b_R1 = 1, senao b_R1 = 0 */
	bool b_L2; /**< se o botao L2 estiver precionado b_L2 = 1, senao b_L2 = 0 */
	bool b_R2; /**< se o botao R2 estiver precionado b_R2 = 1, senao b_R2 = 0 */

  int a_esq_h; /* valores do analogico esquerdo movimento horizontal */
};

struct Controle{
	botoesControle botoes_pressionados; /**< indica quais botoes do controle estao pressionados no momento */
	Joystick *joystick; /**< Ponteiro para instancia de joystick */
};

int main(int argc, char const *argv[]) {

  if (argc != 3) {
    std::cerr << "ERROR: Numero errado de argumentos. " << "Numero de argumentos: " << argc << ". Uso: './Robos_Joystick NUM_JOYSTICKS MODO_OPERACAO' onde NUM_JOYSTICKS eh o numero de controles a serem usados. Deve sao aceitos valores numericos de 1 a " <<  NUM_ROBOS << " (inclusivo para ambas extremidades [1 a " <<  NUM_ROBOS << "]) e MODO_OPERACAO valores numericos de 0 a " << TOTAL_MODO_OPERACAO - 1 << std::endl << std::endl << "TOTAL_MODO_OPERACAO = 0:  Usaremos o analogico para controlar o sentido do robo (esquerda direita) e o L2 e R2 para controlar a direção (Ré e Frente, respectivamente). "<< std::endl << "TOTAL_MODO_OPERACAO = 1: Usaremos apenas os botões L e R para o controle dos robos. Os botões L (L1 para trás L2 para frente) controla a roda esquerda e os botões R (R1 para trás R2 para frente) controlam a roda direita." << std::endl << "Saindo..." << std::endl;

    exit(1);
  }

  /* convertendo string do argumento (argv[1]) para um inteiro base 10 e ignorando o restante dos argumentos (NULL). */
  /**< LEGACY: ATE ARRUMARMOS O CODIDO DO ARDUINO (TX E RX) PRECISAMOS DEFINIR O NUMERO DE JOYSTICKS CONECTADOS AO COMPUTADOR.*/
  const int NUM_JOYSTICKS = strtol(argv[1], NULL, 10);

  if(NUM_JOYSTICKS <= 0 || NUM_JOYSTICKS > 3) {
    std::cerr << "ERROR: Numero invalido de joysticks." << " Uso: './Robos_Joystick NUM_JOYSTICKS' onde NUM_JOYSTICKS eh o numero de controles a serem usados. Deve sao aceitos valores de 1 a 3 (inclusivo para ambas extremidades [1 a 3])." << std::endl << "Saindo..." << std::endl;

    exit(1);
  }

  const int modoOP = strtol(argv[2], NULL, 10);
  if(modoOP < 0 || modoOP >= TOTAL_MODO_OPERACAO) {
    std::cerr << "ERROR: Modo de operacao invalido." << "MODO_OPERACAO valores numericos de 0 a " << TOTAL_MODO_OPERACAO - 1 << std::endl << std::endl << "TOTAL_MODO_OPERACAO = 0:  Usaremos o analogico para controlar o sentido do robo (esquerda direita) e o L2 e R2 para controlar a direção (Ré e Frente, respectivamente). "<< std::endl << "TOTAL_MODO_OPERACAO = 1: Usaremos apenas os botões L e R para o controle dos robos. Os botões L (L1 para trás L2 para frente) controla a roda esquerda e os botões R (R1 para trás R2 para frente) controlam a roda direita." << std::endl << "Saindo..." << std::endl;

    exit(1);
  }


	std::vector<Controle> controle(NUM_JOYSTICKS); /**< aloca o vetor de structs de joysticks e os botoes que estao precionados no joystick. */
	for(int i = 0 ; i < NUM_JOYSTICKS; i++)
		controle[i].joystick = new Joystick(i); /**< instancia do joystic i controlara o robo i. */

	std::vector<Robo> robos(NUM_ROBOS); /**< vetor com os NUM_ROBOS robos. */
	Radio radio(robos); /**< instancia de radio para que possamos enviar os comandos para os robos. */

	// deteccao dos joysticks
	for(int i = 0; i < NUM_JOYSTICKS; i++) {
		// Determinando se eh possivel detectar e utilizar o joystick i
		if (!controle[i].joystick->isFound()) {
			printf("Joystick %d nao encontrado. Saindo...\n", i + 1);
			exit(1);
		}
	}

	while (true) {
		// restricao de tempo para fazermos a leitura e envio das velocidades aos robos
    usleep(INTERVALO_TEMPO);
		// para cada robo e controle determinar quais os botoes estao precionados e quais nao estao (ou foram liberados agora)
		for(int i = 0; i < NUM_JOYSTICKS; i++) {
			JoystickEvent evento_joystick_i;

      /* BEGIN DEBUG */
      // fazendo a leitura dos dados dos controles
      // std::cout << evento_joystick_i << '\n';
      /* END DEBUG */

			// Detectando se houve eventos disparados pelo joystick i
			if (controle[i].joystick->sample(&evento_joystick_i)) {
				// Detectando se eh um botao que esta sendo pressionado ou liberado (pode ser um dos analogicos disparando o evento)
				if (evento_joystick_i.isButton()) {
					// detectando qual botao esta sendo pressionado ou liberado
					switch (evento_joystick_i.number) {
						// L1 esta sendo pressionado
						case botoesControle::BOTAO_L1:
							// se for true, significa que o botao esta sendo precionado agora, logo controle[i].botoes_pressionados.b_L1 eh true, do contrario esta sendo liberado.
							evento_joystick_i.value == true ? controle[i].botoes_pressionados.b_L1 = true : controle[i].botoes_pressionados.b_L1 = false;
						break;
						// R1 esta sendo pressionado
						case botoesControle::BOTAO_R1:
							// se for true, significa que o botao esta sendo precionado agora, logo controle[i].botoes_pressionados.b_R1 eh true, do contrario esta sendo liberado.
							evento_joystick_i.value == true ? controle[i].botoes_pressionados.b_R1 = true : controle[i].botoes_pressionados.b_R1 = false;
						break;
						// L2 esta sendo pressionado
						case botoesControle::BOTAO_L2:
							// se for true, significa que o botao esta sendo precionado agora, logo controle[i].botoes_pressionados.b_L2 eh true, do contrario esta sendo liberado.
							evento_joystick_i.value == true ? controle[i].botoes_pressionados.b_L2 = true : controle[i].botoes_pressionados.b_L2 = false;
						break;
						// R2 esta sendo pressionado
						case botoesControle::BOTAO_R2:
							// se for true, significa que o botao esta sendo precionado agora, logo controle[i].botoes_pressionados.b_R2 eh true, do contrario esta sendo liberado.
							evento_joystick_i.value == true ? controle[i].botoes_pressionados.b_R2 = true : controle[i].botoes_pressionados.b_R2 = false;
						break;
					}
				}
        else if(evento_joystick_i.isAxis()) {
          switch (evento_joystick_i.number) {
            case botoesControle::AXIS_ESQ_H:
              controle[i].botoes_pressionados.a_esq_h = evento_joystick_i.value;
            break;
          }
        }
			}
		}

		// colocando os valores de velocidade nos robos.
		for(int i = 0; i < NUM_JOYSTICKS; i++) {
      unsigned char rodaEsquerda = 0; /**< indica a velocidade atual da roda esquerda. */
      unsigned char rodaDireita = 0; /**< indica a velocidade atual da roda direita. */
      double modificador;

      switch (modoOP) {
        case CONTROLE_ANALOGICO:
          // se R2 esta precionado, o robo vai para frente
          if(controle[i].botoes_pressionados.b_R2){
            rodaEsquerda = MAX_VELOCIDADE_FRENTE;
            rodaDireita = MAX_VELOCIDADE_FRENTE;
          }
          // se L2 esta precionado, o robo para tras
          if(controle[i].botoes_pressionados.b_L2) {
            rodaEsquerda= MAX_VELOCIDADE_TRAZ;
            rodaDireita = MAX_VELOCIDADE_TRAZ;
          }

          modificador = controle[i].botoes_pressionados.a_esq_h / (double)JoystickEvent::MAX_AXES_VALUE;

          /* BEGIN DEBUG */
          // std::cout << modificador << '\n';
          /* END DEBUG */

          /* caso modificador seja zero, nao fazemos alteracao nenhuma nas velocidades (reto) */
          /* inclinando o analogico para a direita */
          if(modificador > 0) {
            rodaDireita = rodaDireita * (1 - modificador);
          }
          /* inclinando o analogico para a esquerda */
          else if (modificador < 0) {
            modificador = (modificador * -1); /* deixando o modificador positivo */
            rodaEsquerda = rodaEsquerda * (1 - modificador);
          }
        break;

        case CONTROLE_LR:
          // se L1 esta precionado, a roda esquerda do robo gira para tras
          if(controle[i].botoes_pressionados.b_L1)
            rodaEsquerda = MAX_VELOCIDADE_TRAZ;
          // se L2 esta precionado, a roda esquerda do robo gira para frente
          if(controle[i].botoes_pressionados.b_L2)
            rodaEsquerda = MAX_VELOCIDADE_FRENTE;
          // se R1 esta precionado, a roda direita do robo gira para tras
          if(controle[i].botoes_pressionados.b_R1)
            rodaDireita = MAX_VELOCIDADE_TRAZ;
          // se R2 esta precionado, a roda direita do robo gira para frente
          if(controle[i].botoes_pressionados.b_R2)
            rodaDireita = MAX_VELOCIDADE_FRENTE;
        break;
      }

      /* BEGIN DEBUG */
      // std::cout << (int)rodaEsquerda<< " " << (int)rodaDireita<< '\n';
      /* END DEBUG */

			// gravando valores de velocidade nos robos
			robos[i].setVelocidadeAtualRobo(rodaEsquerda, rodaDireita);
		}
		// faz o envio das velocidades para todos os robos via radio
		radio.enviaDados();
	}
}
