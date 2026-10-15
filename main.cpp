/**

* Author: Wasi Genius

* Assignment: Pong Clone

* Date due: [10/16/2026]

* I pledge that I have completed this assignment without

* collaborating with anyone else, in conformance with the

* NYU School of Engineering Policies and Procedures on

* Academic Misconduct.

**/

#include "CS3113/cs3113.h"

constexpr int SCREEN_WIDTH  = 800,
              SCREEN_HEIGHT = 450,
              FPS           = 60,
              MAX_BALLS     = 5;

constexpr float PADDLE_SPEED = 350.0f;

constexpr char LEFT_PADDLE_IMAGE[] = "assets/satellite.png";
constexpr char RIGHT_PADDLE_IMAGE[] = "assets/satellite.png";
constexpr char BALL_IMAGE[] = "assets/astronaut.png";
constexpr char BACKGROUND_IMAGE[] = "assets/space background.png";
constexpr char CENTER_LINE_IMAGE[] = "assets/pong-center-line.png";

AppStatus gAppStatus = RUNNING;
float gPreviousTicks = 0.0f;

// Paddle Info
Vector2 gLeftPaddlePosition = { 65.0f, SCREEN_HEIGHT / 2.0f };
Vector2 gRightPaddlePosition = { SCREEN_WIDTH - 65.0f, SCREEN_HEIGHT / 2.0f };

float gLeftPaddleMovement = 0.0f;
float gRightPaddleMovement = 0.0f;

float gRightBotMovement = -1.0f;
bool gRightPaddleIsBot = false;

Vector2 gPaddleDrawScale = { 100.0f, 70.0f };
Vector2 gPaddleCollisionScale = { 70.0f, 100.0f };

// Ball Info
int gBallCount = 1;
Vector2 gBallPositions[MAX_BALLS];
Vector2 gBallMovements[MAX_BALLS] = {
    { 300.0f, 180.0f },
    { -300.0f, -140.0f },
    { 320.0f, 120.0f },
    { -280.0f, 200.0f },
    { 340.0f, -180.0f }
};
Vector2 gBallScale = { 58.0f, 62.0f };

// Texture Setup
Texture2D gLeftPaddleTexture;
Texture2D gRightPaddleTexture;

Texture2D gBallTexture;

Texture2D gBackgroundTexture;
Texture2D gCenterLineTexture;

// Function declarations 
void initialise();
void processInput();
void update();
void render();
void shutdown();
void setBallCount(int ballCount);

void drawTexture(const Texture2D *texture, const Vector2 *position,
                 const Vector2 *scale, float rotation);

bool isColliding(const Vector2 *positionA, const Vector2 *scaleA,
                 const Vector2 *positionB, const Vector2 *scaleB);

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Space Pong");

    // Load textures 
    gLeftPaddleTexture = LoadTexture(LEFT_PADDLE_IMAGE);
    gRightPaddleTexture = LoadTexture(RIGHT_PADDLE_IMAGE);

    gBallTexture = LoadTexture(BALL_IMAGE);

    gBackgroundTexture = LoadTexture(BACKGROUND_IMAGE);
    gCenterLineTexture = LoadTexture(CENTER_LINE_IMAGE);

    setBallCount(gBallCount);

    SetTargetFPS(FPS);
    gPreviousTicks = static_cast<float>(GetTime());
}

void processInput()
{

    // Paddle Movement
    gLeftPaddleMovement = 0.0f;
    gRightPaddleMovement = 0.0f;

    // T switches the right paddle between the second player and the bot.
    if (IsKeyPressed(KEY_T)) gRightPaddleIsBot = !gRightPaddleIsBot;

    // Choose how number of balls, setBallCount also places them on screen.
    if (IsKeyPressed(KEY_ONE)) setBallCount(1);
    else if (IsKeyPressed(KEY_TWO)) setBallCount(2);
    else if (IsKeyPressed(KEY_THREE)) setBallCount(3);
    else if (IsKeyPressed(KEY_FOUR)) setBallCount(4);
    else if (IsKeyPressed(KEY_FIVE)) setBallCount(5);

    if (IsKeyDown(KEY_W)) gLeftPaddleMovement = -1.0f;
    else if (IsKeyDown(KEY_S)) gLeftPaddleMovement = 1.0f;

    if (!gRightPaddleIsBot)
    {
        if (IsKeyDown(KEY_UP)) gRightPaddleMovement = -1.0f;
        else if (IsKeyDown(KEY_DOWN)) gRightPaddleMovement = 1.0f;
    }

    if (WindowShouldClose()) gAppStatus = TERMINATED;
}

