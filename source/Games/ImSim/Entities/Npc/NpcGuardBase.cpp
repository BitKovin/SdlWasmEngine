#include "NpcBase.h"

class NpcGuardBase : public NpcBase
{
public:
	NpcGuardBase();
	~NpcGuardBase();

private:

};

NpcGuardBase::NpcGuardBase() : NpcBase()
{

	isGuard = true;

	modelPath = "GameData/models/npc/guard.glb";

	hostileTags.insert("bandit");

}

NpcGuardBase::~NpcGuardBase()
{
}

REGISTER_ENTITY(NpcGuardBase, "npc_guard")