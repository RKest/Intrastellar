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
	if(!projInstanceTransforms.empty())
	{
		helpers::transformMatVec(projInstanceTransforms, _manager._timer.Scale(_manager._enemyStats.shotSpeed));
		ui intersectionIndex;
		if(_manager._pcInterface.BoundingBox().IsThereAnIntersection(projInstanceTransforms, intersectionIndex))
		{
			projInstanceTransforms.erase(projInstanceTransforms.begin() + intersectionIndex);
			(_manager._pcInterface.HitCb())();
		}
	}
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
		const ft angle = helpers::angleBetweenPoints(instanceTransform, _manager._pcModel);
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
	const glm::vec2 enemyPos{instanceTransform * glm::vec4(0,0,0,1)};
	const ft distanceFromTheCenter = 10.0f;
	const ft circAroundTheCenter = distanceFromTheCenter * TAU;
	const ft scaledProjDistanceToTravelPerFrame = static_cast<ft>(_manager._timer.Scale(_manager._enemyStats.speed));
	const glm::mat4 localProjTransform = glm::translate(glm::vec3(0.0f, scaledProjDistanceToTravelPerFrame, 0.0f));
	const glm::mat4 localTransform = helpers::transformTowards(instanceTransform, _manager._pcModel, scaledProjDistanceToTravelPerFrame);
	const size_t noProjectiles = projInstanceTransforms.size();
	if(noProjectiles < MAX_PROJ_AMOUNT_PER_ORBIT && _manager._timer.HeapIsItTime(_shotClockId))
	{
		projInstanceTransforms.push_back(instanceTransform);
	}
	const ft desieredAngle = TAU / static_cast<ft>(noProjectiles);
	std::vector<std::pair<ft, std::reference_wrapper<glm::mat4>>> orbitProjData;
	std::vector<std::reference_wrapper<glm::mat4>> movingToOrbitProjData;
	orbitProjData.reserve(noProjectiles);
	movingToOrbitProjData.reserve(noProjectiles);

	for(auto &projInstanceTransform : projInstanceTransforms)
	{
		const glm::vec2 projPos{projInstanceTransform * glm::vec4(0,0,0,1)};
		if(glm::distance(projPos, enemyPos) < distanceFromTheCenter)
		{
			movingToOrbitProjData.push_back(projInstanceTransform);
		}
		else
		{
			ft projAngle = glm::orientedAngle(glm::vec2(0, 1), 
				glm::normalize(glm::vec2(glm::inverse(instanceTransform) * projInstanceTransform * glm::vec4(0,0,0,1))));
			if(projAngle < 0)
				projAngle = TAU + projAngle;
			orbitProjData.emplace_back(projAngle, projInstanceTransform);
		}
	}

	std::sort(begin(orbitProjData), end(orbitProjData), 
		[](auto &el1, auto &el2){ return el1.first > el2.first; });

	const auto projBegin = begin(orbitProjData);
	const auto projEnd = end(orbitProjData);
	const ft angleToTravel 			  = TAU * scaledProjDistanceToTravelPerFrame / circAroundTheCenter;
	const ft acceleratedAngleToTravel = TAU * scaledProjDistanceToTravelPerFrame * 1.2f / circAroundTheCenter;
	for(auto i = projBegin; i != projEnd; ++i)
	{
		const auto next = (i + 1) != projEnd ? std::make_pair(i + 1, true) : std::make_pair(projBegin, false);
		const ft angleToNext = next.second ? helpers::angleDiff(i->first, next.first->first) : TAU + i->first - next.first->first;
		const ft angleToAdd = angleToNext < desieredAngle ? acceleratedAngleToTravel : angleToTravel;
		std::cout << angleToNext << '\n';
		const ft nextFrameAngle = i->first + angleToAdd;
		const glm::mat4 nextFrameRotationTransform = instanceTransform * glm::rotate(nextFrameAngle, glm::vec3(0,0,1)) * 
			glm::translate(glm::vec3(0.0f, distanceFromTheCenter + 0.1f, 0.0f));

		ft projAngle = glm::orientedAngle(glm::vec2(0, 1), 
			glm::normalize(glm::vec2(glm::inverse(instanceTransform) * nextFrameRotationTransform * glm::vec4(0,0,0,1))));
		if(projAngle < 0)
			projAngle = TAU + projAngle;
		
		const glm::mat4 nextFrameTransform = nextFrameRotationTransform;
		i->second.get() = nextFrameTransform;
	}

	for(auto &wrap : movingToOrbitProjData)
	{
		wrap.get() *= localProjTransform;
	}

	// instanceTransform *= localTransform;
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

