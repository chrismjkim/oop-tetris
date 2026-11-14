#pragma once

enum {
		BLACK,      /*  0 : 까망 */
		DARK_BLUE,    /*  1 : 어두운 파랑 */
		DARK_GREEN,    /*  2 : 어두운 초록 */
		DARK_SKY_BLUE,  /*  3 : 어두운 하늘 */
		DARK_RED,    /*  4 : 어두운 빨강 */
		DARK_VOILET,  /*  5 : 어두운 보라 */
		DARK_YELLOW,  /*  6 : 어두운 노랑 */
		GRAY,      /*  7 : 회색 */
		DARK_GRAY,    /*  8 : 어두운 회색 */
		BLUE,      /*  9 : 파랑 */
		GREEN,      /* 10 : 초록 */
		SKY_BLUE,    /* 11 : 하늘 */
		RED,      /* 12 : 빨강 */
		VOILET,      /* 13 : 보라 */
		YELLOW,      /* 14 : 노랑 */
		WHITE,      /* 15 : 하양 */
};

extern int ab_x, ab_y;

int gotoxy(int x,int y);	//커서옮기기
void SetColor(int color);	//색표현
int show_cur_block(int shape,int angle,int x,int y);	//진행중인 블럭을 화면에 표시한다
int erase_cur_block(int shape,int angle,int x,int y);	//블럭 진행의 잔상을 지우기 위한 함수
int show_total_block();	//쌓여져있는 블럭을 화면에 표시한다.
int show_gameover();
int show_next_block(int shape);
int show_gamestat();
int show_logo();
