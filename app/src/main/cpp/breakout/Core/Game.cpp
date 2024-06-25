#include <game-activity/native_app_glue/android_native_app_glue.h>
#include "Game.h"
#include "Time/Time.h"
#include "AndroidOut.h"
#include "ECS/Entity.h"
#include "FileSystem/FileSystem.h"


static const char *c_PlayerTag = "Player";
static const char *c_BallTag = "Ball";
constexpr V2 c_BallVelocity = {0.25f, -1.5f};

Game::~Game() {

}

void Game::startGame() {
    loadAssets();
    loadLevels();
    loadUI();

    m_CurrentScene = Scene::copy(m_Levels[0]);
}

void Game::update() {
    Time::startTimeUpdate();

    handleGameLogic();
    // Physics substepping
    for (u32 i = 0; i < 4; i++) {
        handlePhysics(Time::getDeltaTime() / 4);
    }

    updateUI();


    m_Renderer.render(*m_CurrentScene);
    m_Renderer.render(m_HUD, false);

    m_Renderer.flush();

    Time::endTimeUpdate();
}


void Game::handleInput() {
    // handle all queued inputs
    auto *inputBuffer = android_app_swap_input_buffers(m_App);
    if (!inputBuffer) {
        // no inputs yet.
        return;
    }

    // handle motion events (motionEventsCounts can be 0).
    for (auto i = 0; i < inputBuffer->motionEventsCount; i++) {
        auto &motionEvent = inputBuffer->motionEvents[i];
        auto action = motionEvent.action;

        // Find the pointer index, mask and bitshift to turn it into a readable value.
        auto pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)
                >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

        // get the x and y position of this event if it is not ACTION_MOVE.
        auto &pointer = motionEvent.pointers[pointerIndex];
        auto x = GameActivityPointerAxes_getX(&pointer);
        auto y = GameActivityPointerAxes_getY(&pointer);

        // determine the action type and process the event accordingly.
        switch (action & AMOTION_EVENT_ACTION_MASK) {
            case AMOTION_EVENT_ACTION_DOWN:
            case AMOTION_EVENT_ACTION_POINTER_DOWN:
                m_Input.TouchedScreen = true;
                break;

            case AMOTION_EVENT_ACTION_CANCEL:
                // treat the CANCEL as an UP event: doing nothing in the app, except
                // removing the pointer from the cache if pointers are locally saved.
                // code pass through on purpose.
            case AMOTION_EVENT_ACTION_UP:
            case AMOTION_EVENT_ACTION_POINTER_UP:
                break;

            case AMOTION_EVENT_ACTION_MOVE:
                // There is no pointer index for ACTION_MOVE, only a snapshot of
                // all active pointers; app needs to cache previous active pointers
                // to figure out which ones are actually moved.
                for (auto index = 0; index < motionEvent.pointerCount; index++) {
                    pointer = motionEvent.pointers[index];
                    x = GameActivityPointerAxes_getX(&pointer);
                }
                break;
            default:
                break;
        }
        m_Input.LastPosX = x;
        m_Input.LastPosY = y;
    }
    // clear the motion input count in this buffer for main thread to re-use.
    android_app_clear_motion_events(inputBuffer);
    // clear the key input count too.
    android_app_clear_key_events(inputBuffer);
}

// Level layout example
/*
 *  1 1 1 1 1 1 \n
 *  2 2 0 0 2 2 \n
 *  3 3 4 4 3 3 \n
 */

// A number of 0: no brick, an empty space within the level.
// A number of 1: a solid brick, a brick that cannot be destroyed.
// A number higher than 1: a destroyable brick; each subsequent number only differs in color.

