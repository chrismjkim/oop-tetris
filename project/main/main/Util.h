#pragma once

class Util {
private:
	static inline int displayX = 13;
	static inline int displayY = 7;

public:
	static const inline int MAXBOARDROW = 20;
	static const inline int MAXBOARDCOL = 10;
	static void setP1BoardX(int x) {
		displayX = x;
	}
	static void setP1BoardY(int y) {
		displayY = y;
	}
	static int getDisplayX() {
		return displayX;
	}
	static int getDisplayY() {
		return displayY;
	}
};
