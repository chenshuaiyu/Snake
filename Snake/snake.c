#include <reg51.h>
#include <intrins.h>
#include <stdlib.h>
#define uint unsigned int
#define uchar unsigned char
#define MAX_LEN 23		//蛇身最大长度
#define MAX 7			//点阵显示最大值
#define MIN 0			//点阵显示最小值

uchar x[MAX_LEN + 1];	//蛇身数组
uchar y[MAX_LEN + 1];

char xD = 1, yD = 0;	//爬行方向

uchar code num[] = {	//数码管显示数字
    0x3F,	// 0
    0x06,	// 1
    0x5B,	// 2
    0x4F,	// 3
    0x66,	// 4
    0x6D,	// 5
    0x7D,	// 6
    0x07,	// 7
    0x7F,	// 8
    0x6F	// 9
};

uchar n;				//当前蛇身长度
uint score = 0;			//当前得分
uint speed = 70;		//当前爬行速度
uint fail = 0;			//Game Over标记
uint count = 100;		//10ms中断次数
uint shi_score = 0;		//得分-十位
uint ge_score = 0;		//得分-个位

sbit up = P1^1;			//上		
sbit left = P1^2;		//左
sbit right = P1^3;		//右
sbit down = P1^4;		//下

sbit begin = P1^0;		//游戏开始/结束
sbit mode = P1^5; 		//难易模式

sbit shi = P1^6;		//位选-十位
sbit ge = P1^7;			//位选-个位

uint invincible = 0;	//无敌版：碰到墙壁后会从另一侧出现，只有碰到蛇身时，游戏结束

/*************************************************
Name: 随机数函数
Description: 用于生成 0 - 7	内的随机数
*************************************************/
uint random()
{
	return rand() % (MAX + 1 - MIN) + MIN;        	
}

/*************************************************
Name: 制造豆子函数
Description: 用于在屏幕上随机生成豆子
*************************************************/
void makebean()
{
	uint i;
	while(1)
	{
		x[0] = random();
		y[0] = random();
		for(i = 1 ; i < n && !(x[0] == x[i] && y[0] == y[i]) ; i++)
		{	
		}
		if(n == i)
			break;	
	}	
}

/*************************************************
Name: 初始化函数
Description: 用于初始化贪吃蛇程序
*************************************************/
void init()
{	
	uint i;
	TMOD = 0x01;	//模式1  
	TH0 = 0xD8;		//定时时间10ms
	TL0 = 0xF0;		
	ET0 = 1; 		//开定时器0中断
	TR0 = 1; 		//启动定时器0
	EA = 1;			//开总中断

	P0=0x00;
	P1=0xff;
	P2=0x00;

	n = 1 + 3;		//初始化蛇身长度
	score = 0;		//初始化游戏得分
	
	//初始化蛇身位置
	for(i = 1 ; i < n ; i++)
		x[i - 1] = 4 - i;
	for(i = 1 ; i < n ; i++)
		y[i - 1] = 0;

	makebean();		//初始化豆子位置	
}

/*************************************************
Name: 延时函数
Description: 用于延时
*************************************************/
void delay(char n)
{
	char i;
	while(n != 0)
	{
		i = 0xff;
		while (i != 0)
		{
			i--;
		}
		n--;	
	}
}

/*************************************************
Name: 按键检测函数
Description: 用于检测 上下左右、难易模式 按键
*************************************************/
void keyboard()
{
	if(mode == 0)
	{
		delay(120);		//延时消抖
		if(mode == 0)	//调节难易模式
		{
			if(invincible == 1)
			{
				invincible = 0;
				speed = 70;	
			}
			else if(speed == 10)
			{
				invincible = 1;
				speed = 40;
			}
			else
			{
				speed = speed - 30;	
			}	
		}
	}
	if(begin == 0 && left == 0 && yD != -1)		//在游戏状态下，且非右行情况，点击左行按键
	{
		xD = 0;
		yD = 1;
	}
	if(begin == 0 && right == 0 && yD != 1)		//在游戏状态下，且非左行情况，点击右行按键
	{
		xD = 0;
		yD = -1;
	}
	if(begin == 0 && up == 0 && xD != 1)		//在游戏状态下，且非下行情况，点击上行按键
	{
		yD = 0;
		xD = -1;
	}
	if(begin == 0 && down == 0 && xD != -1)	//在游戏状态下，且非上行情况，点击下行按键
	{
		yD = 0;
		xD = 1;
	}
}

