#pragma once
#include <Entities/Brushes/AreaBase.hpp>

class RestrictedArea : public AreaBase
{
public:
	RestrictedArea();

	void FromData(EntityData data);

	int RestrictionLevel = 1;

private:

};