void Game::loadLevelElements(const std::string &level) {
    std::vector<std::vector<u32>> tileData;

    FILE *levelFile = android_fopen(level.c_str(), "r");

    if (levelFile == nullptr) {
        aout << "ERROR: Couldn't read level file" << std::endl;
        return;
    }

    char *line = nullptr;
    size_t len = 0;

    u32 tileCode;
    while (getline(&line, &len, levelFile) != -1) {
        std::vector<u32> row;
        std::istringstream sstream(line);
        while (sstream >> tileCode) {
            row.emplace_back(tileCode);
        }
        tileData.emplace_back(row);
    }
    if (!tileData.empty()) {
        createLevelElements(tileData);
    }

    fclose(levelFile);
    if (line != nullptr) {
        free(line);
    }
}

void Game::createLevelElements(const std::vector<std::vector<u32>> &tileData) {

    auto &scene = m_Levels.emplace_back();

    const u32 levelHeight = ANativeWindow_getHeight(m_App->window) / 2;
    const u32 levelWidth = ANativeWindow_getWidth(m_App->window);

    const u32 height = tileData.size();
    const u32 width = tileData[0].size();


    const f32 offset = (levelWidth / static_cast<float>(width));
    for (u32 y = 0; y < height; y++) {
        for (u32 x = 0; x < width; x++) {
            V3 position = V3{offset / 2 + offset * x, offset / 2 + offset * y, 0.0f};
            V3 scale = V3{offset * 0.5, offset * 0.5, 1.0};
            if (tileData[y][x] == 1) { // Solid block
                Entity brick = scene.createEntity("Brick");
                auto &transform = brick.getComponent<TransformComponent>();
                transform.Translation = position;
                transform.Scale = scale;

                brick.addComponent<TileComponent>(TileComponent(TileType::SOLID));
                brick.addComponent<SpriteComponent>(SpriteComponent({0.8f, 0.8f, 0.8f}, 0));
            } else if (tileData[y][x] > 1) {
                Color color = Color{1.0};

                switch (tileData[y][x]) {
                    case 2:
                        color = {0.2f, 0.6f, 1.0f};
                        break;
                    case 3:
                        color = {0.0f, 0.7f, 0.0f};
                        break;
                    case 4:
                        color = {0.8f, 0.8f, 0.4f};
                        break;
                    case 5:
                        color = {1.0f, 0.5f, 0.0f};
                        break;
                }

                Entity brick = scene.createEntity("Brick");
                auto &transform = brick.getComponent<TransformComponent>();
                transform.Translation = position;
                transform.Scale = scale;

                brick.addComponent<TileComponent>(TileComponent(TileType::BREAKABLE));
                brick.addComponent<SpriteComponent>(SpriteComponent(color, 1));
            }
        }
    }
    // Create the player
    {
        Entity player = scene.createEntity(c_PlayerTag);
        auto &transform = player.getComponent<TransformComponent>();
        transform.Translation = {levelWidth / 2, levelHeight * 1.75, 0.0f};
        transform.Scale = {200, 50, 1.0f};

        player.addComponent<SpriteComponent>(SpriteComponent(2));
        player.addComponent<PlayerComponent>();
    }
    // Create the ball
    {
        Entity ball = scene.createEntity(c_BallTag);
        auto &transform = ball.getComponent<TransformComponent>();
        transform.Translation = {levelWidth / 2, levelHeight * 1.75, 0.0f};
        transform.Scale = {50.f, 50.f, 1.f};

        ball.addComponent<BallComponent>(BallComponent(c_BallVelocity, 50.f));
        ball.addComponent<SpriteComponent>(SpriteComponent(3));
    }

}

void Game::loadAssets() {
    m_Renderer.loadTexture("Textures/block_solid.png");
    m_Renderer.loadTexture("Textures/block.png");
    m_Renderer.loadTexture("Textures/paddle.png");
    m_Renderer.loadTexture("Textures/awesomeface.png");
}

void Game::loadLevels() {
    loadLevelElements("Levels/level01.txt");
    loadLevelElements("Levels/level02.txt");
    loadLevelElements("Levels/level03.txt");
}

