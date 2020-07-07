#include"drive.h"

#define FOSC 11059200UL     //System frequency
#define BAUD 9600       //UART baudrate

uchar code table[] = { 0x01,0x03,0x02,0x06,0x04,0x0c,0x08,0x09 };//正

uint spd = 2000;//速度：中断溢出时间
uchar acc0 = 0;//加速度
uint n = 0;//直线插补步数

char rhy0 = 0, rhy1 = 0;//步进电机节拍数记录
int X0 = 0, Y0 = 0, Xe = 0, Ye = 0, Xc = 0, Yc = 0;//起始坐标和终止坐标,圆心坐标
int Xi = 0, Yi = 0;	//当前坐标
int Ii = 0, Ji = 0;//圆弧相对圆心坐标
int I0 = 0, J0 = 0;//圆弧起始点
int Iee = 0, Je = 0;//圆弧终止点
int Ir = 0, Jr = 0;//圆弧参考点
int F = 0;//f为斜率判别式
uchar JZe = 0;//终点的斜区间
int angle = 0;//-360~360,逆时针为正
bit shape = 1;	//1:直线	0：曲线
bit clcwise = 1;	//1：顺时针	0：逆时针
bit sending;//发送中标志位
bit piflag = 0;//1为已过参考点
bit cirendflag = 1;//1为一组圆弧插补已结束

void timer_init()
{
	TMOD = 0x11;//定时器0工作方式1.16位，定时器1工作方式1,16位定时；
	SCON = 0x50;           //串口工作在模式1

	TH0 = (65536 - 50000) >> 8;
	TL0 = (65536 - 50000);
	TH1 = (65536 - 50000) >> 8;
	TL1 = (65536 - 50000);
	TL2 = RCAP2L = (65536 - (FOSC / 32 / BAUD)); //计算定时器重装值
	TH2 = RCAP2H = (65536 - (FOSC / 32 / BAUD)) >> 8;//产生波特率

	T2CON = 0x34;           //Timer2 start run
	TR2 = 1;		//定时器2开始计时
	TR1 = 0;	  //不启动定时器1
	ET1 = 1;	   //定时器1中断使能

	TR0 = 0;	//定时器0不启动
	ET0 = 1;	//定时器中断使能
	ES = 1;         //串行中断允许
	EA = 1;	//全局中断

}


void send(int d, uchar xy)		  //发送2个字节的数据，d待发送数据,xy为坐标标识
{
	//协议：8位:x轴0 y轴1；低7位数据
	uchar temp;//低位
	uchar temp1;//高位，高二位为正负号标识
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
	SBUF = temp; //将数据写入到串口缓冲
	sending = 1;	 //设置发送标志
	while (sending); //等待发送完毕

	SBUF = temp1; //将数据写入到串口缓冲
	sending = 1;	 //设置发送标志
	while (sending); //等待发送完毕
}

void uart(void) interrupt 4	//串口发送中断
{
	if (RI)    //收到数据
		RI = 0;   //清中断请求
	else      //发送完一字节数据
	{
		TI = 0;
		sending = 0;  //清正在发送标志
	}
}

void stepperA() {//A步进电机信号 x轴
	
	A0 = table[rhy0] & 1;//四位分别赋值
	A1 = (table[rhy0] >> 1) & 1;
	A2 = (table[rhy0] >> 2) & 1;
	A3 = (table[rhy0] >> 3) & 1;
	if (Xdir) 
		rhy0 = rhy0 < 7 ? rhy0 + 1 : 0;//节拍清0
	else 
		rhy0 = rhy0 > 0 ? rhy0 - 1 : 7;
}

void stepperB() {//B步进电机信号 y轴
	
	B0 = table[rhy1] & 1;//四位分别赋值
	B1 = (table[rhy1] >> 1) & 1;
	B2 = (table[rhy1] >> 2) & 1;
	B3 = (table[rhy1] >> 3) & 1;
	if (Ydir) 
		rhy1 = rhy1 < 7 ? rhy1 + 1 : 0;
	else 
		rhy1 = rhy1 > 0 ? rhy1 - 1 : 7;
}

void Timer0() interrupt 1  //直线插补
{
	static bit jo = 0;//奇偶次进入中断
	static uint temp = 50000;
	if (n > 200 && temp > spd)
		temp -= acc0;//加速
	if (n <= 200 && temp < 50000)
		temp += acc0;//减速
	if(!(n > 200 && temp > spd)&&!(n <= 200 && temp < 50000))
		temp=spd;//运行中加减档
	TH0 = (65536 - temp) >> 8;
	TL0 = (65536 - temp);//重新装载计时初
	if (jo) {//奇数拍
		if (n > 0) {//终点判别
			line();//直线插补
			n--;
		}
	}
	else {//偶数拍
		if (Xpul) Xpul = 0;	//脉冲下降
		if (Ypul) Ypul = 0;
	}
	jo = !jo;
}

void Timer1() interrupt 3 {//圆弧插补
	static bit jo = 0;
	static uint temp = 50000;
	if (temp > spd && (abs(Ji - J0) + abs(Ii - I0) < 300)) //速度小且离起点近
		temp -= acc0;//加速
	if (temp < 50000 && (abs(Ji - Je) + abs(Ii - Iee) < 200))
		temp += acc0*3;//减速
//	if(!(temp > spd && (abs(Ji - J0) + abs(Ii - I0) < 300))&&!(temp < 50000 && (abs(Ji - Je) + abs(Ii - Iee) < 200)))
//		temp=spd;////运行中加减档
	TH1 = (65536 - temp) >> 8;
	TL1 = (65536 - temp);//重新装载计时初
	if (jo) {//奇数拍
		if (!cirend()) //终点判别
			cir();//圆弧插补
		else cirendflag = 1;//圆弧插补结束
	}
	else {//偶数拍
		if (Xpul) Xpul = 0;	//脉冲下降
		if (Ypul) Ypul = 0;
	}
	jo = !jo;
}

