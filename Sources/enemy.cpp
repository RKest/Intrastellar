#include "enemy.h"

EnemyBehaviuor::EnemyBehaviuor(EnemyManager &manager, bool isDefault)
	: _manager(manager), _isDefault(isDefault)
{
}

BehavoiurStatus EnemyBehaviuor::EnemyBehaviuorStatus(const glm::mat4 &enemyModel)
{
	if(_isDefault)
		return BehavoiurStatus::DEFAULT;
	else if (HasMetPredicate(enemyModel))
		return BehavoiurStatus::CHOSEN;
	else
		return BehavoiurStatus::NOT_CHOSEN;
}


void EnemyBehaviuor::UpdateProjs(std::vector<glm::mat4> &projInstanceTransforms)
{
	helpers::transformMatVec(projInstanceTransforms, _manager._timer.Scale(_manager._enemyStats.shotSpeed));
	_manager.checkForProjIntersection(projInstanceTransforms);
}

void ChaseBehaviour::Update(glm::mat4 &instanceTransform, std::vector<glm::mat4> &projInstanceTransforms)
{
	instanceTransform *= helpers::transformTowards(instanceTransform, _manager._pcModel, static_cast<ft>(_manager._timer.Scale(_manager._enemyStats.speed)));
	EnemyBehaviuor::UpdateProjs(projInstanceTransforms);
}

void ShootBehavoiur::Update(glm::mat4 &instanceTransform, std::vector<glm::mat4> &projInstanceTransforms)
{
	EnemyBehaviuor::UpdateProjs(projInstanceTransforms);
	if(_manager._timer.HeapIsItTime(_shotClockId))
	{
		const ft angle = helpers::angleBetweenTransforms(instanceTransform, _manager._pcModel);
		const glm::mat4 aimTransform = glm::rotate(angle, glm::vec3(0,0,1));
		const glm::mat4 initProjTransform = instanceTransform * aimTransform;
		helpers::pushToCappedVector(projInstanceTransforms, initProjTransform, _latestShotIndex, MAX_PROJ_AMOUNT_PER_ENEMY);
	}
}

bool ShootBehavoiur::HasMetPredicate(const glm::mat4 &enemyModel)
{
	const glm::vec2 pcPos	{_manager._pcModel * glm::vec4(0,0,0,1)};
	const glm::vec2 enemyPos{enemyModel 	   * glm::vec4(0,0,0,1)};
	if(glm::distance(pcPos, enemyPos) < 15.0f)
	{
		if(!_isActive)
		{
			_manager._timer.InitHeapClock(_shotClockId, _manager._enemyStats.shotDelay);
			_isActive = true;
		}
		return true;
	}
	else
	{
		if(_isActive)
		{
			_manager._timer.DestroyHeapClock(_shotClockId);
			_isActive = false;
		}
		return false;
	}
}

OrbiterBehaviour::OrbiterBehaviour(EnemyManager &manager) : EnemyBehaviuor(manager, true) 
{
	manager._timer.InitHeapClock(_shotClockId, manager._enemyStats.shotDelay);
}

