#include "rcore.c"

// Grappleable Objects
#define MAX_NUM_GRAP_RECTS 10
typedef struct GrappleableRect {
	Rectangle collider;
} GrappleableRect;

#define PLAYER_WIDTH 50
#define PLAYER_HEIGHT 50
#define MOVE_SPEED 200
//#define GRAVITY 980
#define GRAVITY 380
#define GRAPPLE_FORCE GRAVITY
#define MAX_GRAPPLE_POINTS_LEN 5000
#define JUMP_FORCE 350

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define FLOOR_HEIGHT 150

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
	float CurrMaxGrappleLength;
	Vector2 GrapplePathPoints[MAX_GRAPPLE_POINTS_LEN];
	int CurrGrapplePointCount;
	bool IsInGrapplingWindow;
	Rectangle LastCollision;
} Player;

int getDirectionX();
void updatePos(Player*, Rectangle[]);
void updateVelocityY(Player*, Rectangle*);
void Grapple();
void DrawPlayer(Player* player);
void DrawRectangles(Rectangle rectangles[], int numRectangles, Color colorToDraw);
void DrawPlayerDebug(Player* player);
void DrawArrow(Vector2 start, Vector2 end, Color color);
float tensionConstant = 500;

Vector2 GetRectanglePosition(Rectangle rect) {
	return (Vector2) { rect.x, rect.y };
}

Vector2 GetPlayerPos(Player* player) {
	return GetRectanglePosition(player->collider);
}

