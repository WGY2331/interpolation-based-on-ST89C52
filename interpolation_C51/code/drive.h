#ifndef DRIVE_H
#define DRIVE_H
#include"config.h"

void timer_init();//��ʱ����ʼ��
void stepperA();//X���������ֵ
void stepperB();//Y���������ֵ


void line();//ֱ�߲岹����
void startline(int,int);//����ֱ�߲岹����

void cir();//Բ���岹����
uint getRNS(int,int);//��������
void startcir(int,int,int ,bit);//Բ���岹��ʼ��
uchar getJ(int,int);//���ص�ǰ��б45������
bit cirend();//�յ��б�

void send(int,uchar);//���ڷ���
#endif