#include"key_lcd.h"
#include"drive.h"
extern uint n;
uchar starnum = 0;	//判断按了第几次*
uchar numcheck = 0;//判断数字按过几次

uchar KeyScan()  //键盘扫描函数，使用行列反转扫描法
{
	unsigned char cord_h, cord_l;//行列值
	P3 = 0x0f;            //行线输出全为0
	cord_h = P3 & 0x0f;     //读入行值
	if (cord_h != 0x0f)    //先检测有无按键按下
	{
		delay(10);        //去抖
		if ((P3 & 0x0f) != 0x0f)
		{
			cord_h = P3 & 0x0f;  //读入行值
			P3 = cord_h | 0xf0;  //输出当前行值
			cord_l = P3 & 0xf0;  //读入列值
			while ((P3 & 0xf0) != 0xf0);//等待松开并输出
			return(cord_h + cord_l);//键盘最后组合码值
		}
	}
	return(0xff);  //返回该值
}



uchar KeyValue()//编码
{
	switch (KeyScan())
	{
	case 0xee:return '7';  //按下相应的键显示相对应的码值
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


void lcd_init()//初始化函数体
{
	EN = 0;
	delay(5);
	lcd_wcom(0x38);//设置16*2显示，5*7点阵，8位数据接口
	delay(5);
	lcd_wcom(0x0c);//设置开显示，不显示光标
	delay(5);
	lcd_wcom(0x06);//写一个字符时，整屏右移
	delay(5);
	lcd_wcom(0x01);//显示清零
	lcd_line();//显示直线界面

}

void lcd_wcom(uchar com)//写命令的函数体
{
	while (lcd_busy()); //忙则等待
	RS = 0;
	RW = 0;
	EN = 1;
	P0 = com;
	_nop_();
	EN = 0;
}

void lcd_wdata(uchar date)//写数据的函数体
{
	while (lcd_busy()); //忙则等待
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
	lcd_wdata('X');//终点
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
	lcd_wdata('A');//角度
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

	static bit keyin = 0;//不可输入键盘
	uchar value = 0;	//键盘值
	extern int angle;
	extern uint spd;
	extern bit shape;
	extern bit clcwise;
	extern bit cirendflag;
	extern int Xc, Yc, Xe, Ye;
	extern uchar code numtable[];

	value = KeyValue();//扫描键盘
	if (value == '#') {//急停与开始
		if(n){//急停
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
	else if (!n && value == 'a') {//自动图形
		pop();
	}
	else if (value == 'b') {//加档
		if(spd > 5000)
			spd = spd -  5000;
		if (shape)//直线
			lcd_wcom(0x80 + 14);
		else //圆弧
			lcd_wcom(0x80 + 5);
		lcd_wdata(numtable[10 - spd / 5000]);
	}
	else if (value == 'c') {//减挡
		if(spd < 50000)
			spd = spd + 5000;
		if (shape)//直线
			lcd_wcom(0x80 + 14);
		else //圆弧
			lcd_wcom(0x80 + 5);
		lcd_wdata(numtable[10 - spd / 5000]);
	}
	else if(value=='d'){//切换界面
		
		if(shape){
			shape=0;
			lcd_cir();
		}
		else {
			shape=1;
			lcd_line();
		}
	}

	if (shape&& (!n || cirendflag)) {//直线界面
		if (value == '*') {//切换光标位置
			numcheck = 0;
			starnum++;//功能键按下次数记录
			switch (starnum) {
			case 1:
				lcd_wcom(0x80 + 2);//第1次按下光标闪烁定位到角度位置
				lcd_wcom(0x0f); //光标开始闪烁
				keyin = 1;//可以输入
				break;
			case 2:
				lcd_wcom(0x80 + 7);//第2次按下光标闪烁定位到速度位置
				break;
			case 3:
				lcd_wcom(0x80 + 14);//第3次按下光标闪烁定位到顺逆时针位置
				break;
			default:
				starnum = 0;//记录按键数清零
				lcd_wcom(0x0c); //取消光标闪烁
				keyin = 0;//不可输入
			}
		}
		else if (keyin&&value >= '0'&&value <= '9') {//输入数字
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
				starnum = 0;//记录按键数清零
				lcd_wcom(0x0c); //取消光标闪烁
				keyin = 0;//不可输入
			}
		}
	}
	else if( !shape && (!n || cirendflag)) {//圆弧界面
		if (value == '*') {
			numcheck = 0;
			starnum++;//功能键按下次数记录
			switch (starnum) {
			case 1:
				lcd_wcom(0x80 + 1);//第1次按下光标闪烁定位到角度位置
				lcd_wcom(0x0f); //光标开始闪烁
				keyin = 1;//可以输入
				break;
			case 2:
				lcd_wcom(0x80 + 5);//第2次按下光标闪烁定位到速度位置
				break;
			case 3:
				lcd_wcom(0x80 + 7);//第3次按下光标闪烁定位到顺逆时针位置
				break;
			case 4:
				lcd_wcom(0x80 + 10);//第4次按下光标闪烁定位到圆心X位置
				break;
			case 5:
				lcd_wcom(0x80 + 14);//第三次按下光标闪烁定位到圆心Y位置
				break;
			default:
				starnum = 0;//记录按键数清零
				lcd_wcom(0x0c); //取消光标闪烁
				keyin = 0;//不可输入
			}
		}
		else if (keyin&&value >= '0'&&value <= '9') {
			switch (starnum) {
			case 1:		//输入角度
				if (numcheck == 0) {
					lcd_wcom(0x80 + 1);//清此处屏幕
					lcd_wdata(0x20);
					lcd_wdata(0x20);
					lcd_wdata(0x20);

					angle = (value - '0');
					lcd_wcom(0x80 + 1);//按1下
					lcd_wdata(value);
					lcd_wcom(0x80 + 2);
					numcheck++;
				}
				else if (numcheck == 1) {
					angle = angle * 10 + (value - '0');//按2下
					lcd_wcom(0x80 + 2);
					lcd_wdata(value);
					lcd_wcom(0x80 + 3);
					numcheck++;
				}
				else if (numcheck == 2) {
					angle = angle * 10 + (value - '0');//按3下
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
				starnum = 0;//记录按键数清零
				lcd_wcom(0x0c); //取消光标闪烁
				keyin = 0;//不可输入
			}
		}
	}
		value = 0xff;
}


void innum(char wz, char vl,char xy) {//几位，显示位置
	if (!numcheck) {//如果没按过数字
		numcheck++;
		lcd_wcom(0x80 + wz);//清此处屏幕
		lcd_wdata(0x20);
		lcd_wdata(0x20);

		lcd_wcom(0x80 + wz);
		lcd_wdata(vl);
		lcd_wcom(0x80 + wz + 1);
		switch(xy){
			case 1:Xe=(vl - '0') * 150;break;//个位
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
			case 1:Xe=Xe*10+(vl - '0') * 150;break;//十位
			case 2:Ye=Ye*10+(vl - '0') * 150;break;
			case 3:Xc=Xc*10+(vl - '0') * 150;break;
			case 4:Yc=Yc*10+(vl - '0') * 150;break;
		}
	}
}

