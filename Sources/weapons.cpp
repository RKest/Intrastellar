#include "weapons.h"

void WeaponBehaviour::Draw()
{
    helpers::render(_weaponPtr->_projShader, _weaponPtr->_projMesh, _weaponPtr->_projInstanceTransforms, _weaponPtr->_noProjs, 
        _blankTransform, Camera::ViewProjection());
}

void WeaponBehaviour::Fire(const glm::mat4 &pcModel)
{
    const ui replacedProjIndex = helpers::pushToCappedArr(_weaponPtr->_projInstanceTransforms, pcModel, _weaponPtr->_noProjs, 
        _weaponPtr->_oldestProjIndex, MAX_PROJ_AMOUNT);
    _weaponPtr->_noLeftProjPiercings[replacedProjIndex] = _weaponPtr->_weaponStats.noPiercings;
}

void BlasterBehaviour::Update([[maybe_unused]]const std::vector<glm::mat4> &enemyInstanceTransforms)
{
    _weaponPtr->_commonUpdate();
    const ft homingStrength =_weaponPtr->_weaponStats.shotHomingStrength; 
    if(fpclassify(homingStrength) != FP_ZERO)
    {
        const ft maxTurningRadius = Timer::Scale(MAX_PROJ_TURNING_RAD);
        for(size_t i = 0; i < _weaponPtr->_noProjs; ++i)
        {
            _weaponPtr->_projInstanceTransforms[i] *= helpers::rotateTowardsClosest(enemyInstanceTransforms, _weaponPtr->_projInstanceTransforms[i], 
                maxTurningRadius, homingStrength);
        }
    }
}

void RocketBehaviour::Update([[maybe_unused]]const std::vector<glm::mat4> &enemyInstanceTransforms)
{
    _weaponPtr->_commonUpdate();
    const ft homingStrength = _weaponPtr->_weaponStats.shotHomingStrength;
    if(fpclassify(homingStrength) != FP_ZERO)
    {
        const glm::vec2 worldMousePos = helpers::mouseCoordsTransformed(inverse(Camera::ViewProjection()), 0.001f);
        const glm::mat4 worldMouseModel = glm::translate(glm::vec3(worldMousePos * CAMERA_DISTANCE, 0.0f));
        const ft maxTurningRadius = Timer::Scale(MAX_PROJ_TURNING_RAD);
        for(size_t i = 0; i < _weaponPtr->_noProjs; ++i)
        {
            _weaponPtr->_projInstanceTransforms[i] *= helpers::rotateTowards(worldMouseModel, _weaponPtr->_projInstanceTransforms[i], maxTurningRadius);
        }
    }
}

