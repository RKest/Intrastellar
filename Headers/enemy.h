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

#include "glm/gtx/matrix_transform_2d.hpp"

class EnemyManager;

class Enemy
{
public:
	Enemy(const UntexturedMeshParams &params, const glm::mat4 &instanceTransform, EnemyManager *manager);

	void UpdateBehaviour(const glm::mat4 &instanceTransform);
	inline void SetManagerIndex(const ui arg) { managerIndex = arg; }

	ui managerIndex;
	enum CollisionCheckEnum
	{
		NO_COLLISION,
		COLLISION,
		FATALITY
	};

	CollisionCheckEnum CheckForProjectileCollision(const glm::vec2 &projectilePos, const ui projectileDamage = 100);
	inline helpers::BoundingBox &EnemyBoundingBox() const { return const_cast<helpers::BoundingBox&>(boundingBox); }

protected:
private:
	EnemyManager *manager;
	helpers::BoundingBox boundingBox;

	ui health;
};

class EnemyManager
{
public:
	EnemyManager(Shader &enemyShader, Camera &camera, Timer &timer, const UntexturedMeshParams &params, ui seed, ui maxNoEnemies);

	//Called outside
	void Reset();
	void Draw();
	void Spawn(const glm::mat4 &pcModel);
	void RecordCollisions(const std::vector<glm::mat4> &projectileTransforms, 
		const std::function<void(ui)> projectileHitCallback, std::function<void(const glm::mat4&, const ui)> fatalityCallback);
	void RecordPCIntersection(const std::vector<glm::vec2> &pcPositions, const std::function<void()> intersectionCallback);
	void UpdateBehaviour(const glm::mat4 &pcModel);

	//Called by the enemy class
	void Despawn(const ui enemyIndex);

private:
	Shader &_enemyShader;
	Camera &_camera;
	Timer &_timer;

	Transform _enemyTransform;
	UntexturedInstancedMesh _enemyMesh;
	CustomRand _customRand;
	const UntexturedMeshParams _enemyMeshParams;

	std::vector<Enemy*> _enemies;
	std::vector<glm::mat4> _enemyInstanceTransforms;

	const ui _maxNoEnemies;
	const db _enemyPerFrameDistance = 0.01;
	db _scaledPerFrameTravelDistance;

};

#endif