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

enum BehavoiurStatus
{
	NOT_CHOSEN,
	CHOSEN,
	DEFAULT
};

class EnemyManager;

class EnemyBehaviuor
{
public:
	EnemyBehaviuor(EnemyManager &manager, bool isDefault = false);
	virtual ~EnemyBehaviuor() = default;
	virtual void Update(glm::mat4 &instanceTransform, std::vector<glm::mat4> &projInstanceTransforms) = 0;
	[[nodiscard]]BehavoiurStatus EnemyBehaviuorStatus(const glm::mat4 &enemyModel);
	inline bool &IsActive() { return _isActive; }
protected:
	virtual bool HasMetPredicate([[maybe_unused]]const glm::mat4 &enemyModel) { return true; };
	void UpdateProjs(std::vector<glm::mat4> &projInstanceTransforms);
	EnemyManager &_manager;
	bool _isActive{};
	const bool _isDefault;
};

using behavoiurPtrVec = std::vector<std::unique_ptr<EnemyBehaviuor>>;

class ChaseBehaviour : public EnemyBehaviuor
{
public:
	ChaseBehaviour(EnemyManager &manager) : EnemyBehaviuor(manager, true) {}
	void Update(glm::mat4 &instanceTransform, std::vector<glm::mat4> &projInstanceTransforms) override;

private:
};

class ShootBehavoiur : public EnemyBehaviuor
{
public:
	ShootBehavoiur(EnemyManager &manager) : EnemyBehaviuor(manager, false) {}
	void Update(glm::mat4 &instanceTransform, std::vector<glm::mat4> &projInstanceTransforms) override;

private:
	bool HasMetPredicate(const glm::mat4 &enemyModel);
	ui _latestShotIndex{};
	ui _shotClockId;
};

class OrbiterBehaviour : public EnemyBehaviuor
{
public:
	OrbiterBehaviour(EnemyManager &manager);
	void Update(glm::mat4 &instanceTransform, std::vector<glm::mat4> &projInstanceTransforms) override;

private:
	ui _latestShotIndex{};
	ui _shotClockId;
};

inline static auto choseBehaviour(behavoiurPtrVec &behavoiurs, const glm::mat4 &enemyModel)
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

struct EnemyData
{
	EnemyData();
	std::vector<glm::mat4> 				instanceTransforms;
	std::vector<ReqBoundingBox>			boundingBoxes;
	std::vector<si>						healths;
	behavoiurPtrVec						enemyBehaviours;
	std::vector<std::vector<glm::mat4>> projInstanceTransforms;
	size_t size = 0;
	void Clear();
	void Erase(const ui index);
	void Push(const glm::mat4 &instanceTransform, const UntexturedMeshParams &params, EnemyStats &stats);
};

class Enemy
{
public:
	Enemy(EnemyManager &manager, behavoiurPtrVec &behaviours, const glm::vec3 &colour, const ui maxNoInstances);
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
	behavoiurPtrVec _behaviours;
	const vecUni _colourUni;
	const ui _maxNoInstances;
};

class EnemyManager
{
public:
	EnemyManager(Shader &enemyShader, helpers::Core &core, const UntexturedMeshParams &params, EnemyStats &enemyStats, 
		const UntexturedMeshParams &projParams, IPlayerCharacter *pcInterface);

	void Reset();
	void Draw();
	void Spawn();
	void UpdateBehaviour(const std::vector<glm::vec2> &pcPositions, std::function<void(const glm::mat4&, const ui)> fatalityCallback);
	std::vector<std::vector<glm::mat4>*> InstanceTransforms();

private:
	friend class Enemy;
	friend class EnemyData;
	friend class EnemyBehaviuor;
	friend class ChaseBehaviour;
	friend class ShootBehavoiur;
	friend class OrbiterBehaviour;
	enum EnemyTypeEnum : std::size_t
	{
		CHASER_ENEMY,
		SHOOTER_ENEMY,
		ORBITER_ENEMY,

		NO_ENEMY_TYPES
	};

	Shader &_enemyShader;
	Shader _enemyProjShader{"./Shaders/EnemyProjectile"};
	Camera &_camera;
	PlayerStats &_pcStats;
	Timer &_timer;
	EnemyData _enemyData;
	EnemyStats &_enemyStats;
	const UntexturedMeshParams _enemyParams;
	const UntexturedMeshParams _enemyProjParams;
	IPlayerCharacter _pcInterface;

	glm::mat4 _pcModel;
	Transform _enemyTransform;
	CustomRand _customRand{CUSTOM_RAND_SEED};
	std::vector<Enemy> _enemies;

};