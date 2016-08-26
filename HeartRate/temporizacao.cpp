/*------------------------------------------------------------------------------------------------------------------------------------------
TEMPORIZAÇÃO.CPP:
Este é um arquivo de implementação de corpo de métodos.

PIBIC 2015-2016: Software para detecção de frequência cardíaca
Alunos: Maximillian Fan Xavier e Otávio Alves Dias
Professor: Eduardo Peixoto Fernandes da Silva
Universidade de Brasília, Brasília-DF, Brasil
------------------------------------------------------------------------------------------------------------------------------------------*/

/*Declaração das bibliotecas da linguagem C++*/
#include <iostream>

/*Declaração das bibliotecas da linguagem C*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

/*Declaração das bibliotecas da ferramenta OpenCV*/
#include "opencv2/opencv.hpp"

/*Declaração das bibliotecas próprias*/
#include "temporizacao.hpp"
#include "frequencia.hpp"

/*Definicao da imagem em niveis de cinza para conter a ROI do Viola Jones e as restrições de tamanho (minimo e maximo)
para as faces encontradas*/
cv::Mat grayScale;
std::vector<cv::Rect> faces;
cv::Size minSize(200,250);
cv::Size maxSize(400,500);



/*----------------------------------------------------------------------------------------------------------------------------------------*/
/*Método construtor do objeto da classe, inicilizando os parâmetros 'largura' e 'altura' com os valores default zero e carregando o classificador
de face do arquivo XML*/
Temporizacao::Temporizacao () {
    this->largura = 0;
    this->altura = 0;
    this->face.load("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_frontalface_default.xml");
}

/*----------------------------------------------------------------------------------------------------------------------------------------*/
/*Método 'startCam' para inicializar o objeto que representa a câmera e realizar a abertura da mesma*/
cv::VideoCapture Temporizacao::startCam (int camera) {
	/*Criação do objeto de câmera e abertura do dispositivo com a passagem do parâmetro '0' em seu construtor*/
    cv::VideoCapture VC (camera);
	/*Set dos parâmetros 'largura' e 'altura' do objeto da classe Temporizacao com os parâmetros de altura e lagura presentes nas informações de propriedades do objeto de câmera aberto*/
    this->altura = VC.get(CV_CAP_PROP_FRAME_HEIGHT);
    this->largura = VC.get(CV_CAP_PROP_FRAME_WIDTH);
	/*Retorno do objeto de câmera já inicializado*/
    return VC;
}

/*----------------------------------------------------------------------------------------------------------------------------------------*/
/*Método 'getCamera' para obter o objeto de câmera presente no objeto da classe Temporizacao*/
cv::VideoCapture Temporizacao::getCamera () {
	/*Retorno do atributo de câmera*/
    return this->VC;
}

