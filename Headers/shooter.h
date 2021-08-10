#ifndef SHOOTER_H
#define SHOOTER_H

#include "Core/transform.h"
#include "Core/shader.h"
#include "Core/camera.h"
#include "Core/mesh.h"
#include "Core/text.h"
#include "enemy.h"

#include <execution>
#include <algorithm>

class Shooter
{
public:
	Shooter(const UntexturedMeshParams &pcParams, Shader &shader, Camera &camera, Text &text, UntexturedInstancedMesh &projectileMesh, 
	EnemyManager &enemyManager, const ui maxProjectileAmount = 10);

	void Reset();
	void RenderScore();
	void Update(const glm::mat4 pcModel, bool &isPcAlive);
	void Shoot(const glm::mat4 &originTransform);

	~Shooter();

private:
	const UntexturedMeshParams &pcParams;
	UntexturedInstancedMesh &projectileMesh;
	Shader &shader;
	Camera &camera;
	Text &text;
	Transform blankTransform;
	EnemyManager &enemyManager;

	std::vector<glm::vec2> projectilePositions;
	std::vector<glm::mat4> projectileInstanceTransforms;

	glm::mat4 perFrameProjectileTransform;

	ui maxProjectileAmount;
	ui oldestProjectileIndex = 0;

	ui enemiesShotCounter = 0;
};

#endif