#!/usr/bin/env bash

# criando a pasta onde será compilado o programa
mkdir build
# entrando na pasta
cd build
# executando o cmake para gerar o arquivo make
cmake ..
# compilando o programa
make
# copiando o executável gerado para a pasta raiz
cp Robo_Controle ../Robo_Controle