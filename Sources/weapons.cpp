#include "weapons.h"

void BlasterBehaviour::operator()([[maybe_unused]]const std::vector<glm::mat4> &enemyInstanceTransforms)
{
    assert(_weaponPtr != nullptr);
    const ft homingStrength =_weaponPtr->_weaponStats.shotHomingStrength; 
    if(fpclassify(homingStrength) != FP_ZERO)
    {
        const ft maxTurningRadius = _weaponPtr->_manager._timer.Scale(MAX_PROJ_TURNING_RAD);
        for(size_t i = 0; i < _weaponPtr->_noProjs; ++i)
        {
            _weaponPtr->_projInstanceTransforms[i] *= helpers::rotateTowardsClosest(enemyInstanceTransforms, _weaponPtr->_projInstanceTransforms[i], 
                maxTurningRadius, homingStrength);
        }
    }
}

void RocketBehaviour::operator()([[maybe_unused]]const std::vector<glm::mat4> &enemyInstanceTransforms)
{
    assert(_weaponPtr != nullptr);
    const ft homingStrength = _weaponPtr->_weaponStats.shotHomingStrength;
    if(fpclassify(homingStrength) != FP_ZERO)
    {
        const glm::vec2 worldMousePos = helpers::mouseCoordsTransformed(inverse(_weaponPtr->_manager._camera.ViewProjection()), 0.001f);
        const glm::mat4 worldMouseModel = glm::translate(glm::vec3(worldMousePos * CAMERA_DISTANCE, 0.0f));
        const ft maxTurningRadius = _weaponPtr->_manager._timer.Scale(MAX_PROJ_TURNING_RAD);
        for(size_t i = 0; i < _weaponPtr->_noProjs; ++i)
        {
            _weaponPtr->_projInstanceTransforms[i] *= helpers::rotateTowards(worldMouseModel, _weaponPtr->_projInstanceTransforms[i], maxTurningRadius);
        }
    }
}

void LaserBehaviour::operator()([[maybe_unused]]const std::vector<glm::mat4> &enemyInstanceTransforms)
{
    assert(_weaponPtr != nullptr);
}

Weapon::Weapon(WeaponsManager &manager, UntexturedInstancedMesh &projMesh, WeaponBehaviour &behaviour, PlayerStats &weaponStats) 
    : _manager(manager), _projMesh(projMesh), _behaviour(behaviour), _interface(new IWeapon(this))
{
    _weaponStats += weaponStats;
    _behaviour.Construct(this);
}

void Weapon::Update(const std::vector<glm::mat4> &enemyInstanceTransforms)
{
    const ft scaledTravelDistance  = _manager._timer.Scale(_weaponStats.shotSpeed);
    const glm::mat4 localTransform = glm::translate(glm::vec3(0.0f, scaledTravelDistance, 0.0f));
    for (size_t i = 0; i < _noProjs; ++i)
        _projInstanceTransforms[i] *= localTransform;
    
    _behaviour(enemyInstanceTransforms);
}

void Weapon::Fire(const glm::mat4 &pcModel)
{
    if(_manager._timer.HeapIsItTime(_shotClockId))
    {
        const ui replacedProjIndex = helpers::pushToCappedArr(_projInstanceTransforms, pcModel, _noProjs, _oldestProjIndex, MAX_PROJ_AMOUNT);
        _noLeftProjPiercings[replacedProjIndex] = _weaponStats.noPiercings;
    }
}

void Weapon::Init()
{
    _manager._timer.InitHeapClock(_shotClockId, _weaponStats.shotDelay);
}
void Weapon::Uninit()
{
    _manager._timer.DestroyHeapClock(_shotClockId);
}
void Weapon::Draw()
{
    helpers::render(_manager._projShader, _projMesh, _projInstanceTransforms, _noProjs, _blankTransform, _manager._camera.ViewProjection());
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

WeaponsManager::WeaponsManager(helpers::Core &core, const TexturedMeshParams &iconMeshParams, const UntexturedMeshParams &overlayMeshParams, 
        const UntexturedMeshParams &blasterProjParams, const UntexturedMeshParams &rocketMeshParams)
    : _display(core.display), _timer(core.timer), _pcStats(core.stats), _camera(core.camera), _iconMesh(iconMeshParams, WEAPONS_NO_WEAPONS), 
        _overlayMesh(overlayMeshParams), _blasterProjMesh(blasterProjParams, MAX_PROJ_AMOUNT), _rocketProjMesh(rocketMeshParams, MAX_PROJ_AMOUNT),
        _laserProjMesh(blasterProjParams, MAX_PROJ_AMOUNT)
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
    _laserStatAltarations  += stat_altarations::SHOT_SPEED(0.05f)           + stat_altarations::NO_PIERCINGS(99);

    _weapons[Weapons::BLASTER]          .reset(new Weapon(*this, _blasterProjMesh, _blasterBehaviour, _blasterStatAltarations));
    _weapons[Weapons::ROCEKT_LANCHER]   .reset(new Weapon(*this, _rocketProjMesh , _rocketBehaviour , _rocketStatAltarations));
    _weapons[Weapons::LASER]            .reset(new Weapon(*this, _laserProjMesh  , _laserBehaviour  , _laserStatAltarations));
    for(size_t i = 0; i < Weapons::NO_IMPLEMENTED_WEAPONS; ++i)
        _weaponInterfaces[i] = _weapons[i]->Interface();
    _weapons[Weapons::BLASTER]->Init();
}

