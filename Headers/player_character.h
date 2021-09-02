#ifndef SHOOTER_H
#define SHOOTER_H

#include "Core/transform.h"
#include "Core/shader.h"
#include "Core/camera.h"
#include "Core/mesh.h"
#include "Core/text.h"
#include "Core/timer.h"
#include "Core/helpers.h"
#include "Core/stats.h"

#include <execution>
#include <algorithm>
#include <functional>


using namespace std::placeholders;

class PlayerCharacter
{
public:
	PlayerCharacter(const UntexturedMeshParams &pcParams, const UntexturedMeshParams &projectileParams, Stats &pcStats,
		Camera &camera, Text &text, Timer &timer);

	void Reset();
	void RenderScore();
	void Update();
	void Draw();
	void Shoot(const glm::mat4 &originTransform);
	void ExternDraw(const std::vector<glm::mat4> &pcTransforms, std::vector<glm::mat4> &projTransforms, const std::vector<ui> &clockIds, 
	const std::vector<helpers::BoundingBox> &targetBoundingBoxes, const std::vector<Stats*> &pcStats, const glm::mat4 &projection, ui &oldestProjIndex);

	inline Transform &PcTransform() { return _pcTransform; }
	inline bool &IsAlive() { return _isAlive; }
	inline std::vector<glm::mat4> &ProjTransforms() { return _projInstanceTransforms; }
	inline auto &ProjHitCallback() { return _projHitCallback; }
	inline auto &PCHitCallback() { return _pcIntersectionCallback; }
	inline auto ExternDrawCb() { return std::bind(&PlayerCharacter::ExternDraw, this, _1, _2, _3, _4, _5, _6, _7); }

private:
	Shader _projectileShader;
	Shader _pcShader;
	Shader _pcCardShader;
	UntexturedMesh _pcMesh;
	UntexturedInstancedMesh _pcCardMesh;
	UntexturedInstancedMesh _projMesh;
	UntexturedInstancedMesh _projCardMesh;
	Camera &_camera;
	Text &_text;
	Timer &_timer;
    Stats &_pcStats;
	Transform _pcTransform;

	bool _isAlive = true;

	std::vector<glm::mat4> _projInstanceTransforms;

	glm::mat4 _perFrameProjectileTransform;
	db _perFrameProjectileTravel;

	ui _maxProjectileAmount;
	ui _oldestProjectileIndex = 0;
	ui _enemiesShotCounter = 0;

	std::function<void(ui)> _projHitCallback;
	std::function<void()> _pcIntersectionCallback;
	std::vector<glm::vec2> _pcPositions;

};

#endif