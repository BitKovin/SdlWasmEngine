// Task_SmokePoint.cpp
#include "TaskSimpleAction.h"
#include "../NpcBase.h"

class Task_SmokePoint : public TaskSimpleAction
{
public:
    Task_SmokePoint()
    {
        animName = "smoking";
        animLoop = false;
        duration = 23.0f;
    }
};

REGISTER_ENTITY(Task_SmokePoint, "task_smoke")