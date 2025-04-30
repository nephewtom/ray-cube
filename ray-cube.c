#include "raylib.h"
#include "raymath.h"
#define SUPPORT_TRACELOG
#define SUPPORT_TRACELOG_DEBUG
#include "utils.h"

#include "rlgl.h"

// Define axis parameters
const float axisLength = 5.0f;  // Length of each axisC
const float coneLength = 0.3f;  // Length of the cone part
const float coneRadius = 0.1f;  // Radius of the cone base
const float lineRadius = 0.02f; // Radius for the axis lines    
void drawAxis() {
    // Draw coordinate axes with cones
    // X axis (red)
    DrawCylinderEx(Vector3Zero(), (Vector3){axisLength, 0, 0}, lineRadius, lineRadius, 8, RED);
    DrawCylinderEx((Vector3){axisLength, 0, 0}, (Vector3){axisLength + coneLength, 0, 0}, coneRadius, 0.0f, 8, RED);

    // Y axis (green)
    DrawCylinderEx(Vector3Zero(), (Vector3){0, axisLength, 0}, lineRadius, lineRadius, 8, GREEN);
    DrawCylinderEx((Vector3){0, axisLength, 0}, (Vector3){0, axisLength + coneLength, 0}, coneRadius, 0.0f, 8, GREEN);

    // Z axis (blue)
    DrawCylinderEx(Vector3Zero(), (Vector3){0, 0, axisLength}, lineRadius, lineRadius, 8, BLUE);
    DrawCylinderEx((Vector3){0, 0, axisLength}, (Vector3){0, 0, axisLength + coneLength}, coneRadius, 0.0f, 8, BLUE);
}

typedef enum Direction {
	DIR_NONE, DIR_FORWARD, DIR_BACKWARD, DIR_RIGHT, DIR_LEFT
} Direction;

struct Cube;
typedef struct Cube {
	Model model;
	Vector3 position;
	Matrix transforms; // Transformations (translate, rotation and scale)
	Matrix rotations;
	Direction direction;

	float animationProgress;
	float smooth;
	
	bool isSliding;
	float slideSpeed;
	Vector3 slideStep;
	Vector3 startPosition;
	Vector3 endPosition;

	bool isRolling;
	float rollingTime;
	Vector3 rotationAxis;
	Vector3 rotationPivot;
	float rotationAngle;
	
	void (*init)(struct Cube *self, Vector3 v);
    void (*update)(struct Cube *self, float delta);
    void (*updateSliding)(struct Cube *self, float delta);
    void (*updateRolling)(struct Cube *self, float delta);
    void (*draw)(struct Cube *self);
} Cube;

void cube_init(Cube *self, Vector3 initPos);
void cube_update(Cube *self, float delta);
void cube_update_sliding(Cube *self, float delta);
void cube_update_rolling(Cube *self, float delta);
void cube_draw(Cube *self);


Cube makeCube() {
	Cube cube;
	cube.init = cube_init;
	cube.update = cube_update;
	cube.updateSliding = cube_update_sliding;
	cube.updateRolling = cube_update_rolling;
	cube.draw = cube_draw;
	
	return cube;
}

void cube_init(Cube *self, Vector3 initPos) {

	self->model = LoadModel("assets/ray-cube.gltf"); // model cube is size 2x2
	self->position = initPos; // it is moved 1.0f in y so it stays on the floor
	self->transforms = MatrixIdentity();
	// Matrix scale = MatrixScale(0.5, 0.5, 0.5);
	// transforms = MatrixMultiply(transforms, scale);
	
	self->isSliding = false;
	self->slideSpeed = 2.5f;
	
	self->isRolling = false;
	self->rollingTime = 0.5f;
	self->rotations = MatrixIdentity(); // cube has no rotations at start
	
	self->animationProgress = 0.0f;
}

