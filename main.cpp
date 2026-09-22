#include <chrono>
#include <memory>
#include <string>
#include <vector>
#include <SFML/Graphics.hpp>

#include "Registry.h"
#include "Components.h"
#include "System.h"
#include "GameSystems.h"
#include "RenderSystem.h"
#include "Scene.h"
#include "Game.h"
#include "Hub.h"

class SceneManager
{
	std::vector<std::shared_ptr<Scene>> scenes;
	int current = -1;

public:
	void push_scene(std::shared_ptr<Scene> scene) { scenes.push_back(std::move(scene)); }

	void SwitchTo(int index)
	{
		if (index < 0 || index >= (int)scenes.size()) return;
		if (current != -1) scenes[current]->Stop();
		current = index;
		scenes[current]->Init();
	}

	void Run()
	{
		if (scenes.empty()) return;

		sf::RenderWindow window(sf::VideoMode({ 800, 600 }), "Game");
		window.setFramerateLimit(60); // вместо sleep_for(16ms)

		scenes[0]->SetWindow(&window); // до Init, чтобы RenderSystem прошёл Init/Start вместе со всеми
		SwitchTo(0);

		using clock = std::chrono::steady_clock;
		auto last = clock::now();

		while (window.isOpen())
		{
			while (auto event = window.pollEvent())
				if (event->is<sf::Event::Closed>())
					window.close();

			InputState input;
			input.left = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
			input.right = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
			input.up = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W);
			input.down = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);
			input.shoot = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)
				|| sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
			sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));
			input.aimX = mouse.x;
			input.aimY = mouse.y;
			input.restart = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R);

			auto now = clock::now();
			float dt = std::chrono::duration<float>(now - last).count();
			last = now;
			if (dt > 0.1f) dt = 0.1f;

			auto& scene = scenes[current];
			scene->Update(dt, input);

			window.clear();
			scene->Render(dt);
			window.display();
		}
	}

	void Stop()
	{
		if (current != -1) scenes[current]->Stop();
	}
};

class GameScene : public Scene
{
	Registry& reg;
	ModuleRegistry systems;
	sf::RenderWindow* window = nullptr;
	Hud hud;
	GameData data;
	GameState state = GameState::Playing;
	int shownScore = -1;
	int shownWave = -1;

	// Заголовок окна дублирует счёт и статус (работает и без шрифта).
	void UpdateTitle()
	{
		if (!window) return;
		std::string t = "Game - Wave " + std::to_string(data.wave) + " - Score: " + std::to_string(data.score);
		if (state == GameState::GameOver) t += " - GAME OVER (press R to restart)";
		if (state == GameState::Victory)  t += " - YOU WIN (press R to restart)";
		window->setTitle(t);
		shownScore = data.score;
		shownWave = data.wave;
	}

	void SetState(GameState s)
	{
		state = s;
		UpdateTitle();
	}

	void Reset()
	{
		resetWorld(reg);
		data.score = 0;
		data.wave = 0;
		SetState(GameState::Playing);
	}

public:
	explicit GameScene(Registry& r) : reg(r), systems(r)
	{
		systems.push_module(std::make_shared<InputSystem>(reg));
		systems.push_module(std::make_shared<EnemySystem>(reg));
		systems.push_module(std::make_shared<MoveSystem>(reg));
		systems.push_module(std::make_shared<CollisionSystem>(reg, data));
		systems.push_module(std::make_shared<ContactDamageSystem>(reg));
		systems.push_module(std::make_shared<BoundsSystem>(reg, 800.f, 600.f));
		systems.push_module(std::make_shared<WallCollisionSystem>(reg));
		systems.push_module(std::make_shared<BulletWallSystem>(reg));
		systems.push_module(std::make_shared<WaveSystem>(reg, data, 800.f, 600.f));
	}

	Registry& GetRegistry() override { return reg; }

	void SetWindow(sf::RenderWindow* win) override
	{
		window = win;
		systems.push_module(std::make_shared<RenderSystem>(reg, *window));
	}

	void Init() override
	{
		Reset();
		systems.Init();
		systems.Start();
	}

	void Update(float dt, const InputState& input) override
	{
		if (state == GameState::Playing)
		{
			systems.SetInput(input);
			systems.Update(dt);

			GameState next = evaluateState(reg);
			if (next != GameState::Playing)
				SetState(next);
			else if (data.score != shownScore || data.wave != shownWave)
				UpdateTitle();
		}
		else if (input.restart)
		{
			Reset();
		}
	}

	void Render(float dt) override
	{
		systems.Render(dt);
		if (window)
			hud.Draw(*window, reg, state, data);
	}

	void Stop() override { systems.Stop(); }
};

int main()
{
	Registry reg;

	SceneManager scenes;
	scenes.push_scene(std::make_shared<GameScene>(reg));
	scenes.Run();
	scenes.Stop();
}