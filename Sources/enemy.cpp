#include "enemy.h"

void EnemyData::Clear()
{
	instanceTransforms.clear();
	boundingBoxes.clear();
	healths.clear();
	size = 0;
}

void EnemyData::Erase(const ui index)
{
	for (ui i = index + 1; i < size; ++i)
	{
		instanceTransforms[i-1] = instanceTransforms[i];
		boundingBoxes[i-1] = boundingBoxes[i];
		healths[i-1] = healths[i];
	}
	instanceTransforms.pop_back();
	boundingBoxes.pop_back();
	healths.pop_back();
	size--;
}

void EnemyData::Push(const glm::mat4 &instanceTransform, const UntexturedMeshParams &params, const si health)
{
	instanceTransforms.push_back(instanceTransform);
	boundingBoxes.push_back(helpers::BoundingBox(params, instanceTransform));
	healths.push_back(health);
	size++;
}

void EnemyData::Update(const ui index)
{
	boundingBoxes[index].minCoords = glm::vec2(instanceTransforms[index] * glm::vec4(boundingBoxes[index].minDimentions, 0, 1));
	boundingBoxes[index].maxCoords = glm::vec2(instanceTransforms[index] * glm::vec4(boundingBoxes[index].maxDimentions, 0, 1));
}

EnemyManager::EnemyManager(Shader &enemyShader, helpers::Core &core, const UntexturedMeshParams &params)
 : _camera(core.camera), _pcStats(core.stats), _timer(core.timer), _customRand(CUSTOM_RAND_SEED),
 	_enemyShader(enemyShader), _enemyMeshParams(params), _enemyMesh(params, MAX_NO_ENEMIES), _maxNoEnemies(MAX_NO_ENEMIES)
{
}

void EnemyManager::RecordCollisions(const std::vector<glm::mat4> &projectileTransforms,
	const std::function<void(ui)> projectileHitCallback, std::function<void(const glm::mat4&, const ui)> fatalityCallback)
{
	ui collisionIndex;
	for(ui i = 0; i < _enemyData.size; ++i)
	{
		if(_enemyData.boundingBoxes[i].IsThereAnIntersection(projectileTransforms, collisionIndex))
		{
			projectileHitCallback(collisionIndex);
			_enemyData.healths[i] -= _pcStats.actualDamage;
			if(_enemyData.healths[i] <= 0)
			{
				fatalityCallback(_enemyData.instanceTransforms[i], 3);
				_enemyData.Erase(i);
			}
		}
	}
}

void EnemyManager::RecordPCIntersection(const std::vector<glm::vec2> &pcPositions, const std::function<void()> intersectionCallback)
{
	for (auto &enemyBoundingBox : _enemyData.boundingBoxes)
	{
		if(enemyBoundingBox.IsThereAnIntersection(pcPositions))
		{
			intersectionCallback();
			break;
		}
	}
}

void EnemyManager::Reset()
{
	_enemyData.Clear();
}

void EnemyManager::Draw()
{
	helpers::render(_enemyShader, _enemyMesh, _enemyData.instanceTransforms.data(), _enemyData.instanceTransforms.size(), _blankTransform, 
		_camera.ViewProjection());
}

void EnemyManager::UpdateBehaviour(const glm::mat4 &pcModel)
{
	const glm::vec2 pcPos = glm::vec2(pcModel * glm::vec4(0, 0, 0, 1));
	const db scaledPerFrameTravelDistance = _timer.Scale(_enemyPerFrameDistance);
	for (ui i = 0; i < _enemyData.size; ++i)
	{
		const glm::vec2 enemyPos = glm::vec2(_enemyData.instanceTransforms[i] * glm::vec4(0, 0, 0, 1));
		glm::vec2 vecToPc = pcPos - enemyPos;
		helpers::scale2dVec(vecToPc, scaledPerFrameTravelDistance);

		const glm::mat4 localTransform = glm::translate(glm::vec3(vecToPc, 0));
		_enemyData.instanceTransforms[i] *= localTransform;
		_enemyData.Update(i);
	}
}

void EnemyManager::Spawn(const glm::mat4 &pcModel)
{
	if(_enemyData.size >= _maxNoEnemies)
		return;

	glm::vec2 pcPos = glm::vec2(pcModel * glm::vec4(0,0,0,1));
	const ft enemyX = _customRand.NextUi() % 2 ? _customRand.NextFloat(pcPos.x - 9.0f,  pcPos.x - 11.0f) : _customRand.NextFloat(pcPos.x + 9.0f, pcPos.x + 11.0f);
	const ft enemyY = _customRand.NextUi() % 2 ? _customRand.NextFloat(pcPos.y - 9.0f,  pcPos.y - 11.0f) : _customRand.NextFloat(pcPos.y + 9.0f, pcPos.y + 11.0f);

	glm::mat4 instanceTransform = _enemyTransform.Model() * glm::translate(glm::vec3(enemyX, enemyY, 0));
	_enemyData.Push(instanceTransform, _enemyMeshParams);
} 
