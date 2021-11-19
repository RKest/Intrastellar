#include "enemy.h"

EnemyBehaviuor::EnemyBehaviuor(EnemyManager &manager, bool isDefault)
	: _manager(manager), _isDefault(isDefault)
{
}

void EnemyBehaviuor::Fire([[maybe_unused]]const ui dataIndex)
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
	helpers::transformMatVec(projInstanceTransforms, Timer::Scale(_manager._enemyStats.shotSpeed));
	_manager.checkForProjIntersection(projInstanceTransforms);
}

void ChaseBehaviour::Update(const ui dataIndex)
{
	auto &instanceTransform 		= _manager._enemyData.instanceTransforms	[dataIndex];
	auto &projInstanceTransforms 	= _manager._enemyData.projInstanceTransforms[dataIndex];
	instanceTransform *= helpers::transformTowards(instanceTransform, _manager._pcModel, static_cast<ft>(Timer::Scale(_manager._enemyStats.speed)));
	EnemyBehaviuor::UpdateProjs(projInstanceTransforms);
}

ShootBehavoiur::ShootBehavoiur(EnemyManager &manager) : EnemyBehaviuor(manager, false) 
{
}

void ShootBehavoiur::Fire(const ui dataIndex)
{
	const glm::mat4 instanceTransform = _manager._enemyData.instanceTransforms[dataIndex];
	const ft angle = helpers::angleBetweenVectors(instanceTransform, _manager._pcModel);
	const glm::mat4 aimTransform = helpers::rotateZ(angle);
	const glm::mat4 initProjTransform = instanceTransform * aimTransform;
	helpers::pushToCappedVector(_manager._enemyData.projInstanceTransforms[dataIndex], initProjTransform, _latestShotIndex, MAX_PROJ_AMOUNT_PER_ENEMY);
}
void ShootBehavoiur::Update(const ui dataIndex)
{
	auto &instanceTransform 		= _manager._enemyData.instanceTransforms	[dataIndex];
	auto &projInstanceTransforms 	= _manager._enemyData.projInstanceTransforms[dataIndex];
	_manager._enemyData.shotClocks[dataIndex].Inspect(this, dataIndex);
	EnemyBehaviuor::UpdateProjs(projInstanceTransforms);
	helpers::pushToCappedVector(projInstanceTransforms, instanceTransform, _latestShotIndex, MAX_ENEMY_PROJ_AMOUNT);
}

bool ShootBehavoiur::HasMetPredicate(const glm::mat4 &enemyModel)
{
	const glm::vec2 pcPos	{_manager._pcModel * glm::vec4(0,0,0,1)};
	const glm::vec2 enemyPos{enemyModel 	   * glm::vec4(0,0,0,1)};
	return glm::distance(pcPos, enemyPos) < 15.0f;
}

OrbiterBehaviour::OrbiterBehaviour(EnemyManager &manager) : EnemyBehaviuor(manager, true) 
{
}

void OrbiterBehaviour::Fire(const ui dataIndex)
{
	auto &instanceTransform 		= _manager._enemyData.instanceTransforms	[dataIndex];
	auto &projInstanceTransforms 	= _manager._enemyData.projInstanceTransforms[dataIndex];
	projInstanceTransforms.push_back(instanceTransform);
}

