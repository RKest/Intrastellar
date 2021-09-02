#include "enemy.h"

EnemyManager::EnemyManager(Shader &enemyShader, Camera &camera, Timer &timer, Stats &pcStats,
	const UntexturedMeshParams &params, ui seed, ui maxNoEnemies)
 : _enemyShader(enemyShader), _camera(camera), _enemyMesh(params, maxNoEnemies, 4), _pcStats(pcStats),
 	_customRand(seed), _enemyMeshParams(params), _maxNoEnemies(maxNoEnemies), _timer(timer)
{
}

void EnemyManager::RecordCollisions(const std::vector<glm::mat4> &projectileTransforms,
	const std::function<void(ui)> projectileHitCallback, std::function<void(const glm::mat4&, const ui)> fatalityCallback)
{
	ui collisionIndex;
	for(ui i = 0; i < _enemyBoundingBoxes.size(); ++i)
	{
		if(_enemyBoundingBoxes[i].IsThereAnIntersection(projectileTransforms), collisionIndex)
		{
			projectileHitCallback(collisionIndex);
			_enemyHealths[i] -= _pcStats.Damage();
			if(_enemyHealths[i] < 0)
			{
				fatalityCallback(i);
				Despawn(i);
			}
		}
	}
}

void EnemyManager::RecordPCIntersection(const std::vector<glm::vec2> &pcPositions, const std::function<void()> intersectionCallback)
{
	for (auto &enemyBoundingBox : _enemyBoundingBoxes)
	{
		if(enemyBoundingBox.IsThereAnIntersection(pcPositions, _pcStats.Damage()))
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
	_enemyInstanceTransforms.clear();
	_enemyBoundingBoxes.clear();
	_enemyHealths.clear();
}

void EnemyManager::Draw()
{
	helpers::render(_enemyShader, _enemyMesh, _enemyInstanceTransforms.data(), _enemyInstanceTransforms.size(), _blankTransform, _camera.ViewProjection());
}

void EnemyManager::UpdateBehaviour(const glm::mat4 &pcModel)
{
	const glm::vec2 pcPos = glm::vec2(pcModel * glm::vec4(0, 0, 0, 1));
	_scaledPerFrameTravelDistance = _timer.Scale(_enemyPerFrameDistance);
	for (ui i = 0; i < _enemyInstanceTransforms.size(); ++i)
	{
		const glm::vec2 enemyPos = glm::vec2(_enemyInstanceTransforms[i] * glm::vec4(0, 0, 0, 1));
		glm::vec2 vecToPc = pcPos - enemyPos;
		helpers::scale2dVec(vecToPc, _scaledPerFrameTravelDistance);

		const glm::mat4 localTransform = glm::translate(glm::vec3(vecToPc, 0));
		_enemyInstanceTransforms[i] *= localTransform;
		_enemyBoundingBoxes[i].minCoords = glm::vec2(_enemyInstanceTransforms[i] * glm::vec4(_enemyBoundingBoxes.minDimentions, 0, 1));
		_enemyBoundingBoxes[i].maxCoords = glm::vec2(_enemyInstanceTransforms[i] * glm::vec4(_enemyBoundingBoxes.maxDimentions, 0, 1));
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
	_enemyBoundingBoxes.push_back(helpers::BoundingBox(_enemyMeshParams, instanceTransform));
	_enemyInstanceTransforms.push_back(instanceTransform);
	_enemyHealths.push_back(100);
} 

void EnemyManager::Despawn(const ui enemyIndex)
{
	ui enemiesSize = _enemyInstanceTransforms.size();
	for (ui i = enemyIndex + 1; i < enemiesSize; ++i)
	{
		_enemyInstanceTransforms[i-1] = _enemyInstanceTransforms[i];
		_enemyBoundingBoxes[i-1] = _enemyBoundingBoxes[i];
		_enemyHealths[i-1] = _enemyHealths[i];
	}
	_enemyInstanceTransforms.pop_back();
	_enemyHealths.pop_back();
	_enemyBoundingBoxes.pop_back();
}