void LaserBehaviour::Update([[maybe_unused]]const std::vector<glm::mat4> &enemyInstanceTransforms)
{
    typedef std::remove_reference<decltype(enemyInstanceTransforms)>::type nonRefVec;
    typedef std::remove_const<nonRefVec>::type nonConstVec;
    if(m_isLaserVisible)
        m_laserLingerClock.Inspect();
    if(m_hasLaserFired)
    {
        std::vector<nonConstVec::const_iterator> alreadyHitIters;
        _noBezierCurves         = 1;
        m_hasLaserFired         = false;
        _laserBezierCurvesSZ    = 0;
        ft fireAngle            = helpers::rotTransformAngle(_laserOrigin);
        const glm::vec2 firstFirePos{_laserOrigin * glm::vec4(0,0,0,1)};
        _laserBezierCurves[_laserBezierCurvesSZ++] = firstFirePos;
        for (ui i = 0; i < MAX_PROJ_AMOUNT; ++i)
        {
            const glm::vec2 firePos{_laserOrigin * glm::vec4(0,0,0,1)};
            std::vector<nonConstVec::const_iterator> filteredVector;
            filteredVector.reserve(MAX_NO_ENEMIES);
            helpers::copy_iter_if(cbegin(enemyInstanceTransforms), cend(enemyInstanceTransforms), std::back_inserter(filteredVector), 
            [this, fireAngle, alreadyHitIters](auto it){
                if(helpers::contains(alreadyHitIters, it))
                    return false;
                const ft distanceToEnemy = helpers::matDistance(_laserOrigin, *it);
                const ft desieredAngle = WEAPONS_LASER_MAX_HOMING_ANGLE / decl_cast(desieredAngle, std::pow(distanceToEnemy + 0.001f, 2));
                const ft minAcceptableAngle = std::clamp(desieredAngle, WEAPONS_LASER_MIN_HOMING_ANGLE, WEAPONS_LASER_MAX_HOMING_ANGLE);
                return helpers::diff(helpers::angleBetweenVectors(_laserOrigin, *it), fireAngle) < minAcceptableAngle; 
            });
            if(filteredVector.empty())
            {
                _laserBezierCurves[_laserBezierCurvesSZ++] = firePos;
                _laserBezierCurves[_laserBezierCurvesSZ++] = firePos;
                _laserBezierCurves[_laserBezierCurvesSZ++] = helpers::vecDistanceAway(firePos, 99.0f, fireAngle);
                break;
            }
            const auto closestConeEnemyIter = std::min_element(cbegin(filteredVector), cend(filteredVector), [firePos](auto &m1, auto &m2){
                return helpers::matDistance(*m1, firePos) < helpers::matDistance(*m2, firePos);
            });

            const ui hitIndex = decl_cast(hitIndex, std::distance(cbegin(enemyInstanceTransforms), *closestConeEnemyIter));
            alreadyHitIters.push_back(*closestConeEnemyIter);
            (_weaponPtr->_manager.m_enemyInterfacePtr->EnemyHit())(hitIndex);
            const glm::vec2 enemyPos{**closestConeEnemyIter * glm::vec4(0,0,0,1)};
            const ft fireDistance = distance(firePos, enemyPos);
            const ft distanceToControl = fireDistance / 3.0f;
            _laserBezierCurves[_laserBezierCurvesSZ++] = helpers::vecDistanceAway(firePos, distanceToControl, fireAngle);
            _laserBezierCurves[_laserBezierCurvesSZ++] = helpers::vecDistanceAway(firePos, distanceToControl * 2.0f, fireAngle);
            _laserBezierCurves[_laserBezierCurvesSZ++] = enemyPos;
            fireAngle = helpers::angleBetweenVectors(_laserBezierCurves[_laserBezierCurvesSZ - 2], _laserBezierCurves[_laserBezierCurvesSZ - 1]);
            _laserOrigin = **closestConeEnemyIter;
            _noBezierCurves++;
        }
    }
    /*TODO 
        - Make this be flashin according to simulate rate of fire
        - Make homing better when dealing with multiple enemies close together 
        - Figure out how to do collision
    */
}

void LaserBehaviour::Fire(const glm::mat4 &pcModel)
{
    _laserOrigin = pcModel;
    m_laserLingerClock.Init(WEAPONS_LASER_LINGER_DURATION, [this]{
        m_isLaserVisible = false;
    });
    m_isLaserVisible = true;
    m_hasLaserFired = true;
}

void LaserBehaviour::Draw()
{
    if(m_isLaserVisible)
    {
        _projMesh.Update(_laserBezierCurves.data(), _noBezierCurves);
        helpers::render(_weaponPtr->_projShader, _projMesh, _blankTransform, Camera::ViewProjection());
    }
}

Weapon::Weapon(WeaponsManager &manager, UntexturedInstancedMesh &projMesh, WeaponBehaviour &behaviour, PlayerStats &weaponStats, Shader &projShader) 
    : _manager(manager), _projMesh(projMesh), _behaviour(behaviour), _projShader(projShader), _interface(new IWeapon(this))
{
    _weaponStats += weaponStats;
    _behaviour.Construct(this);
}
void Weapon::Update(const std::vector<glm::mat4> &enemyInstanceTransforms)
{
    _behaviour.Update(enemyInstanceTransforms);
}
void Weapon::Fire(const glm::mat4 &pcModel)
{
    m_shotClock.Inspect(pcModel);
}
void Weapon::Draw()
{
    _behaviour.Draw();
}
void Weapon::Init()
{
    m_shotClock.Init(_weaponStats.shotDelay, [this](glm::mat4 const& pcModel){
        _behaviour.Fire(pcModel);
    });
}
void Weapon::_commonUpdate()
{
    const ft scaledTravelDistance  = Timer::Scale(_weaponStats.shotSpeed);
    const glm::mat4 localTransform = glm::translate(glm::vec3(scaledTravelDistance, 0.0f, 0.0f));
    for (size_t i = 0; i < _noProjs; ++i)
        _projInstanceTransforms[i] *= localTransform;
}
bool Weapon::_projHit(const ui projIndex, const ui enemyIndex)
{
    if(!helpers::contains(_alreadyHitEnemyIds[projIndex], enemyIndex))
	{
		_noLeftProjPiercings        [projIndex] -= 1;
		if(!_noLeftProjPiercings    [projIndex])
		{
            --_noProjs;
			_projInstanceTransforms	[projIndex] = _projInstanceTransforms   [_noProjs];
			_noLeftProjPiercings	[projIndex] = _noLeftProjPiercings		[_noProjs];
			_alreadyHitEnemyIds		[projIndex] = _alreadyHitEnemyIds		[_noProjs];
            _alreadyHitEnemyIds                                             [_noProjs].clear();
		}
		else
			_alreadyHitEnemyIds     [projIndex].push_back(enemyIndex);
		return true;
	}
	return false;
}