void OrbiterBehaviour::Update(const ui dataIndex)
{
	auto &instanceTransform 		= _manager._enemyData.instanceTransforms	[dataIndex];
	auto &projInstanceTransforms 	= _manager._enemyData.projInstanceTransforms[dataIndex];
	_manager.checkForProjIntersection(projInstanceTransforms);

	const ft scaledProjDistanceToTravelPerFrame = decl_cast(scaledProjDistanceToTravelPerFrame, Timer::Scale(_manager._enemyStats.shotSpeed));
	const glm::vec2 enemyPos{instanceTransform * glm::vec4(0,0,0,1)};
	const size_t noProjectiles = projInstanceTransforms.size();
	if(noProjectiles < MAX_PROJ_AMOUNT_PER_ORBIT)
	{
		_manager._enemyData.shotClocks[dataIndex].Inspect(this, dataIndex);
	}
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
			const ft projAngle = helpers::angleBetweenVectors(instanceTransform, projInstanceTransform);
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
		const ft angleToAdd = minAngleToTravel + Timer::Scale(cappedAngleDifference);
		const ft nextFrameAngle = i->first + angleToAdd;

		const glm::mat4 nextFrameRotationTransform = instanceTransform * helpers::rotateZ(nextFrameAngle) * ENEMY_ORBIT_TO_ORBIT_TRANSLATE;
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
	instanceTransforms			.clear();
	boundingBoxes				.clear();
	healths						.clear();
	projInstanceTransforms		.clear();
	clockOrphanedProjsParis		.clear();
	shotClocks					.clear();
	ids							.clear();

	size = 0;
}

void EnemyData::Erase(const ui index)
{
	//Can be inlined but would be very unclean
	Clock<ui> orphanedClock(ENEMY_ORPHANDED_PROJ_LIFETIME, [this](ui i){
		const size_t noOrphanedProjs = clockOrphanedProjsParis.size();
		if(i != noOrphanedProjs - 1)
			clockOrphanedProjsParis[i] = clockOrphanedProjsParis[noOrphanedProjs - 1];
		clockOrphanedProjsParis.pop_back();
	});

	clockOrphanedProjsParis.emplace_back(orphanedClock, std::vector<glm::mat4>());
	clockOrphanedProjsParis.back().second.insert(end(clockOrphanedProjsParis.back().second), 
		begin(projInstanceTransforms[index]), end(projInstanceTransforms[index]));

	instanceTransforms		[index] = instanceTransforms	[size - 1];
	boundingBoxes			[index] = boundingBoxes			[size - 1];
	projInstanceTransforms	[index] = projInstanceTransforms[size - 1];
	healths					[index] = healths				[size - 1];
	shotClocks				[index] = shotClocks			[size - 1];
	ids						[index] = ids					[size - 1];
	instanceTransforms		.pop_back();
	boundingBoxes			.pop_back();
	projInstanceTransforms	.pop_back();
	healths					.pop_back();
	shotClocks				.pop_back();
	ids						.pop_back();

	size--;
}

void EnemyData::Push(const glm::mat4 &instanceTransform, const UntexturedMeshParams &params, EnemyStats &stats)
{
	instanceTransforms		.push_back(instanceTransform);
	boundingBoxes			.emplace_back(params, instanceTransform);
	projInstanceTransforms	.emplace_back();
	healths					.push_back(stats.health);
	shotClocks				.emplace_back(stats.shotDelay, [](EnemyBehaviuor* behaviour, ui i){ behaviour->Fire(i); });
	ids						.push_back(_manager.NextId());

	size++;
}

Enemy::Enemy(EnemyManager &manager, behavoiurPtrVec_t &behaviours, const glm::vec3 &colour, const ui maxNoInstances)
	: data(manager), _manager(manager), _mesh(manager._enemyParams, maxNoInstances), _projMesh(_manager._enemyProjParams, MAX_ENEMY_PROJ_AMOUNT),
		_behaviours(std::move(behaviours)), _colourUni("colour", colour), _maxNoInstances(maxNoInstances)
{
}

void Enemy::Update()
{
	for(ui i = 0; i < data.size; ++i)
	{
		const auto chosenBehavoiurIter = choseBehaviour(_behaviours, data.instanceTransforms[i]);
		chosenBehavoiurIter->get()->Update(i);
		data.boundingBoxes[i].UpdateCoords(data.instanceTransforms[i]);
	}
	for(ui i = 0; i < data.clockOrphanedProjsParis.size(); ++i)
	{
		data.clockOrphanedProjsParis[i].first.Inspect(i);
		helpers::transformMatVec(data.clockOrphanedProjsParis[i].second, Timer::Scale(_manager._enemyStats.shotSpeed));
		_manager.checkForProjIntersection(data.clockOrphanedProjsParis[i].second);
	}
}

