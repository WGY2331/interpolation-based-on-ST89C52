#include"drive.h"

#define FOSC 11059200UL     //System frequency
#define BAUD 9600       //UART baudrate

uchar code table[] = { 0x01,0x03,0x02,0x06,0x04,0x0c,0x08,0x09 };//��

uint spd = 2000;//�ٶȣ��ж����ʱ��
uchar acc0 = 0;//���ٶ�
uint n = 0;//ֱ�߲岹����

char rhy0 = 0, rhy1 = 0;//���������������¼
int X0 = 0, Y0 = 0, Xe = 0, Ye = 0, Xc = 0, Yc = 0;//��ʼ�������ֹ����,Բ������
int Xi = 0, Yi = 0;	//��ǰ����
int Ii = 0, Ji = 0;//Բ�����Բ������
int I0 = 0, J0 = 0;//Բ����ʼ��
int Iee = 0, Je = 0;//Բ����ֹ��
int Ir = 0, Jr = 0;//Բ���ο���
int F = 0;//fΪб���б�ʽ
uchar JZe = 0;//�յ��б����
int angle = 0;//-360~360,��ʱ��Ϊ��
bit shape = 1;	//1:ֱ��	0������
bit clcwise = 1;	//1��˳ʱ��	0����ʱ��
bit sending;//�����б�־λ
bit piflag = 0;//1Ϊ�ѹ��ο���
bit cirendflag = 1;//1Ϊһ��Բ���岹�ѽ���

void timer_init()
{
	TMOD = 0x11;//��ʱ��0������ʽ1.16λ����ʱ��1������ʽ1,16λ��ʱ��
	SCON = 0x50;           //���ڹ�����ģʽ1

	TH0 = (65536 - 50000) >> 8;
	TL0 = (65536 - 50000);
	TH1 = (65536 - 50000) >> 8;
	TL1 = (65536 - 50000);
	TL2 = RCAP2L = (65536 - (FOSC / 32 / BAUD)); //���㶨ʱ����װֵ
	TH2 = RCAP2H = (65536 - (FOSC / 32 / BAUD)) >> 8;//����������

	T2CON = 0x34;           //Timer2 start run
	TR2 = 1;		//��ʱ��2��ʼ��ʱ
	TR1 = 0;	  //��������ʱ��1
	ET1 = 1;	   //��ʱ��1�ж�ʹ��

	TR0 = 0;	//��ʱ��0������
	ET0 = 1;	//��ʱ���ж�ʹ��
	ES = 1;         //�����ж�����
	EA = 1;	//ȫ���ж�

}


void send(int d, uchar xy)		  //����2���ֽڵ����ݣ�d����������,xyΪ�����ʶ
{
	//Э�飺8λ:x��0 y��1����7λ����
	uchar temp;//��λ
	uchar temp1;//��λ���߶�λΪ�����ű�ʶ
	if (d >= 0) {
		if (xy == 'x') {
			temp = d & 0x7f;//0xxx xxxx
			temp1 = (d >> 7) & 0x3f;//00xx xxxx
		}
		else {
			temp = d & 0x7f | 0x80;//1xxx xxxx
			temp1 = (d >> 7) & 0x3f | 0x80;//10xx xxxx
		}
	}
	else {
		if (xy == 'x') {
			temp = -d & 0x7f;//0xxx xxxx
			temp1 = (-d >> 7) & 0x3f | 0x40;//01xx xxxx
		}
		else {
			temp = -d & 0x7f | 0x80;//1xxx xxxx
			temp1 = (-d >> 7) & 0x3f | 0xc0;//11xx xxxx
		}
	}
	SBUF = temp; //������д�뵽���ڻ���
	sending = 1;	 //���÷��ͱ�־
	while (sending); //�ȴ��������

	SBUF = temp1; //������д�뵽���ڻ���
	sending = 1;	 //���÷��ͱ�־
	while (sending); //�ȴ��������
}

void uart(void) interrupt 4	//���ڷ����ж�
{
	if (RI)    //�յ�����
		RI = 0;   //���ж�����
	else      //������һ�ֽ�����
	{
		TI = 0;
		sending = 0;  //�����ڷ��ͱ�־
	}
}

void stepperA() {//A��������ź� x��
	
	A0 = table[rhy0] & 1;//��λ�ֱ�ֵ
	A1 = (table[rhy0] >> 1) & 1;
	A2 = (table[rhy0] >> 2) & 1;
	A3 = (table[rhy0] >> 3) & 1;
	if (Xdir) 
		rhy0 = rhy0 < 7 ? rhy0 + 1 : 0;//������0
	else 
		rhy0 = rhy0 > 0 ? rhy0 - 1 : 7;
}

