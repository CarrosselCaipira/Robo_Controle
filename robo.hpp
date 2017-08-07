#ifndef ROBO_H
#define ROBO_H

#include "tipoEstruturas.hpp"
#include <cstring>

class Robo {
    estadoRobo estadoAtualRobo; // Todas as informacoes atuais sobre o robo.
    estadoRobo estadoPrevRobo; // Todas as informacoes sobre as posicoes futuras do robo.
    estadoRobo objRobo; // Informações sobre o objetivo do robo.

    public:
        /*************************** CONSTRUTOR *************************/
        // inicializa todas as componentes do robo com zero e define o roteiro atual do robo como "INDEFINIDO".
        Robo();
        /****************************************************************/

        /***************************** GETTERS **************************/

        /************************ POSICAO ATUAL ROBO ********************/
        // Retorna as coordenadas do robo.
        posXY getPosicaoAtualRobo ();

        // Retorna as componentes de movimentacao do robo.
        vetorSentido getVetorSentidoAtualRobo ();

        // Retorna o angulo do robo.
        float getAnguloAtualRobo ();

        // Retorna a velocidade do robo.
        velocidadeRobo getVelocidadeAtualRobo ();
        /****************************************************************/


        /***************************** SETTERS **************************/

        /************************ POSICAO ATUAL ROBO ********************/
        void setVelocidadeAtualRobo (const unsigned char velRodaEsq, const unsigned char velRodaDir);
        /****************************************************************/

};

#endif /* ROBO_H */
