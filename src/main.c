#include "rcore.c"

// Grappleable Objects
#define MAX_NUM_GRAP_RECTS 10
typedef struct GrappleableRect {
	Rectangle collider;
} GrappleableRect;

#define PLAYER_WIDTH 50
#define PLAYER_HEIGHT 50
//#define GRAVITY 980
#define GRAVITY 380
#define GRAPPLE_FORCE 300
#define MOVE_SPEED 200
#define JUMP_FORCE 350
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800

#define MAX_GRAPPLE_POINTS_LEN 5000

#define PLAYER_COLOR GREEN
#define GRAPPLE_RECT_COLOR RED
#define FLOOR_COLOR DARKGRAY
#define DEFAULT_COLOR BLACK

#define DEFAULT_FONT_SIZE 20
#define DEBUG_FONT_SIZE 20
#define PLAYER_NAME_FONT_SIZE 15
#define PLAYER_NAME_HEIGHT (PLAYER_HEIGHT / 2) + PLAYER_NAME_FONT_SIZE - 10

typedef struct Player {
	Rectangle collider;
	Vector2 velocity;
	bool isMidair;
	// Grapple
	bool isGrappling;
	Vector2 grapplePos;
	float currGrappleLength;
	Vector2 GrapplePathPoints[MAX_GRAPPLE_POINTS_LEN];
	int CurrGrapplePointCount;
} Player;

int getDirectionX();
void updatePos(Player*, Rectangle[]);
void updateVelocityY(Player*, Rectangle*);
void Grapple();
void DrawPlayer(Player* player);
void DrawRectangles(Rectangle rectangles[], int numRectangles, Color colorToDraw);
void DrawPlayerDebug(Player* player);

Vector2 GetRectanglePosition(Rectangle rect) {
	return (Vector2) { rect.x, rect.y };
}

int main() {
	static const int FPS = 60;
	static const char* GAME_TITLE = "Grappler";
	static Player player = { { 100, 300, PLAYER_WIDTH, PLAYER_HEIGHT }, { 0, 50 } };
	static Rectangle floors[] = { {0, 500, 300, 300}, {600, 500, 300, 300} };
	GrappleableRect grappleableRects[MAX_NUM_GRAP_RECTS] = { { 400, 100, PLAYER_WIDTH, PLAYER_HEIGHT } };
	int currNumRects = 1;
	float currGrappleLength;

	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE);
	//HideCursor();
	SetTargetFPS(FPS);
	//SetupGrappleRects(GrappleableRects);
	while (!WindowShouldClose()) {
		//updatePos(&player, floors);
		BeginDrawing();
		ClearBackground(BLUE);
		DrawRectangles(floors, sizeof(floors) / sizeof(Rectangle), FLOOR_COLOR);
		//for (int i = 0; i < sizeof(floors) / sizeof(Rectangle); i++) {
		//	DrawRectangleRec(floors[i], FLOOR_COLOR);
		//}
		// Grapple
		Vector2 MousePos = GetMousePosition();
		for (int currRect = 0; currRect < 1; currRect++)
		{
			const GrappleableRect* currRectRef = &grappleableRects[currRect];
			Vector2 playerToRect = (Vector2){ player.collider.x - currRectRef->collider.x, player.collider.y - currRectRef->collider.y };
			Vector2 grappleDirection = playerToRect;
			Color ColorToDrawRect = RED;
			if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
			{
				DrawRectangle(GetMouseX(), GetMouseY(), 20, 20, PLAYER_COLOR);
				if (CheckCollisionPointRec(MousePos, currRectRef->collider))
				{
					if (!player.isGrappling) {
						player.isGrappling = true;
						player.grapplePos = GetRectanglePosition(currRectRef->collider);
						player.currGrappleLength = Vector2Distance((Vector2) { player.collider.x, player.collider.y }, (Vector2) { currRectRef->collider.x, currRectRef->collider.y });
						player.CurrGrapplePointCount = 0;
						
						DrawLine(player.collider.x + PLAYER_WIDTH / 2, player.collider.y + PLAYER_HEIGHT / 2, GetMouseX(), GetMouseY(), RED);
					}
					ColorToDrawRect = PURPLE;
				}
				else {
					player.isGrappling = false;
				}
			}
			else {
				player.isGrappling = false;
				if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
				{
					player.collider.x = MousePos.x;
					player.collider.y = MousePos.y;
					player.velocity = Vector2Zero();
				}
			}
			DrawRectangleRec(currRectRef->collider, ColorToDrawRect);
		}
		updatePos(&player, floors);
		DrawPlayer(&player);
		DrawPlayerDebug(&player);
		EndDrawing();
	}
	CloseWindow();
	return 0;
}

void SetupGrappleRects(GrappleableRect GrappleableRects[])
{
}

void Grapple()
{
}

void DrawPlayer(Player* player)
{
	DrawRectangleRec(player->collider, PLAYER_COLOR);
	DrawText("Player", player->collider.x, player->collider.y - PLAYER_NAME_HEIGHT, PLAYER_NAME_FONT_SIZE, PLAYER_COLOR);
}

