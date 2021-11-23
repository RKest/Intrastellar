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

    glm::mat4               _projInstanceTransforms [MAX_PROJ_AMOUNT];
	ui		                _noLeftProjPiercings    [MAX_PROJ_AMOUNT];
	std::vector<ui>         _alreadyHitEnemyIds     [MAX_PROJ_AMOUNT];
    ui                      _noProjs = 0;

    Clock<glm::mat4 const&> m_shotClock;
    ui _oldestProjIndex{};
	bool _projHit(const ui projIndex, const ui enemyIndex);
    void _commonUpdate();
};

struct IWeapon
{
    static inline void Init(Weapon* weaponPtr, const ui weaponNumber)
    {
        m_weaponPtrArray[weaponNumber] = weaponPtr;
        weaponProjHitCbs[weaponNumber] = [weaponNumber](const ui projIndex, const ui enemyIndex)
            { return IWeapon::m_weaponPtrArray[weaponNumber]->_projHit(projIndex, enemyIndex); };
    }

    inline static const ui s_noWeapons = Weapons::NO_IMPLEMENTED_WEAPONS;

    static inline const auto &ProjHitCb            (const ui weaponIndex) { return weaponProjHitCbs[weaponIndex]; }
    static inline const auto &ProjInstaceTransorms (const ui weaponIndex) { return m_weaponPtrArray[weaponIndex]->_projInstanceTransforms; }
    static inline const auto &NoProjs              (const ui weaponIndex) { return m_weaponPtrArray[weaponIndex]->_noProjs; }
private:
    inline static std::array<Weapon*,                                 Weapons::NO_IMPLEMENTED_WEAPONS> m_weaponPtrArray;
    inline static std::array<std::function<bool(const ui, const ui)>, Weapons::NO_IMPLEMENTED_WEAPONS> weaponProjHitCbs;
};