WeaponsManager::WeaponsManager(const TexturedMeshParams &iconMeshParams, const UntexturedMeshParams &overlayMeshParams, 
        const UntexturedMeshParams &blasterProjParams, const UntexturedMeshParams &rocketMeshParams)
    : _iconMesh(iconMeshParams, WEAPONS_NO_WEAPONS), _overlayMesh(overlayMeshParams), 
    _blasterProjMesh(blasterProjParams, MAX_PROJ_AMOUNT), _rocketProjMesh(rocketMeshParams, MAX_PROJ_AMOUNT), _laserProjMesh(blasterProjParams, MAX_PROJ_AMOUNT)
{
    constexpr const char *basePath = "./Resources/Textures/WeaponIcons/";
    const ft baseLeftIconMargin = (SCREEN_WIDTH - _iconRealestate * static_cast<ft>(WEAPONS_NO_WEAPONS)) / 2.0f;
    for (ui i = 0; i < WEAPONS_NO_WEAPONS; ++i)
    {
        const ft leftIconMargin = baseLeftIconMargin + (static_cast<ft>(i) * _iconRealestate);
        _baseInstanceTransforms [i] = glm::translate(glm::vec3(leftIconMargin, -_iconRealestate, 0.0f));
        _instanceTransforms     [i] = glm::translate(glm::vec3(leftIconMargin, 0.0f, 0.0f));
        _boundingBoxes          [i] = ReqBoundingBox(iconMeshParams, _instanceTransforms[i]);
        _instanceTransforms     [i] = _baseInstanceTransforms[i];
        _weaponTextures.Instance(basePath + std::to_string(i + 1) + ".png");
    }

    _rocketStatAltarations += stat_altarations::SHOT_HOMING_STRENGTH(99.0f)  + stat_altarations::SHOT_SPEED(-0.01f);
    _laserStatAltarations  += stat_altarations::SHOT_SPEED(0.05f)            + stat_altarations::NO_PIERCINGS(99);

    _weapons[Weapons::BLASTER]          .reset(new Weapon(*this, _blasterProjMesh, _blasterBehaviour, _blasterStatAltarations, _projShader));
    _weapons[Weapons::ROCEKT_LANCHER]   .reset(new Weapon(*this, _rocketProjMesh , _rocketBehaviour , _rocketStatAltarations , _projShader));
    _weapons[Weapons::LASER]            .reset(new Weapon(*this, _laserProjMesh  , _laserBehaviour  , _laserStatAltarations  , _bezierProjShader));
    for(size_t i = 0; i < Weapons::NO_IMPLEMENTED_WEAPONS; ++i)
        _weaponInterfaces[i] = _weapons[i]->Interface();
    _weapons[Weapons::BLASTER]->Init();
}

void WeaponsManager::Update(const glm::mat4 &pcModel, const std::vector<glm::mat4> &enemyInstanceTransforms)
{
    const ui chosenWeaponIndex = _selectedWeaponIndexUni.second;
    if (chosenWeaponIndex >= Weapons::NO_IMPLEMENTED_WEAPONS)
        return;
    _weapons[chosenWeaponIndex]->Fire(pcModel);
    for(size_t i = 0; i < Weapons::NO_IMPLEMENTED_WEAPONS; ++i)
        _weapons[i]->Update(enemyInstanceTransforms);
}