int main() {
	static const int FPS = 60;
	static const char* GAME_TITLE = "Grappler";
	static Player player = { { 100, 300, PLAYER_WIDTH, PLAYER_HEIGHT }, { 0, 50 } };
	static Rectangle floors[] = { {0, SCREEN_HEIGHT - FLOOR_HEIGHT, FLOOR_HEIGHT, FLOOR_HEIGHT}, {600, SCREEN_HEIGHT - FLOOR_HEIGHT, SCREEN_WIDTH, FLOOR_HEIGHT} };
	GrappleableRect grappleableRects[MAX_NUM_GRAP_RECTS] = { 
		{ SCREEN_WIDTH / 4, 300, PLAYER_WIDTH, PLAYER_HEIGHT },
		{ SCREEN_WIDTH / 2, 300, PLAYER_WIDTH, PLAYER_HEIGHT },
	};
	static const int currNumRects = 2;
	float CurrMaxGrappleLength;

	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE);
	//HideCursor();
	SetTargetFPS(FPS);
	//SetupGrappleRects(GrappleableRects);
	while (!WindowShouldClose()) {
		//updatePos(&player, floors);
		BeginDrawing();
		ClearBackground(BLUE);
		DrawRectangles(floors, sizeof(floors) / sizeof(Rectangle), FLOOR_COLOR);
		Vector2 MousePos = GetMousePosition();
		bool foundGrapple = false;
		for (int currRect = 0; currRect < 2; currRect++)
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
					foundGrapple = true;
					if (!player.isGrappling) {
						player.isGrappling = true;
						player.grapplePos = GetRectanglePosition(currRectRef->collider);
						player.CurrMaxGrappleLength = Vector2Distance((Vector2) { player.collider.x, player.collider.y }, (Vector2) { currRectRef->collider.x, currRectRef->collider.y });
						player.CurrGrapplePointCount = 0;
						
						DrawLine(player.collider.x + PLAYER_WIDTH / 2, player.collider.y + PLAYER_HEIGHT / 2, GetMouseX(), GetMouseY(), RED);
					}
					ColorToDrawRect = PURPLE;
				}
				else {
					//player.isGrappling = false;
				}

				if (IsKeyDown(KEY_E)) {
					tensionConstant += 10;
				}

				if (IsKeyDown(KEY_Q)) {
					tensionConstant -= 10;
				}
			}
			else {
				//player.isGrappling = false;
			}
			DrawRectangleRec(currRectRef->collider, ColorToDrawRect);
		}
		player.isGrappling = foundGrapple;

		if (IsKeyDown(KEY_ESCAPE))
		{
			CloseWindow();
		}

		if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
		{
			player.collider.x = MousePos.x;
			player.collider.y = MousePos.y;
			player.velocity = Vector2Zero();
		}
		updatePos(&player, floors);
		DrawPlayer(&player);
		DrawPlayerDebug(&player);
		EndDrawing();
	}
	CloseWindow();
	return 0;
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
	char PositionText[1000], VelocityText[1000], PlayerStateText[1000], TensionText[1000];
	sprintf(PositionText, "Player Pos (%.2f,%.2f)\nRect Pos (%.2f,%.2f)", player->collider.x, player->collider.y, player->grapplePos.x, player->grapplePos.y);
	sprintf(VelocityText, "Player Velocity (%.2f,%.2f)", player->velocity.x, player->velocity.y);
	sprintf(PlayerStateText, "Grappling? (%d)\nMidair?(%d)", player->isGrappling, player->isMidair);
	sprintf(TensionText, "Tension (%.2f)\nTension Constant (%.2f)");

	const int TextStartPosY = 100;
	const int YSpacing = DEBUG_FONT_SIZE * 2;
	int CurrNumText = 0;

	DrawText(PositionText, 50, TextStartPosY + YSpacing * CurrNumText++, DEBUG_FONT_SIZE, BLACK);
	DrawText(VelocityText, 50, TextStartPosY + YSpacing * CurrNumText++, DEBUG_FONT_SIZE, BLACK);
	DrawText(PlayerStateText, 50, TextStartPosY + YSpacing * CurrNumText++, DEBUG_FONT_SIZE, BLACK);
	//DrawLine(GetPlayerPos(player).x, GetPlayerPos(player).y, GetPlayerPos(player).x + player->velocity.x, GetPlayerPos(player).y + player->velocity.y, PLAYER_COLOR);
	DrawArrow(GetPlayerPos(player), Vector2Add(GetPlayerPos(player), player->velocity), PLAYER_COLOR);
	if (player->isGrappling)
	{

		if (player->CurrGrapplePointCount >= MAX_GRAPPLE_POINTS_LEN)
		{
			player->CurrGrapplePointCount = 0;
		}
		player->GrapplePathPoints[player->CurrGrapplePointCount++] = GetRectanglePosition(player->collider);
		for (int i = 0; i < player->CurrGrapplePointCount; i++)
		{
			DrawCircle(player->GrapplePathPoints[i].x, player->GrapplePathPoints[i].y, 1, RED);
		}
	}

	DrawRectangleRec(player->LastCollision, DEFAULT_COLOR);
}
/*
void updatePos(Player* player, Rectangle floorColliders[]) {
	const Vector2 HorizontalPlane = { 1,0 };//, const PlayerPos = GetRectanglePosition(player->collider), const GrappleRectPos = player->grapplePos;
	float theta = Vector2Angle(Vector2Normalize(HorizontalPlane), Vector2Normalize(Vector2Subtract(GetPlayerPos(player), player->grapplePos)));

	// Jump logic
	if (!(player->isGrappling || player->isMidair) && IsKeyPressed(KEY_SPACE)) {
		player->velocity.y = -JUMP_FORCE;
	}

	// Checks if the player moved off the screen and moves the player back if so
	if (player->collider.x + PLAYER_WIDTH > SCREEN_WIDTH) {
		player->collider.x = SCREEN_WIDTH - PLAYER_WIDTH;
		player->velocity.x = 0;
	}
	else if (player->collider.x < 0) {
		player->collider.x = 0;
		player->velocity.x = 0;
	}

	// Updating velocity
	//if (!player->isGrappling)
	{
		DrawText("Applying Gravity!", 500, 100, DEBUG_FONT_SIZE, BLACK);

		player->velocity.y += GRAVITY * GetFrameTime();
	}

	// Updating player position based on input
	player->collider.x += MOVE_SPEED * getDirectionX() * GetFrameTime();
	// Updating player position based on physics
	player->collider.y += player->velocity.y * GetFrameTime();
	player->collider.x += player->velocity.x * GetFrameTime();

	// Grapple movement
	if (player->isGrappling) {

		// Debug Stuff
		char thetaText[100], positionText[1000];

		sprintf(thetaText, "Theta Deg (%.2f)", theta * RAD2DEG);
		DrawText(thetaText, 50, 50, DEBUG_FONT_SIZE, BLACK);
	

		float x = player->CurrMaxGrappleLength * cosf(theta) + player->grapplePos.x;
		float y = player->CurrMaxGrappleLength * sinf(theta) + player->grapplePos.y;
		DrawLine(player->grapplePos.x, player->grapplePos.y, x, y, BLACK);

		const float currGrappleLength = Vector2Distance(GetPlayerPos(player), player->grapplePos);
		const bool isGrappleAtMaxLength = currGrappleLength > player->CurrMaxGrappleLength;

		if (isGrappleAtMaxLength)
		{
			DrawText("Grapple Taught!", 500, 75, DEBUG_FONT_SIZE, BLACK);

			float currVelocityMagnitude = Vector2Length(player->velocity);
			if (currVelocityMagnitude > EPSILON) {
				Vector2 playerToGrappleDir = Vector2Normalize(Vector2Subtract(GetPlayerPos(player), player->grapplePos));
				int angleToRotate = 90;
				if (player->velocity.x > 0 && ((GetTime() - player->lastFlipTime) > 10) && theta < PI && theta > PI/2)
				{
					angleToRotate *= -1;
					player->lastFlipTime = GetTime();
				}

				// Tension
				{
					const float tension = GRAPPLE_FORCE / Vector2Distance(GetPlayerPos(player), player->grapplePos) * tensionConstant * GetFrameTime();
					player->velocity.x -= tension * cosf(theta);
					player->velocity.y -= tension * sinf(theta);

					char TensionText[1000];
					sprintf(TensionText, "Tension (%.2f)\nTension Constant (%.2f)", tension, tensionConstant);
					DrawText(TensionText, 500, 150, DEBUG_FONT_SIZE, BLACK);
				}
				Vector2 rotatedDir = Vector2Rotate(playerToGrappleDir, angleToRotate * DEG2RAD);
				//Vector2 rotatedDir = {1,0};
				Vector2 newVelocity = Vector2Scale(rotatedDir, currVelocityMagnitude);
				player->velocity = newVelocity;


				
			}
		}
	}



	// Checks if the player moved below the floor and moves the player back if so - also nulls velocity
	bool isColliding = false;
	for (int i = 0; i < 0; i++) {
		if (CheckCollisionRecs(player->collider, floorColliders[i])) {
			player->collider.y = floorColliders[i].y - PLAYER_HEIGHT;
			player->velocity.y = 0;
			player->velocity.x = 0;
			isColliding = true;
		}
	}
	player->isMidair = !isColliding;
}*/

