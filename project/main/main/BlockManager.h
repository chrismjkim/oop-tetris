#pragma once

extern char total_block[21][14];

int make_new_block();	//return값으로 block의 모양번호를 알려줌
int strike_check(int shape,int angle,int x,int y);	//블럭이 화면 맨 아래에 부닥쳤는지 검사 부닥치면 1을리턴 아니면 0리턴
int merge_block(int shape,int angle,int x,int y);	//블럭이 바닥에 닿았을때 진행중인 블럭과 쌓아진 블럭을 합침
int move_block(int* shape,int* angle,int* x,int* y,int* next_shape);	//게임오버는 1을리턴 바닥에 블럭이 닿으면 2를 리턴
int rotate_block(int shape,int* angle,int* x,int* y);
int check_full_line();