void Game::handleGameLogic() {
    const f32 dt = Time::getDeltaTime();

    if (!m_CurrentScene) {
        return;
    }


    auto player = m_CurrentScene->findEntityByName(c_PlayerTag);
    if (!player) {
        return;
    }

    auto &playerTransform = player.getComponent<TransformComponent>();
    auto ball = m_CurrentScene->findEntityByName(c_BallTag);
    if (!ball) {
        return;
    }

    auto &ballTransform = ball.getComponent<TransformComponent>();
    auto &ballCmp = ball.getComponent<BallComponent>();

    switch (m_GameState) {
        case GameState::START: {
            if (m_Input.TouchedScreen) {
                m_GameState = GameState::PLAYING;
                m_Input.TouchedScreen = false;
            }

            ballTransform.Translation = playerTransform.Translation +
                                        V3{playerTransform.Scale.x / 2.0f - (ballCmp.Radius * 2.0f),
                                           -ballCmp.Radius * 2.0f, 0.0f};

            {
                Entity gameOver = m_HUD.findEntityByName("GameOver");
                auto &transform = gameOver.getComponent<TransformComponent>();
                transform.Enabled = false;
            }

            {
                Entity retry = m_HUD.findEntityByName("Retry");
                auto &transform = retry.getComponent<TransformComponent>();
                transform.Enabled = false;
            }

            {
                Entity exit = m_HUD.findEntityByName("Exit");
                auto &transform = exit.getComponent<TransformComponent>();
                transform.Enabled = false;
            }
        }
            break;
        case GameState::PLAYING: {
            // Move the player to the cursor position
            playerTransform.Translation.x = m_Input.LastPosX;

            // Move the ball and check for bounds collision
            auto &ballPos = ballTransform.Translation;
            ballPos += V3{ballCmp.Speed, 0.0f} * dt;
            if (ballPos.x <= 0.0f) {
                ballCmp.Speed.x = -ballCmp.Speed.x;
                ballPos.x = 0.0f;
            } else if (ballPos.x + ballCmp.Radius >= m_Renderer.width()) {
                ballCmp.Speed.x = -ballCmp.Speed.x;
                ballPos.x = m_Renderer.width() - ballCmp.Radius;
            }
            if (ballPos.y <= 0.0f) {
                ballCmp.Speed.y = -ballCmp.Speed.y;
                ballPos.y = 0.0f;
            } else if (ballPos.y >= m_Renderer.height()) {
                m_Lives--;
                restartLevel();

                if (m_Lives <= 0) {
                    restartGame();
                }
            }

            auto view = m_CurrentScene->getAllEntitiesWith<TileComponent>();
            u32 brickCount = 0;

            for (const auto &entity: view) {
                auto &brick = view.get<TileComponent>(entity);
                if (brick.Type != TileType::SOLID) {
                    brickCount++;
                }
            }

            if (brickCount == 0) {
                nextLevel();
            }


        }
            break;
        case GameState::RETRY: {

            {
                Entity gameOver = m_HUD.findEntityByName("GameOver");
                auto &transform = gameOver.getComponent<TransformComponent>();
                transform.Enabled = true;
            }

            {
                Entity retry = m_HUD.findEntityByName("Retry");
                auto &transform = retry.getComponent<TransformComponent>();
                transform.Enabled = true;

                Rect rect = Rect{150, 50, transform.Translation.x, transform.Translation.y};

                if (m_Input.TouchedScreen) {
                    if (checkInside(rect, V2{m_Input.LastPosX, m_Input.LastPosY})) {
                        m_GameState = GameState::START;
                        m_Input.TouchedScreen = false;
                    }
                }
            }

            {
                Entity exit = m_HUD.findEntityByName("Exit");
                auto &transform = exit.getComponent<TransformComponent>();
                transform.Enabled = true;

                Rect rect = Rect{150, 50, transform.Translation.x, transform.Translation.y};

                if (m_Input.TouchedScreen &&
                    checkInside(rect, V2{m_Input.LastPosX, m_Input.LastPosY})) {
                    GameActivity_finish(m_App->activity);
                }
            }


        }
            break;
    }

    // Handle touch input

}

