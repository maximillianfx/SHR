/*-------------------------------------------------------------------------------------------------------------------------------------------
MAIN.CPP:
Arquivo main para execução do programa

PIBIC 2015-2016: Software para detecção de frequência cardíaca
Alunos: Maximillian Fan Xavier e Otávio Alves Dias
Professor: Eduardo Peixoto Fernandes da Silva
Universidade de Brasília, Brasília-DF, Brasil
-------------------------------------------------------------------------------------------------------------------------------------------*/

/*Declaração das bibliotecas do C++*/
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <iomanip>
#include <sys/time.h>
#include <thread>

/*Declaração das bibliotecas do OpenCV*/
#include "opencv2/opencv.hpp"
#include <fftw3.h>

/*Declaração das bibliotecas próprias*/
#include "temporizacao.hpp"
#include "frequencia.hpp"

/*Função main*/
int main () {
    /*Declaracao de objetos para obtencao de frames e analise do sinal*/
    Temporizacao WC;
    Frequencia Freq;

    /*Inicializao dos parametros advindos do arquivo params.txt*/
    int exibeFPS, janelaMatriz, espacoPicos, tempoNextLoops, framesViolaJones, flagMediaMovel;
    WC.getParams(&exibeFPS,&janelaMatriz,&espacoPicos,&tempoNextLoops,&framesViolaJones,&flagMediaMovel);

    /*Inicializacao da webcam conectada ao computador*/
    WC.VC = WC.startCam(0);
    if (WC.VC.isOpened()) {
        int fps = WC.getFPS(WC.VC);
        if (exibeFPS) {
            std::cout << "FPS: " << fps << std::endl;
        }

        /*Exibicao de video inicial para posicionamento do usuario e posterior execucao completa do software*/
        WC.exibeVideo(fps);
        WC.batimentoCardiaco(fps, janelaMatriz, espacoPicos, tempoNextLoops, framesViolaJones, flagMediaMovel);
    }
    return(0);
}

