#include "TaskPairedAction.h"
#include "../NpcBase.h"

class Task_ChatPoint : public TaskPairedAction
{
public:
    Task_ChatPoint()
    {
        anim = "chatting";
        duration = 30.0f;
    }
};

REGISTER_ENTITY(Task_ChatPoint, "task_chat")