Enemy::Enemy(EnemyManager &manager, behavoiurPtrVec &behaviours, const glm::vec3 &colour, const ui maxNoInstances)
	: _manager(manager), _mesh(manager._enemyParams, maxNoInstances), _projMesh(_manager._enemyProjParams, MAX_ENEMY_PROJ_AMOUNT),
		_behaviours(std::move(behaviours)), _colourUni("colour", colour), _maxNoInstances(maxNoInstances)
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
	if(!data.projInstanceTransforms.empty())
	{
		const auto flattenedVec = helpers::flattenVec(data.projInstanceTransforms);
		const std::size_t noProjectiles = helpers::twoDVecSize(data.projInstanceTransforms);
		helpers::render(_manager._enemyProjShader, _projMesh, flattenedVec.data(), noProjectiles, _blankTransform, _manager._camera.ViewProjection());
	}
}

EnemyManager::EnemyManager(Shader &enemyShader, helpers::Core &core, const UntexturedMeshParams &params, EnemyStats &enemyStats, 
		const UntexturedMeshParams &projParams, IPlayerCharacter *pcInterface)
 : _enemyShader(enemyShader), _camera(core.camera), _pcStats(core.stats), _timer(core.timer), _enemyStats(enemyStats), 
 	_enemyParams(params), _enemyProjParams(projParams), _pcInterface(pcInterface)
{
	// behavoiurPtrVec chaserVec;
	// behavoiurPtrVec shooterVec;
	behavoiurPtrVec orbiterVec;
	_enemies.reserve(EnemyTypeEnum::NO_ENEMY_TYPES);
	// chaserVec .push_back(std::make_unique<ChaseBehaviour>(*this));
	// shooterVec.push_back(std::make_unique<ChaseBehaviour>(*this));
	// shooterVec.push_back(std::make_unique<ShootBehavoiur>(*this));
	orbiterVec.push_back(std::make_unique<OrbiterBehaviour>(*this));
	// _enemies.emplace_back(*this, chaserVec,  glm::vec3(0.25f, 0.57f, 0.38f), MAX_NO_ENEMIES);
	// _enemies.emplace_back(*this, shooterVec, glm::vec3(0.63f, 0.16f, 0.16f), MAX_NO_SHOOTER_ENEMIES);
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
			if(enemy.data.boundingBoxes[i].IsThereAnIntersection(_pcInterface.ProjTransforms(), collisionIndex))
			{
				(_pcInterface.ProjHitCb())(collisionIndex);
				enemy.data.healths[i] -= decl_cast(enemy.data.healths, _pcStats.actualDamage);
				if(enemy.data.healths[i] <= 0)
				{
					fatalityCallback(enemy.data.instanceTransforms[i], 3);
					enemy.data.Erase(i);
				}
			}
		}
		//Move enemies and check for projectile damage
		enemy.Update();
	}
}

std::vector<std::vector<glm::mat4>*> EnemyManager::InstanceTransforms()
{
	std::vector<std::vector<glm::mat4>*> enemyInstanceTransforms;
	for(auto &enemy : _enemies)
		enemyInstanceTransforms.push_back(&enemy.data.instanceTransforms);
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
