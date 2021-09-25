#include "enemy.h"

EnemyBehaviuor::EnemyBehaviuor(EnemyStats &stats, Timer &timer, bool isDefault)
	: _enemyStats(stats), _timer(timer), _isDefault(isDefault)
{
}

BehavoiurStatus EnemyBehaviuor::EnemyBehaviuorStatus(const glm::mat4 &pcModel, const glm::mat4 &enemyModel)
{
	if (_isDefault)
		return BehavoiurStatus::DEFAULT;
	else if (HasMetPredicate(pcModel, enemyModel))
		return BehavoiurStatus::CHOSEN;
	else
		return BehavoiurStatus::NOT_CHOSEN;
}

void ChaseBehaviour::Update(const glm::mat4 &pcTransform, glm::mat4 &instanceTransform, [[maybe_unused]]std::vector<glm::mat4> &projInstanceTransforms)
{
	const glm::vec2 pcPos	{pcTransform 	   * glm::vec4(0,0,0,1)};
	const glm::vec2 enemyPos{instanceTransform * glm::vec4(0,0,0,1)};
	const glm::vec2 vecToPc {pcPos - enemyPos};
	const ft perFrameDistanceTraveled = decl_cast(perFrameDistanceTraveled, _timer.Scale(_enemyStats.speed));
	const glm::vec2 scaledVecToPc = helpers::scale2dVec(vecToPc, perFrameDistanceTraveled);
	const glm::mat4 perFrameEnemyTransform = glm::translate(glm::vec3(scaledVecToPc, 0));
	instanceTransform *= perFrameEnemyTransform;
}

void ShootBehavoiur::Update(const glm::mat4 &pcTransform, glm::mat4 &instanceTransform, std::vector<glm::mat4> &projInstanceTransforms)
{
	helpers::transformMatVec(projInstanceTransforms, _timer.Scale(_enemyStats.shotSpeed));
	if(_timer.HeapIsItTime(_shotClockId))
	{
		const glm::vec2 pcPos{pcTransform * glm::vec4(0,0,0,1)};
		const glm::vec2 enemyVec = glm::normalize(glm::vec2(glm::inverse(instanceTransform) * glm::vec4(pcPos, 0, 1)));
		const ft angle = -glm::atan(glm::dot({1.0, 0.0}, enemyVec), helpers::det({1.0, 0.0}, enemyVec));
		const glm::mat4 aimTransform = glm::rotate(angle, glm::vec3(0,0,1));
		const glm::mat4 initProjTransform = instanceTransform * aimTransform;
		helpers::pushToCappedVector(projInstanceTransforms, initProjTransform, _latestShotIndex, MAX_PROJ_AMOUNT_PER_ENEMY);
	}
}

bool ShootBehavoiur::HasMetPredicate(const glm::mat4 &pcModel, const glm::mat4 &enemyModel)
{
	const glm::vec2 pcPos	{pcModel 	* glm::vec4(0,0,0,1)};
	const glm::vec2 enemyPos{enemyModel * glm::vec4(0,0,0,1)};
	LOG(glm::distance(pcPos, enemyPos));
	if(glm::distance(pcPos, enemyPos) < 10.0f)
	{
		if(!_isActive)
		{
			_timer.InitHeapClock(_shotClockId, _enemyStats.shotDelay);
			_isActive = true;
		}
		return true;
	}
	else
	{
		if(_isActive)
		{
			_timer.DestroyHeapClock(_shotClockId);
			_isActive = false;
		}
		return false;
	}
}

EnemyData::EnemyData(Timer &timer) 
	: _timer(timer) 
{
	instanceTransforms		.reserve(MAX_NO_ENEMIES);
	boundingBoxes	  		.reserve(MAX_NO_ENEMIES);
	healths			  		.reserve(MAX_NO_ENEMIES);
	enemyBehaviours	  		.reserve(MAX_NO_ENEMIES);
	projInstanceTransforms	.reserve(MAX_NO_ENEMIES);
}

