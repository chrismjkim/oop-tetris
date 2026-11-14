#pragma once

struct STAGE{		//각 스테이지마다의 난이도 설정
	int	speed;	//숫자가 낮을수록 속도가 빠르다
	int stick_rate;		//막대가 나오는 확률 0~99 , 99면 막대기만 나옴
	int clear_line;
};
extern int level;
extern int score;
extern int lines;
extern STAGE stage_data[10];
