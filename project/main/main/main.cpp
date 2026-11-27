#include <stdlib.h>
#include <time.h>
#include "GameManager.h"

int main() {

	srand(time(NULL));
	GameManager gm;
	gm.run();

	return 0;
}