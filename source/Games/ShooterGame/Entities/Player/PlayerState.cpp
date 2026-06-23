#include "PlayerState.h"
#include "Player.hpp"
#include "Weapons/WeaponFirearm.h"

PlayerState PlayerState::FromPlayerPtr(Player* player)
{
    PlayerState state;

    state.position      = player->Position;
    state.rotation      = player->Rotation;
    state.cameraRotation = player->cameraRotation;
    state.velocity      = player->controller.GetVelocity();
    state.playerHeight  = player->controller.isCrouched
        ? player->controller.crouchHeight
        : player->controller.height;

    if (player->currentWeapon)
    {
        WeaponFirearm* fw = dynamic_cast<WeaponFirearm*>(player->currentWeapon);
        state.weaponRModelPath = fw ? fw->params.modelPathTp
                                    : player->currentWeapon->thirdPersonModelPath;
        state.weaponRAkimbo = fw ? fw->akimbo : false;
    }

    if (player->currentOffhandWeapon)
    {
        WeaponFirearm* fw = dynamic_cast<WeaponFirearm*>(player->currentOffhandWeapon);
        state.weaponLModelPath = fw ? fw->params.modelPathTp
                                    : player->currentOffhandWeapon->thirdPersonModelPath;
    }

    return state;
}
