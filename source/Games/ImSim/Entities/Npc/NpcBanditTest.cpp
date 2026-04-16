#include "NpcBase.h"

class NpcBanditBase : public NpcBase
{
public:
	NpcBanditBase();


private:

};

NpcBanditBase::NpcBanditBase()
{
	isGuard = true;
	fractionTag = "bandit";
	hostileTags.insert("citizen");

	NpcBase();
}

REGISTER_ENTITY(NpcBanditBase, "npc_bandit")