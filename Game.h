#pragma once
#include "Registry.h"
#include "Components.h"
#include "GameSystems.h" // findPlayer

// Victory больше не наступает: волны бесконечные. Значение оставлено, чтобы не менять Hud.
enum class GameState { Playing, GameOver, Victory };

// Пересоздаёт мир: остаётся только игрок. Врагов приводит WaveSystem.
inline void resetWorld(Registry& reg)
{
	reg.clear();

	Entity player = reg.create();
	reg.add<Player>(player);
	reg.add<Position>(player, 390.f, 290.f); // по центру: врагов теперь не видно на старте
	reg.add<Velocity>(player);
	reg.add<Health>(player, 5, 5);
	reg.add<Collider>(player);

	auto addWall = [&](float x, float y, float w, float h)
		{
			Entity e = reg.create();
			reg.add<Position>(e, x, y);
			reg.add<Wall>(e, w, h);
		};
	// простая планировка: четыре укрытия вокруг центра, старт игрока свободен
	addWall(250.f, 150.f, 300.f, 30.f);  // сверху
	addWall(250.f, 420.f, 300.f, 30.f);  // снизу
	addWall(150.f, 250.f, 30.f, 100.f);  // слева
	addWall(620.f, 250.f, 30.f, 100.f);  // справа
}

inline GameState evaluateState(const Registry& reg)
{
	return findPlayer(reg).isNull() ? GameState::GameOver : GameState::Playing;
}