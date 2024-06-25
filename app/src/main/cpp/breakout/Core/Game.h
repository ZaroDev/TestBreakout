#ifndef _GAME_H_
#define _GAME_H_


#include <Renderer/Renderer.h>

struct android_app;

struct PlayerInput{
    f32 LastPosX = 0;
    f32 LastPosY = 0;
    bool TouchedScreen = false;
};

enum class GameState{
    START,
    PLAYING,
    RETRY
};

enum class Direction{
    UP,
    RIGHT,
    DOWN,
    LEFT
};
typedef std::tuple<bool, Direction, V2> Collision;
constexpr u32 c_MaxLives = 3;

struct Rect{
    u32 Width, Height;
    f32 PosX, PosY;
};

class Game {
public:
    /*!
     * @param pApp the android_app this Renderer belongs to, needed to configure GL
     */
    inline Game(android_app *pApp) :
            m_App(pApp) {
        m_Renderer.initialize(pApp);
    }

    virtual ~Game();


    void startGame();
    /*!
     * Handles input from the android_app.
     *
     * Note: this will clear the input queue
     */
    void handleInput();

    /*!
     * Updates the game frame and renders the scene
     */
    void update();

private:


    void handleGameLogic();
    void handlePhysics(f32 dt);

    bool checkInside(const Rect& rect, const V2& pointer);


    Collision checkCollision(Entity& object, Entity& ball);
    Direction vectorDirection(const V2& target);

    void loadAssets();
    void loadLevels();
    void loadUI();
    void updateUI();

    void loadLevelElements(const std::string& level);
    void createLevelElements(const std::vector<std::vector<u32>>& tileData);
    void restartGame();
    void restartLevel();
    void nextLevel();

    android_app *m_App;
    Renderer m_Renderer;

    Scene m_HUD;
    std::vector<Scene> m_Levels{};
    std::shared_ptr<Scene> m_CurrentScene = nullptr;
    PlayerInput m_Input = {};
    u32 m_Score = 0;
    u32 m_Lives = c_MaxLives;
    u32 m_CurrentLevel = 0;
    GameState m_GameState = GameState::START;
};

#endif //_GAME_H_