#include "enemy.h"

EnemyBehaviuor::EnemyBehaviuor(EnemyStats &enemyStats, Timer &timer)
	: _enemyStats(enemyStats), _timer(timer), _behavoirPredicate(defBehaviourPredicate), _isDefault(true)
{
	_doesShoot = FP_ZERO != std::fpclassify(_enemyStats.shotDelay);
}
EnemyBehaviuor::EnemyBehaviuor(EnemyStats &enemyStats, Timer &timer, const behavourPredicate_t behavoirPredicate)
	: _enemyStats(enemyStats), _timer(timer), _behavoirPredicate(behavoirPredicate)
{
	_doesShoot = FP_ZERO != std::fpclassify(_enemyStats.shotDelay);
}

bool EnemyBehaviuor::IsChosen(const glm::mat4 &pcModel) 
{
	if(_behavoirPredicate(pcModel))
	{
		if(!_isActive && _doesShoot)
		{
			_timer.InitHeapClock(_shotClockId, _enemyStats.shotDelay);
			_isActive = true;
		}
		return true;
	}
	else
	{
		if(_isActive && _doesShoot)
		{
			_timer.DestroyHeapClock(_shotClockId);
			_isActive = false;
		}
		return false;
	}		
}

void EnemyBehaviuor::Update(const glm::mat4 &pcTransform, glm::mat4 &instanceTransform, std::vector<glm::mat4> &projInstanceTransforms)
{
	const glm::vec2 pcPos{pcTransform * glm::vec4(0,0,0,1)};
	const glm::vec2 enemyPos{instanceTransform * glm::vec4(0,0,0,1)};
	const glm::vec2 vecToPc{pcPos - enemyPos};
	const ft perFrameDistanceTraveled = decl_cast(perFrameDistanceTraveled, _timer.Scale(_enemyStats.speed));
	const glm::vec2 scaledVecToPc = helpers::scale2dVec(vecToPc, perFrameDistanceTraveled);
	const glm::mat4 perFrameEnemyTransform = glm::translate(glm::vec3(scaledVecToPc, 0));
	instanceTransform *= perFrameEnemyTransform;

	if(!_doesShoot)
		return;
	if(_timer.HeapIsItTime(_shotClockId))
		helpers::pushToCappedVector(projInstanceTransforms, instanceTransform, _oldestProjIndex, MAX_PROJ_AMOUNT_PER_ENEMY);
	
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
	instanceTransforms		[index] = instanceTransforms		[size - 1];
	boundingBoxes			[index] = boundingBoxes				[size - 1];
	enemyBehaviours			[index] = enemyBehaviours			[size - 1];
	projInstanceTransforms	[index] = projInstanceTransforms	[size - 1];
	healths					[index] = healths					[size - 1];
	instanceTransforms		.pop_back();
	boundingBoxes			.pop_back();
	enemyBehaviours			.pop_back();
	projInstanceTransforms	.pop_back();
	healths					.pop_back();

	size--;
}

void EnemyData::Push(const glm::mat4 &instanceTransform, const UntexturedMeshParams &params, EnemyStats &stats, Timer &timer)
{
	instanceTransforms		.push_back(instanceTransform);
	boundingBoxes			.emplace_back(params, instanceTransform);
	enemyBehaviours			.emplace_back(std::vector<EnemyBehaviuor>{{EnemyBehaviuor{stats, timer}}});
	projInstanceTransforms	.emplace_back();
	healths					.push_back(stats.health);
	
	size++;
}

void EnemyData::Update(const glm::mat4 &pcModel, const ui index)
{
	const auto behavoiursBegin = enemyBehaviours[index].begin();
	const auto behavoiursEnd   = enemyBehaviours[index].end();
	auto chosenbehavoiurIter = std::find_if(behavoiursBegin, behavoiursEnd, [](auto &behavoiur) { return behavoiur.IsChosen(); } );
	if(chosenbehavoiurIter == behavoiursEnd)
	{
		chosenbehavoiurIter =  std::find_if(behavoiursBegin, behavoiursEnd, [](auto &behavoiur) { return behavoiur.IsDefault();} );
		if(chosenbehavoiurIter == behavoiursEnd)
		{
			LOG("i");
			throw std::runtime_error("ERRORR:ENEMY_DATA: No default behaviour defined");
		}
	}

	chosenbehavoiurIter->Update(pcModel, instanceTransforms[index], projInstanceTransforms[index]);
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
	{
		_enemyData.Update(pcModel, i);
	}
}

void EnemyManager::Spawn(const glm::mat4 &pcModel)
{
	if(_enemyData.size >= MAX_NO_ENEMIES)
		return;

	glm::vec2 pcPos = glm::vec2(pcModel * glm::vec4(0,0,0,1));
	const ft enemyX = _customRand.NextUi() % 2 ? _customRand.NextFloat(pcPos.x - 9.0f,  pcPos.x - 11.0f) : _customRand.NextFloat(pcPos.x + 9.0f, pcPos.x + 11.0f);
	const ft enemyY = _customRand.NextUi() % 2 ? _customRand.NextFloat(pcPos.y - 9.0f,  pcPos.y - 11.0f) : _customRand.NextFloat(pcPos.y + 9.0f, pcPos.y + 11.0f);

	glm::mat4 instanceTransform = _enemyTransform.Model() * glm::translate(glm::vec3(enemyX, enemyY, 0));
	_enemyData.Push(instanceTransform, _enemyMeshParams, _enemyStats, _timer);
} 
