#include "shooter.h"
#include "glm/gtx/string_cast.hpp"
#include <iostream>

Shooter::Shooter(const UntexturedMeshParams &pcParams, Shader &shader, Camera &camera, Text &text, Timer &timer, 
	UntexturedInstancedMesh &projectileMesh, EnemyManager &enemyManager, const ui maxProjectileAmount)
	: pcParams(pcParams), projectileMesh(projectileMesh), maxProjectileAmount(maxProjectileAmount), shader(shader), camera(camera), text(text),
	enemyManager(enemyManager), timer(timer)
{
	perFrameProjectileTravel = 0.05;
}

void Shooter::Reset()
{
	projectileInstanceTransforms.clear();
	projectilePositions.clear();
	oldestProjectileIndex = 0;
	enemiesShotCounter = 0;
}


void Shooter::RenderScore()
{
	//Setting the leading zeroes
	std::string scoreString = "";
	for(ui i = 4; i != 0; --i)
	{
		if (i > std::to_string(enemiesShotCounter).length())
			scoreString += "0";
		else
		{
			scoreString += std::to_string(enemiesShotCounter);
			break;
		}
	}
		
	text.Render(scoreString, 10.0f, static_cast<ft>(SCREEN_HEIGHT) - 40.0f, 1.0f, glm::vec3(1));
}

void Shooter::Update(const glm::mat4 pcModel, bool &isPcAlive)
{
	perFrameProjectileTransform = glm::translate(glm::vec3(0.0f, timer.Scale(perFrameProjectileTravel), 0.0f));
	std::for_each
	(
		std::execution::par_unseq, 
		projectileInstanceTransforms.begin(), projectileInstanceTransforms.end(),
		[this](auto &&mat){ mat *= perFrameProjectileTransform; }
	);

	for (ui i = 0; i < projectileInstanceTransforms.size(); ++i)
		projectilePositions[i] = projectileInstanceTransforms[i] * glm::vec4(0, 0, 0, 1);

	auto projectileHitCallback = [this](ui projectileIndex)
	{
		projectileInstanceTransforms[projectileIndex] = projectileInstanceTransforms.back();
		projectilePositions[projectileIndex] = projectilePositions.back();
		projectileInstanceTransforms.pop_back();
		projectilePositions.pop_back();
		enemiesShotCounter++;
	};

	std::vector<glm::vec2> pcPositions = {{0,0}, {0,0}, {0,0}};
	for (ui i = 0; i < pcParams.noVertices; ++i)
		pcPositions[i] = glm::vec2(pcModel * glm::vec4(pcParams.positions[i], 1));

	enemyManager.RecordCollisions(projectilePositions, projectileHitCallback);
	enemyManager.RecordPCIntersection(pcPositions, [&isPcAlive, this]()
	{ 
		isPcAlive = false;
	});
	projectileMesh.SetInstanceCount(projectilePositions.size());
	projectileMesh.Update(&projectileInstanceTransforms[0], projectileMesh.InstancedBufferPosition());

	shader.Bind();
	shader.Update(blankTransform, camera);
	projectileMesh.Draw();
}

void Shooter::Shoot(const glm::mat4 &originTransform)
{
	const glm::vec4 originPosition = originTransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	if (projectilePositions.size() == maxProjectileAmount)
	{
		projectileInstanceTransforms[oldestProjectileIndex] = originTransform;
		projectilePositions[oldestProjectileIndex] = originPosition;
		oldestProjectileIndex = (oldestProjectileIndex + 1) % maxProjectileAmount;
	}
	else
	{
		projectileInstanceTransforms.push_back(originTransform);
		projectilePositions.push_back(originPosition);
	}
}

Shooter::~Shooter()
{
}
