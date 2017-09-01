#include <unistd.h>
#include "joystick.hh"
#include "radio.hpp"

#define INTERVALO_TEMPO 115200 /**< Intervalo de tempo para a deteccao de eventos do joystick. Deve ser igual ao bit rate da porta serial para nao sobrecarega-la */
#define NUM_ROBOS 3 /**< LEGACY: MANTER EM 3 ENQUANTO O CODIGO DO ARDUINO NAO FOR CORRIGIDO */
#define NUM_JOYSTICKS 1 /**< LEGACY: ATE ARRUMARMOS O CODIDO DO ARDUINO (TX E RX) PRECISAMOS DEFINIR O NUMERO DE JOYSTICKS CONECTADOS AO COMPUTADOR.*/
#define MAX_VELOCIDADE_FRENTE 7 /**< 0111(7). Vai para frente (bit mais significativo indica sentido da rotacao) com velocidade maxima */
#define MAX_VELOCIDADE_TRAZ 15 /**< 0111(7) or 1000(8) = 1111(15). Vai para tras (bit mais significativo indica sentido da rotacao) com velocidade maxima */
#define BOTAO_L1 4
#define BOTAO_R1 5
#define BOTAO_L2 6
#define BOTAO_R2 7

struct botoesControle {
	bool b_L1; /**< se o botao L1 estiver precionado b_L1 = 1, senao b_L1 = 0 */
	bool b_R1; /**< se o botao R1 estiver precionado b_R1 = 1, senao b_R1 = 0 */
	bool b_L2; /**< se o botao L2 estiver precionado b_L2 = 1, senao b_L2 = 0 */
	bool b_R2; /**< se o botao R2 estiver precionado b_R2 = 1, senao b_R2 = 0 */
};

struct Controle{
	botoesControle botoes_pressionados; /**< indica quais botoes do controle estao pressionados no momento */
	Joystick *joystick; /**< Ponteiro para instancia de joystick */
};

int main() {
	std::vector<Controle> controle(NUM_ROBOS); /**< aloca o vetor de structs de joysticks e os botoes que estao precionados no joystick. */
	for(int i = 0 ; i < NUM_JOYSTICKS; i++)
		controle[i].joystick = new Joystick(i); /**< instancia do joystic i controlara o robo i. */

	std::vector<Robo> robos(NUM_ROBOS); /**< vetor com os NUM_ROBOS robos. */
	Radio radio(robos); /**< instancia de radio para que possamos enviar os comandos para os robos. */

	// deteccao dos joysticks
	for(int i = 0; i < NUM_JOYSTICKS; i++) {
		// Determinando se eh possivel detectar e utilizar o joystick i
		if (!controle[i].joystick->isFound()) {
			printf("Joystick %d nao encontrado. Saindo...\n", i);
			exit(1);
		}
	}

	while (true) {
		// restricao de tempo
		usleep(INTERVALO_TEMPO);

		// para cada robo e controle determinar quais os botoes estao precionados e quais nao estao (ou foram liberados agora)
		for(int i = 0; i < NUM_JOYSTICKS; i++) {
			JoystickEvent evento_joystick_i;
			// Detectando se houve eventos disparados pelo joystick i
			if (controle[i].joystick->sample(&evento_joystick_i)) {
				// Detectando se eh um botao que esta sendo pressionado ou liberado (pode ser um dos analogicos disparando o evento)
				if (evento_joystick_i.isButton()) {
					// detectando qual botao esta sendo pressionado ou liberado
					switch (evento_joystick_i.number) {
						// L1 esta sendo pressionado
						case BOTAO_L1:
							// se for true, significa que o botao esta sendo precionado agora, logo controle[i].botoes_pressionados.b_L1 eh true, do contrario esta sendo liberado.
							evento_joystick_i.value == true ? controle[i].botoes_pressionados.b_L1 = true : controle[i].botoes_pressionados.b_L1 = false;
						break;
						// R1 esta sendo pressionado
						case BOTAO_R1:
							// se for true, significa que o botao esta sendo precionado agora, logo controle[i].botoes_pressionados.b_R1 eh true, do contrario esta sendo liberado.
							evento_joystick_i.value == true ? controle[i].botoes_pressionados.b_R1 = true : controle[i].botoes_pressionados.b_R1 = false;
						break;
						// L2 esta sendo pressionado
						case BOTAO_L2:
							// se for true, significa que o botao esta sendo precionado agora, logo controle[i].botoes_pressionados.b_L2 eh true, do contrario esta sendo liberado.
							evento_joystick_i.value == true ? controle[i].botoes_pressionados.b_L2 = true : controle[i].botoes_pressionados.b_L2 = false;
						break;
						// R2 esta sendo pressionado
						case BOTAO_R2:
							// se for true, significa que o botao esta sendo precionado agora, logo controle[i].botoes_pressionados.b_R2 eh true, do contrario esta sendo liberado.
							evento_joystick_i.value == true ? controle[i].botoes_pressionados.b_R2 = true : controle[i].botoes_pressionados.b_R2 = false;
						break;
					}
				}
			}
		}

		unsigned char rodaEsquerda = 0; /**< indica a velocidade atual da roda esquerda. */
  	unsigned char rodaDireita = 0; /**< indica a velocidade atual da roda direita. */
		// colocando os valores de velocidade nos robos.
		for(int i = 0; i < NUM_ROBOS; i++) {
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
			// gravando valores de velocidade nos robos
			robos[i].setVelocidadeAtualRobo(rodaEsquerda, rodaDireita);
		}
		// faz o envio das velocidades para todos os robos via radio
		radio.enviaDados();
	}
}