void DrawRectangles(Rectangle rectangles[], int numRectangles, Color colorToDraw)
{
	for (int i = 0; i < numRectangles; i++)
	{
		DrawRectangleRec(rectangles[i], colorToDraw);
	}
}

int getDirectionX() {
	// Bools are used as ints - if both keys are pressed they cancel each other out
	return IsKeyDown(KEY_D) - IsKeyDown(KEY_A);
}

void DrawPlayerDebug(Player* player) {
	char PositionText[1000], VelocityText[1000], PlayerStateText[1000];
	sprintf(PositionText, "Player Pos (%.2f,%.2f)\nRect Pos (%.2f,%.2f)", player->collider.x, player->collider.y, player->grapplePos.x, player->grapplePos.y);
	sprintf(VelocityText, "Player Velocity (%.2f,%.2f)", player->velocity.x, player->velocity.y);
	sprintf(PlayerStateText, "Grappling? (%d)\nMidair?(%d)", player->isGrappling, player->isMidair);

	const int TextStartPosY = 100;
	const int YSpacing = DEBUG_FONT_SIZE * 2;
	int CurrNumText = 0;

	DrawText(PositionText, 50, TextStartPosY + YSpacing * CurrNumText++, DEBUG_FONT_SIZE, BLACK);
	DrawText(VelocityText, 50, TextStartPosY + YSpacing * CurrNumText++, DEBUG_FONT_SIZE, BLACK);
	DrawText(PlayerStateText, 50, TextStartPosY + YSpacing * CurrNumText++, DEBUG_FONT_SIZE, BLACK);
	if (player->isGrappling)
	{
		if (player->CurrGrapplePointCount >= MAX_GRAPPLE_POINTS_LEN)
		{
			player->CurrGrapplePointCount = 0;
		}
		player->GrapplePathPoints[player->CurrGrapplePointCount++] = GetRectanglePosition(player->collider);
		for (int i = 0; i < player->CurrGrapplePointCount; i++)
		{
			DrawCircle(player->GrapplePathPoints[i].x, player->GrapplePathPoints[i].y, 2, RED);
		}
	}
}

void updatePos(Player* player, Rectangle floorColliders[]) {
	const Vector2 HorizontalPlane = { 1,0 }, const PlayerPos = GetRectanglePosition(player->collider), const GrappleRectPos = player->grapplePos;
	float theta = Vector2Angle(Vector2Normalize(HorizontalPlane), Vector2Normalize(Vector2Subtract(PlayerPos, GrappleRectPos)));

	// Updating velocity
	if (!player->isGrappling)
	{
		DrawText("Applying Gravity!", 500, 100, DEBUG_FONT_SIZE, BLACK);

		player->velocity.y += GRAVITY * GetFrameTime();
	}

	if (player->isGrappling) {
		player->velocity.x -= GRAPPLE_FORCE * GetFrameTime() * cosf(theta);
		player->velocity.y -= GRAPPLE_FORCE * GetFrameTime() * sinf(theta);
	}

	// Jump logic
	if (!(player->isGrappling || player->isMidair) && IsKeyPressed(KEY_SPACE)) {
		player->velocity.y = -JUMP_FORCE;
	}

	// Updating player position based on input
	player->collider.x += MOVE_SPEED * getDirectionX() * GetFrameTime();
	// Updating player position based on physics
	player->collider.y += player->velocity.y * GetFrameTime();
	player->collider.x += player->velocity.x * GetFrameTime();

	// Checks if the player moved off the screen and moves the player back if so
	if (player->collider.x + PLAYER_WIDTH > SCREEN_WIDTH) {
		player->collider.x = SCREEN_WIDTH - PLAYER_WIDTH;
		player->velocity.x = 0;
	}
	else if (player->collider.x < 0) {
		player->collider.x = 0;
		player->velocity.x = 0;
	}

	// Grapple movement
	if (player->isGrappling) {

		// Debug Stuff
		char thetaText[100], positionText[1000];

		sprintf(thetaText, "Theta Deg (%.2f)", theta * RAD2DEG);
		DrawText(thetaText, 50, 50, DEBUG_FONT_SIZE, BLACK);
		DrawLine(0, 0, player->collider.x, player->collider.y, PLAYER_COLOR);
		DrawLine(0, 0, player->grapplePos.x, player->grapplePos.y, GRAPPLE_RECT_COLOR);		

		float x = player->currGrappleLength * cosf(theta) + player->grapplePos.x;
		float y = player->currGrappleLength * sinf(theta) + player->grapplePos.y;
		DrawLine(player->grapplePos.x, player->grapplePos.y, x, y, BLACK);

		//player->collider.x = x;
		//player->collider.y = y;
	}

	// Checks if the player moved below the floor and moves the player back if so - also nulls velocity
	bool isColliding = false;
	for (int i = 0; i < 2; i++) {
		if (CheckCollisionRecs(player->collider, floorColliders[i])) {
			player->collider.y = floorColliders[i].y - PLAYER_HEIGHT;
			player->velocity.y = 0;
			player->velocity.x = 0;
			isColliding = true;
		}
	}
	
	player->isMidair = !isColliding;
}