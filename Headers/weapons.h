#pragma once
#include "_config.h"

#include "Ext/stb_image.h"
#include "Core/shader.h"
#include "Core/mesh.h"
#include "Core/display.h"
#include "Core/helpers.h"
#include "Core/transform.h"
#include "Core/bounding_box.h"
#include "Core/stats.h"

#include <string>

class WeaponsManager
{
public:
    WeaponsManager(helpers::Core &core, const TexturedMeshParams &iconMeshParams, const UntexturedMeshParams &overlayMeshParams);

    void Draw();

    ~WeaponsManager();
private:
    enum Weapons
    {
        BLASTER,
        ROCKET_LAUNCHER,
        LASER
    };


    Display &_display;
    Timer &_timer;
    PlayerStats &_pcStats;
    TexturedInstancedMesh _iconMesh;
    UntexturedMesh _overlayMesh;

    Shader _overlayShader   {"./Shaders/Overlay"    };
    Shader _weaponIconShader{"./Shaders/WeaponIcon" };

    GLuint          _textures               [WEAPONS_NO_WEAPONS];
    glm::mat4       _instanceTransforms     [WEAPONS_NO_WEAPONS];
    glm::mat4       _baseInstanceTransforms [WEAPONS_NO_WEAPONS];
    ReqBoundingBox  _boundingBoxes          [WEAPONS_NO_WEAPONS];
    PlayerStats     _weaponStatAltarations  [WEAPONS_NO_WEAPONS];

    uiUni _selectedWeaponIndexUni   {"chosenWeaponInx", 0};
    ftUni _overlayAlpthaUni         {"overlayAlpha", 0.0f};
    ui _weaponTransitionClockId{};
    db _scaledTransitionTime = WEAPONS_OVERLAY_TRANSITION_TIME;
    bool _isWeaopnsTabVisible{};
    bool _isWeaponsFullyDrawn{};
    bool _isLBMPressed{};
    
    const ft _iconRealestate = WEAPONS_ICON_DIMS + 1.0f;
    const glm::mat4 _projection                 = glm::ortho(0.0f, static_cast<ft>(SCREEN_WIDTH), 0.0f, static_cast<ft>(SCREEN_HEIGHT));
    const glm::mat4 _inverseFlippedProjection   = glm::inverse(glm::ortho(0.0f, static_cast<ft>(SCREEN_WIDTH), static_cast<ft>(SCREEN_HEIGHT), 0.0f));
};