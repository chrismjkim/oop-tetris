#pragma once


#include <string>
#include <memory>

#include "ITrigger.h"
#include "IEffect.h"

using namespace std;

class Augment
{
private: 
	string name;
	unique_ptr<ITrigger> trigger;
	unique_ptr<IEffect> effect;
	
public:
	Augment(string name, unique_ptr<ITrigger> t, unique_ptr<IEffect> e) 
		: name(move(name)), trigger(move(t)), effect(move(e)) {}

	void onEvent(Events e) {
		if (trigger != nullptr && trigger->isTriggered(e)) {
			effect->execute();
		}
	}
};

