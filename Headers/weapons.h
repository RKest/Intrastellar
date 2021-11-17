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

class Weapon;
struct WeaponBehaviour
{
    void Construct(Weapon* weaponPtr){ _weaponPtr = weaponPtr; }
    virtual void Update([[maybe_unused]]const std::vector<glm::mat4> &enemyInstanceTransforms) = 0;
    virtual void Draw();
    virtual void Fire(const glm::mat4 &pcModel);
protected:
    Weapon *_weaponPtr = nullptr;
};

struct BlasterBehaviour : public WeaponBehaviour
{
    void Update([[maybe_unused]]const std::vector<glm::mat4> &enemyInstanceTransforms) override;
};

struct RocketBehaviour : public WeaponBehaviour
{
    void Update([[maybe_unused]]const std::vector<glm::mat4> &enemyInstanceTransforms) override;
};

struct LaserBehaviour : public WeaponBehaviour
{
    void Update([[maybe_unused]]const std::vector<glm::mat4> &enemyInstanceTransforms) override;
    void Fire(const glm::mat4 &pcModel) override;
    void Draw() override;

private:
    UntexturedDynamicBezierMesh _projMesh       {MAX_PROJ_AMOUNT, 10};

    std::vector<glm::vec2>  _laserBezierCurves = std::vector<glm::vec2>(MAX_PROJ_AMOUNT * 3 + 1);
    ui                      _laserBezierCurvesSZ = 0;
    ui                      _noBezierCurves = 1;
    glm::mat4               _laserOrigin;
    ui                      _laserLingerClockId;
    db                      _laserLingerClockDuration = WEAPONS_LASER_LINGER_DURATION;
    bool                    _hasTheLaserFired{};
};

struct IWeapon;
using weaponInterfaceArray_t = std::array<IWeapon*, Weapons::NO_IMPLEMENTED_WEAPONS>;

class WeaponsManager;
class Weapon {
public:
    Weapon(WeaponsManager &manager, UntexturedInstancedMesh &projMesh, WeaponBehaviour &behaviour, PlayerStats &weaponStats, Shader &projShader);
    void Draw();
    void Fire(const glm::mat4 &pcModel);
    void Init();
    void Uninit();
    void Update(const std::vector<glm::mat4> &enemyInstanceTransforms);
    inline IWeapon *Interface() { return _interface; }

protected:
    friend struct IWeapon;
    friend struct WeaponBehaviour;
    friend struct BlasterBehaviour;
    friend struct RocketBehaviour;
    friend struct LaserBehaviour;
    PlayerStats             _weaponStats = defaultStats;
    WeaponsManager          &_manager;
    UntexturedInstancedMesh &_projMesh;
    WeaponBehaviour         &_behaviour;
    Shader                  &_projShader;
    IWeapon                 *_interface;

    glm::mat4               _projInstanceTransforms [MAX_PROJ_AMOUNT];
	ui		                _noLeftProjPiercings    [MAX_PROJ_AMOUNT];
	std::vector<ui>         _alreadyHitEnemyIds     [MAX_PROJ_AMOUNT];
    ui                      _noProjs = 0;

    ui _shotClockId{};
    ui _oldestProjIndex{};
	bool _projHit(const ui projIndex, const ui enemyIndex);
    void _commonUpdate();
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

class WeaponsManager
{
public:
    WeaponsManager(helpers::Core &core, const TexturedMeshParams &iconMeshParams, const UntexturedMeshParams &overlayMeshParams, 
        const UntexturedMeshParams &blasterProjParams, const UntexturedMeshParams &rocketMeshParams);

    void Draw();
    void Update(const glm::mat4 &pcModel, const std::vector<glm::mat4> &enemyInstanceTransforms);
    void Reset();

    inline auto &WeaponInterfaces() { return _weaponInterfaces; }

private:
    friend class Weapon;
    friend struct WeaponBehaviour;
    friend struct BlasterBehaviour;
    friend struct RocketBehaviour;
    friend struct LaserBehaviour;

    Display                     &_display;
    PlayerStats                 &_pcStats;
    Camera                      &_camera;
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

    Shader                      _overlayShader          {"./Shaders/Overlay"    };
    Shader                      _weaponIconShader       {"./Shaders/WeaponIcon" };
    Shader                      _projShader             {"./Shaders/Projectile" };
    Shader                      _bezierProjShader       {"./Shaders/Bezier"     };    
    Texture                     _weaponTextures         {WEAPONS_NO_WEAPONS};

    glm::mat4                   _instanceTransforms     [WEAPONS_NO_WEAPONS];
    glm::mat4                   _baseInstanceTransforms [WEAPONS_NO_WEAPONS];
    ReqBoundingBox              _boundingBoxes          [WEAPONS_NO_WEAPONS];
    std::unique_ptr<Weapon>     _weapons                [WEAPONS_NO_WEAPONS];
    weaponInterfaceArray_t      _weaponInterfaces;
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
    const glm::mat4 _projection         = glm::ortho(0.0f, static_cast<ft>(SCREEN_WIDTH), 0.0f, static_cast<ft>(SCREEN_HEIGHT));
    const glm::mat4 _inverseProjection  = glm::inverse(_projection);

    void _closeWeaponTab();
    void _switchWeapons(const ui weaponIndex);
};