void stepperB() {//B��������ź� y��
	
	B0 = table[rhy1] & 1;//��λ�ֱ�ֵ
	B1 = (table[rhy1] >> 1) & 1;
	B2 = (table[rhy1] >> 2) & 1;
	B3 = (table[rhy1] >> 3) & 1;
	if (Ydir) 
		rhy1 = rhy1 < 7 ? rhy1 + 1 : 0;
	else 
		rhy1 = rhy1 > 0 ? rhy1 - 1 : 7;
}

void Timer0() interrupt 1  //ֱ�߲岹
{
	static bit jo = 0;//��ż�ν����ж�
	static uint temp = 50000;
	if (n > 200 && temp > spd)
		temp -= acc0;//����
	if (n <= 200 && temp < 50000)
		temp += acc0;//����
	if(!(n > 200 && temp > spd)&&!(n <= 200 && temp < 50000))
		temp=spd;//�����мӼ���
	TH0 = (65536 - temp) >> 8;
	TL0 = (65536 - temp);//����װ�ؼ�ʱ��
	if (jo) {//������
		if (n > 0) {//�յ��б�
			line();//ֱ�߲岹
			n--;
		}
	}
	else {//ż����
		if (Xpul) Xpul = 0;	//�����½�
		if (Ypul) Ypul = 0;
	}
	jo = !jo;
}

void Timer1() interrupt 3 {//Բ���岹
	static bit jo = 0;
	static uint temp = 50000;
	if (temp > spd && (abs(Ji - J0) + abs(Ii - I0) < 300)) //�ٶ�С��������
		temp -= acc0;//����
	if (temp < 50000 && (abs(Ji - Je) + abs(Ii - Iee) < 200))
		temp += acc0*3;//����
//	if(!(temp > spd && (abs(Ji - J0) + abs(Ii - I0) < 300))&&!(temp < 50000 && (abs(Ji - Je) + abs(Ii - Iee) < 200)))
//		temp=spd;////�����мӼ���
	TH1 = (65536 - temp) >> 8;
	TL1 = (65536 - temp);//����װ�ؼ�ʱ��
	if (jo) {//������
		if (!cirend()) //�յ��б�
			cir();//Բ���岹
		else cirendflag = 1;//Բ���岹����
	}
	else {//ż����
		if (Xpul) Xpul = 0;	//�����½�
		if (Ypul) Ypul = 0;
	}
	jo = !jo;
}

void line() {//һ�β岹
	if (F >= 0) {
		if (Xdir) {
			Xi++;
			Xpul = 1;//�������� 
			stepperA();//�Ĵ�������λX��ֵ
		}
		else {
			Xi--;
			Xpul = 1;//��������
			stepperA();//�Ĵ�������λX��ֵ
		}
		if (Ydir)F -= (Ye - Y0);//�б�ʽ
		else F -= (Y0 - Ye);
	}
	else {
		if (Ydir) {
			Yi++;
			Ypul = 1;//�������� 
			stepperB();//�Ĵ�������λY��ֵ
		}
		else {
			Yi--;
			Ypul = 1;//�������� 
			stepperB();//�Ĵ�������λY��ֵ
		}
		if (Xdir)F += (Xe - X0);//�б�ʽ
		else F += (X0 - Xe);
	}

}

uint getRNS(int X, int Y) {//�����������
	if (clcwise) {	//˳ʱ��
		if (X > 0 && Y >= 0)return 1;	//��һ����˳ʱ��
		else if (X <= 0 && Y > 0)return 2;//��˳
		else if (X < 0 && Y <= 0)return 3;//��˳
		else return 4;//��˳	
	}
	else {	//��ʱ��
		if (X >= 0 && Y > 0)return 5;//��һ������ʱ��
		else if (X < 0 && Y >= 0)return 6;//����
		else if (X <= 0 && Y < 0)return 7;//����
		else return 8;//����
	}
}
void cir(void) {//һ��Բ���岹
	unsigned int temp;
	temp = getRNS(Ii, Ji);//��ǰ�����޺ͷ���
	if (F >= 0) {
		if (temp == 1 || temp == 6) {
			F = F - 2 * (Ji--) + 1;//�б�ʽ
			Ydir = 0;
			Ypul = 1;
			stepperB();//�Ĵ�������λY��ֵ
		}
		else if (temp == 2 || temp == 7) {
			F = F + 2 * (Ii++) + 1;
			Xdir = 1;
			Xpul = 1;
			stepperA();//�Ĵ�������λX��ֵ
		}
		else if (temp == 3 || temp == 8) {
			F = F + 2 * (Ji++) + 1;
			Ydir = 1;
			Ypul = 1;
			stepperB();//�Ĵ�������λY��ֵ
		}
		else if (temp == 4 || temp == 5) {
			F = F - 2 * (Ii--) + 1;
			Xdir = 0;
			Xpul = 1;
			stepperA();//�Ĵ�������λX��ֵ
		}
	}
	else {
		if (temp == 1 || temp == 8) {
			F = F + 2 * (Ii++) + 1;
			Xdir = 1;
			Xpul = 1;
			stepperA();//�Ĵ�������λX��ֵ
		}
		else if (temp == 4 || temp == 7) {
			F = F - 2 * (Ji--) + 1;
			Ydir = 0;
			Ypul = 1;
			stepperB();//�Ĵ�������λY��ֵ
		}
		else if (temp == 2 || temp == 5) {
			F = F + 2 * (Ji++) + 1;
			Ydir = 1;
			Ypul = 1;
			stepperB();//�Ĵ�������λY��ֵ
		}
		else if (temp == 3 || temp == 6) {
			F = F - 2 * (Ii--) + 1;
			Xdir = 0;
			Xpul = 1;
			stepperA();//�Ĵ�������λX��ֵ
		}
	}
	Xi = Ii + Xc; //ʵ������
	Yi = Ji + Yc;

}

