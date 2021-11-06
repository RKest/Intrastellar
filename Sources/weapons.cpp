#include "weapons.h"

Weapon::Weapon(WeaponsManager &manager, UntexturedInstancedMesh &projMesh) 
    : _manager(manager), _projMesh(projMesh), _interface(new IWeapon(this))
{
}

void Weapon::Update()
{
    const ft scaledTravelDistance  = _manager._timer.Scale(_weaponStats.shotSpeed);
    const glm::mat4 localTransform = glm::translate(glm::vec3(0.0f, scaledTravelDistance, 0.0f));
    for (size_t i = 0; i < _noProjs; ++i)
        _projInstanceTransforms[i] *= localTransform;
    
    _specUpdate();
}

void Weapon::Fire(const glm::mat4 &pcModel)
{
    if(_noProjs < MAX_PROJ_AMOUNT && _manager._timer.HeapIsItTime(_shotClockId))
    {
        _projInstanceTransforms [_noProjs] = pcModel;
        _noLeftProjPiercings    [_noProjs] = _weaponStats.noPiercings;
        ++_noProjs;
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

BlasterWeapon::BlasterWeapon(WeaponsManager &manager, UntexturedInstancedMesh &projMesh) : Weapon(manager, projMesh)
{

}

void BlasterWeapon::_specUpdate()
{
}

RocketLauncherWeapon::RocketLauncherWeapon(WeaponsManager &manager, UntexturedInstancedMesh &projMesh) : Weapon(manager, projMesh)
{
    _weaponStats += stat_altarations::SHOT_HOMING_STRENGTH(2.0f) + stat_altarations::SHOT_SPEED(-0.01f);
}

void RocketLauncherWeapon::_specUpdate()
{
}

LaserWeapon::LaserWeapon(WeaponsManager &manager, UntexturedInstancedMesh &projMesh) : Weapon(manager, projMesh)
{
    _weaponStats += stat_altarations::SHOT_SPEED(0.05f) + stat_altarations::NO_PIERCINGS(99);
}

void LaserWeapon::_specUpdate()
{
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

    _weapons[Weapons::BLASTER]          .reset(new BlasterWeapon        (*this, _blasterProjMesh));
    _weapons[Weapons::ROCEKT_LANCHER]   .reset(new RocketLauncherWeapon (*this, _rocketProjMesh));
    _weapons[Weapons::LASER]            .reset(new LaserWeapon          (*this, _laserProjMesh));
    for(size_t i = 0; i < Weapons::NO_IMPLEMENTED_WEAPONS; ++i)
        _weaponInterfaces[i] = _weapons[i]->Interface();
    _weapons[Weapons::BLASTER]->Init();
}

void WeaponsManager::Update(const glm::mat4 &pcModel)
{
    for(size_t i = 0; i < Weapons::NO_IMPLEMENTED_WEAPONS; ++i)
        _weapons[i]->Update();
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
            if(_boundingBoxes[i].IsThereAnIntersection(helpers::mouseCoordsTransformed(_inverseFlippedProjection)))
                tempSelectedWeaponIndex = i;
        }
        bool tempIsLBMPressed = helpers::IsLBMPressed();
        if(_isLBMPressed && !tempIsLBMPressed)
        {
            _closeWeaponTab();

            _weapons[_selectedWeaponIndexUni.second]->Uninit();
            _selectedWeaponIndexUni.second = tempSelectedWeaponIndex;
            _weapons[_selectedWeaponIndexUni.second]->Init();

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
    _pcStats -= _weaponStatAltarations[_selectedWeaponIndexUni.second];
    _selectedWeaponIndexUni.second = 0;
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
    _isWeaopnsTabVisible = false;
    _isWeaponsFullyDrawn = false;
    _timer.SetScalingFactor(1.0);
}
