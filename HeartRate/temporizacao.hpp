/*-------------------------------------------------------------------------------------------------------------------------------------------
TEMPORIZAÇÃO.HPP:
Este arquivo de cabeçalho contém a classe Temporizacacao, cujo propósito é inicializar a câmera conectada (usb ou embutida)
ao computador e também capturar os frames dentro de um intervalo de tempo previamente definido, podendo ser escolhido
um certo delay entre a captura de frames consecutivos.

PIBIC 2015-2016: Software para detecção de frequência cardíaca
Alunos: Maximillian Fan Xavier e Otávio Alves Dias
Professor: Eduardo Peixoto Fernandes da Silva
Universidade de Brasília, Brasília-DF, Brasil
-------------------------------------------------------------------------------------------------------------------------------------------*/

/*Declaração das bibliotecas*/
#include <iostream>
#include "opencv2/opencv.hpp"

/*Macros para definição de espaçamento temporal entre os frames*/
/*TIMES10: define um espaçamento temporal de 100000 microsegundos (1/10 S) (10fps) entre a captura de dois frames consecutivos*/
#define TIME1S10 100000
/*TIMES20: define um espaçamento temporal de 50000 microsegundos (1/20 S) (20fps) entre a captura de dois frames consecutivos*/
#define TIME1S20 50000
/*TIMES30: define um espaçamento temporal de 33333 microsegundos (1/30 S) (30fps) entre a captura de dois frames consecutivos*/
#define TIME1S30 33333
/*TEMPOTOTAL: define o tempo de duração de captura de frames*/
#define TEMPOTOTAL 10000000

/*Classe Temporizacao*/
class Temporizacao {
            /*Atributos*/
	public:
                        cv::Point pt1; /*Atributo do tipo 'Point' para armazenar coordenadas do metodo Viola Jones*/
                        cv::Point pt2; /*Atributo do tipo 'Point' para armazenar coordenadas do metodo Viola Jones*/
                        cv::Point ptF1; /*Atributo do tipo 'Point' para armazenar coordenadas do metodo Viola Jones*/
                        cv::Point ptF2; /*Atributo do tipo 'Point' para armazenar coordenadas do metodo Viola Jones*/
                        cv::CascadeClassifier face; /*Atributo do tipo 'CascadeClassifier' para carregar o padrão XML para o Viola Jones*/
                        int largura; /*Atributo 'largura' de tipo INTEIRO, cuja responsabilidade é armazenar a largura do frame capturado pela WebCam*/
                        int altura; /*Atributo 'altura' de tipo INTEIRO, cuja responsabilidade é armazenar a altura do frame capturado pela WebCam*/
                        cv::VideoCapture VC; /*Objeto 'VC' que representa o objeto de execução da câmera USB/Embutida ao computador*/

            /*Métodos*/
	public:
	           Temporizacao(); /*Método construtor da classe Temporizacao*/
                        cv::VideoCapture startCam(int); /*Método para abertura de câmera*/
	           cv::VideoCapture getCamera(); /*Método para capturar objeto que representa a câmera previamente aberta*/
                        void batimentoCardiaco(int, int, int, int, int, int); /*Método para captura de frames e obtencao de batimento cardiaco*/
	           void exibeVideo (int FPS); /*Método 'exibeVideo' para realiza a captura de frames da webcam e exibi-los em tela*/
                        int getFPS (cv::VideoCapture); /*Metodo para capturar o FPS da camera conectada ao computador*/
                        void getFacePosition (cv::Mat); /*Metodo para obter a posicao facial presente no frame capturado*/
                        void getParams(int *, int *,int *,int *, int *, int *); /*Metodo para carregar os parametros presentes no arquivo params.txt*/
};
