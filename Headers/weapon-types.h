#pragma once
#include "_config.h"

#include <array>

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
    UntexturedDynamicBezierMesh _projMesh       { MAX_PROJ_AMOUNT, 10 };

    std::vector<glm::vec2>  _laserBezierCurves = std::vector<glm::vec2>( MAX_PROJ_AMOUNT * 3 + 1 );
    ui                      _laserBezierCurvesSZ = 0;
    ui                      _noBezierCurves = 1;
    glm::mat4               _laserOrigin;
    Clock<>                 m_laserLingerClock;
    bool                    m_isLaserVisible{};
    bool                    m_hasLaserFired{};
};

using shotClock_t = Clock<glm::mat4 const&>;

struct IWeapon;
class WeaponsManager;
class Weapon {
public:
    Weapon(WeaponsManager &manager, UntexturedInstancedMesh &projMesh, WeaponBehaviour &behaviour, PlayerStats &weaponStats, Shader &projShader);
    void Draw();
    void Fire(const glm::mat4 &pcModel);
    void Init();
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

    shotClock_t m_shotClock;
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

using weaponInterfaceArray_t = std::array<IWeapon*, Weapons::NO_IMPLEMENTED_WEAPONS>;