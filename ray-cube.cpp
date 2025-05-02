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

enum Direction {
	DIR_NONE, DIR_FORWARD, DIR_BACKWARD, DIR_RIGHT, DIR_LEFT
};


struct Cube {
	Model model;
	Vector3 position;
	Matrix transforms; // Transformations (translate, rotation and scale)
	Matrix rotations;
	Direction direction;

	float animationProgress = 0.0f;
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
	
	void init(Vector3 pos, bool smooth = true);
	void update(float);
	void updateSliding(float);
	void updateRolling(float);
	void draw();
	
	Sound rollWav;
	Sound slideWav;
};
Cube cube;

void Cube::init(Vector3 initPos, bool smoothBehaviour) {

	model = LoadModel("assets/ray-cube.gltf"); // model cube is size 2x2
	position = initPos; // it is moved 1.0f in y so it stays on the floor
	transforms = MatrixIdentity();
	// Matrix scale = MatrixScale(0.5, 0.5, 0.5);
	// transforms = MatrixMultiply(transforms, scale);
	
	isSliding = false;
	slideSpeed = 2.5f;
	
	isRolling = false;
	rollingTime = 0.5f;
	rotations = MatrixIdentity(); // cube has no rotations at start
	
	animationProgress = 0.0f;
	smooth = smoothBehaviour;
	
	TRACELOGD("Loading Sound...");
	rollWav = LoadSound("assets/roll.wav");
	slideWav = LoadSound("assets/slide.wav");
	TRACELOGD("Finished loading Sound");
}

void Cube::updateSliding(float delta) {
	
	Direction keyDirection = 
		IsKeyPressed(KEY_UP) ? DIR_FORWARD :
		IsKeyPressed(KEY_DOWN) ? DIR_BACKWARD :
		IsKeyPressed(KEY_LEFT) ? DIR_LEFT :
		IsKeyPressed(KEY_RIGHT) ? DIR_RIGHT :
		DIR_NONE;

	if (!isSliding && !isRolling && keyDirection != DIR_NONE) {

		if (keyDirection == DIR_FORWARD) {
			slideStep = {0, 0, -2};
		}
		else if (keyDirection == DIR_BACKWARD) {
			slideStep = {0, 0, 2};
		}		
		else if (keyDirection == DIR_LEFT) {
			slideStep = {-2, 0, 0};
		}		
		else if (keyDirection == DIR_RIGHT) {
			slideStep = {2, 0, 0};
		}
		
		direction = keyDirection;
		startPosition = position;
		endPosition = Vector3Add(position, slideStep);
		
		isSliding = true;
		PlaySound(slideWav);
	}

	if (isSliding) {
		
		animationProgress += delta * slideSpeed;

		// Use smooth easing for animation
		float t = animationProgress;
		float smoothT = t * t * (3.0f - 2.0f * t); // Smoothstep formula
		
		if (smooth) {
			position = Vector3Lerp(startPosition, endPosition, smoothT);
		} else {
			position = Vector3Lerp(startPosition, endPosition, t);
		}

		Matrix translationToOrigin = MatrixTranslate(-position.x, -position.y, -position.z);
		Matrix translationBackFromOrigin = MatrixTranslate(position.x, position.y, position.z);
		transforms = MatrixMultiply(translationToOrigin, rotations);
		transforms = MatrixMultiply(transforms, translationBackFromOrigin);
		
		if (animationProgress >= 1.0f) {
			position = endPosition;
			isSliding = false;
			animationProgress = 0.0f;			
		}
	}
}


