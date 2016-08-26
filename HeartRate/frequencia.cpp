/*-------------------------------------------------------------------------------------------------------------------------------------------
FREQUENCIA.CPP:
Este é um arquivo de implementação de corpo de métodos

PIBIC 2015-2016: Software para detecção de frequência cardíaca
Alunos: Maximillian Fan Xavier e Otávio Alves Dias
Professor: Eduardo Peixoto Fernandes da Silva
Universidade de Brasília, Brasília-DF, Brasil

-------------------------------------------------------------------------------------------------------------------------------------------*/

/*Declaração das bibliotecas da linguagem C++*/
#include <iostream>

/*Declaração das bibliotecas da linguagem C*/
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <fftw3.h>
#include <iomanip>
#include <complex.h>
#include <math.h>

/*Declaração das bibliotecas da ferramenta OpenCV*/
#include "opencv2/opencv.hpp"

/*Declaração das bibliotecas próprias*/
#include "frequencia.hpp"

/*----------------------------------------------------------------------------------------------------------------------------------------*/
/*Método construtor da classe Frequencia*/
Frequencia::Frequencia() {
}

/*----------------------------------------------------------------------------------------------------------------------------------------*/
/*Método para alocar os vetores  que armazenarao as médias dos canais para cada frame*/
void Frequencia::alocaVetoresGB () {
    Mediagreen = (double*) malloc (309*sizeof(double));
    Mediablue = (double*) malloc (309*sizeof(double));
}

/*----------------------------------------------------------------------------------------------------------------------------------------*/
/*Função 'getMedias' para obter a media das intensidades dos pixels presentes na janela demarcada pelo retângulo selecionado*/
void Frequencia::getMedias (cv::Mat Frame, int Y, int X, int altura, int largura, int frame) {
    /*Declaração de variáveis de tipo inteiro para contagem*/
    int counter = 0;
    /*Declaração de variável double para armazenar a média das intensidades de pixels presentes no retângulo*/
    double mediaG = 0;
    double mediaB = 0;
    /*Declaração do objeto da classe Vec3b para capturar as intensidades de pixel*/
    cv::Vec3b pixels;

    /*Estrutura de repetição contada para varrer a imagem na direção horizontal*/
    for (int i = X; i < (X+largura); i++) {
        /*Estrutura de repetição contada para varrer a imagem na direção vertical*/
        for (int j = Y; j < (Y+altura); j++) {
            /*Captura do pixel na posição indicada por (i,j)*/
            pixels = Frame.at<cv::Vec3b>(i, j);
            /*Captura da intensidade da camada designada por plane do pixel capturado e incremento do valor de média*/
            mediaG = mediaG + pixels.val[1];
            mediaB = mediaB + pixels.val[0];
            /*Incremento da variável de contagem*/
            counter++;
        }
    }
    /*Cálculo da média das intensidades obtidas*/
    mediaG = (double)(mediaG/counter);
    mediaB = (double)(mediaB/counter);
    /*Armazenamento da média no vetor de médias*/
    Mediagreen[frame] = mediaG;
    Mediablue[frame] = mediaB;
}

