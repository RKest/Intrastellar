#pragma once
#include "_config.h"
#include "Core/mesh.h"
#include "Core/display.h"
#include "Core/helpers.h"

class WeaponsManager
{
public:
    WeaponsManager(helpers::Core &core, TexturedMeshParams &iconMeshParams, UntexturedMeshParams &overlayMeshParams);
    ~WeaponsManager();
private:
    Display &_display;
    TexturedInstancedMesh _iconMesh;
    UntexturedMesh _overlayMesh;

};