#include "DisplayManager.h"

#include "Block.h"
#include "BlockManager.h"
#include "StageManager.h"

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

int ab_x,ab_y;	//화면중 블럭이 나타나는 좌표의 절대위치

int gotoxy(int x,int y)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD pos;
	pos.Y=y;
	pos.X=x;
	SetConsoleCursorPosition(hConsole, pos);
	return 0;
}

void SetColor(int color)
{
	static HANDLE std_output_handle=GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(std_output_handle, color);
}

int show_cur_block(int shape,int angle,int x,int y)
{
	int i,j;

	switch(shape)
	{
	case 0:
		SetColor(RED);
		break;
	case 1:
		SetColor(BLUE);
		break;
	case 2:
		SetColor(SKY_BLUE);
		break;
	case 3:
		SetColor(WHITE);
		break;
	case 4:
		SetColor(YELLOW);
		break;
	case 5:
		SetColor(VOILET);
		break;
	case 6:
		SetColor(GREEN);
		break;
	}

	for(i=0;i<4;i++)
	{
		for(j=0;j<4;j++)
		{
			if( (j+y) <0)
				continue;

			if(block[shape][angle][j][i] == 1)
			{
				gotoxy((i+x)*2+ab_x,j+y+ab_y);
				printf("■");

			}
		}
	}
	SetColor(BLACK);
	gotoxy(77,23);
	return 0;
}

int erase_cur_block(int shape,int angle,int x,int y)
{
	int i,j;
	for(i=0;i<4;i++)
	{
		for(j=0;j<4;j++)
		{
			if(block[shape][angle][j][i] == 1)
			{
				gotoxy((i+x)*2+ab_x,j+y+ab_y);
				printf("  ");
				//break;

			}
		}
	}
	return 0;
}

int show_total_block()
{
	int i,j;
	SetColor(DARK_GRAY);
	for(i=0;i<21;i++)
	{
		for(j=0;j<14;j++)
		{
			if(j==0 || j==13 || i==20)		//레벨에 따라 외벽 색이 변함
			{
				SetColor((level %6) +1);

			}else{
				SetColor(DARK_GRAY);
			}
			gotoxy( (j*2)+ab_x, i+ab_y );
			if(total_block[i][j] == 1)
			{
				printf("■");
			}else{
				printf("  ");
			}

		}
	}
	SetColor(BLACK);
	gotoxy(77,23);
	return 0;
}

int show_gameover()
{
	SetColor(RED);
	gotoxy(15,8);
	printf("┏━━━━━━━━━━━━━━━━━━━━━━━━━━┓");
	gotoxy(15,9);
	printf("┃**************************┃");
	gotoxy(15,10);
	printf("┃*        GAME OVER       *┃");
	gotoxy(15,11);
	printf("┃**************************┃");
	gotoxy(15,12);
	printf("┗━━━━━━━━━━━━━━━━━━━━━━━━━━┛");
	fflush(stdin);
	Sleep(1000);

	_getche();
	system("cls");

	return 0;
}

int show_next_block(int shape)
{
	int i,j;
	SetColor((level+1)%6+1);
	for(i=1;i<7;i++)
	{
		gotoxy(33,i);
		for(j=0;j<6;j++)
		{
			if(i==1 || i==6 || j==0 || j==5)
			{
				printf("■");
			}else{
				printf("  ");
			}

		}
	}
	show_cur_block(shape,0,15,1);
	return 0;
}

int show_gamestat()
{
	static int printed_text=0;
	SetColor(GRAY);
	if(printed_text == 0)
	{
		gotoxy(35,7);
		printf("STAGE");

		gotoxy(35,9);
		printf("SCORE");

		gotoxy(35,12);
		printf("LINES");


	}
	gotoxy(41,7);
	printf("%d",level+1);
	gotoxy(35,10);
	printf("%10d",score);
	gotoxy(35,13);
	printf("%10d",stage_data[level].clear_line - lines);
	return 0;
}

int show_logo()
{
	int i,j;
	gotoxy(13,3);
	printf("┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓");
	Sleep(100);
	gotoxy(13,4);
	printf("┃◆◆◆  ◆◆◆  ◆◆◆   ◆◆     ◆   ◆◆◆ ┃");
	Sleep(100);
	gotoxy(13,5);
	printf("┃  ◆    ◆        ◆     ◆ ◆    ◆   ◆     ┃");
	Sleep(100);
	gotoxy(13,6);
	printf("┃  ◆    ◆◆◆    ◆     ◆◆     ◆     ◆   ┃");
	Sleep(100);
	gotoxy(13,7);
	printf("┃  ◆    ◆        ◆     ◆ ◆    ◆       ◆ ┃");
	Sleep(100);
	gotoxy(13,8);
	printf("┃  ◆    ◆◆◆    ◆     ◆  ◆   ◆   ◆◆◆ ┃");
	Sleep(100);
	gotoxy(13,9);
	printf("┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛");

	gotoxy(28,20);
	printf("Please Press Any Key~!");

	for(i=0;i>=0;i++){
		if(i%40==0)
		{


			for(j=0;j<5;j++)
			{
				gotoxy(18,14+j);
				printf("                                                          ");


			}
			show_cur_block(rand()%7,rand()%4,6,14);
			show_cur_block(rand()%7,rand()%4,12,14);
			show_cur_block(rand()%7,rand()%4,19,14);
			show_cur_block(rand()%7,rand()%4,24,14);
		}
		if(_kbhit())
			break;
		Sleep(30);
	}

	_getche();
	system("cls");

	return 0;
}
