#pragma once
#include "Events.h"
#include <string>

using namespace std;

class IEffect
{
private:

public:
	virtual ~IEffect() = default;
	virtual bool execute() = 0;
	virtual string getEffectDesc() const = 0;

};