/*************************************************
Name: 检测 Game Over 函数
Description: 用于检测游戏是否结束
*************************************************/
void accident()
{
	uint i;		   	
	if(invincible == 0)
	{
		//非无敌版状态下碰到墙壁，游戏失败
		if(x[1] > 7 || y[1] > 7)
			fail = 1;
	}
	else
	{
		//无敌版状态下碰到墙壁后，从另一侧出现
		x[1] = x[1] % 8;
		y[1] = y[1] % 8;	
	}
		
	for(i = 2 ; i < n ; i++)
	{
		//任意状态下碰到蛇身，游戏失败
		if((x[1] == x[i]) & (y[1] == y[i]))
			fail = 1;
	}
}

/*************************************************
Name: 检测贪吃蛇是否吃到豆子函数
Description: 用于检测贪吃蛇是否可以吃到豆子
*************************************************/
uint eat()
{
	uint i, flag = 0;
	if((x[0] == x[1] + xD) & (y[0] == y[1] + yD))		//贪吃蛇是否吃到东西
	{
		flag = 1;
		score++;
		
		//蛇身+1
		n++;
		for(i = n - 1 ; i > 0 ; i--)
		{
			x[i] = x[i - 1];
			y[i] = y[i - 1];
		}
		
		makebean();
		
		if(n == MAX_LEN + 1)		//由于点阵是8×8，故设置蛇身长度达到23时，游戏重新开始
		{
			init();
		}
	}
	return flag;	
}

/*************************************************
Name: 幂函数
Description: 用于计算 2 的幂
*************************************************/
uchar power(uchar n) 
{
	return 1 << n;
}

/*************************************************
Name: 显示函数
Description: 用于显示蛇身位置、豆子位置、游戏得分
*************************************************/
void display()
{
	uint i;

	//点阵显示蛇身位置、豆子位置
	for(i = 0 ; i < n ; i++)
	{
		P2 = power(x[i]);
		P0 = 0xff - power(y[i]);
		keyboard();
		delay(1);
		P2 = 0x00;
		P0 = 0xff;
		P3 = 0x00;
	}

	shi_score = score / 10;
	ge_score = score % 10;

	//数码管显示游戏得分
	shi = 0;
	ge = 1;
	P3 = num[shi_score];
	delay(1);

	shi = 1;
	ge = 0;
	P3 = num[ge_score];
	delay(1);
} 

/*************************************************
Name: 中断服务程序
Description: 用于实现贪吃蛇前进一格
*************************************************/
void timer0() interrupt 1
{
	uint i;
	TH0 = 0xD8;		//10ms
	TL0 = 0xF0; 
	count--;
	
	if (count == 0)		//到达前进时间
	{
		if(begin == 0)	//游戏开始
		{
			if(!eat())	//没有吃到豆子
			{
				//朝运动方向前进一格
				for(i = n - 1 ; i > 1 ; i--)
				{
					x[i] = x[i - 1];
					y[i] = y[i - 1];
				}
				x[1] = x[2] + xD;
				y[1] = y[2] + yD;

				display();
				accident();
			}	
		}
		count = speed;
	}
}

/*************************************************
Name: 主函数
Description: 用于完成贪吃蛇程序主要逻辑
*************************************************/
void main(void)
{
	init();

	while(!fail)
	{
		keyboard();
		display();
	}
}