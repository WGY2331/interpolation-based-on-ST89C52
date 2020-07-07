#ifndef KEY_H
#define KEY_H
#include"config.h"
#include<intrins.h>
sbit P35=P3^5;
sbit P36=P3^6;

uchar KeyScan();//键盘扫描
uchar KeyValue();//编码

void lcd_init();//声明初始化函数
void lcd_wcom(uchar);//声明写命令函数
void lcd_wdata(uchar);//声明写数据函数
bit lcd_busy();//忙碌判别
void lcd_line();//直线界面
void lcd_cir();//圆弧界面
void innum(char , char, char);//显示输入坐标

void GUI();//键盘扫描
	
#endif 
