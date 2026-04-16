#include "NpcBase.h"

class NpcCivilianBase : public NpcBase
{
public:
	NpcCivilianBase();


private:

};

NpcCivilianBase::NpcCivilianBase() : NpcBase()
{
	isGuard = false;
}

REGISTER_ENTITY(NpcCivilianBase,"npc_civilian")