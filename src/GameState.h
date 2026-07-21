#pragma once

enum class GameStateType
{
    MainMenu,
    LevelSelect,
    LevelOne,
    LevelTwo,
    LevelThree,
    LevelFour
};

class GameState
{
public:
    GameStateType state;
};