void Game::handlePhysics(f32 dt) {
    if (!m_CurrentScene) {
        return;
    }

    auto view = m_CurrentScene->getAllEntitiesWith<IDComponent, TransformComponent, TileComponent>();
    auto ball = m_CurrentScene->findEntityByName(c_BallTag);
    auto player = m_CurrentScene->findEntityByName(c_PlayerTag);

    auto &ballTransform = ball.getComponent<TransformComponent>();
    auto &ballCmp = ball.getComponent<BallComponent>();

    for (auto &entity: view) {
        auto &uuid = view.get<IDComponent>(entity);
        auto brick = m_CurrentScene->getEntityByUuid(uuid.ID);
        auto &brickCmp = brick.getComponent<TileComponent>();


        const auto &collision = checkCollision(brick, ball);

        if (std::get<0>(collision)) {
            Direction dir = std::get<1>(collision);
            V2 diffVector = std::get<2>(collision);

            if (dir == Direction::LEFT || dir == Direction::RIGHT) {
                ballCmp.Speed.x = -ballCmp.Speed.x * 1.02f;

                const f32 pen = ballCmp.Radius - glm::abs(diffVector.x);
                ballTransform.Translation.x += dir == Direction::LEFT ? pen : -pen;
            } else {
                ballCmp.Speed.y = -ballCmp.Speed.y * 1.02f;

                const f32 pen = ballCmp.Radius - glm::abs(diffVector.y);
                ballTransform.Translation.y += dir == Direction::UP ? pen : -pen;
            }

            if (brickCmp.Type != TileType::SOLID) {
                m_Score++;
                m_CurrentScene->destroyEntity(brick);
            }
        }
    }

    Collision playerCollision = checkCollision(player, ball);
    if (std::get<0>(playerCollision)) {
        auto &playerTransform = player.getComponent<TransformComponent>();
        const f32 centerBoard = playerTransform.Translation.x + playerTransform.Scale.x / 2.0f;
        const f32 distance = (ballTransform.Translation.x + ballCmp.Radius) - centerBoard;
        const f32 percentage = distance / (playerTransform.Scale.x / 2.0f);

        constexpr f32 strength = 5.0f;
        V2 oldVelocity = ballCmp.Speed;
        ballCmp.Speed.x = c_BallVelocity.x * percentage * strength;
        ballCmp.Speed.y = -1.0f * glm::abs(ballCmp.Speed.y);
        ballCmp.Speed = glm::normalize(ballCmp.Speed) * glm::length(oldVelocity);
    }
}

Direction Game::vectorDirection(const V2 &target) {
    constexpr V2 compass[] = {
            {0.0f,  1.0f},
            {1.0f,  0.0f},
            {0.0f,  -1.0f},
            {-1.0f, 0.0f}
    };

    f32 max = 0.0f;
    u32 bestMatch = -1;

    for (u32 i = 0; i < 4; i++) {
        const f32 dot = glm::dot(glm::normalize(target), compass[i]);
        if (dot > max) {
            max = dot;
            bestMatch = i;
        }
    }
    return static_cast<Direction>(bestMatch);
}

Collision Game::checkCollision(Entity &object, Entity &ball) {
    const auto &objectTransform = object.getComponent<TransformComponent>();
    const auto &ballPos = ball.getComponent<TransformComponent>().Translation;
    const auto ballRad = ball.getComponent<BallComponent>().Radius;


    const V2 center(ballPos + ballRad);
    const V2 aabbHalfExtents(objectTransform.Scale.x / 2.0f, objectTransform.Scale.y / 2.0f);
    const V2 aabbCenter(objectTransform.Translation.x + aabbHalfExtents.x,
                        objectTransform.Translation.y + aabbHalfExtents.y);

    V2 diff = center - aabbCenter;
    const V2 clamped = glm::clamp(diff, -aabbHalfExtents, aabbHalfExtents);
    const V2 closest = aabbCenter + clamped;

    diff = closest - center;

    if (glm::length(diff) <= ballRad) {
        return std::make_tuple(true, vectorDirection(diff), diff);
    }

    return std::make_tuple(false, Direction::UP, V2{0.0f});
}

