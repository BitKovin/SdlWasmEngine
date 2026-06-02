#include "NpcBase.h"

class NpcBanditBase : public NpcBase
{
public:
	NpcBanditBase();


private:

};

NpcBanditBase::NpcBanditBase()
{
	npcType = NpcType::Guard;
	fractionTag = "bandit";
	hostileTags.insert("citizen");

	NpcBase();
}

REGISTER_ENTITY(NpcBanditBase, "npc_bandit")