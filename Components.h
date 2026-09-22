#pragma once

struct Position { float x = 0.f; float y = 0.f; };
struct Velocity { float x = 0.f; float y = 0.f; };

struct Player {};
struct Enemy {};
struct Bullet { bool fromPlayer = true; }; // владелец снаряда

struct Health
{
	int hp = 1;
	int max = 1;
};

struct Shooter
{
	float cooldown = 1.5f;
	float timer = 0.0f;
};

struct InputState
{
	bool left = false;
	bool right = false;
	bool up = false;
	bool down = false;
	bool shoot = false;
	bool restart = false;
	float aimX = 0.f; // позиция курсора в координатах мира
	float aimY = 0.f;
};

struct Wall { float w = 40.f; float h = 40.f; };       // статичное препятствие
struct Collider { float w = 20.f; float h = 20.f; };   // кто не проходит сквозь стены

struct GameData
{
	int score = 0;
	int wave = 0; // номер текущей волны (0 - ещё не началась)
};