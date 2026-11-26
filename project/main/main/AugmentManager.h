#pragma once
#include <vector>
#include "Augment.h"

class AugmentManager
{
private: 
	vector<Augment> currentAug;
public:
	void addAug();
	void clearAug();
	void checkEvent();
};