void update()
{
    float ticks = static_cast<float>(GetTime());
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks = ticks;

    // Paddle movement
    gLeftPaddlePosition.y += PADDLE_SPEED * gLeftPaddleMovement * deltaTime;

    if (gRightPaddleIsBot)
        gRightPaddleMovement = gRightBotMovement;

    gRightPaddlePosition.y += PADDLE_SPEED * gRightPaddleMovement * deltaTime;

    // Keep left paddle from moving off the top edge of the screen
    if (gLeftPaddlePosition.y < gPaddleCollisionScale.y / 2.0f)
        gLeftPaddlePosition.y = gPaddleCollisionScale.y / 2.0f;

    // Keep left paddle from moving off the bottom edge of the screen
    if (gLeftPaddlePosition.y > SCREEN_HEIGHT - gPaddleCollisionScale.y / 2.0f)
        gLeftPaddlePosition.y = SCREEN_HEIGHT - gPaddleCollisionScale.y / 2.0f;

    // Keep right paddle from moving off the top edge of the screen 
    if (gRightPaddlePosition.y < gPaddleCollisionScale.y / 2.0f)
    {
        gRightPaddlePosition.y = gPaddleCollisionScale.y / 2.0f;
        // Bounce the bot downward if it hits the top edge
        if (gRightPaddleIsBot) gRightBotMovement = 1.0f;
    }

    // Keep right paddle from moving off the bottom edge of the screen 
    if (gRightPaddlePosition.y > SCREEN_HEIGHT - gPaddleCollisionScale.y / 2.0f)
    {
        gRightPaddlePosition.y = SCREEN_HEIGHT - gPaddleCollisionScale.y / 2.0f;
        
        // Bounce the bot upward if it hits the bottom edge
        if (gRightPaddleIsBot) gRightBotMovement = -1.0f;
    }

}

void render()
{

    // Background render
    Vector2 backgroundPosition = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    Vector2 backgroundScale = {

        static_cast<float>(SCREEN_WIDTH),
        static_cast<float>(SCREEN_HEIGHT)

    };

    // Center line render
    Vector2 centerLinePosition = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    Vector2 centerLineScale = { 12.0f, SCREEN_HEIGHT - 10.0f };

    BeginDrawing();
    ClearBackground(BLACK);

    // Draw assets onto screen
    drawTexture(&gBackgroundTexture, &backgroundPosition, &backgroundScale, 0.0f);
    drawTexture(&gCenterLineTexture, &centerLinePosition, &centerLineScale, 0.0f);

    drawTexture(&gLeftPaddleTexture, &gLeftPaddlePosition, &gPaddleDrawScale, 270.0f);
    drawTexture(&gRightPaddleTexture, &gRightPaddlePosition, &gPaddleDrawScale, 180.0f);

    for (int i = 0; i < gBallCount; i++)
        drawTexture(&gBallTexture, &gBallPositions[i], &gBallScale, 0.0f);

    EndDrawing();
}

void shutdown()
{
    //Unload assets to prevent memory leak
    UnloadTexture(gLeftPaddleTexture);
    UnloadTexture(gRightPaddleTexture);
    UnloadTexture(gBallTexture);
    UnloadTexture(gBackgroundTexture);
    UnloadTexture(gCenterLineTexture);
    CloseWindow();
}

void setBallCount(int ballCount)
{
    gBallCount = ballCount;
    for (int i = 0; i < gBallCount; i++)
    {
        gBallPositions[i].x = SCREEN_WIDTH / 2.0f;
        // Spread the  balls vertically around the line for visibility.
        gBallPositions[i].y = SCREEN_HEIGHT / 2.0f + (i - (gBallCount - 1) / 2.0f) * 75.0f;
    }
}

// Draw texture function for organization and to avoid code duplication
void drawTexture(const Texture2D *texture, const Vector2 *position,
                 const Vector2 *scale, float rotation)
{

    Rectangle textureArea = {
        0.0f, 0.0f,
        static_cast<float>(texture->width),
        static_cast<float>(texture->height)
    };

    Rectangle destinationArea = {
        position->x,
        position->y,
        scale->x,
        scale->y
    };

    Vector2 originOffset = { scale->x / 2.0f, scale->y / 2.0f };

    DrawTexturePro(
        *texture,
        textureArea,
        destinationArea,
        originOffset,
        rotation,
        WHITE
    );

}


// Check for collision between two objects
bool isColliding(const Vector2 *positionA, const Vector2 *scaleA,
                 const Vector2 *positionB, const Vector2 *scaleB)
{
    float xDistance = 
    fabs(positionA->x - positionB->x) - ((scaleA->x + scaleB->x) / 2.0f);

    float yDistance = 
    fabs(positionA->y - positionB->y) - ((scaleA->y + scaleB->y) / 2.0f);

    if (xDistance < 0.0f && yDistance < 0.0f) return true;

    return false;
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
    }

    shutdown();

    return 0;
}
