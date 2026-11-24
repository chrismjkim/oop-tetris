#pragma once
class Stage
{
private: 
	int speed = 40;
	int stickRate = 20;
	int clearLine = 20;
public:
	Stage() = default;
	Stage(int speed, int stickRate, int clearLine);
	int getSpeed() const;
	int getStickRate() const;
};

