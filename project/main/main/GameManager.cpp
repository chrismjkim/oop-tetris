#include "GameManager.h"

#include "Block.h"
#include "BlockManager.h"
#include "DisplayManager.h"
#include "InputManager.h"
#include "StageManager.h"

#include <conio.h>
#include <stdlib.h>
#include <time.h>
#include <Windows.h>

namespace {

int block_shape,block_angle,block_x,block_y;
int next_block_shape;

int init()
{
	int i,j;

	srand((unsigned)time(NULL));


	for(i=0;i<20;i++)
	{
		for(j=0;j<14;j++)
		{
			if((j==0) || (j==13))
			{
				total_block[i][j]=1;
			}else{
				total_block[i][j]=0;
			}
		}
	}

	for(j=0;j<14;j++)			//화면의 제일 밑의 줄은 1로 채운다.
		total_block[20][j]=1;

	//전역변수 초기화
	level=0;
	lines=0;
	ab_x = 5;
	ab_y = 1;

	stage_data[0].speed=40;
	stage_data[0].stick_rate=20;
	stage_data[0].clear_line=20;
	stage_data[1].speed=38;
	stage_data[1].stick_rate=18;
	stage_data[1].clear_line=20;
	stage_data[2].speed=35;
	stage_data[2].stick_rate=18;
	stage_data[2].clear_line=20;
	stage_data[3].speed=30;
	stage_data[3].stick_rate=17;
	stage_data[3].clear_line=20;
	stage_data[4].speed=25;
	stage_data[4].stick_rate=16;
	stage_data[4].clear_line=20;
	stage_data[5].speed=20;
	stage_data[5].stick_rate=14;
	stage_data[5].clear_line=20;
	stage_data[6].speed=15;
	stage_data[6].stick_rate=14;
	stage_data[6].clear_line=20;
	stage_data[7].speed=10;
	stage_data[7].stick_rate=13;
	stage_data[7].clear_line=20;
	stage_data[8].speed=6;
	stage_data[8].stick_rate=12;
	stage_data[8].clear_line=20;
	stage_data[9].speed=4;
	stage_data[9].stick_rate=11;
	stage_data[9].clear_line=99999;
	return 0;
}

} // namespace

int run_game()
{
	int i;
	int is_gameover=0;
	char keytemp;
	init();
	show_logo();
	while(1)
	{

		input_data();
		show_total_block();
		block_shape = make_new_block();
		next_block_shape = make_new_block();
		show_next_block(next_block_shape);
		block_start(block_shape,&block_angle,&block_x,&block_y);
		show_gamestat();
		for(i=1;1;i++)
		{
			if(_kbhit())
			{
				keytemp = _getche();
				if(keytemp == EXT_KEY)
				{
					keytemp = _getche();
					switch(keytemp)
					{
					case KEY_UP:		//회전하기

						if(strike_check(block_shape,(block_angle+1)%4,block_x,block_y) == 0)
						{
							erase_cur_block(block_shape,block_angle,block_x,block_y);
							block_angle = (block_angle+1)%4;
							show_cur_block(block_shape,block_angle,block_x,block_y);
						}
						break;
					case KEY_LEFT:		//왼쪽으로 이동
						if(block_x>1)
						{
							erase_cur_block(block_shape,block_angle,block_x,block_y);
							block_x--;
							if(strike_check(block_shape,block_angle,block_x,block_y) == 1)
								block_x++;

							show_cur_block(block_shape,block_angle,block_x,block_y);
						}
						break;
					case KEY_RIGHT:		//오른쪽으로 이동

						if(block_x<14)
						{
							erase_cur_block(block_shape,block_angle,block_x,block_y);
							block_x++;
							if(strike_check(block_shape,block_angle,block_x,block_y) == 1)
								block_x--;
							show_cur_block(block_shape,block_angle,block_x,block_y);
						}
						break;
					case KEY_DOWN:		//아래로 이동
						is_gameover = move_block(&block_shape,&block_angle,&block_x,&block_y,&next_block_shape);
						show_cur_block(block_shape,block_angle,block_x,block_y);
						break;
					}
				}
				if(keytemp == 32 )	//스페이스바를 눌렀을때
				{
					while(is_gameover == 0)
					{
						is_gameover = move_block(&block_shape,&block_angle,&block_x,&block_y,&next_block_shape);
					}
					show_cur_block(block_shape,block_angle,block_x,block_y);
				}
			}
			if(i%stage_data[level].speed == 0)
			{
				is_gameover = move_block(&block_shape,&block_angle,&block_x,&block_y,&next_block_shape);

				show_cur_block(block_shape,block_angle,block_x,block_y);
			}

			if(stage_data[level].clear_line == lines)	//클리어 스테이지
			{
				level++;
				lines = 0;
			}
			if(is_gameover == 1)
			{
				show_gameover();
				SetColor(GRAY);
				break;
			}

			gotoxy(77,23);
			Sleep(15);
			gotoxy(77,23);
		}
		init();
	}
	return 0;
}
