#include "CS3113/cs3113.h"

// Global Constants
constexpr int SCREEN_WIDTH  = 1600 / 2,
              SCREEN_HEIGHT = 900 / 2,
              FPS           = 60,
              SIZE          = 500 / 2,
              SPEED         = 200;

constexpr char    BG_COLOUR[]    = "#F8F1C8";
constexpr Vector2 ORIGIN         = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 },
                  BASE_SIZE      = { static_cast<float>(SIZE), static_cast<float>(SIZE) },
                  INIT_POS       = { ORIGIN.x - 200.0f, ORIGIN.y },
                  RUPEE_INIT_POS = { ORIGIN.x + 250.0f, ORIGIN.y };

// Images owned by Nintendo — please don't sue me.
constexpr char LINK_FP[]  = "assets/game/awakening-link.png";
constexpr char RUPEE_FP[] = "assets/game/awakening-rupee.png";
constexpr char BGM_FP[]   = "assets/game/ost.mp3";
constexpr char PICKUP_FP[] = "assets/game/rupee.mp3";

// Global Variables
AppStatus gAppStatus     = RUNNING;
float     gAngle         = 0.0f,
          gPreviousTicks = 0.0f;

Vector2 gLinkPosition  = INIT_POS,
        gLinkMovement  = { 0.0f, 0.0f },
        gLinkScale     = BASE_SIZE,

        gRupeePosition = RUPEE_INIT_POS,
        gRupeeMovement = { 0.0f, 0.0f },
        gRupeeScale    = { BASE_SIZE.x / 2.0f, BASE_SIZE.y / 2.0f * 1.6f },

        gMousePosition = GetMousePosition();

Texture2D gLinkTexture;
Texture2D gRupeeTexture;
Music     gBGM;
Sound     gRupeeSound;

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();
bool isColliding(const Vector2 *positionA, const Vector2 *scaleA, const Vector2 *positionB, const Vector2 *scaleB);

// Function Definitions

bool isColliding(const Vector2 *positionA, const Vector2 *scaleA,
                 const Vector2 *positionB, const Vector2 *scaleB)
{
    // Box-to-box (AABB) collision detection.
    // We compare the centre-to-centre distance on each axis, minus the two
    // half-sizes. If both overlaps are negative, the boxes intersect.
    float xDistance = fabs(positionA->x - positionB->x) - ((scaleA->x + scaleB->x) / 2.0f);
    float yDistance = fabs(positionA->y - positionB->y) - ((scaleA->y + scaleB->y) / 2.0f);

    if (xDistance < 0.0f && yDistance < 0.0f) return true;

    return false;
}

void renderObject(const Texture2D *texture, const Vector2 *position,
                  const Vector2 *scale)
{
    Rectangle textureArea = {
        0.0f, 0.0f,
        static_cast<float>(texture->width),
        static_cast<float>(texture->height)
    };

    Rectangle destinationArea = {
        position->x,
        position->y,
        static_cast<float>(scale->x),
        static_cast<float>(scale->y)
    };

    Vector2 originOffset = {
        static_cast<float>(scale->x) / 2.0f,
        static_cast<float>(scale->y) / 2.0f
    };

    DrawTexturePro(
        *texture,
        textureArea, destinationArea, originOffset,
        gAngle, WHITE
    );
}

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "User Input / Collision Detection");
    InitAudioDevice();

    gLinkTexture  = LoadTexture(LINK_FP);
    gRupeeTexture = LoadTexture(RUPEE_FP);
    gBGM          = LoadMusicStream(BGM_FP);
    gRupeeSound   = LoadSound(PICKUP_FP);

    SetMusicVolume(gBGM, 0.33f);
    PlayMusicStream(gBGM);

    SetTargetFPS(FPS);
}

void processInput()
{
    // Reset the movement vector each frame so movement stops when the player
    // releases the keys or stops holding the mouse button.
    gLinkMovement = { 0.0f, 0.0f };

    // WASD movement: each axis gets a direction of -1, 0, or 1.
    // We use else-if so only one horizontal and one vertical direction can win
    // at a time (matching the normal "key press" feel in the tutorial).
    if      (IsKeyDown(KEY_A)) gLinkMovement.x = -1.0f;
    else if (IsKeyDown(KEY_D)) gLinkMovement.x =  1.0f;

    if      (IsKeyDown(KEY_W)) gLinkMovement.y = -1.0f;
    else if (IsKeyDown(KEY_S)) gLinkMovement.y =  1.0f;

    // Mouse-controlled movement: while the left mouse button is held, Link
    // moves toward the current mouse position. We compare the current position to
    // the mouse position and set a directional sign for each axis.
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        if (gLinkPosition.x < gMousePosition.x) gLinkMovement.x =  1.0f;
        if (gLinkPosition.x > gMousePosition.x) gLinkMovement.x = -1.0f;
        if (gLinkPosition.y < gMousePosition.y) gLinkMovement.y =  1.0f;
        if (gLinkPosition.y > gMousePosition.y) gLinkMovement.y = -1.0f;
    }

    // Prevent diagonal movement from being faster than cardinal movement.
    // If the vector length is greater than 1, we reduce it to a unit vector.
    if (GetLength(&gLinkMovement) > 1.0f) Normalise(&gLinkMovement);

    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;
}

void update()
{
    float ticks    = static_cast<float>(GetTime());
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    // Keep the background music streaming smoothly every frame.
    UpdateMusicStream(gBGM);

    // Update the mouse position every frame so our click-to-move input stays
    // responsive and always uses the current cursor location.
    gMousePosition = GetMousePosition();

    // Move Link by the current input direction, scaled by speed and by the time
    // passed since the last frame. This keeps movement consistent at different
    // frame rates.
    gLinkPosition = {
        gLinkPosition.x + SPEED * gLinkMovement.x * deltaTime,
        gLinkPosition.y + SPEED * gLinkMovement.y * deltaTime
    };

    // Check whether Link intersects the rupee's bounding box. This is a simple
    // box-to-box collision test using the object positions and scales.
    if (isColliding(&gLinkPosition, &gLinkScale, &gRupeePosition, &gRupeeScale))
    {
        // Play the pickup sound only once per collision window so the effect does
        // not restart every frame while Link is still touching the rupee.
        if (!IsSoundPlaying(gRupeeSound)) PlaySound(gRupeeSound);
    }
}

void render()
{
    BeginDrawing();
    ClearBackground(ColorFromHex(BG_COLOUR));

    renderObject(&gLinkTexture,  &gLinkPosition,  &gLinkScale);
    renderObject(&gRupeeTexture, &gRupeePosition, &gRupeeScale);

    EndDrawing();
}

void shutdown()
{
    StopMusicStream(gBGM);
    UnloadMusicStream(gBGM);
    UnloadSound(gRupeeSound);
    UnloadTexture(gLinkTexture);
    UnloadTexture(gRupeeTexture);
    CloseAudioDevice();
    CloseWindow();
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
