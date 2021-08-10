#ifndef ENEMY_H
#define ENEMY_H

#include "_config.h"

#include "Core/mesh.h"
#include "Core/random.h"
#include "Core/shader.h"

#include <vector>
#include <execution>
#include <algorithm>
#include <functional>

#include "glm/gtx/matrix_transform_2d.hpp"

class EnemyManager;

struct BoundingBox
{
	glm::vec2 botLeftPos;
	glm::vec2 topRightPos;
	glm::vec2 botLeftPosRel;
	glm::vec2 topRightPosRel;
	bool IsThereAnIntersection(const glm::vec2&) const;
};

class Enemy
{
public:
	Enemy(const glm::vec3 *positions, const glm::mat4 &instanceTransform, const ui noVertices, EnemyManager *manager);

	void UpdateBehaviour(const glm::mat4 &instanceTransform);
	inline void SetManagerIndex(const ui arg) { managerIndex = arg; }

	~Enemy();

	ui managerIndex;
	enum CollisionCheckEnum
	{
		NO_COLLISION,
		COLLISION,
		FATALITY
	};

	CollisionCheckEnum CheckForProjectileCollision(const glm::vec2 &projectilePos, const ui projectileDamage = 100);
	inline BoundingBox &EnemyBoundingBox() const { return const_cast<BoundingBox&>(boundingBox); }

protected:
private:
	EnemyManager *manager;
	BoundingBox boundingBox;

	ui health;
};

class EnemyManager
{
public:
	EnemyManager(Shader &enemyShader, Camera &camera, const UntexturedMeshParams &params, ui seed, ui maxNoEnemies);

	//Called outside
	void Reset();
	void Draw();
	void Spawn(const glm::mat4 &pcModel);
	void RecordCollisions(const std::vector<glm::vec2> &projectilePositions, const std::function<void(ui)> projectileHitCallback);
	void RecordPCIntersection(const std::vector<glm::vec2> &pcPositions, const std::function<void()> intersectionCallback);
	void UpdateBehaviour(const glm::mat4 &pcModel);

	//Called by the enemy class
	void Despawn(const ui enemyIndex);
	
	~EnemyManager();

private:
	Shader &enemyShader;
	Camera &camera;

	Transform enemyTransform;
	Transform blankTransform;
	UntexturedInstancedMesh enemyMesh;
	CustomRand customRand;
	const UntexturedMeshParams params;

	std::vector<Enemy*> enemies;
	std::vector<glm::mat4> enemyInstanceTransforms;

	const ui maxNoEnemies;
	const ft enemyPerFrameDistance = 0.05f;

	void scale2dVec(glm::vec2 &vecToScale, const ft scaleToLength);
};

#endif