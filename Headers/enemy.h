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
	EnemyBehaviuor(const EnemyStats &enemyStats, Timer &timer);
	Update(const glm::mat4 pcTransform, glm::mat4 &instanceTransform, std::vector<glm::mat4> &projInstanceTransforms);
private:
	const EnemyStats &_enemyStats;
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
	void Push(const glm::mat4 &instanceTransform, const UntexturedMeshParams &params, const EnemyStats &stats, Timer &timer);
	void Update(const ui index);
private:
	Timer &_timer;
};

class EnemyManager
{
public:
	EnemyManager(Shader &enemyShader, helpers::Core &core, const UntexturedMeshParams &params);

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
	Stats &_pcStats;

	Transform _enemyTransform;
	UntexturedInstancedMesh _enemyMesh;
	CustomRand _customRand;
	const UntexturedMeshParams _enemyMeshParams;

	EnemyData _enemyData;

	const ui _maxNoEnemies;
	const db _enemyPerFrameDistance = 0.01;
};

#endif