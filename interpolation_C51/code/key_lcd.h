#ifndef KEY_H
#define KEY_H
#include"config.h"
#include<intrins.h>
sbit P35=P3^5;
sbit P36=P3^6;

uchar KeyScan();//����ɨ��
uchar KeyValue();//����

void lcd_init();//������ʼ������
void lcd_wcom(uchar);//����д�����
void lcd_wdata(uchar);//����д���ݺ���
bit lcd_busy();//æµ�б�
void lcd_line();//ֱ�߽���
void lcd_cir();//Բ������
void innum(char , char, char);//��ʾ��������

void GUI();//����ɨ��
	
#endif 