void EnemyData::Clear()
{
	instanceTransforms		.clear();
	boundingBoxes			.clear();
	healths					.clear();
	enemyBehaviours			.clear();
	projInstanceTransforms	.clear();

	size = 0;
}

void EnemyData::Erase(const ui index)
{
	instanceTransforms		[index] = 			instanceTransforms		[size - 1];
	boundingBoxes			[index] = 			boundingBoxes				[size - 1];
	enemyBehaviours			[index] = std::move(enemyBehaviours			[size - 1]);
	projInstanceTransforms	[index] = 			projInstanceTransforms	[size - 1];
	healths					[index] = 			healths					[size - 1];
	instanceTransforms		.pop_back();
	boundingBoxes			.pop_back();
	enemyBehaviours			.pop_back();
	projInstanceTransforms	.pop_back();
	healths					.pop_back();

	size--;
}

void EnemyData::Push(const glm::mat4 &instanceTransform, const UntexturedMeshParams &params, EnemyStats &stats, behavoiurPtrVec &behaviours)
{
	instanceTransforms		.push_back(instanceTransform);
	boundingBoxes			.emplace_back(params, instanceTransform);
	enemyBehaviours			.push_back(std::move(behaviours));
	projInstanceTransforms	.emplace_back();
	healths					.push_back(stats.health);
	
	size++;
}

void EnemyData::Update(const glm::mat4 &pcModel, const ui index)
{
	const auto chosenBehavoiurIter = choseBehaviour(enemyBehaviours[index], pcModel, instanceTransforms[index]);
	chosenBehavoiurIter->get()->Update(pcModel, instanceTransforms[index], projInstanceTransforms[index]);
	boundingBoxes[index].minCoords = glm::vec2(instanceTransforms[index] * glm::vec4(boundingBoxes[index].minDimentions, 0, 1));
	boundingBoxes[index].maxCoords = glm::vec2(instanceTransforms[index] * glm::vec4(boundingBoxes[index].maxDimentions, 0, 1));
}

EnemyManager::EnemyManager(Shader &enemyShader, helpers::Core &core, const UntexturedMeshParams &params, EnemyStats &enemyStats)
 : _enemyShader(enemyShader), _camera(core.camera), _pcStats(core.stats), _timer(core.timer), _enemyData(core.timer),
 	 _enemyMeshParams(params), _enemyMesh(params, MAX_NO_ENEMIES), _enemyStats(enemyStats)
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
			_enemyData.healths[i] -= decl_cast(_enemyData.healths, _pcStats.actualDamage);
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
	for (ui i = 0; i < _enemyData.size; ++i)
		_enemyData.Update(pcModel, i);
}

void EnemyManager::Spawn(const glm::mat4 &pcModel)
{
	if(_enemyData.size >= MAX_NO_ENEMIES)
		return;

	glm::vec2 pcPos{pcModel * glm::vec4(0,0,0,1)};
	const ft enemyX = _customRand.NextUi() % 2 ? _customRand.NextFloat(pcPos.x - 9.0f,  pcPos.x - 11.0f) : _customRand.NextFloat(pcPos.x + 9.0f, pcPos.x + 11.0f);
	const ft enemyY = _customRand.NextUi() % 2 ? _customRand.NextFloat(pcPos.y - 9.0f,  pcPos.y - 11.0f) : _customRand.NextFloat(pcPos.y + 9.0f, pcPos.y + 11.0f);

	behavoiurPtrVec behaviours;
	behaviours.push_back(std::make_unique<ChaseBehaviour>(_enemyStats, _timer));
	if(!(_customRand.NextUi() % 10))
	{
		behaviours.push_back(std::make_unique<ShootBehavoiur>(_enemyStats, _timer));
	}

	glm::mat4 instanceTransform = _enemyTransform.Model() * glm::translate(glm::vec3(enemyX, enemyY, 0));
	_enemyData.Push(instanceTransform, _enemyMeshParams, _enemyStats, behaviours);
} 
