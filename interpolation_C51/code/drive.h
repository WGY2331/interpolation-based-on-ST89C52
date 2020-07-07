#ifndef DRIVE_H
#define DRIVE_H
#include"config.h"

void timer_init();//定时器初始化
void stepperA();//X步进电机赋值
void stepperB();//Y步进电机赋值


void line();//直线插补程序
void startline(int,int);//计算直线插补步数

void cir();//圆弧插补程序
uint getRNS(int,int);//返回象限
void startcir(int,int,int ,bit);//圆弧插补初始化
uchar getJ(int,int);//返回当前点斜45度区间
bit cirend();//终点判别

void send(int,uchar);//串口发送
#endif