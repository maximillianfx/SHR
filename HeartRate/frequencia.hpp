/*-------------------------------------------------------------------------------------------------------------------------------------------
FREQUENCIA.HPP:
Este arquivo de cabeçalho contém a classe Frequencia, cuja qual implementa a execução da Fast Fourier Transform
 sobre um vetor de valor previamente definido, através da utilização da biblioteca FFTW3 disponibilizada na Internet.

PIBIC 2015-2016: Software para detecção de frequência cardíaca
Alunos: Maximillian Fan Xavier e Otávio Alves Dias
Professor: Eduardo Peixoto Fernandes da Silva
Universidade de Brasília, Brasília-DF, Brasil
-------------------------------------------------------------------------------------------------------------------------------------------*/

/*Declaração das bibliotecas*/
#include <iostream>
#include "opencv2/opencv.hpp"

/*Valores DEFINE para execucao do codigo*/
#define SIZEVECTOR 309
#define TAMANHOFFT 2048
#define FPSRUN 30
#define POSINICIAL 0
#define SIZEBASE 309
#define DESLOCAMENTO 60

/*Classe Frequencia*/
class Frequencia {
            /*Atributos*/
	public:
                        double MediaFinal[SIZEBASE]; /*Vetor de double que vai armazenar a média dos pixels da camada R do frame de uma área selecionada*/
                        double * Mediablue; /*Vetor de double que vai armazenar a média dos pixels da camada B do frame de uma área selecionada*/
                        double * Mediagreen; /*Vetor de double que vai armazenar a média dos pixels da camada G do frame de uma área selecionada*/
                        cv::Mat vetorFrame[SIZEVECTOR]; /*Vetor de objetos da classe Mat para armazenar os frames de um vídeo*/
                        double Filtro[1025];/*Vetor para conter o filtro a ser aplicado sobre o sinal no dominio da frequencia*/

            /*Métodos*/
	public:
	           Frequencia(); /*Método construtor da classe*/
	           void getMedias(cv::Mat Frame, int Y, int X, int altura, int largura, int frames); /*Método para capturar as médias das intensidades de pixel dos pixels presentes na área delimitada pelo retângulo descrito pelos parâmetros X, Y, altura e largura*/
                              void FastFourierTransform (double * vetor, int n, const int padding, int fpsSaida, double * picos, int * indices, int janelaMatriz);/*Método para realizar o cálculo da FFT sobre um array de dados reais e retornar a representação desses dados no domínio da frequência*/
                              void capturarPicos (double * magnitudes, int nc, bool media, int fpsSaida, double * picos, int * indices, int janelaMatriz);/*Metodo para capturas os N maiores picos e indices do sinal*/
                              double getBPM (int pico, int nc, int fps);/*Metodo para calculo do BPM*/
                              void getVectorFinal (int posInicial, int numFrames);/*Metodo para reoganizar o vetor com as amostras*/
                              void getVectorFinalResizable (int bloco, int deslocamento, int final);/*Metodo para organizar o vetor quando ocorrer a chegada de novas amostras*/
                              void alocaVetoresGB ();/*Metodo para inicializar os vetores dos canais G e B*/
                              void removeDCeDivisaoDP (int posInicial, int posFinal);/*Metodo para normalizar os vetores dos canais G e B*/
                              void criaFiltro();/*Metodo para inicializar o filtro no vetor*/
                              void ordenaPicoseIndices (double * picos, int * indices, int janelaMatriz);/*Método para organizar os picos obtidos de acordo com sua amplitude*/
                              void inicializaMatriz (int **matriz, int janelaMatriz);/*Método para inicializar a matriz de recorrencia*/
                              void inicializaSomatorio (int **somatorio, int janelaMatriz);/*Método para inicializar a matriz com o somatorio*/
                              void adicionaMatriz (int * indices, int **matriz, int loopBPM, int **somatorio, int janelaMatriz, int espacoPicos);/*Metodo para adicionar bpms na matriz*/
                              void limpaSomatorio (int **somatorio, int janelaMatriz);/*Metodo para limpar a matriz do somatorio*/
                              int getBPMFinal (int **somatorio, int janelaMatriz);/*Metodo para obtencao dos bpm's finais de acordo com a utilizacao dos pesos e seu respectivo somatorio*/
                              void adicionaMediaMovel(int *, int, int, int);/*Método para organizar o vetor que conterá as amostras para o cálculo da média movel*/
                              int getBPMMediaMovel (int *, int);/*Método para calcular a média móvel das amostras*/
};

