#include "enemy.h"

EnemyBehaviuor::EnemyBehaviuor(EnemyStats &stats, Timer &timer, const bool isDefault)
	: _enemyStats(stats), _timer(timer), _isDefault(isDefault)
{
}

BehavoiurStatus EnemyBehaviuor::EnemyBehaviuorStatus(const glm::mat4 &pcModel, const glm::mat4 &enemyModel)
{
	if(_isDefault)
		return BehavoiurStatus::DEFAULT;
	else if (HasMetPredicate(pcModel, enemyModel))
		return BehavoiurStatus::CHOSEN;
	else
		return BehavoiurStatus::NOT_CHOSEN;
}

void ChaseBehaviour::Update(const glm::mat4 &pcTransform, glm::mat4 &instanceTransform, std::vector<glm::mat4> &projInstanceTransforms, 
	const TriBoundingBox &pcBoundingBox, const std::function<void()> intersectionCallback)
{
	const glm::vec2 pcPos	{pcTransform 	   * glm::vec4(0,0,0,1)};
	const glm::vec2 enemyPos{instanceTransform * glm::vec4(0,0,0,1)};
	const glm::vec2 vecToPc {pcPos - enemyPos};
	const ft perFrameDistanceTraveled = decl_cast(perFrameDistanceTraveled, _timer.Scale(_enemyStats.speed));
	const glm::vec2 scaledVecToPc = helpers::scale2dVec(vecToPc, perFrameDistanceTraveled);
	const glm::mat4 perFrameEnemyTransform = glm::translate(glm::vec3(scaledVecToPc, 0));
	instanceTransform *= perFrameEnemyTransform;
	if(!projInstanceTransforms.empty())
	{
		helpers::transformMatVec(projInstanceTransforms, _timer.Scale(_enemyStats.shotSpeed));
		ui intersectionIndex;
		if(pcBoundingBox.IsThereAnIntersection(projInstanceTransforms, intersectionIndex))
		{
			projInstanceTransforms.erase(projInstanceTransforms.begin() + intersectionIndex);
			intersectionCallback();
		}
	}
}

