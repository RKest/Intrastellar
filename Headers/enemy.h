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

class EnemyBehaviuor
{
public:
	EnemyBehaviuor(EnemyStats &stats, Timer &timer, const bool isDefault = false);
	EnemyBehaviuor(EnemyBehaviuor&&) = default;
	EnemyBehaviuor(const EnemyBehaviuor&) = default;
	EnemyBehaviuor operator=(const EnemyBehaviuor& rhs) { return EnemyBehaviuor(rhs); }
	virtual ~EnemyBehaviuor() = default;
	virtual void Update([[maybe_unused]]const glm::mat4 &pcTransform, [[maybe_unused]]glm::mat4 &instanceTransform, 
		[[maybe_unused]]std::vector<glm::mat4> &projInstanceTransforms) {};
	[[nodiscard]]BehavoiurStatus EnemyBehaviuorStatus(const glm::mat4 &pcModel, const glm::mat4 &enemyModel);
	inline bool &IsActive() { return _isActive; }
protected:
	virtual bool HasMetPredicate([[maybe_unused]]const glm::mat4 &pcModel, [[maybe_unused]]const glm::mat4 &enemyModel) { return true; };
	EnemyStats &_enemyStats;
	Timer &_timer;
	bool _isActive{};
	const bool _isDefault;
};

using behavoiurPtrVec = std::vector<std::unique_ptr<EnemyBehaviuor>>;

class ChaseBehaviour : public EnemyBehaviuor
{
public:
	ChaseBehaviour(EnemyStats &enemyStats, Timer &timer) : EnemyBehaviuor(enemyStats, timer, true) {};
	void Update(const glm::mat4 &pcTransform, glm::mat4 &instanceTransform, [[maybe_unused]]std::vector<glm::mat4> &projInstanceTransforms) override;

private:
};

class ShootBehavoiur : public EnemyBehaviuor
{
public:
	ShootBehavoiur(EnemyStats &enemyStats, Timer &timer) : EnemyBehaviuor(enemyStats, timer) {};
	void Update(const glm::mat4 &pcTransform, glm::mat4 &instanceTransform, std::vector<glm::mat4> &projInstanceTransforms) override;

private:
	bool HasMetPredicate(const glm::mat4 &pcModel, const glm::mat4 &enemyModel);
	ui _latestShotIndex{};
	ui _shotClockId;
};

inline static auto choseBehaviour(behavoiurPtrVec &behavoiurs, const glm::mat4 &pcModel, const glm::mat4 &enemyModel)
{
	const auto bbegin = behavoiurs.begin();
	const auto bend = behavoiurs.end();
	auto chosenIt = bend;
	auto defaultIt = bend;
	for(auto i = bbegin; i != bend; ++i)
	{
		const BehavoiurStatus behaviourStatus = i->get()->EnemyBehaviuorStatus(pcModel, enemyModel);
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
	std::vector<helpers::BoundingBox>	boundingBoxes;
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
	Enemy(Timer &timer, Shader &enemyShader, EnemyStats &stats, const UntexturedMeshParams &params, behavoiurPtrVec &behaviours, 
		const glm::vec3 &colour, const ui maxNoInstances, Shader *projShaderPtr = nullptr, const UntexturedMeshParams *projParamsPtr = nullptr);
	void Update(const glm::mat4 &pcModel);
	void Spawn(const glm::mat4 &instanceTransform);
	void Draw(const glm::mat4 &cameraProjection);
	EnemyData data;
private:
	Timer &_timer;
	Shader &_enemyShader;
	EnemyStats &_stats;
	const UntexturedMeshParams _params;
	UntexturedInstancedMesh _mesh;
	behavoiurPtrVec _behaviours;
	const vecUni _colourUni;
	const ui _maxNoInstances;
	Shader *_projShaderPtr;
	const UntexturedMeshParams *_projParamsPtr;
	std::unique_ptr<UntexturedInstancedMesh> _projMeshPtr;
};

class EnemyManager
{
public:
	EnemyManager(Shader &enemyShader, helpers::Core &core, const UntexturedMeshParams &params, EnemyStats &enemyStats, 
		const UntexturedMeshParams &projParams);

	void Reset();
	void Draw();
	void Spawn(const glm::mat4 &pcModel);
	void RecordCollisions(const std::vector<glm::mat4> &projectileTransforms, 
		const std::function<void(ui)> projectileHitCallback, std::function<void(const glm::mat4&, const ui)> fatalityCallback);
	void RecordPCIntersection(const std::vector<glm::vec2> &pcPositions, const std::function<void()> intersectionCallback);
	void UpdateBehaviour(const glm::mat4 &pcModel);
	std::vector<std::vector<glm::mat4>*> InstanceTransforms();

private:
	enum EnemyTypeEnum
	{
		CHASER_ENEMY,
		SHOOTER_ENEMY,

		NO_ENEMY_TYPES
	};

	Shader &_enemyShader;
	Shader _enemyProjShader{"./Shaders/EnemyProjectile"};
	Camera &_camera;
	PlayerStats &_pcStats;
	Timer &_timer;
	EnemyData _enemyData;
	EnemyStats &_enemyStats;
	Transform _enemyTransform;
	CustomRand _customRand{CUSTOM_RAND_SEED};
	std::vector<Enemy> _enemies;

};