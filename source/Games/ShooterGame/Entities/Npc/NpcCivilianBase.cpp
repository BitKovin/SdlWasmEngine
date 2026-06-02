#include "NpcBase.h"

class NpcCivilianBase : public NpcBase
{
public:
	NpcCivilianBase();


private:

};

NpcCivilianBase::NpcCivilianBase() : NpcBase()
{
	npcType = NpcType::Civilian;
}

REGISTER_ENTITY(NpcCivilianBase,"npc_civilian")