void Cube::updateRolling(float delta) {

	Direction keyDirection = 
		IsKeyPressed(KEY_W) ? DIR_FORWARD :
		IsKeyPressed(KEY_A) ? DIR_LEFT :
		IsKeyPressed(KEY_S) ? DIR_BACKWARD :
		IsKeyPressed(KEY_D) ? DIR_RIGHT :
		DIR_NONE;

	if (!isRolling && !isSliding && keyDirection != DIR_NONE) {

		Vector3 offset; // offset for rotationOrigin
		Vector3 translation;
		
		if (keyDirection == DIR_FORWARD) {
			offset = {0, -1, -1};
			rotationAxis = {-1, 0, 0};
			translation = {0, 0, -2};
		}
		else if (keyDirection == DIR_BACKWARD) {
			offset = {0, -1, 1};
			rotationAxis = {1, 0, 0};
			translation = {0, 0, 2};
		}		
		else if (keyDirection == DIR_LEFT) {
			offset = {-1, -1, 0};
			rotationAxis = {0, 0, 1};
			translation = {-2, 0, 0};
		}		
		else if (keyDirection == DIR_RIGHT) {
			offset = {1, -1, 0};
			rotationAxis = {0, 0, -1};
			translation = {2, 0, 0};
		}
		
		direction = keyDirection;

		rotationPivot = Vector3Add(position, offset);
		endPosition = Vector3Add(position, translation);
		isRolling = true;
	}

	if (isRolling) {
		
		animationProgress += delta / rollingTime;
		
		float t = animationProgress;
		float smoothT = t * t * (3.0f - 2.0f * t); // Smoothstep formula

		if (smooth) {
			rotationAngle = Lerp(0.0f, 90.0f, smoothT);
		} else {
			rotationAngle = Lerp(0.0f, 90.0f, t);
		}
		
		// The cube can have suffered some rotations, so it is likely not to be in the initial rotation state.
		// To proceed with the next rotation, first it is needed to reproduce its rotation state.
		
		// So, for a cube model translated in current position but with a coordinate system not rotated:
		// 1) it has to be translated to the origin
		// 2) rotated with the stored rotations and 
		// 3) translated back.
		// This is achieved with these matrix multiplications
		Matrix translationToOrigin = MatrixTranslate(-position.x, -position.y, -position.z);
		Matrix translationBackFromOrigin = MatrixTranslate(position.x, position.y, position.z);
		transforms = MatrixMultiply(translationToOrigin, rotations);
		transforms = MatrixMultiply(transforms, translationBackFromOrigin);
		
		// Now the current rotation is applied using rotationPivot, rotationAxis and the changing rotationAngle
		// Combining the matrices: first translate to rotationPivot, then rotating, then translate back
		Matrix translationToPivot = MatrixTranslate(-rotationPivot.x, -rotationPivot.y, -rotationPivot.z);
		Matrix rotation = MatrixRotate(rotationAxis, rotationAngle * DEG2RAD);
		Matrix translationBackFromPivot = MatrixTranslate(rotationPivot.x, rotationPivot.y, rotationPivot.z);
		
		Matrix translationPlusRotation = MatrixMultiply(translationToPivot, rotation);
		transforms = MatrixMultiply(transforms, translationPlusRotation);
		transforms = MatrixMultiply(transforms, translationBackFromPivot);
		
		// Matrix scale = MatrixScale(0.5, 0.5, 0.5);
		// transforms = MatrixMultiply(transforms, scale);

		if (animationProgress >= 1.0f) {

			position = endPosition;
			isRolling = false;
			animationProgress = 0.0f;

			// Add the finished rotation (always 90 degrees) to accumulated rotations
			Matrix finishedRotation = MatrixRotate(rotationAxis, 90.0f * DEG2RAD);
			rotations = MatrixMultiply(rotations, finishedRotation);
			
			Matrix translationToOrigin = MatrixTranslate(-position.x, -position.y, -position.z);
			Matrix translationBackFromOrigin = MatrixTranslate(position.x, position.y, position.z);
			transforms = MatrixMultiply(translationToOrigin, rotations);
			transforms = MatrixMultiply(transforms, translationBackFromOrigin);

			PlaySound(rollWav);
		}
	}
}

void Cube::update(float delta) {

	updateSliding(delta);
	updateRolling(delta);
}

void Cube::draw() {
	
	rlPushMatrix();
	rlMultMatrixf(MatrixToFloat(transforms));
	DrawModel(model, position, 1.0f, WHITE);
	rlPopMatrix();
}

bool cameraUpdateEnabled = false;
Camera3D camera;


#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

void DrawScaledText(const char* text, int x, int y, int fontSize, Color color) {
    float scaleX = (float)GetScreenWidth() / SCREEN_WIDTH;
    float scaleY = (float)GetScreenHeight() / SCREEN_HEIGHT;
    float scale = fminf(scaleX, scaleY); // keep aspect ratio

    DrawText(text, x * scale, y * scale, fontSize * scale, color);
}


