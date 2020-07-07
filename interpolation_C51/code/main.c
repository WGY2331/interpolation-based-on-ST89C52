#include"config.h"
#include"key_lcd.h"
#include"drive.h"
//���ߣ�����ҫ
//2020.6.17
//51��Ƭ������������ϵͳ
uchar code numtable[]={"0123456789"};
extern int Xi,Yi,Xe,Ye;
	extern uint n;
	extern bit cirendflag;
	
void main(void) {
	lcd_init();
	delay(50);
	timer_init();
	
	while(1){
		wait();
	}

}
//void star() {//����� �ɵ���
//	
//	startline(5*150+ 5*150* 0.309, 0);//��ʼֱ�߲岹(��ֹx����ֹy)
//	while(n)wait();
//	startline(-5*150* 0.809, -5*150* 0.809*3.0777 + 5*150* 0.951);//��ʼֱ�߲岹(��ֹx����ֹy)
//	while(n)wait();
//	startline(0, 5*150* 0.951);//��ʼֱ�߲岹(��ֹx����ֹy)
//	while(n)wait();
//	startline(5*150* 0.809, -5*150* 0.809 *3.0777 + 5*150* 0.951);//��ʼֱ�߲岹(��ֹx����ֹy)
//	while(n)wait();
//	startline(-5*150- 5*150* 0.309, 0);//��ʼֱ�߲岹(��ֹx����ֹy)
//	while(n)wait();
//	startline(x, y);
//	while(n)wait();
//}
void pop(){//��ɡ
	startline(0,0);
	while(n)wait();
	startcir(75,-75,180,0);
	while(!cirendflag)wait();
	startline(600,300);
	while(n)wait();
	startcir(450,300,90,0);
	while(!cirendflag)wait();
	startcir(300,450,90,0);
	while(!cirendflag)wait();
	startcir(600,300,180,1);
	while(!cirendflag)wait();
	startcir(750,0,90,0);
	while(!cirendflag)wait();
	startcir(600,150,90,0);
	while(!cirendflag)wait();
	
}



void wait(){//����λ�����ȴ�����
	int x = Xi/15, y =Yi/15;
	if(n>0||!cirendflag){
	if(x<0){//��ʾ������
		lcd_wcom(0xc0 +2);
		lcd_wdata('-');
		x = -x;
	}
	else{
		lcd_wcom(0xc0 +2);
		lcd_wdata('+');
	}
	if(y<0){
		lcd_wcom(0xc0 +10);
		lcd_wdata('-');
		y = -y;
	}
	else{
		lcd_wcom(0xc0 +10);
		lcd_wdata('+');
	}
	
	send(Xi , 'x');
	send(Yi , 'y');//��������
	lcd_wcom(0xc0 +3);//��ʾʵʱ����
	lcd_wdata(numtable[x/1000%10]);
	lcd_wdata(numtable[x/100%10]);
	lcd_wdata(numtable[x/10%10]);
	lcd_wcom(0xc0 +7);
	lcd_wdata(numtable[x%10]);
		
	lcd_wcom(0xc0 +11);
	lcd_wdata(numtable[y/1000%10]);
	lcd_wdata(numtable[y/100%10]);
	lcd_wdata(numtable[y/10%10]);
	lcd_wcom(0xc0 +15);
	lcd_wdata(numtable[y%10]);
}
	GUI();//����ɨ��
	
}

void delay(uchar xms)
{
	uchar x, y;
	for (x = xms; x > 0; x--)
		for (y = 110; y > 0; y--);
}
