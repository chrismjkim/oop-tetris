#pragma once

enum class BlockType
{
	I, // 일자형
	O, // 2x2 정사각형
	T, // T자형
	J, // J자형, L자형과는 좌우반전 
	L, // L자형, J자형과는 좌우반전
	S, // S자형, Z와 좌우반전
	Z, // Z자형, S와 좌우반전
};

