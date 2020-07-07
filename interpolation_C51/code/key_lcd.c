#include"key_lcd.h"
#include"drive.h"
extern uint n;
uchar starnum = 0;	//�жϰ��˵ڼ���*
uchar numcheck = 0;//�ж����ְ�������

uchar KeyScan()  //����ɨ�躯����ʹ�����з�תɨ�跨
{
	unsigned char cord_h, cord_l;//����ֵ
	P3 = 0x0f;            //�������ȫΪ0
	cord_h = P3 & 0x0f;     //������ֵ
	if (cord_h != 0x0f)    //�ȼ�����ް�������
	{
		delay(10);        //ȥ��
		if ((P3 & 0x0f) != 0x0f)
		{
			cord_h = P3 & 0x0f;  //������ֵ
			P3 = cord_h | 0xf0;  //�����ǰ��ֵ
			cord_l = P3 & 0xf0;  //������ֵ
			while ((P3 & 0xf0) != 0xf0);//�ȴ��ɿ������
			return(cord_h + cord_l);//������������ֵ
		}
	}
	return(0xff);  //���ظ�ֵ
}



uchar KeyValue()//����
{
	switch (KeyScan())
	{
	case 0xee:return '7';  //������Ӧ�ļ���ʾ���Ӧ����ֵ
	case 0xde:return '8';
	case 0xbe:return '9';//	7	8	9	a
	case 0x7e:return 'a';//	4	5	6	b 
	case 0xed:return '4';//	1	2	3	c 
	case 0xdd:return '5';//	*	0	#	d 
	case 0xbd:return '6';
	case 0x7d:return 'b';
	case 0xeb:return '1';
	case 0xdb:return '2';
	case 0xbb:return '3';
	case 0x7b:return 'c';
	case 0xe7:return '*';
	case 0xd7:return '0';
	case 0xb7:return '#';
	case 0x77:return 'd';
	default:return 0xff;
	}
}


void lcd_init()//��ʼ��������
{
	EN = 0;
	delay(5);
	lcd_wcom(0x38);//����16*2��ʾ��5*7����8λ���ݽӿ�
	delay(5);
	lcd_wcom(0x0c);//���ÿ���ʾ������ʾ���
	delay(5);
	lcd_wcom(0x06);//дһ���ַ�ʱ����������
	delay(5);
	lcd_wcom(0x01);//��ʾ����
	lcd_line();//��ʾֱ�߽���

}

void lcd_wcom(uchar com)//д����ĺ�����
{
	while (lcd_busy()); //æ��ȴ�
	RS = 0;
	RW = 0;
	EN = 1;
	P0 = com;
	_nop_();
	EN = 0;
}

void lcd_wdata(uchar date)//д���ݵĺ�����
{
	while (lcd_busy()); //æ��ȴ�
	RS = 1;
	RW = 0;
	EN = 1;
	P0 = date;
	_nop_();
	EN = 0;
}

bit lcd_busy() {
	P0 = 0xFF;
	RS = 0;
	RW = 1;
	EN = 0;
	_nop_();
	EN = 1;
	return (bit)(P0 & 0x80);
}
void lcd_line() {
	lcd_wcom(0x01);

	lcd_wcom(0x80);
	lcd_wdata('X');//�յ�
	lcd_wdata('e');
	lcd_wdata('0');
	lcd_wdata('0');


	lcd_wcom(0x80 + 5);
	lcd_wdata('Y');
	lcd_wdata('e');
	lcd_wdata('0');
	lcd_wdata('0');

	lcd_wcom(0x80 + 10);
	lcd_wdata('s');
	lcd_wdata('p');
	lcd_wdata('d');
	lcd_wdata(':');
	lcd_wdata('0');

	lcd_wcom(0xc0);
	lcd_wdata('X');
	lcd_wdata(':');
	lcd_wcom(0xc0 + 6);
	lcd_wdata('.');

	lcd_wcom(0xc0 + 8);
	lcd_wdata('Y');
	lcd_wdata(':');
	lcd_wcom(0xc0 + 14);
	lcd_wdata('.');
}
void lcd_cir() {
	lcd_wcom(0x01);

	lcd_wcom(0x80);
	lcd_wdata('A');//�Ƕ�
	lcd_wdata('0');
	lcd_wdata('0');
	lcd_wdata('0');

	lcd_wcom(0x80 + 4);
	lcd_wdata('S');
	lcd_wdata('0');

	lcd_wcom(0x80 + 6);
	lcd_wdata('C');
	lcd_wdata('1');

	lcd_wcom(0x80 + 8);
	lcd_wdata('X');
	lcd_wdata('c');
	lcd_wdata('0');
	lcd_wdata('0');

	lcd_wcom(0x80 + 12);
	lcd_wdata('Y');
	lcd_wdata('c');
	lcd_wdata('0');
	lcd_wdata('0');

	lcd_wcom(0xc0);
	lcd_wdata('X');
	lcd_wdata(':');
	lcd_wcom(0xc0 + 6);
	lcd_wdata('.');

	lcd_wcom(0xc0 + 8);
	lcd_wdata('Y');
	lcd_wdata(':');
	lcd_wcom(0xc0 + 14);
	lcd_wdata('.');
}

