#ifndef ENEMY_H
#define ENEMY_H

#include "_config.h"

#include "Core/mesh.h"
#include "Core/random.h"
#include "Core/shader.h"
#include "Core/timer.h"
#include "Core/helpers.h"

#include <vector>
#include <execution>
#include <algorithm>
#include <functional>
#include <cmath>

#include "glm/gtx/matrix_transform_2d.hpp"

enum BehavoiurStatus
{
	NOT_CHOSEN,
	CHOSEN,
	DEFAULT
};

template <bool isDefault>
class EnemyBehaviuor
{
public:
	EnemyBehaviuor(EnemyStats &stats, Timer &timer);
	virtual EnemyBehaviuor(EnemyBehaviuor&&) = default;
	virtual EnemyBehaviuor(const EnemyBehaviuor&) = default;
	virtual EnemyBehaviuor operator=(const EnemyBehaviuor& rhs) { return EnemyBehaviuor(rhs); }
	virtual ~EnemyBehaviuor() = default;
	virtual void Update(const glm::mat4 &pcTransform, glm::mat4 &instanceTransform, 
		[[maybe_unused]]std::vector<glm::mat4> &projInstanceTransforms) = 0;
	[[nodiscard]]BehavoiurStatus EnemyBehaviuorStatus(const glm::mat4 &pcModel, const glm::mat4 &enemyModel) const;
	inline bool &IsActive() { return _isActive; }
protected:
	virtual bool HasMetPredicate(const glm::mat4 &pcModel, const glm::mat4 &enemyModel) const  = 0;
	EnemyStats &_enemyStats;
	Timer &_timer;
	bool _isActive{};
};

class ChaseBehaviour : public EnemyBehaviuor<true>
{
public:
	ChaseBehaviour(EnemyStats &enemyStats, Timer &timer) : EnemyBehaviuor(enemyStats, timer) {};
	void Update(const glm::mat4 &pcTransform, glm::mat4 &instanceTransform, std::vector<glm::mat4> &projInstanceTransforms) override;
	bool IsChosen(const glm::mat4&) override;

private:
	EnemyStats &_enemyStats;
	Timer &_timer;
};

class ShootBehavoiur : public EnemyBehaviuor<false>
{
public:
	ShootBehavoiur(EnemyStats &enemyStats, Timer &timer) : EnemyBehaviuor(enemyStats, timer) {};
	void Update(const glm::mat4 &pcTransform, glm::mat4 &instanceTransform, std::vector<glm::mat4> &projInstanceTransforms) override;

private:
	bool HasMetPredicate(const glm::mat4 &pcModel, const glm::mat4 &enemyModel) const;
	EnemyStats &_enemyStats;
	Timer &_timer;
	ui _latestShotIndex{};
	ui _chotClockId;
};

static auto choseBehaviour(const std::vector<EnemyBehaviuor> &behavoiurs);

class EnemyData
{
public:
	EnemyData(Timer &_timer);
	std::vector<glm::mat4> 						instanceTransforms;
	std::vector<helpers::BoundingBox>			boundingBoxes;
	std::vector<si>								healths;
	std::vector<std::vector<EnemyBehaviuor>>	enemyBehaviours;
	std::vector<std::vector<glm::mat4>> 		projInstanceTransforms;
	ui size = 0;
	void Clear();
	void Erase(const ui index);
	void Push(const glm::mat4 &instanceTransform, const UntexturedMeshParams &params, EnemyStats &stats, Timer &timer);
	void Update(const glm::mat4 &pcModel, const ui index);
private:
	[[nodiscard]] int ChoseBehavoiur() const;
	Timer &_timer;
};

class EnemyManager
{
public:
	EnemyManager(Shader &enemyShader, helpers::Core &core, const UntexturedMeshParams &params, EnemyStats &enemyStats);

	void Reset();
	void Draw();
	void Spawn(const glm::mat4 &pcModel);
	void RecordCollisions(const std::vector<glm::mat4> &projectileTransforms, 
		const std::function<void(ui)> projectileHitCallback, std::function<void(const glm::mat4&, const ui)> fatalityCallback);
	void RecordPCIntersection(const std::vector<glm::vec2> &pcPositions, const std::function<void()> intersectionCallback);
	void UpdateBehaviour(const glm::mat4 &pcModel);
	inline std::vector<glm::mat4> &InstanceTransforms() { return _enemyData.instanceTransforms; }

private:
	Shader &_enemyShader;
	Shader _enemyProjShader{"./Shaders/EnemyProjectile"};
	Camera &_camera;
	PlayerStats &_pcStats;
	Timer &_timer;
	EnemyData _enemyData;
	const UntexturedMeshParams _enemyMeshParams;
	UntexturedInstancedMesh _enemyMesh;
	EnemyStats &_enemyStats;
	Transform _enemyTransform;
	CustomRand _customRand{CUSTOM_RAND_SEED};

};

#endif