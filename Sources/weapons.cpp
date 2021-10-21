#include "weapons.h"

WeaponsManager::WeaponsManager(helpers::Core &core, UntexturedMeshParams &iconMeshParams, UntexturedMeshParams &overlayMeshParams)
    : _display(core.display), _iconMesh(iconMeshParams, WEAPONS_NO_WEAPONS), _overlayMesh(overlayMeshParams)
{
    
}

WeaponsManager::~WeaponsManager()
{

}