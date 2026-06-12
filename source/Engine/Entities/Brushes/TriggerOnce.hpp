#pragma once

#include "TriggerBase.hpp"

class TriggerOnce : public TriggerBase
{
public:

	TriggerOnce() : TriggerBase()
	{
		ClassName = "triggerOnce";
	}
	
	void DoEnterAction() override
	{
		TriggerBase::DoEnterAction();
		Destroy();
	}

private:

};