void startline(int xe, int ye) {//ֱ�߲岹��ʼ��
	//(x��ֹ���꣬y��ֹ )ֵΪ����ֵ*150
	TR0 = 1;
	TR1 = 0;
	X0 = Xi;
	Xe = xe;
	Y0 = Yi; 
	Ye = ye;
	F = 0;
	acc0 = (50000 - spd) / 200;//������ٶ�
	if (xe >= X0) {//�ж�������
		Xdir = 1; n = xe - X0;//����
	}
	else {
		Xdir = 0; n = X0 - xe;//����
	}
	if (ye >= Y0) {
		Ydir = 1; n += ye - Y0;
	}
	else {
		Ydir = 0; n += Y0 - ye;
	}
	TR0 = 1;
	TR1 = 0;
}
void startcir(int xc, int yc, int ang, bit cl) {//Բ���岹��ʼ��
//Բ��x��Բ��y���Ƕȣ�˳ʱ��1��ʱ��0
	TR0 = 0;
	TR1 = 0;
	piflag = 0;//δ�����ο���
	Xc = xc;//Բ��
	Yc = yc;
	I0 = Ii = Xi - xc;//��ʼ��
	J0 = Ji = Yi - yc;
	clcwise = cl;
	F = 0;
	angle = ang;
	acc0 = (50000 - spd) / 200;//������ٶ�
	//�����յ�
	if (!cl)
		Iee =  I0*cos((float)ang * 0.0174533) - J0*sin((float)ang * 0.0174533);

	else
		Iee =  I0*cos((float)ang * 0.0174533) + J0*sin((float)ang * 0.0174533);

	Je = I0*sin((float)ang * 0.0174533) + J0*cos((float)ang * 0.0174533);

	Ir = -Iee;//�ο��㣬���˲ο��㿪����ֹ�б�
	Jr = -Je;
	//��ȡ�յ��J����
	
	JZe=getJ(Iee,Je);
	
	////////////////////////////
	cirendflag = 0;//���ڲ岹Բ��
	TR1 = 1;
	
}

uchar getJ(int irhs,int jrhs) {		//�ж�J����				1

	if (clcwise) {//˳ʱ��								2		4
		if (abs(jrhs) > abs(irhs) || jrhs == -irhs) {//		3
			if (jrhs > 0) return 1;
			else if (jrhs < 0) return 3;
		}
		else if (abs(jrhs) < abs(irhs) || jrhs == irhs) {
			if (irhs < 0)return 2;
			else if (irhs > 0)return 4;
		}
	}
	else {
		if (abs(jrhs) > abs(irhs) || jrhs == irhs) {
			if (jrhs > 0) return 1;
			else if (jrhs < 0) return 3;
		}
		else if (abs(jrhs) < abs(irhs) || jrhs == -irhs) {
			if (irhs < 0)return 2;
			else if (irhs > 0)return 4;
		}
	}
}

bit cirend() {
	if (!JZe) return 1;
	if (!piflag && angle > 250) {
		if ((Ii - I0) + (Ji - J0) <= ((Ir - I0) + (Jr - J0)) >> 1)//û���ο���
			return 0;
		else 
			piflag = 1;
	}

	if (JZe == getJ(Ii,Ji)) {	//�յ�б�����뵱ǰб������ͬ
		if (clcwise) {//˳ʱ��
			switch (JZe) {
			case 1:
				if (Ii - Iee >= 0)return 1;
				else return 0;
			case 2:
				if (Ji - Je >= 0)return 1;
				else return 0;
			case 3:
				if (Ii - Iee <= 0)return 1;
				else return 0;
			case 4:
				if (Ji - Je <= 0)return 1;
				else return 0;
			}
		}
		else {//��ʱ��
			switch (JZe) {
			case 1:
				if (Ii - Iee <= 0)return 1;
				else return 0;
			case 2:
				if (Ji - Je <= 0)return 1;
				else return 0;
			case 3:
				if (Ii - Iee >= 0)return 1;
				else return 0;
			case 4:
				if (Ji - Je >= 0)return 1;
				else return 0;
			}
		}
	}
	else return 0;
}

