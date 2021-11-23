#pragma once
#include "_config.h"

#include "Core/shader.h"
#include "Core/mesh.h"
#include "Core/helpers.h"
#include "Core/transform.h"
#include "Core/bounding_box.h"
#include "Core/stats.h"
#include "Core/texture.h"
#include "Core/camera.h"

#include <string>
#include <algorithm>
#include <cmath>
#include <type_traits>

#include "enemy.h"
#include "weapon-types.h"

using samplerArray_t = std::array<uiUni, WEAPONS_NO_WEAPONS>;

namespace arr_ini
{
    inline uiUni initSmaplerUni(ui i){ return uiUni(std::string("samps[" + std::to_string(i) + "]"), i); }
    template <size_t ...I>
    inline samplerArray_t makeSampArr(std::index_sequence<I...>){ return samplerArray_t {{ initSmaplerUni(I)... }}; }
}

class WeaponsManager
{
public:
    WeaponsManager(const TexturedMeshParams &iconMeshParams, const UntexturedMeshParams &overlayMeshParams, 
        const UntexturedMeshParams &blasterProjParams, const UntexturedMeshParams &rocketMeshParams);

    void Draw();
    void Update(const glm::mat4 &pcModel, const std::vector<glm::mat4> &enemyInstanceTransforms);
    void Reset();

private:
    friend class Weapon;
    friend struct WeaponBehaviour;
    friend struct BlasterBehaviour;
    friend struct RocketBehaviour;
    friend struct LaserBehaviour;

    TexturedInstancedMesh       _iconMesh;
    UntexturedMesh              _overlayMesh;

    UntexturedInstancedMesh     _blasterProjMesh;
    UntexturedInstancedMesh     _rocketProjMesh;
    UntexturedInstancedMesh     _laserProjMesh;

    BlasterBehaviour            _blasterBehaviour;
    RocketBehaviour             _rocketBehaviour;
    LaserBehaviour              _laserBehaviour;

    PlayerStats                 _blasterStatAltarations;
    PlayerStats                 _rocketStatAltarations;
    PlayerStats                 _laserStatAltarations;

    Shader                      _overlayShader          { "./Shaders/Overlay"    };
    Shader                      _weaponIconShader       { "./Shaders/WeaponIcon" };
    Shader                      _projShader             { "./Shaders/Projectile" };
    Shader                      _bezierProjShader       { "./Shaders/Bezier"     };    
    Texture                     _weaponTextures         {WEAPONS_NO_WEAPONS};

    glm::mat4                   _instanceTransforms     [WEAPONS_NO_WEAPONS];
    glm::mat4                   _baseInstanceTransforms [WEAPONS_NO_WEAPONS];
    ReqBoundingBox              _boundingBoxes          [WEAPONS_NO_WEAPONS];
    std::unique_ptr<Weapon>     _weapons                [WEAPONS_NO_WEAPONS];
    samplerArray_t              _samplerIds = arr_ini::makeSampArr(std::make_index_sequence<WEAPONS_NO_WEAPONS>{}); 

    uiUni                       _selectedWeaponIndexUni   { "chosenWeaponInx", 0 };
    ftUni                       _overlayAlpthaUni         { "overlayAlpha", 0.0f };

    Clock<>                     m_weaponTransitionClock;
    Clock<>                     m_weaponCooldownClock;

    bool                        _isThereWeaponCooldown{};
    bool                        _isWeaopnsTabVisible{};
    bool                        _isWeaponsFullyDrawn{};
    bool                        _isLBMPressed{};
    
    const ft                    _iconRealestate     = WEAPONS_ICON_DIMS + 1.0f;
    const glm::mat4             _projection         = glm::ortho(0.0f, static_cast<ft>(SCREEN_WIDTH), 0.0f, static_cast<ft>(SCREEN_HEIGHT));
    const glm::mat4             _inverseProjection  = glm::inverse(_projection);

    void _closeWeaponTab();
    void _switchWeapons(const ui weaponIndex);
};