void GUI() {

	static bit keyin = 0;//�����������
	uchar value = 0;	//����ֵ
	extern int angle;
	extern uint spd;
	extern bit shape;
	extern bit clcwise;
	extern bit cirendflag;
	extern int Xc, Yc, Xe, Ye;
	extern uchar code numtable[];

	value = KeyValue();//ɨ�����
	if (value == '#') {//��ͣ�뿪ʼ
		if(n){//��ͣ
			TR1 = 0;
			TR0 = 0;
			n = 0;
			cirendflag = 1;
		}
		else{
			if(shape)
				startline(Xe,Ye);
			else startcir(Xc,Yc,angle,clcwise);
		}
	}
	else if (!n && value == 'a') {//�Զ�ͼ��
		pop();
	}
	else if (value == 'b') {//�ӵ�
		if(spd > 5000)
			spd = spd -  5000;
		if (shape)//ֱ��
			lcd_wcom(0x80 + 14);
		else //Բ��
			lcd_wcom(0x80 + 5);
		lcd_wdata(numtable[10 - spd / 5000]);
	}
	else if (value == 'c') {//����
		if(spd < 50000)
			spd = spd + 5000;
		if (shape)//ֱ��
			lcd_wcom(0x80 + 14);
		else //Բ��
			lcd_wcom(0x80 + 5);
		lcd_wdata(numtable[10 - spd / 5000]);
	}
	else if(value=='d'){//�л�����
		
		if(shape){
			shape=0;
			lcd_cir();
		}
		else {
			shape=1;
			lcd_line();
		}
	}

	if (shape&& (!n || cirendflag)) {//ֱ�߽���
		if (value == '*') {//�л����λ��
			numcheck = 0;
			starnum++;//���ܼ����´�����¼
			switch (starnum) {
			case 1:
				lcd_wcom(0x80 + 2);//��1�ΰ��¹����˸��λ���Ƕ�λ��
				lcd_wcom(0x0f); //��꿪ʼ��˸
				keyin = 1;//��������
				break;
			case 2:
				lcd_wcom(0x80 + 7);//��2�ΰ��¹����˸��λ���ٶ�λ��
				break;
			case 3:
				lcd_wcom(0x80 + 14);//��3�ΰ��¹����˸��λ��˳��ʱ��λ��
				break;
			default:
				starnum = 0;//��¼����������
				lcd_wcom(0x0c); //ȡ�������˸
				keyin = 0;//��������
			}
		}
		else if (keyin&&value >= '0'&&value <= '9') {//��������
			switch (starnum) {
			case 1:
				innum(2,  value,1);
				break;
			case 2:
				innum(7,  value,2);
				break;
			case 3:
				lcd_wcom(0x80 + 14);
				lcd_wdata(value);
				spd = 50000 - 5000 * (value - '0');
				starnum = 4;
				break;
			default:
				starnum = 0;//��¼����������
				lcd_wcom(0x0c); //ȡ�������˸
				keyin = 0;//��������
			}
		}
	}
	else if( !shape && (!n || cirendflag)) {//Բ������
		if (value == '*') {
			numcheck = 0;
			starnum++;//���ܼ����´�����¼
			switch (starnum) {
			case 1:
				lcd_wcom(0x80 + 1);//��1�ΰ��¹����˸��λ���Ƕ�λ��
				lcd_wcom(0x0f); //��꿪ʼ��˸
				keyin = 1;//��������
				break;
			case 2:
				lcd_wcom(0x80 + 5);//��2�ΰ��¹����˸��λ���ٶ�λ��
				break;
			case 3:
				lcd_wcom(0x80 + 7);//��3�ΰ��¹����˸��λ��˳��ʱ��λ��
				break;
			case 4:
				lcd_wcom(0x80 + 10);//��4�ΰ��¹����˸��λ��Բ��Xλ��
				break;
			case 5:
				lcd_wcom(0x80 + 14);//�����ΰ��¹����˸��λ��Բ��Yλ��
				break;
			default:
				starnum = 0;//��¼����������
				lcd_wcom(0x0c); //ȡ�������˸
				keyin = 0;//��������
			}
		}
		else if (keyin&&value >= '0'&&value <= '9') {
			switch (starnum) {
			case 1:		//����Ƕ�
				if (numcheck == 0) {
					lcd_wcom(0x80 + 1);//��˴���Ļ
					lcd_wdata(0x20);
					lcd_wdata(0x20);
					lcd_wdata(0x20);

					angle = (value - '0');
					lcd_wcom(0x80 + 1);//��1��
					lcd_wdata(value);
					lcd_wcom(0x80 + 2);
					numcheck++;
				}
				else if (numcheck == 1) {
					angle = angle * 10 + (value - '0');//��2��
					lcd_wcom(0x80 + 2);
					lcd_wdata(value);
					lcd_wcom(0x80 + 3);
					numcheck++;
				}
				else if (numcheck == 2) {
					angle = angle * 10 + (value - '0');//��3��
					lcd_wcom(0x80 + 3);
					lcd_wdata(value);
					numcheck = 0;
					
				}
				break;
			case 2:
				spd = 50000 - 5000 * (value - '0');
				lcd_wcom(0x80 + 5);
				lcd_wdata(value);
				//lcd_wcom(0x80 + 7);
				//starnum = 3;
				break;
			case 3:
				if (value == '0')
					clcwise = 0;
				else clcwise = 1;
				lcd_wcom(0x80 + 7);
				lcd_wdata(value);
				//starnum = 4;
				//lcd_wcom(0x80 + 10);
				break;
			case 4:
				innum(10,  value, 3);
				break;
			case 5:
				innum(14,  value, 4);
				break;
			default:
				starnum = 0;//��¼����������
				lcd_wcom(0x0c); //ȡ�������˸
				keyin = 0;//��������
			}
		}
	}
		value = 0xff;
}


void innum(char wz, char vl,char xy) {//��λ����ʾλ��
	if (!numcheck) {//���û��������
		numcheck++;
		lcd_wcom(0x80 + wz);//��˴���Ļ
		lcd_wdata(0x20);
		lcd_wdata(0x20);

		lcd_wcom(0x80 + wz);
		lcd_wdata(vl);
		lcd_wcom(0x80 + wz + 1);
		switch(xy){
			case 1:Xe=(vl - '0') * 150;break;//��λ
			case 2:Ye=(vl - '0') * 150;break;
			case 3:Xc=(vl - '0') * 150;break;
			case 4:Yc=(vl - '0') * 150;break;
		}
	}
	else  {
		lcd_wcom(0x80 + wz + 1);
		lcd_wdata(vl);
		numcheck = 0;	
		//starnum++;
		switch(xy){
			case 1:Xe=Xe*10+(vl - '0') * 150;break;//ʮλ
			case 2:Ye=Ye*10+(vl - '0') * 150;break;
			case 3:Xc=Xc*10+(vl - '0') * 150;break;
			case 4:Yc=Yc*10+(vl - '0') * 150;break;
		}
	}
}