void OrbiterBehaviour::Update(glm::mat4 &instanceTransform, std::vector<glm::mat4> &projInstanceTransforms)
{
	_manager.checkForProjIntersection(projInstanceTransforms);

	const ft scaledProjDistanceToTravelPerFrame = decl_cast(scaledProjDistanceToTravelPerFrame, _manager._timer.Scale(_manager._enemyStats.shotSpeed));
	const glm::vec2 enemyPos{instanceTransform * glm::vec4(0,0,0,1)};
	const size_t noProjectiles = projInstanceTransforms.size();
	if(noProjectiles < MAX_PROJ_AMOUNT_PER_ORBIT && _manager._timer.HeapIsItTime(_shotClockId))
		projInstanceTransforms.push_back(instanceTransform);
		
	const ft desieredAngleBetweenShots = TAU / static_cast<ft>(noProjectiles);
	std::vector<std::pair<ft, std::reference_wrapper<glm::mat4>>> orbitProjData;
	std::vector<std::reference_wrapper<glm::mat4>> movingToOrbitProjData;
	orbitProjData.reserve(noProjectiles);
	movingToOrbitProjData.reserve(noProjectiles);

	for(auto &projInstanceTransform : projInstanceTransforms)
	{
		const glm::vec2 projPos{projInstanceTransform * glm::vec4(0,0,0,1)};
		if(glm::distance(projPos, enemyPos) < ENEMY_ORBIT_RADIUS)
		{
			movingToOrbitProjData.push_back(projInstanceTransform);
		}
		else
		{
			const ft projAngle = helpers::angleBetweenTransforms(instanceTransform, projInstanceTransform);
			orbitProjData.emplace_back(projAngle, projInstanceTransform);
		}
	}

	std::sort(begin(orbitProjData), end(orbitProjData), 
		[](auto &el1, auto &el2){ return el1.first < el2.first; });

	const auto projBegin = begin(orbitProjData);
	const auto projEnd = end(orbitProjData);
	const ft minAngleToTravel = TAU * scaledProjDistanceToTravelPerFrame / ENEMY_ORBIT_CIRC;
	instanceTransform *= helpers::transformTowards(instanceTransform, _manager._pcModel, scaledProjDistanceToTravelPerFrame);
	for(auto i = projBegin; i != projEnd; ++i)
	{
		const ui distanceFromBegin = decl_cast(distanceFromBegin, std::distance(projBegin, i));
		const ft desieredAngleFromFirst = desieredAngleBetweenShots * decl_cast(desieredAngleFromFirst, distanceFromBegin);
		const ft desieredAngle = projBegin->first + desieredAngleFromFirst;
		const ft angleDifference = desieredAngle - i->first;
		const ft absAngleDifference = angleDifference < -minAngleToTravel ? -minAngleToTravel : angleDifference;
		const ft cappedAngleDifference = absAngleDifference / ENEMY_ORBIT_TICS_TO_DESIERED_ANGLE; 
		const ft angleToAdd = minAngleToTravel + _manager._timer.Scale(cappedAngleDifference);
		const ft nextFrameAngle = i->first + angleToAdd;

		const glm::mat4 nextFrameRotationTransform = instanceTransform * glm::rotate(nextFrameAngle, glm::vec3(0,0,1)) * ENEMY_ORBIT_TO_ORBIT_TRANSLATE;
		const glm::mat4 nextFrameTransform = nextFrameRotationTransform;
		i->second.get() = nextFrameTransform;
	}

	for(auto &wrap : movingToOrbitProjData)
	{
		wrap.get() *=  glm::translate(glm::vec3(0.0f, scaledProjDistanceToTravelPerFrame, 0.0f));
	}

}

EnemyData::EnemyData(EnemyManager &manager) : _manager(manager)
{
	instanceTransforms		.reserve(MAX_NO_ENEMIES);
	boundingBoxes	  		.reserve(MAX_NO_ENEMIES);
	healths			  		.reserve(MAX_NO_ENEMIES);
	projInstanceTransforms	.reserve(MAX_NO_ENEMIES);
	ids						.reserve(MAX_NO_ENEMIES);
}

void EnemyData::Clear()
{
	for(auto &pair : clockIdOrphanedProjsPairs)
		_manager._timer.DestroyHeapClock(pair.first);

	instanceTransforms			.clear();
	boundingBoxes				.clear();
	healths						.clear();
	projInstanceTransforms		.clear();
	clockIdOrphanedProjsPairs	.clear();
	ids							.clear();

	size = 0;
}

void EnemyData::Erase(const ui index)
{
	ui orphanedProjClockId;
	_manager._timer.InitHeapClock(orphanedProjClockId, ENEMY_ORPHANDED_PROJ_LIFETIME);
	clockIdOrphanedProjsPairs.emplace_back(orphanedProjClockId, std::vector<glm::mat4>());
	clockIdOrphanedProjsPairs.back().second.insert(end(clockIdOrphanedProjsPairs.back().second), 
		begin(projInstanceTransforms[index]), end(projInstanceTransforms[index]));

	instanceTransforms		[index] = instanceTransforms	[size - 1];
	boundingBoxes			[index] = boundingBoxes			[size - 1];
	projInstanceTransforms	[index] = projInstanceTransforms[size - 1];
	healths					[index] = healths				[size - 1];
	ids						[index] = ids					[size - 1];
	instanceTransforms		.pop_back();
	boundingBoxes			.pop_back();
	projInstanceTransforms	.pop_back();
	healths					.pop_back();
	ids						.pop_back();

	size--;
}