void drawText() {
	DrawFPS(10, 10);
	
	DrawScaledText(TextFormat("T - toggle camera control: %s", cameraUpdateEnabled ? "ON" : "OFF"),
			 10, 30, 20, BLUE);

	DrawScaledText(TextFormat("WASD & Arrows - control %s", cameraUpdateEnabled ? "camera" : "cube"),
			 10, 60, 20, BLUE);

	if (!cameraUpdateEnabled) {
		
		DrawScaledText("Some data", 10, 150, 20, RED);		
		DrawScaledText(TextFormat("position: (%.2f, %.2f, %.2f)", cube.position.x, cube.position.y, cube.position.z), 
					   10, 170, 20, DARKGRAY);
		DrawScaledText(TextFormat("endPosition: (%.2f, %.2f, %.2f)", 
								  cube.endPosition.x, cube.endPosition.y, cube.endPosition.z), 
				 10, 190, 20, DARKGRAY);
		DrawScaledText(TextFormat("direction: %s", 
								  cube.direction == DIR_FORWARD ? "forward" :
							cube.direction == DIR_BACKWARD ? "backward" :
							cube.direction == DIR_RIGHT ? "right" :
							cube.direction == DIR_LEFT ? "left" : "idle"),
				 10, 210, 20, DARKGRAY);

		
		DrawScaledText("Arrows slide the cube", 10, 280, 20, RED);
		DrawScaledText(TextFormat("isSliding: %s", cube.isSliding ? "true" : "false"),
					   10, 300, 20, DARKGRAY);
		DrawScaledText(TextFormat("slideStep: (%.1f, %.1f, %.1f)", cube.slideStep.x, cube.slideStep.y, cube.slideStep.z), 
					   10, 320, 20, DARKGRAY);

		
		
		DrawScaledText("WASD rolls the cube", 10, 400, 20, RED);
		DrawScaledText(TextFormat("isRolling: %s", cube.isRolling ? "true" : "false"),
					   10, 420, 20, DARKGRAY);		
		DrawScaledText(TextFormat("rotationAngle: %.2f", cube.rotationAngle),
					   10, 440, 20, DARKGRAY);

		
		DrawScaledText(TextFormat("animationProgress: %.2f", cube.animationProgress),
					   1000, 30, 20, DARKGRAY);
	}
}

void updateFrameWindow() {
	
	float delta = GetFrameTime();

	if (IsKeyPressed(KEY_T)) {
		cameraUpdateEnabled = !cameraUpdateEnabled;
		if (cameraUpdateEnabled) {
			SetMousePosition(GetScreenWidth() /2, GetScreenHeight() /2);
		}
	}

	if (cameraUpdateEnabled) {
		UpdateCamera(&camera, CAMERA_THIRD_PERSON);
	} else {
		cube.update(delta);
	}
	
	BeginDrawing();
	ClearBackground(RAYWHITE);

	BeginMode3D(camera);
	{
		cube.draw();
		DrawGrid(10, 2.0f);
		drawAxis();
	}
	EndMode3D();
            
	drawText();
	EndDrawing();
}

#ifdef PLATFORM_WEB
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

EM_BOOL ResizeCallback(int eventType, const EmscriptenUiEvent *e, void *userData) {
    double w, h;
    emscripten_get_element_css_size("#canvas", &w, &h);
    SetWindowSize((int)w, (int)h);
    return EM_TRUE;
}
#endif

int main(void)
{
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Raylib Rotating Cube");
	

	SetTraceLogLevel(LOG_ALL);
	SetTargetFPS(60);
	
	InitAudioDevice();
	
	camera.position = (Vector3){ 3.0f, 10.0f, 12.0f };
	camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
	camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
	camera.fovy = 45.0f;
	camera.projection = CAMERA_PERSPECTIVE;

	cube.init({1, 1, 1}, true);
	
#ifdef PLATFORM_WEB
#ifdef RESIZE_CALLBACK
	emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, EM_TRUE, ResizeCallback);
	ResizeCallback(0, NULL, NULL);
#endif
	emscripten_set_main_loop(updateFrameWindow, 0, 1);	
#else
	while (!WindowShouldClose()) {
		updateFrameWindow();
	}
	CloseWindow();
#endif		
    
	return 0;
}