/*----------------------------------------------------------------------------------------------------------------------------------------*/
/*Método para criar o vetor representante do filtro a ser aplciado sobre o sinal*/
void Frequencia::criaFiltro () {
    int j = 1;
    for (int i = 0; i < 1025; i++) {
        if (i < 21) {
            Filtro[i] = 0.2;
        } else if (i >= 21 && i <= 150) {
            Filtro[i] = ((0.006153846*j)+0.2);
            j++;
        } else {
            Filtro[i] = 1;
        }
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------------*/
/*Método para reorganizar as posições no vetor quando surgirem novas amostras*/
void Frequencia::getVectorFinal (int posInicial, int numFrames) {
    int j = 0;
    for(int k = posInicial; k < numFrames; k++) {
        MediaFinal[k] = Mediagreen[j] - Mediablue[j];
        j++;
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------------*/
/*Método para reorganizar as posições no vetor quando surgirem novas amostras a partir das execucoes posteriores aos 10s iniciais*/
void Frequencia::getVectorFinalResizable(int bloco, int deslocamento, int final) {
    memmove (MediaFinal, MediaFinal+deslocamento, sizeof(double)*(bloco+1));
    for (int i = bloco+1, j = 0; i < final; i++, j++) {
        MediaFinal[i] = Mediagreen[j] - Mediablue[j];
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------------*/
/*Método para organizar o vetor que conterá as amostras para o cálculo da média movel*/
void Frequencia::adicionaMediaMovel(int * mediaMovel, int bpm, int contadorMediaMovel, int flagMediaMovel) {
    if (contadorMediaMovel <= flagMediaMovel) {
        mediaMovel[contadorMediaMovel-1] = bpm;
    } else {
        for (int i = 0; i < flagMediaMovel-1; i++) {
            mediaMovel[i] = mediaMovel[i+1];
        }
        mediaMovel[flagMediaMovel-1] = bpm;
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------------*/
/*Método para calcular a média móvel das amostras*/
int Frequencia::getBPMMediaMovel(int * mediaMovel, int flagMediaMovel) {
    int soma = 0;
    for (int i = 0; i < flagMediaMovel; i++) {
        soma += mediaMovel[i];
    }
    return (int)(soma/flagMediaMovel);
}

/*----------------------------------------------------------------------------------------------------------------------------------------*/
/*Função 'FastFourierTransform' que realiza o cálculo da transformada rápida de Fourier sobre os valores fornecidos*/
/*O argumento inteiro n recebido é equivalente ao tamanho do vetor enviado*/
void Frequencia::FastFourierTransform (double * vetor, int n, const int padding, int fpsSaida, double * picos, int * indices, int janelaMatriz) {
    /*Armazena o valor de metade do vetor*/
    int nc;
    /*Iterador de contagem*/
    int k;
    /*Vetor com os dados para computar a FFT*/
    double * vetorExecucao;
    /*Declaração de um vetor de saída*/
    fftw_complex * out;
    /*Cria um plano um plano de ida, ou seja, uma estrutura que armazena todos os valores necessários para que o cálculo da transformada de Fourier seja efetuado*/
    fftw_plan plan_forward;

    /*Alocação de valores iniciais zero ao vetor de execução da FFT*/
    vetorExecucao = (double*) calloc (padding,sizeof(double));

    /*Estrutura de repetição contada para carregar os valores de entrada (removidos da média) para o vetor de execução da FFT*/
    for (k = 0; k < n; k++) {
        vetorExecucao[k] = vetor[k];
    }

    /*Calcula o tamanho de metade do vetor fornecido*/
    nc = padding/2 + 1;

    /*Alocação de um vetor de saída complexo (domínio da frequência) do tamanho de nc (metade do vetor fornecido)*/
    out = (fftw_complex*) fftw_malloc (sizeof(fftw_complex)*nc);

    /*Cria um plano para a transformada. Os argumentos de entrada da função abaixo são: a quantidade de elementos do vetor, o vetor enviado, um vetor de saída (complexo) e a forma como o plano será analisado (no caso, FFTW_ESTIMATE realiza a busca de uma solução sub-ótima e mais simples e mais rápida)*/
    plan_forward = fftw_plan_dft_r2c_1d (padding,vetorExecucao,out,FFTW_ESTIMATE);

    /*Executa a transformada de Fourier sobre o plano gerado, armazenando o resultado no vetor de saída "out"*/
    fftw_execute (plan_forward);

    /*Valor double que armazenará as magnitudes dos coeficientes da FFT*/
    double magnitudes[nc];
    /*Estrutura de repetição contada para obter a magnitude dos coeficientes através dos valores reais e imaginários de cada amostra computada*/
    for (int i = 0; i < nc; i++) {
        magnitudes[i] = sqrt(((out[i][0])*(out[i][0]))+((out[i][1])*(out[i][1])));
    }
    for (int i = 0; i < nc; i++) {
        magnitudes[i] = magnitudes[i]*Filtro[i];
    }

    /*Obtencao dos indices dos picos presentes no sinal de acordo com a faixa de bpm considerada*/
    capturarPicos(magnitudes,nc-1,0, fpsSaida, picos, indices, janelaMatriz);
    for (int i = 0; i < janelaMatriz; i++) {
        indices[i] = getBPM(indices[i],nc,fpsSaida);
    }

    /*Libera a memória alocada para os planos e para o vetor de saída temporário*/
    fftw_destroy_plan ( plan_forward );
    fftw_free (out);
}

/*----------------------------------------------------------------------------------------------------------------------------------------*/
/*Método para calcular a frequência cardíaca com base no indice do pico obtido*/
double Frequencia::getBPM (int pico, int nc, int fps) {
    int bpmMaximo = (fps*60)/2;
    return (((double)pico/(double)nc)*(double)bpmMaximo);
}

/*----------------------------------------------------------------------------------------------------------------------------------------*/
/*Método para organizar os picos obtidos de acordo com sua amplitude*/
void Frequencia::ordenaPicoseIndices (double * picos, int * indices, int janelaMatriz) {
    int i, j;
    double picosAux;
    int indicesAux;
    for (i = 0; i < janelaMatriz; i++) {
        for (j = 0; j < janelaMatriz-1; j++) {
            if (picos[j] < picos[j+1]) {
                picosAux = picos[j+1];
                picos[j+1] = picos[j];
                picos[j] = picosAux;
                indicesAux = indices[j+1];
                indices[j+1] = indices[j];
                indices[j] = indicesAux;
            }
        }
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------------*/
/*Método para inicializar a matriz de recorrencia*/
void Frequencia::inicializaMatriz (int **matriz, int janelaMatriz) {
    int i,j;
    for (i = 0; i < janelaMatriz; i++) {
        for (j = 0; j < janelaMatriz; j++) {
            matriz[i][j] = 0;
        }
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------------*/
/*Método para inicializar a matriz com o somatorio*/
void Frequencia::inicializaSomatorio (int **somatorio, int janelaMatriz) {
    for (int i = 0; i < janelaMatriz; i++) {
        for (int j = 0; j < 2; j++) {
            somatorio[i][j] = 0;
        }
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------------*/
/*Metodo para adicionar bpms na matriz*/
void Frequencia::adicionaMatriz (int * indices, int **matriz, int loopBPM, int **somatorio, int janelaMatriz, int espacoPicos) {
    int posicao, peso;
    /*Adicao dos N primeiros valores, o valor N é fornecido como parametro*/
    if (loopBPM <= janelaMatriz-1) {
        posicao = 0;
        for (int i = 0; i < janelaMatriz; i++) {
            for (int j = loopBPM; j < loopBPM+1; j++) {
                matriz[i][j] = indices[posicao];
                posicao++;
            }
        }
        /*Caso contrario, adiciona novos valores excluindo os iniciais*/
    } else {
        posicao = 0;
        for (int i = 0; i < janelaMatriz; i++) {
            for (int j = 1; j < janelaMatriz; j++) {
                matriz[i][j-1] = matriz[i][j];
            }
        }
        for (int i = 0; i < janelaMatriz; i++) {
            for (int j = janelaMatriz-1; j < janelaMatriz; j++) {
                matriz[i][j] = indices[posicao];
                posicao++;
            }
        }
    }
    /*Adicao da coluna de pesos iniciais*/
    if (loopBPM >= janelaMatriz-1) {
        peso = 5;
        for (int i = 0; i < janelaMatriz; i++) {
            for (int j = 0; j < 1; j++) {
                somatorio[i][j] = matriz[i][j];
                somatorio[i][j+1] = peso;
                peso--;
            }
        }
        /*Determinacao dos pesos para cada valor de bpm*/
        for (int j = 0; j < 1; j++) {
            for (int i = 0; i < janelaMatriz; i++) {
                for (int p = 0; p < janelaMatriz; p++) {
                    for (int t = 1; t < janelaMatriz; t++) {
                        if (abs(matriz[i][j]-matriz[p][t]) <= espacoPicos) {
                            somatorio[i][j] = somatorio[i][j];
                            somatorio[i][j+1] = somatorio[i][j+1] + ((janelaMatriz+1)-(p+1));
                        }
                    }
                }
            }
        }
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------------*/
/*Método para captura dos indices dos picos*/
void Frequencia::capturarPicos (double * magnitudes, int nc, bool media, int fpsSaida, double * picosSaida, int * indicesSaida, int janelaMatriz) {

    int i, j;
    double picos[janelaMatriz];
    double picosindices[janelaMatriz];
    double max = 0;
    double picosaux = 0;
    double picosMin = 1000;
    int picosMinIndice = 0;
    j = 0;

    /*Estrutura de repeticao contada para analisar entre os indices 36 e 210 (equivalente a 31 e 180 bpm)*/
    for (i = 36; i < 210; i++) {
        /*Captura dos N maiores picos e indices*/
        if ((magnitudes[i-1] < magnitudes[i]) && (magnitudes[i+1] < magnitudes[i]) && j < janelaMatriz) {
            picos[j] = magnitudes[i];
            picosindices[j] = i;
            j++;
            /*Caso contrario obtem o menor pico para exclui-lo na chegada de novos valores*/
        } else if ((magnitudes[i-1] < magnitudes[i]) && (magnitudes[i+1] < magnitudes[i]) && j == janelaMatriz) {
            for (int k = 0; k < janelaMatriz; k++) {
                if (picos[k] < picosMin) {
                    picosMin = picos[k];
                    picosMinIndice = k;
                }
            }
            /*Troca de novo valor por menor valor entre as N amostras anteriores*/
            if (picos[picosMinIndice] < magnitudes[i]) {
                picosindices[picosMinIndice] = i;
                picos[picosMinIndice] = magnitudes[i];
            }
        }
    }
    /*Flag 'media' para computar a media dos picos*/
    if (media) {
        for (int k = 0; k < 3; k++) {
            max = max + picosindices[k];
        }
        std::cout << "Retornando media" << std::endl;
        max = max/3;
        /*Caso contrario, armazena os indices dos picos*/
    } else {
        for (int k = 0; k < janelaMatriz; k++) {
            picosSaida[k] = picos[k];
            indicesSaida[k] = picosindices[k];
        }
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------------*/
/*Metodo para obtencao dos bpm's finais de acordo com a utilizacao dos pesos e seu respectivo somatorio*/
int Frequencia::getBPMFinal (int **somatorio, int janelaMatriz) {
    int maiorSoma = 0;
    int bpmS = 0;
    for (int j = 0; j < 1; j++) {
        for (int i = 0; i < janelaMatriz; i++) {
            if (somatorio[i][j+1] > maiorSoma) {
                    maiorSoma = somatorio[i][j+1];
                    bpmS = somatorio[i][j];
            }
        }
    }
    return bpmS;
}

/*----------------------------------------------------------------------------------------------------------------------------------------*/
/*Metodo para limpar a matriz do somatorio*/
void Frequencia::limpaSomatorio (int **somatorio, int janelaMatriz) {
    for (int j = 0; j < 1; j++) {
        for (int i = 0; i < janelaMatriz; i++) {
            somatorio[i][j] = 0;
        }
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------------*/
/*Metod para normalizar os canais G e B dos frames capturados*/
void Frequencia::removeDCeDivisaoDP (int posInicial, int posFinal) {
    double mediaG = 0;
    double sumMediaG = 0;
    double sumMediaB = 0;
    double mediaB = 0;
    double DPG;
    double DPB;
    int contador = 0;
    for (int i = posInicial; i < posFinal; i++) {
        mediaG += Mediagreen[i];
        mediaB += Mediablue[i];
        contador++;
    }

    /*Calculo da media*/
    mediaG = ((double)mediaG/(double)contador);
    mediaB = ((double)mediaB/(double)contador);

    /*Remocao do valor DC*/
    for (int i = posInicial; i < posFinal; i++) {
        Mediagreen[i] = Mediagreen[i] - mediaG;
        Mediablue[i] = Mediablue[i] - mediaB;
    }

    for (int i = posInicial; i < posFinal; i++) {
        sumMediaG += pow(Mediagreen[i],2);
        sumMediaB += pow(Mediablue[i],2);
    }

    /*Normalizacao*/
    DPG = sqrt(sumMediaG/(double)contador);
    DPB = sqrt(sumMediaB/(double)contador);

    for (int i = posInicial; i < posFinal; i++) {
        Mediagreen[i] = ((double)Mediagreen[i]/(double)DPG);
        Mediablue[i] = ((double)Mediablue[i]/(double)DPB);
    }
}