void WeaponsManager::Update(const glm::mat4 &pcModel, const std::vector<glm::mat4> &enemyInstanceTransforms)
{
    for(size_t i = 0; i < Weapons::NO_IMPLEMENTED_WEAPONS; ++i)
        _weapons[i]->Update(enemyInstanceTransforms);
    const ui chosenWeaponIndex = _selectedWeaponIndexUni.second;
    if (chosenWeaponIndex >= Weapons::NO_IMPLEMENTED_WEAPONS)
        return;
    _weapons[chosenWeaponIndex]->Fire(pcModel);
}

void WeaponsManager::Draw()
{
    if(!_isThereWeaponCooldown && _display.ReadKeyboardState(_display.KeyScancodeMap()[_display.TAB]))
    {
        if(!_isWeaopnsTabVisible)
        {
            _isWeaopnsTabVisible = true;
            _timer.InitHeapClock(_weaponTransitionClockId, _scaledTransitionTime);
        }
        else if(!_isWeaponsFullyDrawn)
        {
            if(_timer.HeapIsItTime(_weaponTransitionClockId))
            {
                for(ui i = 0; i < WEAPONS_NO_WEAPONS; ++i)
                {
                    _instanceTransforms[i] = _baseInstanceTransforms[i] * glm::translate(glm::vec3(0.0f, _iconRealestate, 0.0f));
                    _overlayAlpthaUni.second = OVERLAY_MAX_APLHA; 
                }
                _timer.DestroyHeapClock(_weaponTransitionClockId);
                _isWeaponsFullyDrawn = true;
            }
            else
            {
                const db remainingClockTime = _timer.RemainingTime(_weaponTransitionClockId);
                const db remainingTimeFraction = 1.0 - remainingClockTime / _scaledTransitionTime;
                const ft overlayAlpha = decl_cast(overlayAlpha, remainingTimeFraction) * OVERLAY_MAX_APLHA;
                const ft hiddenIconDimY =  _iconRealestate * decl_cast(hiddenIconDimY, remainingTimeFraction);
                for(ui i = 0; i < WEAPONS_NO_WEAPONS; ++i)
                    _instanceTransforms[i] = _baseInstanceTransforms[i] * glm::translate(glm::vec3(0.0f, hiddenIconDimY, 0.0f));
                _overlayAlpthaUni.second = overlayAlpha;
            }
        }
        ui tempSelectedWeaponIndex = _selectedWeaponIndexUni.second;
        _timer.SetScalingFactor(WEAPONS_TIMER_SCALING_ARG);
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

            _timer.InitHeapClock(_weaponCooldownClockId, _weaponCooldownTime);
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
    else
    {
        if(_isThereWeaponCooldown && _timer.HeapIsItTime(_weaponCooldownClockId))
        {
            _timer.DestroyHeapClock(_weaponCooldownClockId);
            _isThereWeaponCooldown = false;
        }
        _closeWeaponTab();
    }
    for(size_t i = 0; i < Weapons::NO_IMPLEMENTED_WEAPONS; ++i)
        _weapons[i]->Draw();
}

void WeaponsManager::Reset()
{
    _switchWeapons(0);
    if(_isWeaopnsTabVisible && !_isWeaponsFullyDrawn)
        _timer.DestroyHeapClock(_weaponCooldownClockId);
    _isLBMPressed           = false;
    _isThereWeaponCooldown  = false;
    _isWeaopnsTabVisible    = false;
    _isWeaponsFullyDrawn    = false;
    _timer.SetScalingFactor(1.0);
}

void WeaponsManager::_closeWeaponTab()
{
    if(_isWeaopnsTabVisible)
    {
        _timer.DestroyHeapClock(_weaponTransitionClockId);
        _weaponTextures.Bind();
        for(ui i = 0; i < WEAPONS_NO_WEAPONS; ++i)
            _instanceTransforms[i] = _baseInstanceTransforms[i];
    }
    _isWeaopnsTabVisible    = false;
    _isWeaponsFullyDrawn    = false;
    _timer.SetScalingFactor(1.0);
}

void WeaponsManager::_switchWeapons(const ui weaponIndex)
{
    _weapons[_selectedWeaponIndexUni.second]->Uninit();
    _selectedWeaponIndexUni.second = weaponIndex;
    _weapons[_selectedWeaponIndexUni.second]->Init();
}