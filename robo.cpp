#include "robo.hpp"

/*************************** CONSTRUTOR *************************/
Robo::Robo() {
  /* zerando todas as componentes do robo */
  std::memset(&estadoAtualRobo, 0, sizeof(estadoRobo));
  std::memset(&estadoPrevRobo, 0, sizeof(estadoRobo));
  std::memset(&objRobo, 0, sizeof(estadoRobo));
}
/****************************************************************/

/***************************** GETTERS **************************/

/************************ POSICAO ATUAL ROBO ********************/
posXY Robo::getPosicaoAtualRobo () {
  return estadoAtualRobo.posicao;
}

vetorSentido Robo::getVetorSentidoAtualRobo () {
  return estadoAtualRobo.direcao;
}

float Robo::getAnguloAtualRobo () {
  return estadoAtualRobo.angulo;
}

velocidadeRobo Robo::getVelocidadeAtualRobo () {
  return estadoAtualRobo.velocidade;
}
/****************************************************************/


/***************************** SETTERS **************************/

/************************ POSICAO ATUAL ROBO ********************/

void Robo::setVelocidadeAtualRobo (const unsigned char velRodaEsq, const unsigned char velRodaDir) {
  estadoAtualRobo.velocidade.rodaEsq = velRodaEsq;
  estadoAtualRobo.velocidade.rodaDir = velRodaDir;
}
