#include "enemy.h"
#include <iostream>

bool BoundingBox::IsThereAnIntersection(const glm::vec2 &vec) const
{
	return vec.x >= botLeftPosRel.x  &&
	vec.x <= topRightPosRel.x &&
	vec.y >= botLeftPosRel.y  &&
	vec.y <= topRightPosRel.y;
}

Enemy::Enemy(const glm::vec3 *positions, const glm::mat4 &instanceTransform, const ui noVertices, EnemyManager *manager)
: manager(manager), health(100)
{
	ft maxX = -100.0f, minX = 100.0f, maxY = -100.0f, minY = 100.0f;
	for (ui i = 0; i < noVertices; ++i)
	{
		if (positions[i][0] > maxX) maxX = positions[i][0];
		if (positions[i][0] < minX) minX = positions[i][0];
		if (positions[i][1] > maxY) maxY = positions[i][1];
		if (positions[i][1] < minY) minY = positions[i][1];
	}

	const glm::vec2 minDimentions = glm::vec2(minX, minY);
	const glm::vec2 maxDimentions = glm::vec2(maxX, maxY);
	const glm::vec2 minCoords = instanceTransform * glm::vec4(minDimentions, 0, 1);
	const glm::vec2 maxCoords = instanceTransform * glm::vec4(maxDimentions, 0, 1);

	boundingBox = BoundingBox{minDimentions, maxDimentions, minCoords, maxCoords};
}

Enemy::CollisionCheckEnum Enemy::CheckForProjectileCollision(const glm::vec2 &projectilePos, const ui projectileDamage)
{
	bool didCollisionOccur = boundingBox.IsThereAnIntersection(projectilePos);

	if(didCollisionOccur)
	{
		health -= projectileDamage;
		if(!health)
			return FATALITY;
		return COLLISION;
	}

	return NO_COLLISION;
}

void Enemy::UpdateBehaviour(const glm::mat4 &instanceTransform)
{
	boundingBox.botLeftPosRel  = instanceTransform * glm::vec4(boundingBox.botLeftPos,  0, 1);
	boundingBox.topRightPosRel = instanceTransform * glm::vec4(boundingBox.topRightPos, 0, 1);
}

Enemy::~Enemy()
{
}

EnemyManager::EnemyManager(Shader &enemyShader, Camera &camera, const UntexturedMeshParams &params, ui seed, ui maxNoEnemies)
 : enemyShader(enemyShader), camera(camera), enemyMesh(params, maxNoEnemies, 4), customRand(seed), params(params), maxNoEnemies(maxNoEnemies)
{
	enemyTransform.Scale() *= 0.05;
}

void EnemyManager::RecordCollisions(const std::vector<glm::vec2> &projectilePositions, const std::function<void(ui)> projectileHitCallback)
{
	for (ui i = 0; i < projectilePositions.size(); ++i)
		for (Enemy *enemyPtr : enemies)
		{
			Enemy::CollisionCheckEnum collisionStatus = enemyPtr->CheckForProjectileCollision(projectilePositions[i]);
			if(collisionStatus == Enemy::CollisionCheckEnum::COLLISION)
			{
				projectileHitCallback(i);
				break;
			}
			if(collisionStatus == Enemy::CollisionCheckEnum::FATALITY)
			{
				projectileHitCallback(i);
				ui enemyIndex = enemyPtr->managerIndex;
				Despawn(enemyIndex);
				break;
			}
		}
}

void EnemyManager::RecordPCIntersection(const std::vector<glm::vec2> &pcPositions, const std::function<void()> intersectionCallback)
{
	for (Enemy *enemyPtr : enemies)
	{
		const BoundingBox &enemyBoundingBox = enemyPtr->EnemyBoundingBox();
		bool isThereAnIntersection = std::any_of(pcPositions.begin(), pcPositions.end(), 
		[&enemyBoundingBox](auto &vec){ return enemyBoundingBox.IsThereAnIntersection(vec); });
		if(isThereAnIntersection)
		{
			intersectionCallback();
			break;
		}
	}
}

void EnemyManager::Reset()
{
	for(auto p : enemies)
		delete p;
	enemies.clear();
	enemyInstanceTransforms.clear();
}

void EnemyManager::Draw()
{
	enemyShader.Bind();
	enemyShader.Update(blankTransform, camera);

	enemyMesh.SetInstanceCount(enemies.size());
	enemyMesh.Update(&enemyInstanceTransforms[0], enemyMesh.InstancedBufferPosition());
	enemyMesh.Draw();
}

void EnemyManager::UpdateBehaviour(const glm::mat4 &pcModel)
{
	const glm::vec2 pcPos = pcModel * glm::vec4(0, 0, 0, 1);

	for (ui i = 0; i < enemies.size(); ++i)
	{
		const glm::vec2 enemyPos = enemyInstanceTransforms[i] * glm::vec4(0, 0, 0, 1);
		glm::vec2 vecToPc = pcPos - enemyPos;
		scale2dVec(vecToPc, enemyPerFrameDistance);

		const glm::mat4 localTransform = glm::translate(glm::vec3(vecToPc, 0));
		enemyInstanceTransforms[i] *= localTransform;
		enemies[i]->UpdateBehaviour(enemyInstanceTransforms[i]);
	}
}

void EnemyManager::Spawn(const glm::mat4 &pcModel)
{
	if(enemies.size() >= maxNoEnemies)
		return;

	glm::vec2 pcPos = pcModel * glm::vec4(0,0,0,1);
	pcPos *= 20;
	const ft enemyX = customRand.NextUi() % 2 ? customRand.NextFloat(pcPos.x - 3.0f,  pcPos.x - 5.0f) : customRand.NextFloat(pcPos.x + 3.0f, pcPos.x + 5.0f);
	const ft enemyY = customRand.NextUi() % 2 ? customRand.NextFloat(pcPos.y - 3.0f,  pcPos.y - 5.0f) : customRand.NextFloat(pcPos.y + 3.0f, pcPos.y + 5.0f);

	glm::mat4 instanceTransform = enemyTransform.Model() * glm::translate(glm::vec3(enemyX, enemyY, 0));
	Enemy *enemyPtr = new Enemy(params.positions, instanceTransform, params.noVertices, this);

	enemyPtr->SetManagerIndex(enemies.size());
	enemies.push_back(enemyPtr);
	enemyInstanceTransforms.push_back(instanceTransform);
} 

void EnemyManager::Despawn(const ui enemyIndex)
{
	ui enemiesSize = enemies.size();
	delete enemies[enemyIndex];
	for (ui i = enemyIndex + 1; i < enemiesSize; ++i)
	{
		enemies[i]->managerIndex--;
		enemies[i-1] = enemies[i];
		enemyInstanceTransforms[i-1] = enemyInstanceTransforms[i];
	}
	enemies.pop_back();
	enemyInstanceTransforms.pop_back();
}

EnemyManager::~EnemyManager()
{
}

void EnemyManager::scale2dVec(glm::vec2 &vecToScale, const ft scaleToLength)
{
	ft h = hypot(vecToScale.x, vecToScale.y);
	vecToScale.x = scaleToLength * (vecToScale.x / h);
	vecToScale.y = scaleToLength * (vecToScale.y / h);
}
