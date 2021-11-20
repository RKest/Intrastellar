#pragma once

#include "_config.h"

#include "Core/transform.h"
#include "Core/shader.h"
#include "Core/camera.h"
#include "Core/mesh.h"
#include "Core/text.h"
#include "Core/timer.h"
#include "Core/helpers.h"
#include "Core/stats.h"
#include "Core/bounding_box.h"
#include "player_character.h"

#include <execution>
#include <algorithm>
#include <functional>
#include <vector>
#include <memory>
#include <array>
#include <iterator>

#include "weapon-types.h"

enum BehavoiurStatus
{
	NOT_CHOSEN,
	CHOSEN,
	DEFAULT
};

enum EnemyTypeEnum : ui 
{
	CHASER_ENEMY,
	SHOOTER_ENEMY,
	ORBITER_ENEMY,

	NO_ENEMY_TYPES
};

class EnemyManager;
class EnemyData;

class EnemyBehaviuor
{
public:
	EnemyBehaviuor(EnemyManager &manager, EnemyTypeEnum enemyType, bool isDefault = false);
	virtual ~EnemyBehaviuor() = default;
	virtual void Update	(const ui dataIndex) = 0;
	virtual void Fire	(const ui dataIndex);
	[[nodiscard]]BehavoiurStatus EnemyBehaviuorStatus(const glm::mat4 &enemyModel);
	inline bool &IsActive() { return _isActive; }
protected:
	virtual bool HasMetPredicate([[maybe_unused]]const glm::mat4 &enemyModel) { return true; };
	void UpdateProjs(std::vector<glm::mat4> &projInstanceTransforms);
	EnemyManager &_manager;
	EnemyTypeEnum m_enemyType;
	bool _isActive{};
	const bool _isDefault;
};

using behavoiurPtrVec_t = std::vector<std::unique_ptr<EnemyBehaviuor>>;
class ChaseBehaviour : public EnemyBehaviuor
{
public:
	ChaseBehaviour(EnemyManager &manager, EnemyTypeEnum enemyType) : EnemyBehaviuor(manager, enemyType, true) {}
	void Update	(const ui dataIndex) override;

private:
};

class ShootBehavoiur : public EnemyBehaviuor
{
public:
	ShootBehavoiur(EnemyManager &manager, EnemyTypeEnum enemyType);
	void Update	(const ui dataIndex) override;
	void Fire	(const ui dataIndex) override;

private:
	bool HasMetPredicate(const glm::mat4 &enemyModel);
	ui _latestShotIndex{};
};

class OrbiterBehaviour : public EnemyBehaviuor
{
public:
	OrbiterBehaviour(EnemyManager &manager, EnemyTypeEnum enemyType);
	void Update	(const ui dataIndex) override;
	void Fire	(const ui dataIndex) override;

private:
	ui _latestShotIndex{};
};

inline static auto choseBehaviour(behavoiurPtrVec_t &behavoiurs, const glm::mat4 &enemyModel)
{
	const auto bbegin = behavoiurs.begin();
	const auto bend = behavoiurs.end();
	auto chosenIt = bend;
	auto defaultIt = bend;
	for(auto i = bbegin; i != bend; ++i)
	{
		const BehavoiurStatus behaviourStatus = i->get()->EnemyBehaviuorStatus(enemyModel);
		if(BehavoiurStatus::CHOSEN == behaviourStatus)
			chosenIt = i;
		else if(BehavoiurStatus::DEFAULT == behaviourStatus)
			defaultIt = i;
	}
	if(chosenIt != bend)
		return chosenIt;
	if(defaultIt != bend)
		return defaultIt;
	throw std::runtime_error("ERROR:choseBehaviour: Failed to chose a behaviour");
}

using orphanedProjPair_t = std::pair<Clock<>, std::vector<glm::mat4>>;
struct EnemyData
{
	EnemyData(EnemyManager &manager);
	std::vector<glm::mat4>					instanceTransforms;
	std::vector<ReqBoundingBox>				boundingBoxes;
	std::vector<si>							healths;
	behavoiurPtrVec_t						enemyBehaviours;
	std::vector<std::vector<glm::mat4>>		projInstanceTransforms;
	std::vector<orphanedProjPair_t> 		clockOrphanedProjsParis;
	std::vector<Clock<EnemyBehaviuor*, ui>>	shotClocks;
	std::vector<ui>							ids;

	size_t 	size = 0;
	void Clear();
	void Erase(const ui index);
	void Push(const glm::mat4 &instanceTransform, const UntexturedMeshParams &params, EnemyStats &stats);
private:
	EnemyManager &_manager;
};

class Enemy
{
public:
	Enemy(EnemyManager &manager, behavoiurPtrVec_t &behaviours, const glm::vec3 &colour, const ui maxNoInstances);
	Enemy(const Enemy&) = delete;
	Enemy(Enemy&&) = default;
	void Update();
	void Spawn(const glm::mat4 &instanceTransform);
	void Draw();
	EnemyData data;
private:
	EnemyManager &_manager;
	UntexturedInstancedMesh _mesh;
	UntexturedInstancedMesh _projMesh;
	behavoiurPtrVec_t _behaviours;
	const vecUni _colourUni;
	const ui _maxNoInstances;
};

using fatalityCallback_t = std::function<void(const glm::mat4&, const ui)>;
struct EnemyInterface;
class EnemyManager
{
public:
	EnemyManager(helpers::Core &core, const UntexturedMeshParams &params, EnemyStats &enemyStats, const UntexturedMeshParams &projParams, 
		IPlayerCharacter *pcInterface, weaponInterfaceArray_t &weaponInterfaces, fatalityCallback_t fatalityCallback);

	void Reset();
	void Draw();
	void Spawn();
	void UpdateBehaviour(const std::vector<glm::vec2> &pcPositions);
	std::vector<glm::mat4> InstanceTransforms();
	inline ui NextId() { return _newestEnemyId++; }
	inline auto Interface() { return m_interfacePtr; }

private:
	friend class Enemy;
	friend class EnemyData;
	friend class EnemyBehaviuor;
	friend class ChaseBehaviour;
	friend class ShootBehavoiur;
	friend class OrbiterBehaviour;
	friend class EnemyInterface;

	Shader _enemyShader{"./Shaders/Enemy"};
	Shader _enemyProjShader{"./Shaders/EnemyProjectile"};
	PlayerStats &_pcStats;
	EnemyStats &_enemyStats;
	const UntexturedMeshParams _enemyParams;
	const UntexturedMeshParams _enemyProjParams;
	IPlayerCharacter _pcInterface;
	weaponInterfaceArray_t &_weaponInterfaces;
	Clock<> m_spawnClock;
	EnemyInterface *m_interfacePtr;
	fatalityCallback_t m_fatalityCallback;

	glm::mat4 _pcModel;
	Transform _enemyTransform;
	CustomRand _customRand{CUSTOM_RAND_SEED};
	std::vector<Enemy> _enemies;
	std::vector<std::pair<ui, ui>> m_instanceTransformToEnemyIndexMap;

	ui _newestEnemyId = 0;

	void checkForProjIntersection(std::vector<glm::mat4> &projInstanceTransforms);
	void m_spawn();
	void m_hit(const ui enemyIndex);
};

using namespace std::placeholders;
struct EnemyInterface
{
	EnemyInterface(EnemyManager *managerPtr) : m_managerPtr(managerPtr) {}
	inline auto EnemyHit() { return enemyHit; }
private:
	EnemyManager *m_managerPtr;
	DECL_INST(enemyHit, std::bind(&EnemyManager::m_hit, m_managerPtr, _1));
};