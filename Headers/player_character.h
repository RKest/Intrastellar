#ifndef SHOOTER_H
#define SHOOTER_H

#include "Core/transform.h"
#include "Core/shader.h"
#include "Core/camera.h"
#include "Core/mesh.h"
#include "Core/text.h"
#include "Core/timer.h"
#include "Core/helpers.h"

#include <execution>
#include <algorithm>
#include <functional>

class PlayerCharacter
{
public:
	PlayerCharacter(Shader &pcShader, Shader &projectileShader, const UntexturedMeshParams &pcParams, const UntexturedMeshParams &projectileParams, 
		Camera &camera, Text &text, Timer &timer, const ui maxProjectileAmount = 10);

	void Reset();
	void RenderScore();
	void Update();
	void Draw();
	void Shoot(const glm::mat4 &originTransform);

	inline Transform &PcTransform() { return _pcTransform; };
	inline bool &IsAlive() { return _isAlive; };
	inline std::vector<glm::mat4> &ProjectileTransforms() { return _projectileInstanceTransforms; };
	inline auto &ProjectileHitCallback() { return _projectileHitCallback; };
	inline auto &PlayerCharacterHitCallback() { return _pcIntersectionCallback; };

	~PlayerCharacter();

private:
	UntexturedMesh _pcMesh;
	UntexturedInstancedMesh _projectileMesh;
	Shader &_projectileShader;
	Shader &_pcShader;
	Camera &_camera;
	Text &_text;
	Timer &_timer;
	Transform _pcTransform;

	bool _isAlive = true;

	std::vector<glm::mat4> _projectileInstanceTransforms;

	glm::mat4 _perFrameProjectileTransform;
	db _perFrameProjectileTravel;

	ui _maxProjectileAmount;
	ui _oldestProjectileIndex = 0;
	ui _enemiesShotCounter = 0;

	std::function<void(ui)> _projectileHitCallback;
	std::function<void()> _pcIntersectionCallback;
	std::vector<glm::vec2> _pcPositions;

};

#endif