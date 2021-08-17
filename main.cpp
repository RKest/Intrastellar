#include "_config.h"

#include "Core/controler.h"
#include "shooter.h"
#include "expmanager.h"

#include "glm/gtx/string_cast.hpp"

int main(int argc, char **argv)
{
	const std::vector<glm::vec3> colours = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};

	Display display(SCREEN_WIDTH, SCREEN_HEIGHT, "Intrastellar");
	Camera camera(glm::vec3(0, 0, -1), 70.0f, display.Aspect(), 0.01f, 1000.0f);

	Shader pcShader("./Shaders/PC", PC_PARAMS, PC_PARAMS_NO, UNIFORMS, UNIFORMS_NO);
	Shader projectileShader("./Shaders/Projectile", PROJECTILE_PARAMS, PROJECTILE_PARAMS_NO, UNIFORMS, UNIFORMS_NO);
	Shader enemyShader("./Shaders/Enemy", PROJECTILE_PARAMS, PROJECTILE_PARAMS_NO, UNIFORMS, UNIFORMS_NO);
	Shader textShader("./Shaders/Text", TEXT_PARAMS, TEXT_PARAMS_NO, {}, {});
	Shader expShader("./Shaders/Exp", EXP_PARAMS, EXP_PARAMS_NO, UNIFORMS, UNIFORMS_NO);

	const glm::vec3 pcVertices[] = {{0, 0.5, 0}, {-0.5, -0.5, 0}, {0.5, -0.5, 0}};
	const ui pcIndices[] = {0, 1, 2};
	const glm::vec3 projectileVertices[] = {{-0.1, 0, 0}, {0.1, 0, 0}, {-0.1, 0.3, 0}, {0.1, 0.3, 0}};
	const ui projectileIndices[] = {0, 1, 2, 3, 1, 2};
	const glm::vec3 enemyVertices[] = {{1, 0, 0}, {1, 1, 0}, {0, 1, 0}, {0, 0, 0}};
	const ui enemyIndices[] = {0, 1, 2, 2, 3, 0};
	const glm::vec3 expVertices[] = {{0.1, 0, 0}, {0.1, 0.1, 0}, {0, 0.1, 0}, {0, 0, 0}};
	const ui expIndices[] = {0, 1, 2, 2, 3, 0};

	UntexturedMeshParams pcParams = {pcVertices, pcIndices, ARR_SIZE(pcVertices), ARR_SIZE(pcIndices)};
	UntexturedMeshParams enemyMeshParams = {enemyVertices, enemyIndices, ARR_SIZE(enemyVertices), ARR_SIZE(enemyIndices)};
	UntexturedMeshParams projectileParams = {projectileVertices, projectileIndices, ARR_SIZE(projectileVertices), ARR_SIZE(projectileIndices)};
	UntexturedMeshParams expParams = {expVertices, expIndices, ARR_SIZE(expVertices), ARR_SIZE(expIndices)};

	UntexturedMesh pcMesh(pcParams);
	UntexturedInstancedMesh projectileMesh(projectileParams);

	Transform pcTransform;
	Transform blankTransform;

	Text text(textShader, "./Resources/Fonts/slkscr.ttf", SCREEN_WIDTH, SCREEN_HEIGHT);
	Timer timer(text, ENEMY_SPAWN_FREQUENCY, SHOOTER_FREQUENCY);
	ExpManager expManager(expShader, camera, timer, expParams, CUSTOM_RAND_SEED, 50);
	EnemyManager enemyManager(enemyShader, camera, timer, expManager, enemyMeshParams, CUSTOM_RAND_SEED, MAX_NO_ENEMIES);
	Shooter shooter(pcParams, projectileShader, camera, text, timer, projectileMesh, enemyManager, MAX_NO_SHOOTER_PROJECTILES);

	pcTransform.Scale() *= 0.05;
	Controler controler(display, camera, timer, pcTransform);

	projectileShader.Bind();
	for (ui i = 0; i < colours.size(); ++i)
		projectileShader.SetVec3("colours[" + std::to_string(i) + ']', colours[i]);

	ui counter = 0;
	glm::mat4 pcModel;

	bool isPcAlive = true;

	auto render = [&]
	{
		display.Clear(0.1, 0.1, 0.2, 1.0);
		enemyManager.Draw();
		shooter.RenderScore();
		pcShader.Bind();
		pcShader.Update(pcTransform, camera);
		pcMesh.Draw();
		timer.RenderFPS();
	};

	while (!display.IsClosed())
	{
		timer.RecordFrame();
		pcModel = pcTransform.Model();

		controler.CaptureKeyboardPresses(isPcAlive);
		controler.CaptureMouseMovement();

		enemyManager.UpdateBehaviour(pcModel);
		if (timer.IsItTime(Timer::ClocksEnum::SPAWN_CLOCK))
			enemyManager.Spawn(pcModel);

		render();

		if (timer.IsItTime(Timer::ClocksEnum::SHOT_CLOCK))
			shooter.Shoot(pcModel);
		shooter.Update(pcModel, isPcAlive);
		expManager.UpdateExpParticles(pcModel);

		display.Update();
		counter++;

		while (!isPcAlive && !display.IsClosed())
		{
			timer.RecordFrame();
			render();

			text.Render("GAME OVER!", (static_cast<ft>(SCREEN_WIDTH) / 2.0f) - 300.0f, (static_cast<ft>(SCREEN_HEIGHT) / 2.0f), 2.0f, glm::vec3(1));
			text.Render("Space to restart", (static_cast<ft>(SCREEN_WIDTH) / 2.0f) - 120.0f, (static_cast<ft>(SCREEN_HEIGHT) / 2.0f) - 60.0f, 0.5f, glm::vec3(1));

			display.Update();
			controler.CaptureKeyboardPresses(isPcAlive);

			if (isPcAlive)
			{
				expManager.Reset();
				enemyManager.Reset();
				shooter.Reset();
				pcTransform.Pos().x = 0;
				pcTransform.Pos().y = 0;
				camera.Pos().x = 0;
				camera.Pos().y = 0;
			}
		}
	}

	return 0;
}