void DrawVector(Vector2 start, Vector2 end, Color color) {
	DrawLine(start.x, start.y, end.x, end.y, color);
}

void DrawArrow(Vector2 start, Vector2 end, Color color) {
	DrawVector(start, end, color);
	Vector2 startToEnd = Vector2Subtract(end, start);
	const float tickLength = 50;
	Vector2 direction = Vector2Normalize(startToEnd);
	Vector2 leftTick = Vector2Rotate(direction, 160 * DEG2RAD);
	Vector2 rightTick = Vector2Rotate(direction, -160 * DEG2RAD);
	DrawVector(end, Vector2Add(end, Vector2Scale(leftTick, tickLength)), color);
	DrawVector(end, Vector2Add(end, Vector2Scale(rightTick, tickLength)), color);
}

void updatePos(Player* player, Rectangle floorColliders[]) {
	player->velocity.y += GRAVITY * GetFrameTime();

	if (player->isGrappling) {
		DrawArrow(GetPlayerPos(player), player->grapplePos, DEFAULT_COLOR);
		
		const float currGrappleLength = Vector2Distance(GetPlayerPos(player), player->grapplePos);
		const bool isGrappleAtMaxLength = currGrappleLength > player->CurrMaxGrappleLength;
		if (isGrappleAtMaxLength) {
			Vector2 direction = Vector2Normalize(Vector2Subtract(player->grapplePos, GetPlayerPos(player)));
			float tension = GRAVITY * GetFrameTime() / pow(Vector2Distance(GetPlayerPos(player), player->grapplePos), 1) * tensionConstant;
			Vector2 tensionVector = Vector2Scale(direction, tension);
			player->velocity = Vector2Add(player->velocity, tensionVector);

			char TensionText[1000];
			sprintf(TensionText, "Tension (%.2f)\nTension Constant (%.2f)", tension, tensionConstant);
			DrawText(TensionText, 500, 150, DEBUG_FONT_SIZE, BLACK);
		}
	}

	// Set velocity with user input
	if (IsKeyDown(KEY_D) && IsKeyDown(KEY_A))
	{
		player->velocity.x = 0;
	}
	else if (IsKeyDown(KEY_A))
	{
		player->velocity.x = -MOVE_SPEED;
	}
	else if (IsKeyDown(KEY_D))
	{
		player->velocity.x = MOVE_SPEED;
	}

	if (IsKeyPressed(KEY_SPACE) && !player->isMidair)
	{
		player->velocity.y = -JUMP_FORCE;
	}

	// Apply velocity to position
	player->collider.x += player->velocity.x * GetFrameTime();
	player->collider.y += player->velocity.y * GetFrameTime();

	bool isColliding = false;
	if (!player->isGrappling) {
		for (int i = 0; i < 2; i++) {
			if (CheckCollisionRecs(player->collider, floorColliders[i])) {
				Rectangle CollisionRec = GetCollisionRec(player->collider, floorColliders[i]);
				player->LastCollision = CollisionRec;
				// Did we collide on the...
				//  left?
				if (CollisionRec.x == player->collider.x && CollisionRec.width < player->collider.width)
				{
					DrawText("Left", 50, 250, DEBUG_FONT_SIZE, BLACK);
					player->collider.x += CollisionRec.width;
					CollisionRec = GetCollisionRec(player->collider, floorColliders[i]);
				}
				//	bottom?
				if ((player->collider.height + player->collider.y) > CollisionRec.y)
				{
					DrawText("Bottom", 50, 310, DEBUG_FONT_SIZE, BLACK);
					player->collider.y -= CollisionRec.height;
					CollisionRec = GetCollisionRec(player->collider, floorColliders[i]);
				}
				//	right?
				//if ((CollisionRec.x + CollisionRec.width) == (player->collider.x + player->collider.width))
				//{
				//	DrawText("Right", 50, 270, DEBUG_FONT_SIZE, BLACK);
				//	player->collider.x -= CollisionRec.width;
				//}
				//  top?
				//if (CollisionRec.y == player->collider.y)
				//{
				//	DrawText("Top", 50, 290, DEBUG_FONT_SIZE, BLACK);
				//	player->collider.y += CollisionRec.height;
				//}

				//player->collider.y = floorColliders[i].y - PLAYER_HEIGHT;
				player->velocity.y = 0;
				player->velocity.x = 0;
				isColliding = true;
				break;
			}
		}
	player->isMidair = !isColliding;

	}
}