void Enemy::Spawn(const glm::mat4 &instanceTransform)
{
	if(_maxNoInstances > data.size)
		data.Push(instanceTransform, _manager._enemyParams, _manager._enemyStats);
}

void Enemy::Draw()
{
	helpers::render(_manager._enemyShader, _mesh, data.instanceTransforms.data(), data.size, _blankTransform, Camera::ViewProjection(), 
		_colourUni);
	if(!data.clockOrphanedProjsParis.empty() || !data.projInstanceTransforms.empty())
	{
		const auto flattenedVec = helpers::flattenVec(data.projInstanceTransforms);
		std::vector<glm::mat4> projectilesVector;
		std::for_each(begin(data.clockOrphanedProjsParis), end(data.clockOrphanedProjsParis),
			[&projectilesVector](auto& pair){ projectilesVector.insert(projectilesVector.end(), pair.second.begin(), pair.second.end()); });
		projectilesVector.insert(end(projectilesVector), begin(flattenedVec), end(flattenedVec));
		helpers::render(_manager._enemyProjShader, _projMesh, projectilesVector.data(), projectilesVector.size(), _blankTransform, Camera::ViewProjection());
	}
}

EnemyManager::EnemyManager(helpers::Core &core, const UntexturedMeshParams &params, EnemyStats &enemyStats, const UntexturedMeshParams &projParams, 
		IPlayerCharacter *pcInterface, weaponInterfaceArray_t &weaponInterfaces, fatalityCallback_t fatalityCallback)
 :  _pcStats(core.stats), _enemyStats(enemyStats), _enemyParams(params), _enemyProjParams(projParams), _pcInterface(pcInterface), 
 	_weaponInterfaces(weaponInterfaces), m_spawnClock(ENEMY_SPAWN_DELAY, [this]{ m_spawn(); }), m_interfacePtr(new EnemyInterface(this)),
 	m_fatalityCallback(fatalityCallback)
{
	behavoiurPtrVec_t chaserVec;
	behavoiurPtrVec_t shooterVec;
	behavoiurPtrVec_t orbiterVec;
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

void EnemyManager::UpdateBehaviour(const std::vector<glm::vec2> &pcPositions)
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
							m_fatalityCallback(enemy.data.instanceTransforms[i], 3);
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
	ui nextIndex = 0;
	std::vector<glm::mat4> enemyInstanceTransforms;
	for(auto &enemy : _enemies)
		totalSize += enemy.data.instanceTransforms.size();
	m_instanceTransformToEnemyIndexMap	.resize(totalSize);
	enemyInstanceTransforms				.reserve(totalSize);
	for(ui i = 0; i < EnemyTypeEnum::NO_ENEMY_TYPES; ++i)
	{
		for(ui j = 0; j < _enemies[i].data.size; ++j)
		{
			m_instanceTransformToEnemyIndexMap[nextIndex].first = i;
			m_instanceTransformToEnemyIndexMap[nextIndex].second = j;
			++nextIndex; 
		}
		enemyInstanceTransforms.insert(end(enemyInstanceTransforms), begin(_enemies[i].data.instanceTransforms), end(_enemies[i].data.instanceTransforms));
	}
	return enemyInstanceTransforms;
}

void EnemyManager::Spawn()
{
	m_spawnClock.Inspect();
}

void EnemyManager::m_spawn()
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

void EnemyManager::m_hit(const ui i)
{
	const auto enemyIndexPair = m_instanceTransformToEnemyIndexMap[i];
	auto &enemy = _enemies[enemyIndexPair.first];
	enemy.data.healths[enemyIndexPair.second] -= decl_cast(enemy.data.healths, _pcStats.actualDamage);
	if(enemy.data.healths[enemyIndexPair.second] <= 0)
	{
		m_fatalityCallback(enemy.data.instanceTransforms[enemyIndexPair.second], 3);
		enemy.data.Erase(enemyIndexPair.second);
	}
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
