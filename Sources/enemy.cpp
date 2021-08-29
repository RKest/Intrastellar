#include "enemy.h"

Enemy::Enemy(const UntexturedMeshParams &params, const glm::mat4 &instanceTransform, EnemyManager *manager)
: manager(manager), health(100)
{
	boundingBox = helpers::BoundingBox(params, instanceTransform);
}

Enemy::CollisionCheckEnum Enemy::CheckForProjectileCollision(const glm::vec2 &projectilePos, const ui projectileDamage)
{
	if(boundingBox.IsThereAnIntersection(projectilePos))
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
	boundingBox.minCoords = instanceTransform * glm::vec4(boundingBox.minDimentions,  0, 1);
	boundingBox.maxCoords = instanceTransform * glm::vec4(boundingBox.maxDimentions, 0, 1);
}

EnemyManager::EnemyManager(Shader &enemyShader, Camera &camera, Timer &timer, 
	const UntexturedMeshParams &params, ui seed, ui maxNoEnemies)
 : _enemyShader(enemyShader), _camera(camera), _enemyMesh(params, maxNoEnemies, 4), 
 	_customRand(seed), _enemyMeshParams(params), _maxNoEnemies(maxNoEnemies), _timer(timer)
{
}

void EnemyManager::RecordCollisions(const std::vector<glm::mat4> &projectileTransforms,
	const std::function<void(ui)> projectileHitCallback, std::function<void(const glm::mat4&, const ui)> fatalityCallback)
{
	for (ui i = 0; i < projectileTransforms.size(); ++i) 
	{
		const glm::vec2 projectilePosition = glm::vec2(projectileTransforms[i] * glm::vec4(0,0,0,1));
		for (Enemy *enemyPtr : _enemies)
		{
			Enemy::CollisionCheckEnum collisionStatus = enemyPtr->CheckForProjectileCollision(projectilePosition);
			if(collisionStatus == Enemy::CollisionCheckEnum::COLLISION)
			{
				projectileHitCallback(i);
				break;
			}
			if(collisionStatus == Enemy::CollisionCheckEnum::FATALITY)
			{
				projectileHitCallback(i);
				fatalityCallback(_enemyInstanceTransforms[enemyPtr->managerIndex], 3);
				Despawn(enemyPtr->managerIndex);
				break;
			}
		}
	}
}

void EnemyManager::RecordPCIntersection(const std::vector<glm::vec2> &pcPositions, const std::function<void()> intersectionCallback)
{
	for (Enemy *enemyPtr : _enemies)
	{
		const helpers::BoundingBox &enemyBoundingBox = enemyPtr->EnemyBoundingBox();
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
	for(auto p : _enemies)
		delete p;
	_enemies.clear();
	_enemyInstanceTransforms.clear();
}

void EnemyManager::Draw()
{
	helpers::render(_enemyShader, _enemyMesh, _enemyInstanceTransforms.data(), _enemies.size(), _blankTransform, _camera.ViewProjection());
}

void EnemyManager::UpdateBehaviour(const glm::mat4 &pcModel)
{
	const glm::vec2 pcPos = glm::vec2(pcModel * glm::vec4(0, 0, 0, 1));
	_scaledPerFrameTravelDistance = _timer.Scale(_enemyPerFrameDistance);
	for (ui i = 0; i < _enemies.size(); ++i)
	{
		const glm::vec2 enemyPos = glm::vec2(_enemyInstanceTransforms[i] * glm::vec4(0, 0, 0, 1));
		glm::vec2 vecToPc = pcPos - enemyPos;
		helpers::scale2dVec(vecToPc, _scaledPerFrameTravelDistance);

		const glm::mat4 localTransform = glm::translate(glm::vec3(vecToPc, 0));
		_enemyInstanceTransforms[i] *= localTransform;
		_enemies[i]->UpdateBehaviour(_enemyInstanceTransforms[i]);
	}
}

void EnemyManager::Spawn(const glm::mat4 &pcModel)
{
	if(_enemies.size() >= _maxNoEnemies)
		return;

	glm::vec2 pcPos = glm::vec2(pcModel * glm::vec4(0,0,0,1));
	const ft enemyX = _customRand.NextUi() % 2 ? _customRand.NextFloat(pcPos.x - 9.0f,  pcPos.x - 11.0f) : _customRand.NextFloat(pcPos.x + 9.0f, pcPos.x + 11.0f);
	const ft enemyY = _customRand.NextUi() % 2 ? _customRand.NextFloat(pcPos.y - 9.0f,  pcPos.y - 11.0f) : _customRand.NextFloat(pcPos.y + 9.0f, pcPos.y + 11.0f);

	glm::mat4 instanceTransform = _enemyTransform.Model() * glm::translate(glm::vec3(enemyX, enemyY, 0));
	Enemy *enemyPtr = new Enemy(_enemyMeshParams, instanceTransform, this);

	enemyPtr->SetManagerIndex(_enemies.size());
	_enemies.push_back(enemyPtr);
	_enemyInstanceTransforms.push_back(instanceTransform);
} 

void EnemyManager::Despawn(const ui enemyIndex)
{
	ui enemiesSize = _enemies.size();
	delete _enemies[enemyIndex];
	for (ui i = enemyIndex + 1; i < enemiesSize; ++i)
	{
		_enemies[i]->managerIndex--;
		_enemies[i-1] = _enemies[i];
		_enemyInstanceTransforms[i-1] = _enemyInstanceTransforms[i];
	}
	_enemies.pop_back();
	_enemyInstanceTransforms.pop_back();
}