void WeaponsManager::Draw()
{
    if(!_isThereWeaponCooldown && IDisplay::I.ReadKeyboardState(IDisplay::I.KeyScancodeMap()[IDisplay::I.TAB]))
    {
        if(!_isWeaopnsTabVisible)
        {
            _isWeaopnsTabVisible = true;
            m_weaponTransitionClock.Init(OVERLAY_TRANSITION_TIME, [this](){
                for(ui i = 0; i < WEAPONS_NO_WEAPONS; ++i)
                {
                    _instanceTransforms[i] = _baseInstanceTransforms[i] * glm::translate(glm::vec3(0.0f, _iconRealestate, 0.0f));
                    _overlayAlpthaUni.second = OVERLAY_MAX_APLHA; 
                }
                _isWeaponsFullyDrawn = true;
            });
        }
        else if(!_isWeaponsFullyDrawn && !m_weaponTransitionClock.Inspect<ClockOptions::NOSCALE>())
        {
            const db remainingClockTime = m_weaponTransitionClock.RemainingTime();
            const db remainingTimeFraction = 1.0 - remainingClockTime / OVERLAY_TRANSITION_TIME;
            const ft overlayAlpha = decl_cast(overlayAlpha, remainingTimeFraction) * OVERLAY_MAX_APLHA;
            const ft hiddenIconDimY =  _iconRealestate * decl_cast(hiddenIconDimY, remainingTimeFraction);
            for(ui i = 0; i < WEAPONS_NO_WEAPONS; ++i)
                _instanceTransforms[i] = _baseInstanceTransforms[i] * glm::translate(glm::vec3(0.0f, hiddenIconDimY, 0.0f));
            _overlayAlpthaUni.second = overlayAlpha;
        }
        ui tempSelectedWeaponIndex = _selectedWeaponIndexUni.second;
        Timer::SetScalingFactor(WEAPONS_TIMER_SCALING_ARG);
        for(ui i = 0; i < WEAPONS_NO_WEAPONS; ++i)
        {
            if(_boundingBoxes[i].IsThereAnIntersection(helpers::mouseCoordsTransformed(_inverseProjection)))
                tempSelectedWeaponIndex = i;
        }
        bool tempIsLBMPressed = helpers::IsLBMPressed();
        if(_isLBMPressed && !tempIsLBMPressed && tempSelectedWeaponIndex != _selectedWeaponIndexUni.second)
        {
            _closeWeaponTab();
            _switchWeapons(tempSelectedWeaponIndex);

            m_weaponCooldownClock.Init(WEAPONS_COOLDOWN, [this]{
                LOG("asd");
                _isThereWeaponCooldown = false;
            });
            _isThereWeaponCooldown = true;
            _isLBMPressed = false;
            return;
        }
        _isLBMPressed = tempIsLBMPressed;

        _weaponTextures.Bind();
        helpers::render(_overlayShader, _overlayMesh, _overlayAlpthaUni);
        _weaponIconShader.Bind();
        _weaponIconShader.SetUni(_selectedWeaponIndexUni.first, tempSelectedWeaponIndex);
        helpers::render(_weaponIconShader, _iconMesh, _instanceTransforms, WEAPONS_NO_WEAPONS, _blankTransform, _projection, _samplerIds);
    }
    else if(_isThereWeaponCooldown)
    {
        m_weaponCooldownClock.Inspect<ClockOptions::NOSCALE>();
    }
    else
    {
        _closeWeaponTab();
    }
    for(size_t i = 0; i < Weapons::NO_IMPLEMENTED_WEAPONS; ++i)
        _weapons[i]->Draw();
}

void WeaponsManager::Reset()
{
    _switchWeapons(0);
    _isLBMPressed           = false;
    _isThereWeaponCooldown  = false;
    _isWeaopnsTabVisible    = false;
    _isWeaponsFullyDrawn    = false;
    Timer::SetScalingFactor(1.0);
}
void WeaponsManager::SetEnemyInterface(EnemyInterface* interface)
{
    m_enemyInterfacePtr = interface; 
}
void WeaponsManager::_closeWeaponTab()
{
    if(_isWeaopnsTabVisible)
    {
        _weaponTextures.Bind();
        for(ui i = 0; i < WEAPONS_NO_WEAPONS; ++i)
            _instanceTransforms[i] = _baseInstanceTransforms[i];
    }
    _isWeaopnsTabVisible    = false;
    _isWeaponsFullyDrawn    = false;
    Timer::SetScalingFactor(1.0);
}

void WeaponsManager::_switchWeapons(const ui weaponIndex)
{
    _selectedWeaponIndexUni.second = weaponIndex;
    _weapons[_selectedWeaponIndexUni.second]->Init();
}