void EnemyData::Push(const glm::mat4 &instanceTransform, const UntexturedMeshParams &params, EnemyStats &stats)
{
	instanceTransforms		.push_back(instanceTransform);
	boundingBoxes			.emplace_back(params, instanceTransform);
	projInstanceTransforms	.emplace_back();
	healths					.push_back(stats.health);
	ids						.push_back(_manager.NextId());
	
	size++;
}

Enemy::Enemy(EnemyManager &manager, behavoiurPtrVec &behaviours, const glm::vec3 &colour, const ui maxNoInstances)
	: _manager(manager), _mesh(manager._enemyParams, maxNoInstances), _projMesh(_manager._enemyProjParams, MAX_ENEMY_PROJ_AMOUNT),
		_behaviours(std::move(behaviours)), _colourUni("colour", colour), _maxNoInstances(maxNoInstances), data(manager)
{
}

void Enemy::Update()
{
	for(ui i = 0; i < data.size; ++i)
	{
		const auto chosenBehavoiurIter = choseBehaviour(_behaviours, data.instanceTransforms[i]);
		chosenBehavoiurIter->get()->Update(data.instanceTransforms[i], data.projInstanceTransforms[i]);
		data.boundingBoxes[i].UpdateCoords(data.instanceTransforms[i]);
	}
	for(ui i = 0; i < data.clockIdOrphanedProjsPairs.size(); ++i)
	{
		if(_manager._timer.HeapIsItTime(data.clockIdOrphanedProjsPairs[i].first))
		{
			const size_t noOrphanedProjs = data.clockIdOrphanedProjsPairs.size();
			if(i != noOrphanedProjs - 1)
				data.clockIdOrphanedProjsPairs[i] = data.clockIdOrphanedProjsPairs[noOrphanedProjs - 1];
			data.clockIdOrphanedProjsPairs.pop_back();
		}
		helpers::transformMatVec(data.clockIdOrphanedProjsPairs[i].second, _manager._timer.Scale(_manager._enemyStats.shotSpeed));
		_manager.checkForProjIntersection(data.clockIdOrphanedProjsPairs[i].second);
	}
}

void Enemy::Spawn(const glm::mat4 &instanceTransform)
{
	if(_maxNoInstances > data.size)
		data.Push(instanceTransform, _manager._enemyParams, _manager._enemyStats);
}

void Enemy::Draw()
{
	helpers::render(_manager._enemyShader, _mesh, data.instanceTransforms.data(), data.size, _blankTransform, _manager._camera.ViewProjection(), 
		_colourUni);
	if(!data.clockIdOrphanedProjsPairs.empty() || !data.projInstanceTransforms.empty())
	{
		const auto flattenedVec = helpers::flattenVec(data.projInstanceTransforms);
		std::vector<glm::mat4> projectilesVector;
		std::for_each(begin(data.clockIdOrphanedProjsPairs), end(data.clockIdOrphanedProjsPairs),
			[&projectilesVector](auto& pair){ projectilesVector.insert(projectilesVector.end(), pair.second.begin(), pair.second.end()); });
		projectilesVector.insert(end(projectilesVector), begin(flattenedVec), end(flattenedVec));
		helpers::render(_manager._enemyProjShader, _projMesh, projectilesVector.data(), projectilesVector.size(), _blankTransform, 
			_manager._camera.ViewProjection());
	}
}

EnemyManager::EnemyManager(helpers::Core &core, const UntexturedMeshParams &params, EnemyStats &enemyStats, const UntexturedMeshParams &projParams, 
		IPlayerCharacter *pcInterface, weaponInterfaceArray_t &weaponInterfaces)
 : _camera(core.camera), _pcStats(core.stats), _timer(core.timer), _enemyStats(enemyStats), 
 	_enemyParams(params), _enemyProjParams(projParams), _pcInterface(pcInterface), _weaponInterfaces(weaponInterfaces)
{
	behavoiurPtrVec chaserVec;
	behavoiurPtrVec shooterVec;
	behavoiurPtrVec orbiterVec;
	_enemies.reserve(EnemyTypeEnum::NO_ENEMY_TYPES);
	chaserVec .push_back(std::make_unique<ChaseBehaviour>(*this));
	shooterVec.push_back(std::make_unique<ChaseBehaviour>(*this));
	shooterVec.push_back(std::make_unique<ShootBehavoiur>(*this));
	orbiterVec.push_back(std::make_unique<OrbiterBehaviour>(*this));
	_enemies.emplace_back(*this, chaserVec,  glm::vec3(0.25f, 0.57f, 0.38f), MAX_NO_ENEMIES);
	_enemies.emplace_back(*this, shooterVec, glm::vec3(0.63f, 0.16f, 0.16f), MAX_NO_SHOOTER_ENEMIES);
	_enemies.emplace_back(*this, orbiterVec, glm::vec3(0.94f, 0.90f, .055f), MAX_NO_ORBITER_ENEMIES);
}