void cube_update_sliding(Cube *self, float delta) {
	
	Direction keyDirection = 
		IsKeyPressed(KEY_UP) ? DIR_FORWARD :
		IsKeyPressed(KEY_DOWN) ? DIR_BACKWARD :
		IsKeyPressed(KEY_LEFT) ? DIR_LEFT :
		IsKeyPressed(KEY_RIGHT) ? DIR_RIGHT :
		DIR_NONE;

	if (!self->isSliding && !self->isRolling && keyDirection != DIR_NONE) {

		if (keyDirection == DIR_FORWARD) {
			self->slideStep = (Vector3){0, 0, -2};
		}
		else if (keyDirection == DIR_BACKWARD) {
			self->slideStep = (Vector3){0, 0, 2};
		}		
		else if (keyDirection == DIR_LEFT) {
			self->slideStep = (Vector3){-2, 0, 0};
		}		
		else if (keyDirection == DIR_RIGHT) {
			self->slideStep = (Vector3){2, 0, 0};
		}
		
		self->direction = keyDirection;
		self->startPosition = self->position;
		self->endPosition = Vector3Add(self->position, self->slideStep);
		
		self->isSliding = true;
	}

	if (self->isSliding) {
		
		self->animationProgress += delta * self->slideSpeed;

		// Use smooth easing for animation
		float t = self->animationProgress;
		float smoothT = t * t * (3.0f - 2.0f * t); // Smoothstep formula
		
		self->position = Vector3Lerp(self->startPosition, self->endPosition, smoothT);

		
		Matrix translationToOrigin = MatrixTranslate(-self->position.x, -self->position.y, -self->position.z);
		Matrix translationBackFromOrigin = MatrixTranslate(self->position.x, self->position.y, self->position.z);
		self->transforms = MatrixMultiply(translationToOrigin, self->rotations);
		self->transforms = MatrixMultiply(self->transforms, translationBackFromOrigin);
		
		if (self->animationProgress >= 1.0f) {
			self->position = self->endPosition;
			self->isSliding = false;
			self->animationProgress = 0.0f;
		}
	}
}


void cube_update_rolling(Cube *self, float delta) {

	Direction keyDirection = 
		IsKeyPressed(KEY_W) ? DIR_FORWARD :
		IsKeyPressed(KEY_A) ? DIR_LEFT :
		IsKeyPressed(KEY_S) ? DIR_BACKWARD :
		IsKeyPressed(KEY_D) ? DIR_RIGHT :
		DIR_NONE;

	if (!self->isRolling && !self->isSliding && keyDirection != DIR_NONE) {

		Vector3 offset; // offset for rotationOrigin
		Vector3 translation;
		
		if (keyDirection == DIR_FORWARD) {
			offset = (Vector3){0, -1, -1};
			self->rotationAxis = (Vector3){-1, 0, 0};
			translation = (Vector3){0, 0, -2};
		}
		else if (keyDirection == DIR_BACKWARD) {
			offset = (Vector3){0, -1, 1};
			self->rotationAxis = (Vector3){1, 0, 0};
			translation = (Vector3){0, 0, 2};
		}		
		else if (keyDirection == DIR_LEFT) {
			offset = (Vector3){-1, -1, 0};
			self->rotationAxis = (Vector3){0, 0, 1};
			translation = (Vector3){-2, 0, 0};
		}		
		else if (keyDirection == DIR_RIGHT) {
			offset = (Vector3){1, -1, 0};
			self->rotationAxis = (Vector3){0, 0, -1};
			translation = (Vector3){2, 0, 0};
		}
		
		self->direction = keyDirection;

		self->rotationPivot = Vector3Add(self->position, offset);
		self->endPosition = Vector3Add(self->position, translation);
		self->isRolling = true;
	}

	if (self->isRolling) {
		
		self->animationProgress += delta / self->rollingTime;
		
		float t = self->animationProgress;
		float smoothT = t * t * (3.0f - 2.0f * t); // Smoothstep formula

		self->rotationAngle = Lerp(0.0f, 90.0f, smoothT);
		
		// The cube can have suffered some self->rotations, so it is likely not to be in the initial rotation state.
		// To proceed with the next rotation, first it is needed to reproduce self->its rotation state.
		
		// So, self->for a cube model translated in current position but with a coordinate system not self->self->rotated:
		// 1) it has to be translated to the origin
		// 2) rotated with the stored rotations and 
		// 3) translated back.
		// This is achieved with these matrix multiplications
		Matrix translationToOrigin = MatrixTranslate(-self->position.x, -self->position.y, -self->position.z);
		Matrix translationBackFromOrigin = MatrixTranslate(self->position.x, self->position.y, self->position.z);
		self->transforms = MatrixMultiply(translationToOrigin, self->rotations);
		self->transforms = MatrixMultiply(self->transforms, translationBackFromOrigin);
		
		// Now the current rotation is applied using rotationPivot, rotationAxis and the changing rotationAngle
		// Combining the matrices: first translate to rotationPivot, then rotating, then translate back
		Matrix translationToPivot = MatrixTranslate(-self->rotationPivot.x, -self->rotationPivot.y, -self->rotationPivot.z);
		Matrix rotation = MatrixRotate(self->rotationAxis, self->rotationAngle * DEG2RAD);
		Matrix translationBackFromPivot = MatrixTranslate(self->rotationPivot.x, self->rotationPivot.y, self->rotationPivot.z);
		
		Matrix translationPlusRotation = MatrixMultiply(translationToPivot, rotation);
		self->transforms = MatrixMultiply(self->transforms, translationPlusRotation);
		self->transforms = MatrixMultiply(self->transforms, translationBackFromPivot);
		
		// Matrix scale = MatrixScale(0.5, 0.5, 0.5);
		// self->transforms = MatrixMultiply(self->transforms, scale);

		if (self->animationProgress >= 1.0f) {

			self->position = self->endPosition;
			self->isRolling = false;
			self->animationProgress = 0.0f;

			// Add the finished rotation (always 90 degrees) to accumulated rotations
			Matrix finishedRotation = MatrixRotate(self->rotationAxis, 90.0f * DEG2RAD);
			self->rotations = MatrixMultiply(self->rotations, finishedRotation);
			
			Matrix translationToOrigin = MatrixTranslate(-self->position.x, -self->position.y, -self->position.z);
			Matrix translationBackFromOrigin = MatrixTranslate(self->position.x, self->position.y, self->position.z);
			self->transforms = MatrixMultiply(translationToOrigin, self->rotations);
			self->transforms = MatrixMultiply(self->transforms, translationBackFromOrigin);
			// transforms = MatrixMultiply(transforms, scale);
		}
	}
}