void Game::restartLevel() {
    m_CurrentScene = Scene::copy(m_Levels[0]);
    m_GameState = GameState::START;
}

void Game::restartGame() {
    m_CurrentScene = Scene::copy(m_Levels[0]);
    m_GameState = GameState::RETRY;
    m_Score = 0;
    m_CurrentLevel = 0;
    m_Lives = c_MaxLives;
}

void Game::nextLevel() {
    m_CurrentLevel++;
    if (m_CurrentLevel < m_Levels.size()) {
        m_CurrentScene = Scene::copy(m_Levels[m_CurrentLevel]);
    } else {
        m_CurrentScene = Scene::copy(m_Levels[m_Levels.size() - 1]);
    }

    m_GameState = GameState::START;
}

void Game::loadUI() {
    const u32 levelWidth = ANativeWindow_getWidth(m_App->window);
    const u32 levelHeight = ANativeWindow_getHeight(m_App->window);
    {
        Entity score = m_HUD.createEntity("Score");
        auto &transform = score.getComponent<TransformComponent>();
        transform.Translation = {0.f, 150.f, 0.0f};
        transform.Scale = {2.f, 2.f, 1.f};
        score.addComponent<TextComponent>(TextComponent("Score", V3{1.0, 1.0, 0.0}));
    }
    {
        Entity score = m_HUD.createEntity("Lives");
        auto &transform = score.getComponent<TransformComponent>();
        transform.Translation = {0, 0, 0.0f};
        transform.Scale = {2.f, 2.f, 1.f};
        score.addComponent<TextComponent>(TextComponent("Lives", V3{1.0, 1.0, 0.0}));
    }
    {
        Entity gameOver = m_HUD.createEntity("GameOver");
        auto &transform = gameOver.getComponent<TransformComponent>();
        transform.Translation = {levelWidth / 2 - 250.f, levelHeight / 2, 0.0f};
        transform.Scale = {2.f, 2.f, 1.f};
        transform.Enabled = false;
        gameOver.addComponent<TextComponent>(TextComponent("Game Over", V3{1.0, 1.0, 0.0}));
    }

    {
        Entity retry = m_HUD.createEntity("Retry");
        auto &transform = retry.getComponent<TransformComponent>();
        transform.Translation = {levelWidth / 2 - 250.f, levelHeight / 2 + 150, 0.0f};
        transform.Scale = {2.f, 2.f, 1.f};
        transform.Enabled = false;
        retry.addComponent<TextComponent>(TextComponent("Retry", V3{1.0, 1.0, 0.0}));
    }

    {
        Entity exit = m_HUD.createEntity("Exit");
        auto &transform = exit.getComponent<TransformComponent>();
        transform.Translation = {levelWidth / 2 - 250.f, levelHeight / 2 + 300, 0.0f};
        transform.Scale = {2.f, 2.f, 1.f};
        transform.Enabled = false;
        exit.addComponent<TextComponent>(TextComponent("Exit", V3{1.0, 1.0, 0.0}));
    }

}

void Game::updateUI() {

    {
        Entity score = m_HUD.findEntityByName("Score");
        auto &text = score.getComponent<TextComponent>();
        text.Text = "Score: " + std::to_string(m_Score);
    }

    {
        Entity lives = m_HUD.findEntityByName("Lives");
        auto &text = lives.getComponent<TextComponent>();
        text.Text = "Lives: " + std::to_string(m_Lives);
    }

}

bool Game::checkInside(const Rect &rect, const V2 &pointer) {

    bool collisionX = rect.PosX + rect.Width >= pointer.x &&
                      pointer.x >= rect.PosX;
    // collision y-axis?
    bool collisionY = rect.PosY + rect.Height >= pointer.y &&
                      pointer.y >= rect.PosY;
    // collision only if on both axes
    return collisionX && collisionY;
}