void ShootBehavoiur::Update(const glm::mat4 &pcTransform, glm::mat4 &instanceTransform, std::vector<glm::mat4> &projInstanceTransforms, 
	const TriBoundingBox &pcBoundingBox, const std::function<void()> intersectionCallback)
{
	if(!projInstanceTransforms.empty())
	{
		helpers::transformMatVec(projInstanceTransforms, _timer.Scale(_enemyStats.shotSpeed));
		ui intersectionIndex;
		if(pcBoundingBox.IsThereAnIntersection(projInstanceTransforms, intersectionIndex))
		{
			projInstanceTransforms.erase(projInstanceTransforms.begin() + intersectionIndex);
			intersectionCallback();
		}
	}
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
	if(glm::distance(pcPos, enemyPos) < 15.0f)
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

EnemyData::EnemyData()
{
	instanceTransforms		.reserve(MAX_NO_ENEMIES);
	boundingBoxes	  		.reserve(MAX_NO_ENEMIES);
	healths			  		.reserve(MAX_NO_ENEMIES);
	projInstanceTransforms	.reserve(MAX_NO_ENEMIES);
}

void EnemyData::Clear()
{
	instanceTransforms		.clear();
	boundingBoxes			.clear();
	healths					.clear();
	projInstanceTransforms	.clear();

	size = 0;
}

void EnemyData::Erase(const ui index)
{
	instanceTransforms		[index] = instanceTransforms	[size - 1];
	boundingBoxes			[index] = boundingBoxes			[size - 1];
	projInstanceTransforms	[index] = projInstanceTransforms[size - 1];
	healths					[index] = healths				[size - 1];
	instanceTransforms		.pop_back();
	boundingBoxes			.pop_back();
	projInstanceTransforms	.pop_back();
	healths					.pop_back();

	size--;
}

void EnemyData::Push(const glm::mat4 &instanceTransform, const UntexturedMeshParams &params, EnemyStats &stats)
{
	instanceTransforms		.push_back(instanceTransform);
	boundingBoxes			.emplace_back(params, instanceTransform);
	projInstanceTransforms	.emplace_back();
	healths					.push_back(stats.health);
	
	size++;
}

Enemy::Enemy(Timer &timer, Shader &enemyShader, EnemyStats &stats, const UntexturedMeshParams &params, behavoiurPtrVec &behaviours, 
		const glm::vec3 &colour, const ui maxNoInstances, Shader *projShaderPtr, const UntexturedMeshParams *projParamsPtr)
	: _timer(timer), _enemyShader(enemyShader), _stats(stats), _params(params), _mesh(_params, maxNoInstances), _behaviours(std::move(behaviours)), 
		_colourUni("colour", colour), _maxNoInstances(maxNoInstances), _projShaderPtr(projShaderPtr), _projParamsPtr(projParamsPtr)
{
	if(_projParamsPtr != nullptr)
		_projMeshPtr.reset(new UntexturedInstancedMesh(*_projParamsPtr));
}

void Enemy::Update(const glm::mat4 &pcModel, const TriBoundingBox &pcBoundingBox, const std::function<void()> intersectionCallback)
{
	for(ui i = 0; i < data.size; ++i)
	{
		const auto chosenBehavoiurIter = choseBehaviour(_behaviours, pcModel, data.instanceTransforms[i]);
		chosenBehavoiurIter->get()->Update(pcModel, data.instanceTransforms[i], data.projInstanceTransforms[i], pcBoundingBox, intersectionCallback);
		data.boundingBoxes[i].UpdateCoords(data.instanceTransforms[i]);
	}
}

void Enemy::Spawn(const glm::mat4 &instanceTransform)
{
	if(_maxNoInstances > data.size)
		data.Push(instanceTransform, _params, _stats);
}

void Enemy::Draw(const glm::mat4 &cameraProjection)
{
	helpers::render(_enemyShader, _mesh, data.instanceTransforms.data(), data.size, _blankTransform, cameraProjection, _colourUni);
	if(_projShaderPtr != nullptr)
	{
		const auto flattenedVec = helpers::flattenVec(data.projInstanceTransforms);
		const std::size_t noProjectiles = helpers::twoDVecSize(data.projInstanceTransforms);
		helpers::render(*_projShaderPtr, *_projMeshPtr.get(), flattenedVec.data(), noProjectiles, _blankTransform, cameraProjection);
	}
}

EnemyManager::EnemyManager(Shader &enemyShader, helpers::Core &core, const UntexturedMeshParams &params, EnemyStats &enemyStats, 
	const UntexturedMeshParams &projParams)
 : _enemyShader(enemyShader), _camera(core.camera), _pcStats(core.stats), _timer(core.timer), _enemyStats(enemyStats)
{
	behavoiurPtrVec chaserVec;
	behavoiurPtrVec shooterVec;
	_enemies.reserve(EnemyTypeEnum::NO_ENEMY_TYPES);
	chaserVec .push_back(std::make_unique<ChaseBehaviour>(_enemyStats, _timer));
	shooterVec.push_back(std::make_unique<ChaseBehaviour>(_enemyStats, _timer));
	shooterVec.push_back(std::make_unique<ShootBehavoiur>(_enemyStats, _timer));
	_enemies.emplace_back(_timer, _enemyShader, _enemyStats, params, chaserVec,  glm::vec3(0.25f, 0.57f, 0.38f), MAX_NO_ENEMIES);
	_enemies.emplace_back(_timer, _enemyShader, _enemyStats, params, shooterVec, glm::vec3(0.63f, 0.16f, 0.16f), MAX_NO_SHOOTER_ENEMIES, 
		&_enemyProjShader, &projParams);
}

void EnemyManager::RecordCollisions(const std::vector<glm::mat4> &projectileTransforms,
	const std::function<void(ui)> projectileHitCallback, std::function<void(const glm::mat4&, const ui)> fatalityCallback)
{
	for(auto &enemy : _enemies)
	{
		ui collisionIndex;
		for(ui i = 0; i < enemy.data.size; ++i)
		{
			if(enemy.data.boundingBoxes[i].IsThereAnIntersection(projectileTransforms, collisionIndex))
			{
				projectileHitCallback(collisionIndex);
				enemy.data.healths[i] -= decl_cast(enemy.data.healths, _pcStats.actualDamage);
				if(enemy.data.healths[i] <= 0)
				{
					fatalityCallback(enemy.data.instanceTransforms[i], 3);
					enemy.data.Erase(i);
				}
			}
		}
	}
}

void EnemyManager::RecordPCIntersection(const std::vector<glm::vec2> &pcPositions, const std::function<void()> intersectionCallback)
{
	for(auto &enemy : _enemies)
	{
		for (auto &enemyBoundingBox : enemy.data.boundingBoxes)
		{
			if(enemyBoundingBox.IsThereAnIntersection(pcPositions))
			{
				intersectionCallback();
				break;
			}
		}
	}
}

void EnemyManager::Reset()
{
	for(auto &enemy : _enemies)
		enemy.data.Clear();
}

void EnemyManager::Draw()
{
	for(auto &enemy : _enemies)
		enemy.Draw(_camera.ViewProjection());
}

void EnemyManager::UpdateBehaviour(const glm::mat4 &pcModel, const TriBoundingBox &pcBoundingBox, const std::function<void()> intersectionCallback)
{
	for(auto &enemy : _enemies)
		enemy.Update(pcModel, pcBoundingBox, intersectionCallback);
}

std::vector<std::vector<glm::mat4>*> EnemyManager::InstanceTransforms()
{
	std::vector<std::vector<glm::mat4>*> enemyInstanceTransforms;
	for(auto &enemy : _enemies)
		enemyInstanceTransforms.push_back(&enemy.data.instanceTransforms);
	return enemyInstanceTransforms;
}

void EnemyManager::Spawn(const glm::mat4 &pcModel)
{
	glm::vec2 pcPos{pcModel * glm::vec4(0,0,0,1)};
	const ft enemyX = _customRand.NextUi() % 2 ? _customRand.NextFloat(pcPos.x - 9.0f, pcPos.x - 11.0f) : 
												 _customRand.NextFloat(pcPos.x + 9.0f, pcPos.x + 11.0f) ;
	const ft enemyY = _customRand.NextUi() % 2 ? _customRand.NextFloat(pcPos.y - 9.0f, pcPos.y - 11.0f) : 
												 _customRand.NextFloat(pcPos.y + 9.0f, pcPos.y + 11.0f) ;

	glm::mat4 instanceTransform = _enemyTransform.Model() * glm::translate(glm::vec3(enemyX, enemyY, 0));
	if(!(_customRand.NextUi() % 10))
		_enemies[SHOOTER_ENEMY].Spawn(instanceTransform);
	else
		_enemies[CHASER_ENEMY] .Spawn(instanceTransform);
}
