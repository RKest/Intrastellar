#pragma once
#include "_config.h"

#include "Core/shader.h"
#include "Core/mesh.h"
#include "Core/display.h"
#include "Core/helpers.h"
#include "Core/transform.h"
#include "Core/bounding_box.h"
#include "Core/stats.h"
#include "Core/texture.h"
#include "Core/camera.h"

#include <string>

using samplerArray_t = std::array<uiUni, WEAPONS_NO_WEAPONS>;

namespace arr_ini
{
    inline uiUni initSmaplerUni(ui i){ return uiUni(std::string("samps[" + std::to_string(i) + "]"), i); }
    template <size_t ...I>
    inline samplerArray_t makeSampArr(std::index_sequence<I...>){ return samplerArray_t {{ initSmaplerUni(I)... }}; }
}

enum Weapons : ui
{
    BLASTER,
    ROCEKT_LANCHER,
    LASER,
    NO_IMPLEMENTED_WEAPONS
};


struct IWeapon;
using weaponInterfaceArray_t = std::array<IWeapon*, Weapons::NO_IMPLEMENTED_WEAPONS>;

class WeaponsManager;
class Weapon {
public:
    Weapon(WeaponsManager &manager, UntexturedInstancedMesh &projMesh);
    void Draw();
    void Fire(const glm::mat4 &pcModel);
    void Init();
    void Uninit();
    void Update();
    inline IWeapon *Interface() { return _interface; }

protected:
    friend struct IWeapon;
    PlayerStats _weaponStats = defaultStats;
    WeaponsManager &_manager;
    UntexturedInstancedMesh &_projMesh;
    IWeapon *_interface;

    glm::mat4           _projInstanceTransforms [MAX_PROJ_AMOUNT];
	ui		            _noLeftProjPiercings    [MAX_PROJ_AMOUNT];
	std::vector<ui>     _alreadyHitEnemyIds     [MAX_PROJ_AMOUNT];
    size_t              _noProjs = 0;

    ui _shotClockId{};

    virtual void _specUpdate() = 0;
	bool _projHit(const ui projIndex, const ui enemyIndex);
};

using namespace std::placeholders;
struct IWeapon
{
    IWeapon(Weapon *weaponPtr) : _weaponPtr(weaponPtr) {}
    IWeapon(IWeapon *iPtr) : _weaponPtr(iPtr->_weaponPtr) {}

    inline const auto &ProjHitCb()              const { return weaponProjHitCb; }
    inline const auto &ProjInstaceTransorms()   const { return _weaponPtr->_projInstanceTransforms; }
    inline const auto &NoProjs()                const { return _weaponPtr->_noProjs; }
private:
    Weapon *_weaponPtr;
	DECL_INST(weaponProjHitCb, std::bind(&Weapon::_projHit, _weaponPtr, _1, _2));
};

class BlasterWeapon : public Weapon
{
public:
    BlasterWeapon(WeaponsManager &manager, UntexturedInstancedMesh &projMesh);
private:
    void _specUpdate() override;
};

class RocketLauncherWeapon : public Weapon
{
public:
    RocketLauncherWeapon(WeaponsManager &manager, UntexturedInstancedMesh &projMesh);
private:
    void _specUpdate() override;
};

class LaserWeapon : public Weapon
{
public:
    LaserWeapon(WeaponsManager &manager, UntexturedInstancedMesh &projMesh);
private:
    void _specUpdate() override;
};

class WeaponsManager
{
public:
    WeaponsManager(helpers::Core &core, const TexturedMeshParams &iconMeshParams, const UntexturedMeshParams &overlayMeshParams, 
        const UntexturedMeshParams &blasterProjParams, const UntexturedMeshParams &rocketMeshParams);

    void Draw();
    void Update(const glm::mat4 &pcModel);
    void Reset();

    inline auto &WeaponInterfaces() { return _weaponInterfaces; }

private:
    friend class Weapon;
    friend class BlasterWeapon;
    friend class RocketLauncherWeapon;
    friend class LaserWeapon;

    Display &_display;
    Timer &_timer;
    PlayerStats &_pcStats;
    Camera &_camera;
    TexturedInstancedMesh _iconMesh;
    UntexturedMesh _overlayMesh;

    UntexturedInstancedMesh _blasterProjMesh;
    UntexturedInstancedMesh _rocketProjMesh;
    UntexturedInstancedMesh _laserProjMesh;

    Shader                          _overlayShader          {"./Shaders/Overlay"    };
    Shader                          _weaponIconShader       {"./Shaders/WeaponIcon" };
    Shader                          _projShader             {"./Shaders/Projectile" };
    Texture                         _weaponTextures         {WEAPONS_NO_WEAPONS};

    glm::mat4                       _instanceTransforms     [WEAPONS_NO_WEAPONS];
    glm::mat4                       _baseInstanceTransforms [WEAPONS_NO_WEAPONS];
    ReqBoundingBox                  _boundingBoxes          [WEAPONS_NO_WEAPONS];
    PlayerStats                     _weaponStatAltarations  [WEAPONS_NO_WEAPONS];
    std::unique_ptr<Weapon>         _weapons                [WEAPONS_NO_WEAPONS];
    weaponInterfaceArray_t          _weaponInterfaces;
    samplerArray_t  _samplerIds = arr_ini::makeSampArr(std::make_index_sequence<WEAPONS_NO_WEAPONS>{}); 

    uiUni _selectedWeaponIndexUni   {"chosenWeaponInx", 0};
    ftUni _overlayAlpthaUni         {"overlayAlpha", 0.0f};
    ui _weaponTransitionClockId{};
    db _scaledTransitionTime = OVERLAY_TRANSITION_TIME;

    ui _weaponCooldownClockId{};
    db _weaponCooldownTime = WEAPONS_COOLDOWN;

    bool _isThereWeaponCooldown{};
    bool _isWeaopnsTabVisible{};
    bool _isWeaponsFullyDrawn{};
    bool _isLBMPressed{};
    
    const ft _iconRealestate = WEAPONS_ICON_DIMS + 1.0f;
    const glm::mat4 _projection                 = glm::ortho(0.0f, static_cast<ft>(SCREEN_WIDTH), 0.0f, static_cast<ft>(SCREEN_HEIGHT));
    const glm::mat4 _inverseFlippedProjection   = glm::inverse(glm::ortho(0.0f, static_cast<ft>(SCREEN_WIDTH), static_cast<ft>(SCREEN_HEIGHT), 0.0f));

    void _closeWeaponTab();
};