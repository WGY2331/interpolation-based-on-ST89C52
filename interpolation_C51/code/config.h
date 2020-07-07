#ifndef CONFIG_H
#define CONFIG_H
#include<reg52.h>
#include<math.h>

#define uchar unsigned char
#define uint unsigned int
	
//lcd
sbit RS = P3^5;	
sbit RW = P3^6;
sbit EN = P3^4;

//步进电机
sbit A0=P1^0;//X
sbit A1=P1^1;
sbit A2=P1^2;
sbit A3=P1^3;
sbit B0=P1^4;//Y
sbit B1=P1^5;
sbit B2=P1^6;
sbit B3=P1^7;

//伺服电机
sbit Xdir=P2^4;//X方向
sbit Xpul=P2^5;//X脉冲
sbit Ydir=P2^6;//Y
sbit Ypul=P2^7;

void delay(uchar xms);
void wait();
void star();
void pop();

#endif 