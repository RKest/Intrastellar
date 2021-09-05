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

class EnemyManager
{
public:
	EnemyManager(Shader &enemyShader, Camera &camera, Timer &timer, Stats &pcStats, const UntexturedMeshParams &params, ui seed, ui maxNoEnemies);

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
	Stats &_pcStats;

	Transform _enemyTransform;
	UntexturedInstancedMesh _enemyMesh;
	CustomRand _customRand;
	const UntexturedMeshParams _enemyMeshParams;

	std::vector<glm::mat4> _enemyInstanceTransforms;
	std::vector<helpers::BoundingBox> _enemyBoundingBoxes;
	std::vector<si> _enemyHealths;

	const ui _maxNoEnemies;
	const db _enemyPerFrameDistance = 0.01;
	db _scaledPerFrameTravelDistance;

};

#endif