/*----------------------------------------------------------------------------------------------------------------------------------------*/
/*Método 'exibeVideo' para realiza a captura de frames da webcam e exibi-los em tela*/
void Temporizacao::batimentoCardiaco (int FPS, int janelaMatriz, int espacoPicos, int tempoNextLoops, int framesViolaJones, int flagMediaMovel) {
    /*Declaração de objeto 'quadro' da classe Mat (matriz representando o frame com métodos de manipulações do mesmo)*/
    cv::Mat quadro;
    Frequencia frequencia;
    /*Chamada do método 'namedWindow' para criar uma janela na tela de tamanho automático que conterá a saída dos frames da webcam*/
    cv::namedWindow("Camera", cv::WINDOW_AUTOSIZE);
    struct timeval t1, t2;
    double tempo = 0;
    char numString[3];
    int bpm = 0;
    int bpmFinal;
    int firstLoop = 0;
    int frameCounter = 0;
    int frameCounterVariante, framesLoop10S, fpsSaida;
    double picos[janelaMatriz];
    int indices[janelaMatriz];
    int mediaMovel[flagMediaMovel];
    int loopBPMCounter = 0;
    int contadorMediaMovel = 1;

    /*Alocacao da matriz de recorrencia*/
    int **matrizReferencia = (int**) malloc (janelaMatriz*sizeof(int));
    for(int i = 0; i < janelaMatriz; i++) {
        matrizReferencia[i] = (int*) malloc (janelaMatriz*sizeof(int));
    }

    /*Alocacao da matriz de somatorio*/
    int **somatorio = (int**) malloc (janelaMatriz*sizeof(int));
    for(int i = 0; i < janelaMatriz; i++) {
        somatorio[i] = (int*) malloc (2*sizeof(int));
    }

    /*Alocacao dos vetores G e B e inicializacao das matrizes e do filtro derivativo*/
    frequencia.alocaVetoresGB();
    frequencia.criaFiltro();
    frequencia.inicializaMatriz(matrizReferencia, janelaMatriz);
    frequencia.inicializaSomatorio(somatorio, janelaMatriz);
    /*Estrutura de repeticao contada para executar um loop infinito durante a execucao do programa*/
    while (1) {
        if (!firstLoop) {
            /*Estrutura de repeticao para executar o laço inicial de 10s para obtermos as N amostras iniciais*/
            while (tempo < 10000000) {
                gettimeofday(&t1, NULL);
                /*Chamada do método 'read' pertencente à classe VideoCapture. O método 'read' realiza a combinação de dois sub métodos:
                1 - grab() - analisa a existência de um frame (da câmera ou de um arquivo consecutivo de imagens) e retorna um booelano indicando a sua existência (TRUE - existe, FALSE - não existe)
                2 - retrive() - decodifica e retorna o frame capturado, caso haja problemas ele retorna NULL
                O retorno da função 'read' (frame capturado) é armazenado no objeto 'quadro'*/
                this->VC >> quadro;
                /*Verifica-se se o frame capturado é o primeiro no segundo corrente, se for obtem-se a posicao facial no frame*/
                if (frameCounter%framesViolaJones == 0) {
                    getFacePosition(quadro);
                }
                /*Desenho da bounding box sobre o frame*/
                cv::rectangle(quadro, cv::Rect(this->ptF1.x,this->ptF1.y,this->ptF2.x,this->ptF2.y), cv::Scalar(0,255,0),1,8,0);
                /*Desenho da box para exibicao da frequencia cardiaca*/
                cv::rectangle(quadro,cv::Rect(VC.get(CV_CAP_PROP_FRAME_WIDTH)-200,VC.get(CV_CAP_PROP_FRAME_HEIGHT)-200,200,200),cv::Scalar(0,0,0,255),0,8,0);
                /*Adicao de texto na box de saida*/
                cv::putText(quadro, " X", cv::Point(480,410), cv::FONT_HERSHEY_SIMPLEX, 3, cv::Scalar(255,255,255), 1, 8,false);
                /*Chamada do método 'imshow' para exibição do frame capturado na janela previamente criada*/
                cv::imshow("Camera", quadro);
                /*Estrutura condicional que realiza a chamada do método 'waitKey' que realiza a espera por uma entrada do teclado, para que a captura seja encerrada*/
                if (cv::waitKey((int)(((double)1/(double)FPS)*(double)1000)) >= 0) {
                    cv::destroyAllWindows();
                    exit(0);
                }
                /*Obtenção da media dos pixels sobre os canais presentes nos frames*/
                frequencia.getMedias(quadro, this->ptF1.x, this->ptF1.y,this->ptF2.y, this->ptF2.x, frameCounter);
                frameCounter++;
                gettimeofday(&t2, NULL);
                tempo += (((t2.tv_sec*1000000)+t2.tv_usec)-((t1.tv_sec*1000000)+t1.tv_usec));
            }
            /*Setagem de flags*/
            frameCounterVariante = frameCounter;
            framesLoop10S = frameCounter;
            frameCounter = 0;
            firstLoop = 1;
            tempo = 0;
            fpsSaida = floor(frameCounterVariante/10);
            /*Normalizacao dos vetores G e B*/
            frequencia.removeDCeDivisaoDP(0, frameCounterVariante);
            frequencia.getVectorFinal(0,frameCounterVariante);
            /*Aplicacao da FFT sobre o sinal resultante*/
            frequencia.FastFourierTransform(frequencia.MediaFinal,framesLoop10S,TAMANHOFFT, fpsSaida, picos, indices, janelaMatriz);
            /*Ordenacao dos picos capturados*/
            frequencia.ordenaPicoseIndices (picos, indices, janelaMatriz);
            /*Adicao dos bpm's à matriz de recorrencia*/
            frequencia.adicionaMatriz(indices,matrizReferencia, loopBPMCounter, somatorio, janelaMatriz, espacoPicos);
            loopBPMCounter++;
        } else {
            /*Estrutura de repeticao para executar o software durante 2 segundos com novas amostras*/
            while (tempo < tempoNextLoops) {
                gettimeofday(&t1, NULL);
                this->VC >> quadro;
                /*Verifica-se se o frame capturado é o primeiro no segundo corrente, se for obtem-se a posicao facial no frame*/
                if (frameCounter%framesViolaJones == 0) {
                    getFacePosition(quadro);
                }
                 /*Desenho da bounding box sobre o frame*/
                cv::rectangle(quadro, cv::Rect(this->ptF1.x,this->ptF1.y,this->ptF2.x,this->ptF2.y), cv::Scalar(0,255,0),1,8,0);
                 /*Desenho da box para exibicao da frequencia cardiaca*/
                cv::rectangle(quadro,cv::Rect(VC.get(CV_CAP_PROP_FRAME_WIDTH)-200,VC.get(CV_CAP_PROP_FRAME_HEIGHT)-200,200,200),cv::Scalar(0,0,0),CV_FILLED,8,0);
                /*Verifica-se se a contagem de frames indica um final de processamento, para assim aplicar a media movel
                e determinar o BPM na saida em tela*/
                if (contadorMediaMovel > flagMediaMovel) {
                    bpmFinal = frequencia.getBPMMediaMovel(mediaMovel, flagMediaMovel);
                    sprintf(numString,"%d",bpmFinal);
                    cv::putText(quadro, numString, cv::Point(480,410), cv::FONT_HERSHEY_SIMPLEX, 3, cv::Scalar(255,255,255), 1, 8,false);
                } else {
                    cv::putText(quadro, " X", cv::Point(480,410), cv::FONT_HERSHEY_SIMPLEX, 3, cv::Scalar(255,255,255), 1, 8,false);
                }
                 /*Chamada do método 'imshow' para exibição do frame capturado na janela previamente criada*/
                cv::imshow("Camera", quadro);
                if (cv::waitKey((int)(((double)1/(double)FPS)*(double)1000)) >= 0) {
                    cv::destroyAllWindows();
                    exit(0);
                }
                 /*Obtenção da media dos pixels sobre os canais presentes nos frames*/
                frequencia.getMedias(quadro,this->ptF1.x, this->ptF1.y,this->ptF2.y, this->ptF2.x, frameCounter);
                frameCounter++;
                gettimeofday(&t2, NULL);
                tempo += (((t2.tv_sec*1000000)+t2.tv_usec)-((t1.tv_sec*1000000)+t1.tv_usec));
            }
            /*Setagem de flags*/
            frameCounterVariante = frameCounter;
            frameCounter = 0;
            tempo = 0;
             /*Normalizacao dos vetores G e B*/
            frequencia.removeDCeDivisaoDP(0, frameCounterVariante);
            frequencia.getVectorFinalResizable((framesLoop10S-frameCounterVariante)-1, frameCounterVariante, framesLoop10S);
             /*Aplicacao da FFT sobre o sinal resultante*/
            frequencia.FastFourierTransform(frequencia.MediaFinal,framesLoop10S,TAMANHOFFT, fpsSaida, picos, indices, janelaMatriz);
             /*Ordenacao dos picos capturados*/
            frequencia.ordenaPicoseIndices (picos, indices, janelaMatriz);
            /*Adicao dos bpm's à matriz de recorrencia*/
            frequencia.adicionaMatriz(indices,matrizReferencia, loopBPMCounter, somatorio, janelaMatriz, espacoPicos);
            loopBPMCounter++;
            /*Verifica-se se a contagem de frames indica um tamanho maior que N da matriz, indicando a necessidade de
            computar um valor de frequencia cardiaca com os valores presentes na matriz de recorrencia e armazenando
            esse valor no vetor de media movel para futura exibicao*/
            if (loopBPMCounter >= janelaMatriz) {
                bpm = frequencia.getBPMFinal(somatorio, janelaMatriz);
                frequencia.adicionaMediaMovel(mediaMovel,bpm,contadorMediaMovel,flagMediaMovel);
                contadorMediaMovel++;
                frequencia.limpaSomatorio(somatorio, janelaMatriz);
            }
        }
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------------*/
/*Metodo para obter a posicao facial presente no frame capturado*/
void Temporizacao::getFacePosition (cv::Mat quadro) {
    /*Conversao do frame capturado em niveis de cinza*/
    cv::cvtColor(quadro, grayScale, CV_BGR2GRAY);
    /*Aplicacao do metodo de Viola Jones*/
    this->face.detectMultiScale(grayScale, faces,1.3,3,0,minSize,maxSize);
    /*Obtencao das coordenadas e dimensao da bounding box obtida*/
    if (faces.size() == 1) {
        this->ptF1.x = faces[0].x*(1.55);
        this->ptF1.y = faces[0].y;
        this->ptF2.x = faces[0].width*(0.45);
        this->ptF2.y = faces[0].height;
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------------*/
/*Metodo para carregar os parametros presentes no arquivo params.txt*/
void Temporizacao::getParams(int * exibeFPS, int * janelaMatriz, int * espacoPicos, int * tempoNextLoops, int * framesViolaJones, int * flagMediaMovel) {
    FILE * fp = fopen("params.txt","r");
    if (fp != NULL) {
        fscanf(fp,"%d,%d,%d,%d,%d,%d",exibeFPS,janelaMatriz,espacoPicos,tempoNextLoops,framesViolaJones,flagMediaMovel);
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------------*/
/*Método 'exibeVideo' para realiza a captura de frames da webcam e exibi-los em tela*/
void Temporizacao::exibeVideo (int FPS) {
    /*Declaração de objeto 'quadro' da classe Mat (matriz representando o frame com métodos de manipulações do mesmo)*/
    cv::Mat quadro;
    bool box = true;
    bool getValue = true;
    int fpsTime = floor(((double)1/(double)FPS)*(double)1000);
    /*Estrutura de repetição que realiza a captura de frames e exibe-os na tela*/
    while (box) {
        /*Chamada do método 'read' pertencente à classe VideoCapture. O método 'read' realiza a combinação de dois sub métodos:
        1 - grab() - analisa a existência de um frame (da câmera ou de um arquivo consecutivo de imagens) e retorna um booelano indicando a sua existência (TRUE - existe, FALSE - não existe)
        2 - retrive() - decodifica e retorna o frame capturado, caso haja problemas ele retorna NULL
        O retorno da função 'read' (frame capturado) é armazenado no objeto 'quadro'*/
        this->VC >> quadro;
        cv::cvtColor(quadro, grayScale, CV_BGR2GRAY);
        /*Aplicacao do metodo de Viola Jones sobre o frame capturado*/
        this->face.detectMultiScale(grayScale, faces,1.1,3,0,minSize,maxSize);
        if (faces.size() == 1) {
            this->pt1.x = faces[0].width;
            this->pt1.y = faces[0].height;
            this->pt2.x = faces[0].x;
            this->pt2.y = faces[0].y;
        }

        /*Ajuste de ROI sobre a bounding box obtida*/
        this->ptF1.x = (int)(this->pt2.x*(1.55));
        this->ptF1.y = this->pt2.y;
        this->ptF2.x = (int)(this->pt1.x*(0.45));
        this->ptF2.y = this->pt1.y;

        /*Desenho das bounding box obtidas sobre o frame capturado*/
        cv::rectangle(quadro, cv::Rect(this->pt2.x,this->pt2.y,this->pt1.x,this->pt1.y),cv::Scalar(255,0,0),1,8,0);
        cv::rectangle(quadro, cv::Rect(this->ptF1.x,this->ptF1.y,this->ptF2.x,this->ptF2.y), cv::Scalar(0,255,0),1,8,0);
        cv::imshow("Camera", quadro);
        if (cv::waitKey(fpsTime) >= 0) {
            break;
        }
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------------*/
/*Metodo para capturar o FPS da camera conectada ao computador*/
int Temporizacao::getFPS (cv::VideoCapture camera) {

    cv::Mat frame;
    struct timeval t1, t2;
    double tempo;

    /*Obtencao de frames durante uma estrutura de repeticao*/
    std::cout << "Capturando FPS da camera..." << std::endl;
    gettimeofday(&t1, NULL);
    for (int k = 0; k < 200; k++) {
        camera >> frame;
    }
    gettimeofday(&t2, NULL);

    /*Calculo do tempo gasto para capturar os frames, obtendo assim o fps*/
    tempo = ((t2.tv_sec*1000000)+t2.tv_usec)-((t1.tv_sec*1000000)+t1.tv_usec);

    return ceil(200/(tempo/1000000));
}
