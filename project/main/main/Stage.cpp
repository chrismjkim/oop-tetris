#include "Stage.h"


Stage::Stage(int speed, int stickRate, int clearLine)
: speed(speed), stickRate(stickRate), clearLine(clearLine) {

}

int Stage::getStickRate() {
	return stickRate;
}