void line() {//一次插补
	if (F >= 0) {
		if (Xdir) {
			Xi++;
			Xpul = 1;//脉冲升高 
			stepperA();//寄存器低四位X赋值
		}
		else {
			Xi--;
			Xpul = 1;//脉冲升高
			stepperA();//寄存器低四位X赋值
		}
		if (Ydir)F -= (Ye - Y0);//判别式
		else F -= (Y0 - Ye);
	}
	else {
		if (Ydir) {
			Yi++;
			Ypul = 1;//脉冲升高 
			stepperB();//寄存器高四位Y赋值
		}
		else {
			Yi--;
			Ypul = 1;//脉冲升高 
			stepperB();//寄存器高四位Y赋值
		}
		if (Xdir)F += (Xe - X0);//判别式
		else F += (X0 - Xe);
	}

}

uint getRNS(int X, int Y) {//获得坐标象限
	if (clcwise) {	//顺时针
		if (X > 0 && Y >= 0)return 1;	//第一象限顺时针
		else if (X <= 0 && Y > 0)return 2;//二顺
		else if (X < 0 && Y <= 0)return 3;//三顺
		else return 4;//四顺	
	}
	else {	//逆时针
		if (X >= 0 && Y > 0)return 5;//第一象限逆时针
		else if (X < 0 && Y >= 0)return 6;//二逆
		else if (X <= 0 && Y < 0)return 7;//三逆
		else return 8;//四逆
	}
}
void cir(void) {//一次圆弧插补
	unsigned int temp;
	temp = getRNS(Ii, Ji);//当前的象限和方向
	if (F >= 0) {
		if (temp == 1 || temp == 6) {
			F = F - 2 * (Ji--) + 1;//判别式
			Ydir = 0;
			Ypul = 1;
			stepperB();//寄存器高四位Y赋值
		}
		else if (temp == 2 || temp == 7) {
			F = F + 2 * (Ii++) + 1;
			Xdir = 1;
			Xpul = 1;
			stepperA();//寄存器低四位X赋值
		}
		else if (temp == 3 || temp == 8) {
			F = F + 2 * (Ji++) + 1;
			Ydir = 1;
			Ypul = 1;
			stepperB();//寄存器高四位Y赋值
		}
		else if (temp == 4 || temp == 5) {
			F = F - 2 * (Ii--) + 1;
			Xdir = 0;
			Xpul = 1;
			stepperA();//寄存器低四位X赋值
		}
	}
	else {
		if (temp == 1 || temp == 8) {
			F = F + 2 * (Ii++) + 1;
			Xdir = 1;
			Xpul = 1;
			stepperA();//寄存器低四位X赋值
		}
		else if (temp == 4 || temp == 7) {
			F = F - 2 * (Ji--) + 1;
			Ydir = 0;
			Ypul = 1;
			stepperB();//寄存器高四位Y赋值
		}
		else if (temp == 2 || temp == 5) {
			F = F + 2 * (Ji++) + 1;
			Ydir = 1;
			Ypul = 1;
			stepperB();//寄存器高四位Y赋值
		}
		else if (temp == 3 || temp == 6) {
			F = F - 2 * (Ii--) + 1;
			Xdir = 0;
			Xpul = 1;
			stepperA();//寄存器低四位X赋值
		}
	}
	Xi = Ii + Xc; //实际坐标
	Yi = Ji + Yc;

}

void startline(int xe, int ye) {//直线插补初始化
	//(x终止坐标，y终止 )值为毫米值*150
	TR0 = 1;
	TR1 = 0;
	X0 = Xi;
	Xe = xe;
	Y0 = Yi; 
	Ye = ye;
	F = 0;
	acc0 = (50000 - spd) / 200;//计算加速度
	if (xe >= X0) {//判断正反向
		Xdir = 1; n = xe - X0;//正向
	}
	else {
		Xdir = 0; n = X0 - xe;//反向
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
void startcir(int xc, int yc, int ang, bit cl) {//圆弧插补初始化
//圆心x，圆心y，角度，顺时针1逆时针0
	TR0 = 0;
	TR1 = 0;
	piflag = 0;//未经过参考点
	Xc = xc;//圆心
	Yc = yc;
	I0 = Ii = Xi - xc;//起始点
	J0 = Ji = Yi - yc;
	clcwise = cl;
	F = 0;
	angle = ang;
	acc0 = (50000 - spd) / 200;//计算加速度
	//计算终点
	if (!cl)
		Iee =  I0*cos((float)ang * 0.0174533) - J0*sin((float)ang * 0.0174533);

	else
		Iee =  I0*cos((float)ang * 0.0174533) + J0*sin((float)ang * 0.0174533);

	Je = I0*sin((float)ang * 0.0174533) + J0*cos((float)ang * 0.0174533);

	Ir = -Iee;//参考点，过了参考点开启终止判别
	Jr = -Je;
	//获取终点的J区间
	
	JZe=getJ(Iee,Je);
	
	////////////////////////////
	cirendflag = 0;//正在插补圆弧
	TR1 = 1;
	
}

uchar getJ(int irhs,int jrhs) {		//判断J区间				1

	if (clcwise) {//顺时针								2		4
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
		if ((Ii - I0) + (Ji - J0) <= ((Ir - I0) + (Jr - J0)) >> 1)//没过参考点
			return 0;
		else 
			piflag = 1;
	}

	if (JZe == getJ(Ii,Ji)) {	//终点斜区间与当前斜区间相同
		if (clcwise) {//顺时针
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
		else {//逆时针
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

