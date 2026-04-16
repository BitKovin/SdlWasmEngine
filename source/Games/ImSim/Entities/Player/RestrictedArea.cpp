#include "RestrictedArea.h"

RestrictedArea::RestrictedArea() : AreaBase()
{

	DefaultBrushGroup = BodyType::Area1;

}

void RestrictedArea::FromData(EntityData data)
{
	RestrictionLevel = data.GetPropertyFloat("RestrictionLevel", RestrictionLevel);
}

REGISTER_ENTITY(RestrictedArea, "area_restricted")