void EnemyManager::Reset()
{
	for(auto &enemy : _enemies)
		enemy.data.Clear();
}

void EnemyManager::Draw()
{
	for(auto &enemy : _enemies)
		enemy.Draw();
}

void EnemyManager::UpdateBehaviour(const std::vector<glm::vec2> &pcPositions, std::function<void(const glm::mat4&, const ui)> fatalityCallback)
{
	_pcModel = _pcInterface.Transform().Model();
	for(auto &enemy : _enemies)
	{
		//Check for contact damage
		for (auto &enemyBoundingBox : enemy.data.boundingBoxes)
		{
			if(enemyBoundingBox.IsThereAnIntersection(pcPositions))
			{
				(_pcInterface.HitCb())();
				break;
			}
		}
		//Check for enemies getting hit with projectiles
		ui collisionIndex;
		for(ui i = 0; i < enemy.data.size; ++i)
		{
			for(auto weaponInterface : _weaponInterfaces)
			{
				if(enemy.data.boundingBoxes[i].IsThereAnIntersection(weaponInterface->ProjInstaceTransorms(), weaponInterface->NoProjs(), collisionIndex))
				{
					if((weaponInterface->ProjHitCb())(collisionIndex, enemy.data.ids[i]))
					{
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
		//Move enemies and check for projectile damage
		enemy.Update();
	}
}

std::vector<glm::mat4> EnemyManager::InstanceTransforms()
{
	size_t totalSize = 0;
	std::vector<glm::mat4> enemyInstanceTransforms;
	for(auto &enemy : _enemies)
		totalSize += enemy.data.instanceTransforms.size();
	enemyInstanceTransforms.reserve(totalSize);
	for(auto &enemy : _enemies)
		enemyInstanceTransforms.insert(end(enemyInstanceTransforms), begin(enemy.data.instanceTransforms), end(enemy.data.instanceTransforms));
	return enemyInstanceTransforms;
}

void EnemyManager::Spawn()
{
	const glm::vec2 pcPos{_pcModel * glm::vec4(0,0,0,1)};
	const ft enemyX = _customRand.NextUi() % 2 ? _customRand.NextFloat(pcPos.x - 9.0f, pcPos.x - 11.0f) : 
												 _customRand.NextFloat(pcPos.x + 9.0f, pcPos.x + 11.0f) ;
	const ft enemyY = _customRand.NextUi() % 2 ? _customRand.NextFloat(pcPos.y - 9.0f, pcPos.y - 11.0f) : 
												 _customRand.NextFloat(pcPos.y + 9.0f, pcPos.y + 11.0f) ;

	const glm::mat4 instanceTransform = _enemyTransform.Model() * glm::translate(glm::vec3(enemyX, enemyY, 0));
	if(!(_customRand.NextUi() % 10))
		_enemies[SHOOTER_ENEMY].Spawn(instanceTransform);
	else if (!(_customRand.NextUi() % 10))
		_enemies[ORBITER_ENEMY].Spawn(instanceTransform);
	else
		_enemies[CHASER_ENEMY] .Spawn(instanceTransform);
}

void EnemyManager::checkForProjIntersection(std::vector<glm::mat4> &projInstanceTransforms)
{
	if(!projInstanceTransforms.empty())
	{
		ui intersectionIndex;
		if(_pcInterface.BoundingBox().IsThereAnIntersection(projInstanceTransforms, intersectionIndex))
		{
			projInstanceTransforms.erase(projInstanceTransforms.begin() + intersectionIndex);
			(_pcInterface.HitCb())();
		}
	}
}
