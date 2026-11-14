#include "BlockManager.h"

#include "Block.h"
#include "DisplayManager.h"
#include "StageManager.h"

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

char total_block[21][14];		//화면에 표시되는 블럭들

int make_new_block()
{
	int shape;
	int i;
	i=rand()%100;
	if(i<=stage_data[level].stick_rate)		//막대기 나올확률 계산
		return 0;							//막대기 모양으로 리턴

	shape = (rand()%6)+1;		//shape에는 1~6의 값이 들어감
	show_next_block(shape);
	return shape;
}

int strike_check(int shape,int angle,int x,int y)
{
	int i,j;
	int block_dat;

	for(i=0;i<4;i++)
	{
		for(j=0;j<4;j++)
		{
			if(  ((x+j) == 0)  || ((x+j) == 13) )
				block_dat = 1;
			else
				block_dat = total_block[y+i][x+j];


			if((block_dat == 1) && (block[shape][angle][i][j] == 1))																							//좌측벽의 좌표를 빼기위함
			{
				return 1;
			}
		}
	}
	return 0;
}

int merge_block(int shape,int angle,int x,int y)
{
	int i,j;
	for(i=0;i<4;i++)
	{
		for(j=0;j<4;j++)
		{
			total_block[y+i][x+j] |=  block[shape][angle][i][j];
		}
	}
	check_full_line();
	show_total_block();

	return 0;
}

int move_block(int* shape,int* angle,int* x,int* y,int* next_shape)
{
	erase_cur_block(*shape,*angle,*x,*y);

	(*y)++;	//블럭을 한칸 아래로 내림
	if(strike_check(*shape,*angle,*x,*y) == 1)
	{
		if(*y<0)	//게임오버
		{

			return 1;
		}
		(*y)--;
		merge_block(*shape,*angle,*x,*y);
		*shape = *next_shape;
		*next_shape = make_new_block();

		block_start(*shape,angle,x,y);	//angle,x,y는 포인터임
		show_next_block(*next_shape);
		return 2;
	}
	return 0;
}

int rotate_block(int shape,int* angle,int* x,int* y)
{
	return 0;
}

int check_full_line()
{
	int i,j,k;
	for(i=0;i<20;i++)
	{
		for(j=1;j<13;j++)
		{
			if(total_block[i][j] == 0)
				break;
		}
		if(j == 13)	//한줄이 다 채워졌음
		{
			lines++;
			show_total_block();
			SetColor(BLUE);
			gotoxy(1*2+ab_x,i+ab_y);
			for(j=1;j<13;j++)
			{
				printf("□");
				Sleep(10);
			}
			gotoxy(1*2+ab_x,i+ab_y);
			for(j=1;j<13;j++)
			{
				printf("  ");
				Sleep(10);
			}

			for(k=i;k>0;k--)
			{
				for(j=1;j<13;j++)
					total_block[k][j] = total_block[k-1][j];
			}
			for(j=1;j<13;j++)
				total_block[0][j] = 0;
			score+= 100+(level*10) + (rand()%10);
			show_gamestat();
		}
	}
	return 0;
}
