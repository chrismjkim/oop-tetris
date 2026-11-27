#pragma once
#include <string>
#include "Events.h"

using namespace std;

class ITrigger
{
public:
	virtual	~ITrigger() = default;
	virtual bool isTriggered(Events e) = 0;
	virtual string getTriggerDesc() const = 0;

private:

};

