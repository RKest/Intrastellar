#include "weapons.h"

WeaponsManager::WeaponsManager(helpers::Core &core, const TexturedMeshParams &iconMeshParams, const UntexturedMeshParams &overlayMeshParams)
    : _display(core.display), _timer(core.timer), _pcStats(core.stats), _iconMesh(iconMeshParams, WEAPONS_NO_WEAPONS), _overlayMesh(overlayMeshParams)
{
    constexpr const char *basePath = "./Resources/Textures/WeaponIcons/";
    constexpr const ft iconRealestate = WEAPONS_ICON_DIMS + 1.0f;
    constexpr const ft leftIconMargin = (SCREEN_WIDTH - iconRealestate * static_cast<ft>(WEAPONS_NO_WEAPONS)) / 2.0f;
    glGenTextures(WEAPONS_NO_WEAPONS, _textures);
    for (ui i = 0; i < WEAPONS_NO_WEAPONS; ++i)
    {
        _instanceTransforms [i] = glm::translate(glm::vec3(leftIconMargin + (static_cast<ft>(i) * iconRealestate), -SCREEN_ASPECT / 2.0f, 0.0f));
        _boundingBoxes      [i] = ReqBoundingBox(iconMeshParams, _instanceTransforms[i]);

        si width, height, noComponents;
        const std::string fullPath = basePath + std::to_string(i + 1) + ".png";
        stbi_uc *imageData = stbi_load(fullPath.c_str(), &width, &height, &noComponents, 4);

        if(imageData == NULL)
            throw std::runtime_error("No Image at: " + fullPath);

        glBindTexture(GL_TEXTURE_2D, _textures[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);

        stbi_image_free(imageData);
    }

    _weaponStatAltarations[Weapons::ROCKET_LAUNCHER]    += stat_altarations::SHOT_HOMING_STRENGTH(2.0f) + stat_altarations::SHOT_SPEED(-0.01f) + 
        stat_altarations::SHOT_DELAY_MULTIPLAYER(0.7f);
    _weaponStatAltarations[Weapons::LASER]              += stat_altarations::SHOT_SPEED(0.05f) + stat_altarations::NO_PIERCINGS(99);
}

void WeaponsManager::Draw()
{
    if(_display.ReadKeyboardState(_display.KeyScancodeMap()[_display.TAB]))
    {
        ui tempSelectedWeaponIndex = _selectedWeaponIndexUni.second;
        _timer.SetScalingFactor(0.05);
        for(ui i = 0; i < WEAPONS_NO_WEAPONS; ++i)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, _textures[i]);
            if(_boundingBoxes[i].IsThereAnIntersection(helpers::mouseCoordsTransformed(_inverseFlippedProjection)))
                tempSelectedWeaponIndex = i;
        }
        bool localIsLBMPressed;
        _display.FetchMouseState(_dummy_si, _dummy_si, localIsLBMPressed);
        if(_isLBMPressed && !localIsLBMPressed)
        {
            _pcStats -= _weaponStatAltarations[_selectedWeaponIndexUni.second];
            _selectedWeaponIndexUni.second = tempSelectedWeaponIndex;
            _pcStats += _weaponStatAltarations[_selectedWeaponIndexUni.second];
        }
        _isLBMPressed = localIsLBMPressed;

        helpers::render(_overlayShader, _overlayMesh);
        
        _weaponIconShader.Bind();
        _weaponIconShader.Update(_blankTransform, _projection);
        _weaponIconShader.SetUnis(_selectedWeaponIndexUni);
        for(ui i = 0; i < WEAPONS_NO_WEAPONS; ++i)
            _weaponIconShader.SetUni("samps[" + std::to_string(i) + "]", i);
        _weaponIconShader.SetUni(_selectedWeaponIndexUni.first, tempSelectedWeaponIndex);
        _iconMesh.SetInstanceCount(WEAPONS_NO_WEAPONS);
        _iconMesh.Update(_instanceTransforms, _iconMesh.InstancedBufferPosition());
        _iconMesh.Draw();

    }
    else
        _timer.SetScalingFactor(1.0);
}

WeaponsManager::~WeaponsManager()
{
    glDeleteTextures(WEAPONS_NO_WEAPONS, _textures);
}