void cube_update(Cube *self, float delta) {

	self->updateSliding(self, delta);
	self->updateRolling(self, delta);
}

void cube_draw(Cube *self) {
	
	rlPushMatrix();
	rlMultMatrixf(MatrixToFloat(self->transforms));
	DrawModel(self->model, self->position, 1.0f, WHITE);
	rlPopMatrix();
}

bool cameraUpdateEnabled = false;

void drawText(Cube *cube) {
	DrawFPS(10, 10);
	
	DrawText(TextFormat("F1 - toggle camera control: %s", cameraUpdateEnabled ? "ON" : "OFF"),
			 10, 30, 20, BLUE);

	DrawText(TextFormat("WASD & Arrows - control %s", cameraUpdateEnabled ? "camera" : "cube"),
			 10, 60, 20, BLUE);

	if (!cameraUpdateEnabled) {
		
		DrawText("Some data", 10, 150, 20, RED);		
		DrawText(TextFormat("position: (%.2f, %.2f, %.2f)", cube->position.x, cube->position.y, cube->position.z), 
				 10, 170, 20, DARKGRAY);
		DrawText(TextFormat("endPosition: (%.2f, %.2f, %.2f)", 
							cube->endPosition.x, cube->endPosition.y, cube->endPosition.z), 
				 10, 190, 20, DARKGRAY);
		DrawText(TextFormat("direction: %s", 
							cube->direction == DIR_FORWARD ? "forward" :
							cube->direction == DIR_BACKWARD ? "backward" :
							cube->direction == DIR_RIGHT ? "right" :
							cube->direction == DIR_LEFT ? "left" : "idle"),
				 10, 210, 20, DARKGRAY);

		
		DrawText("Arrows slide the cube", 10, 280, 20, RED);
		DrawText(TextFormat("isSliding: %s", cube->isSliding ? "true" : "false"),
				 10, 300, 20, DARKGRAY);
		DrawText(TextFormat("slideStep: (%.1f, %.1f, %.1f)", cube->slideStep.x, cube->slideStep.y, cube->slideStep.z), 
				 10, 320, 20, DARKGRAY);

		
		
		DrawText("WASD rolls the cube", 10, 400, 20, RED);
		DrawText(TextFormat("isRolling: %s", cube->isRolling ? "true" : "false"),
				 10, 420, 20, DARKGRAY);		
		DrawText(TextFormat("rotationAngle: %.2f", cube->rotationAngle),
				 10, 440, 20, DARKGRAY);

		
		DrawText(TextFormat("animationProgress: %.2f", cube->animationProgress),
				 1000, 30, 20, DARKGRAY);
	}
}

int main(void)
{
	const int screenWidth = 1280;
	const int screenHeight = 720;

	InitWindow(screenWidth, screenHeight, "Raylib Rotating Cube");
	SetTraceLogLevel(LOG_ALL);
	SetTargetFPS(60);
	
	Camera3D camera;
	camera.position = (Vector3){ 5.0f, 10.0f, 10.0f };
	camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
	camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
	camera.fovy = 45.0f;
	camera.projection = CAMERA_PERSPECTIVE;

	Cube cube = makeCube();
	cube.init(&cube, (Vector3){1, 1, 1});
	
	while (!WindowShouldClose())
	{
		float delta = GetFrameTime();

		if (IsKeyPressed(KEY_F1)) {
			cameraUpdateEnabled = !cameraUpdateEnabled;
		}
		
		if (cameraUpdateEnabled) {
			UpdateCamera(&camera, CAMERA_THIRD_PERSON);
		} else {
			cube.update(&cube, delta);
		}
		
		BeginDrawing();
		ClearBackground(RAYWHITE);

		BeginMode3D(camera);
		{
			cube.draw(&cube);
			DrawGrid(10, 2.0f);
			drawAxis();
		}
		EndMode3D();

		drawText(&cube);
		
		EndDrawing();
	}

	CloseWindow();
    
	return 0;
}

