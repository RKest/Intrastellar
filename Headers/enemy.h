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

//TODO look into std::fpclassify

class EnemyBehaviuor 
{
public:
	EnemyBehaviuor(EnemyStats &enemyStats, Timer &timer);
	EnemyBehaviuor(EnemyBehaviuor&&) = default;
	EnemyBehaviuor(const EnemyBehaviuor&) = default;
	EnemyBehaviuor operator=(const EnemyBehaviuor& rhs) { return EnemyBehaviuor(rhs); }
	void Update(const glm::mat4 pcTransform, glm::mat4 &instanceTransform, std::vector<glm::mat4> &projInstanceTransforms);
private:
	EnemyStats &_enemyStats;
	Timer &_timer;
	bool doesShoot{};
	ui _shotClockId{};
	ui _oldestProjIndex{};
};

class EnemyData
{
public:
	EnemyData(Timer &_timer);
	std::vector<glm::mat4> 				instanceTransforms;
	std::vector<helpers::BoundingBox>	boundingBoxes;
	std::vector<si>						healths;
	std::vector<EnemyBehaviuor>			enemyBehaviours;
	std::vector<std::vector<glm::mat4>> projInstanceTransforms;
	ui size = 0;
	void Clear();
	void Erase(const ui index);
	void Push(const glm::mat4 &instanceTransform, const UntexturedMeshParams &params, EnemyStats &stats, Timer &timer);
	void Update(const ui index);
private:
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
	Camera &_camera;
	Timer &_timer;
	PlayerStats &_pcStats;
	EnemyStats &_enemyStats;

	Transform _enemyTransform;
	UntexturedInstancedMesh _enemyMesh;
	CustomRand _customRand;
	const UntexturedMeshParams _enemyMeshParams;

	EnemyData _enemyData;
};

#endif