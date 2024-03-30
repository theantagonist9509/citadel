#include <alloca.h>
#include <stdint.h>
#include <stdio.h> // Debugging
#include <stdlib.h> // rand()
#include <time.h>

#include <raylib.h>
#include <raymath.h>

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

typedef struct {
	Vector2 position;
	Vector2 velocity;
	Vector2 acceleration;
} TankPhysics;

typedef struct {
	uint8_t path_segment_index;
	float health;
} TankLogic;

typedef struct {
	Rectangle atlas_source_rectangle;
	Rectangle destination_rectangle;
	float rotation;
} TankDrawData; // rename to TankDrawing or TankDrawingData

typedef struct {
	Rectangle rectangle;
	Color rectangle_color;
	Color text_color;
	char *text;
} ButtonDrawData;

typedef enum {
	GAME,
	SETTINGS,
	QUIT,
	TITLE_SCREEN,
} MetaState;

typedef struct {
	Vector2 tanks_velocity;
	float *tanks_seconds_since_last_tick;
	Rectangle *buttons_original_rectangles;
	uint8_t tanks_count;
	uint8_t buttons_count;
} TitleScreenState;

typedef struct {
	Texture2D texture_atlas;
	Color background_color;
	TankDrawData *tanks_draw_data;
	ButtonDrawData *buttons_draw_data;
	uint8_t tanks_count;
	uint8_t buttons_count;
} TitleScreenDrawData;




typedef struct {
	Vector2 *tanks_path_points;
	TankLogic *tanks_logic;
	uint8_t tanks_count;
	uint8_t tanks_path_points_count;
} GameLogic;

typedef struct {
	TankPhysics *tanks_physics;
	uint8_t tanks_count;
} GamePhysics;

typedef struct {
	Texture2D texture_atlas;
	TankDrawData *tanks_draw_data;
	Vector2 *tanks_path_points;
	float tanks_seconds_since_last_tick;
	uint16_t tanks_texture_x_offset;
	uint8_t tanks_count;
	uint8_t tanks_path_points_count;
} GameDrawData;




void drawBackground(Texture2D texture_atlas)
{
	DrawTexturePro(
		texture_atlas,
		(Rectangle) {
			.x = 0,
			.y = 300,
			.width = 200,
			.height = ((float) WINDOW_HEIGHT / WINDOW_WIDTH) * 200,
		},
		(Rectangle) {
			.x = 0,
			.y = 0,
			.width = WINDOW_WIDTH,
			.height = WINDOW_HEIGHT,
		},
		(Vector2) {0, 0},
		0,
		WHITE
	);
}



void drawTank(TankDrawData const *draw_data, Texture2D texture_atlas)
{
	DrawTexturePro(
		texture_atlas,
		draw_data->atlas_source_rectangle,
		draw_data->destination_rectangle,
		(Vector2) {draw_data->destination_rectangle.width / 2, draw_data->destination_rectangle.height / 2},
		draw_data->rotation,
		WHITE
	);
}

void enlargeButton(ButtonDrawData *draw_data, Rectangle const *original_rectangle)
{
#define BUTTON_ENLARGE_FACTOR 4.f / 3
	draw_data->rectangle.x = original_rectangle->x - original_rectangle->width * ((BUTTON_ENLARGE_FACTOR - 1) / 2);
	draw_data->rectangle.y = original_rectangle->y - original_rectangle->height * ((BUTTON_ENLARGE_FACTOR - 1) / 2);
	draw_data->rectangle.width = BUTTON_ENLARGE_FACTOR * original_rectangle->width;
	draw_data->rectangle.height = BUTTON_ENLARGE_FACTOR * original_rectangle->height;
#undef BUTTON_ENLARGE_FACTOR
}

#define TITLE_SCREEN_SPAWN_PADDING 200
void updateMetaStateAndTitleScreen(MetaState *meta_state, TitleScreenState const *title_screen_state, TitleScreenDrawData *title_screen_draw_data)
{
	for (uint8_t i = 0; i < title_screen_state->tanks_count; i++) {
		title_screen_state->tanks_seconds_since_last_tick[i] += GetFrameTime();

		title_screen_draw_data->tanks_draw_data[i].destination_rectangle.x += title_screen_state->tanks_velocity.x * GetFrameTime();
		title_screen_draw_data->tanks_draw_data[i].destination_rectangle.y += title_screen_state->tanks_velocity.y * GetFrameTime();

		if (title_screen_draw_data->tanks_draw_data[i].destination_rectangle.x > WINDOW_WIDTH + TITLE_SCREEN_SPAWN_PADDING)
			title_screen_draw_data->tanks_draw_data[i].destination_rectangle.x = -TITLE_SCREEN_SPAWN_PADDING;
		if (title_screen_draw_data->tanks_draw_data[i].destination_rectangle.y < -TITLE_SCREEN_SPAWN_PADDING)
			title_screen_draw_data->tanks_draw_data[i].destination_rectangle.y = WINDOW_HEIGHT + TITLE_SCREEN_SPAWN_PADDING;

		if (title_screen_state->tanks_seconds_since_last_tick[i] > 0.05f) {
			if (title_screen_draw_data->tanks_draw_data[i].atlas_source_rectangle.x == 0)
				title_screen_draw_data->tanks_draw_data[i].atlas_source_rectangle.x = 100;
			else
				title_screen_draw_data->tanks_draw_data[i].atlas_source_rectangle.x = 0;

			title_screen_state->tanks_seconds_since_last_tick[i] = 0;
		}

	}

	for (uint8_t i = 0; i < title_screen_state->buttons_count; i++) {
		if (CheckCollisionPointRec(GetMousePosition(), title_screen_state->buttons_original_rectangles[i])) {
			if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
				*meta_state = i;

			enlargeButton(&title_screen_draw_data->buttons_draw_data[i], &title_screen_state->buttons_original_rectangles[i]);
		} else {
			title_screen_draw_data->buttons_draw_data[i].rectangle = title_screen_state->buttons_original_rectangles[i];
		}
	}
}

void drawButton(ButtonDrawData const *draw_data)
{
#define BUTTON_TEXT_SCALE_FACTOR 2.f / 3
	DrawRectangleRec(draw_data->rectangle, draw_data->rectangle_color);

	uint8_t font_size = draw_data->rectangle.height * BUTTON_TEXT_SCALE_FACTOR;
	DrawText(
		draw_data->text,
		draw_data->rectangle.x + (draw_data->rectangle.width - MeasureText(draw_data->text, font_size)) / 2,
		draw_data->rectangle.y + draw_data->rectangle.height * ((1 - BUTTON_TEXT_SCALE_FACTOR) / 2),
		font_size,
		draw_data->text_color
	);
#undef BUTTON_TEXT_SCALE_FACTOR
}

void drawTitleScreen(TitleScreenDrawData const *title_screen_draw_data)
{
	drawBackground(title_screen_draw_data->texture_atlas);

	for (uint8_t i = 0; i < title_screen_draw_data->tanks_count; i++)
		drawTank(&title_screen_draw_data->tanks_draw_data[i], title_screen_draw_data->texture_atlas);

	for (uint8_t i = 0; i < title_screen_draw_data->buttons_count; i++)
		drawButton(&title_screen_draw_data->buttons_draw_data[i]);

	DrawText("Citadel", 100, 100, 350, WHITE);
}




void updateGameLogic(MetaState *meta_state, GameLogic *game_logic, GamePhysics const *game_physics)
{
	for (uint8_t i = 0; i < game_logic->tanks_count; i++) {
		if (Vector2Distance(game_physics->tanks_physics[i].position, game_logic->tanks_path_points[game_logic->tanks_logic[i].path_segment_index + 1]) < 100) {
			Vector2 difference = Vector2Subtract(
				game_logic->tanks_path_points[game_logic->tanks_logic[i].path_segment_index + 2],
				game_logic->tanks_path_points[game_logic->tanks_logic[i].path_segment_index + 1]
			);
			game_physics->tanks_physics[i].acceleration = Vector2Scale(difference, 300 / Vector2Length(difference));

			game_logic->tanks_logic[i].path_segment_index++;
		}
	}
}

void updateGamePhysics(GamePhysics *game_physics, GameLogic const *game_logic)
{
#define TANK_SPEED 150
	for (uint8_t i = 0; i < game_physics->tanks_count; i++) {
		game_physics->tanks_physics[i].velocity.x += game_physics->tanks_physics[i].acceleration.x * GetFrameTime();
		game_physics->tanks_physics[i].velocity.y += game_physics->tanks_physics[i].acceleration.y * GetFrameTime();

		game_physics->tanks_physics[i].velocity = Vector2ClampValue(game_physics->tanks_physics[i].velocity, TANK_SPEED, TANK_SPEED);

		game_physics->tanks_physics[i].position.x += game_physics->tanks_physics[i].velocity.x * GetFrameTime();
		game_physics->tanks_physics[i].position.y += game_physics->tanks_physics[i].velocity.y * GetFrameTime();
	}
#undef TANK_SPEED
}

void updateGameDrawData(GameDrawData *game_draw_data, GamePhysics const *game_physics)
{
	drawBackground(game_draw_data->texture_atlas);

	if (game_draw_data->tanks_seconds_since_last_tick > 0.05f) {
		if (game_draw_data->tanks_texture_x_offset == 0)
			game_draw_data->tanks_texture_x_offset = 100;
		else
			game_draw_data->tanks_texture_x_offset = 0;

		game_draw_data->tanks_seconds_since_last_tick = 0;
	}

	for (uint8_t i = 0; i < game_draw_data->tanks_count; i++) {
		game_draw_data->tanks_draw_data[i].atlas_source_rectangle.x = game_draw_data->tanks_texture_x_offset;

		game_draw_data->tanks_draw_data[i].destination_rectangle.x = game_physics->tanks_physics[i].position.x;
		game_draw_data->tanks_draw_data[i].destination_rectangle.y = game_physics->tanks_physics[i].position.y;

		game_draw_data->tanks_draw_data[i].rotation = atan2f(game_physics->tanks_physics[i].velocity.y, game_physics->tanks_physics[i].velocity.x) * 180 / M_PI - 90;
	}

	game_draw_data->tanks_seconds_since_last_tick += GetFrameTime();
}

void drawGame(GameDrawData const *game_draw_data)
{
#define TANKS_PATH_THICKNESS 150
	for (uint8_t i = 0; i < game_draw_data->tanks_path_points_count - 1; i++) { // Replace with baked background texture
		DrawLineEx(game_draw_data->tanks_path_points[i], game_draw_data->tanks_path_points[i + 1], TANKS_PATH_THICKNESS, BEIGE);
		DrawCircleV(game_draw_data->tanks_path_points[i + 1], TANKS_PATH_THICKNESS / 2, BEIGE);
	}

	for (uint8_t i = 0; i < game_draw_data->tanks_count; i++)
		drawTank(&game_draw_data->tanks_draw_data[i], game_draw_data->texture_atlas);
#undef TANKS_PATH_THICKNESS
}

#define GREEN_TANK_ATLAS_SOURCE_RECTANGLE\
	((Rectangle) {\
	 	.x = 0,\
		.y = 0,\
		.width = 62,\
		.height = 83,\
	 })\

#define BLUE_TANK_ATLAS_SOURCE_RECTANGLE\
	((Rectangle) {\
	 	.x = 0,\
		.y = 100,\
		.width = 61,\
		.height = 67,\
	 })\

#define RED_TANK_ATLAS_SOURCE_RECTANGLE\
	((Rectangle) {\
	 	.x = 0,\
		.y = 182,\
		.width = 58,\
		.height = 68,\
	 })\

int main(void)
{
	SetConfigFlags(FLAG_MSAA_4X_HINT); // Antialiasing (must be called before InitWindow())
	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Citadel");
	ToggleFullscreen();
	SetTargetFPS(60);




#define TITLE_SCREEN_TANKS_COUNT 10

	ButtonDrawData title_screen_button_specifications[] = {
		(ButtonDrawData) {
		        .text = "Play",
		        .rectangle = {
		        	.x = 100,
		        	.y = WINDOW_HEIGHT / 2,
		        	.width = 400,
		        	.height = 100,
		        },
		        .text_color = WHITE,
		        .rectangle_color = RED,
		},
		(ButtonDrawData) {
		        .text = "Settings",
		        .rectangle = {
		        	.x = 100,
		        	.y = WINDOW_HEIGHT / 2 + 150,
		        	.width = 400,
		        	.height = 100,
		        },
		        .text_color = WHITE,
		        .rectangle_color = RED,
		},
		(ButtonDrawData) {
			.text = "Quit",
			.rectangle = {
				.x = 100,
				.y = WINDOW_HEIGHT / 2 + 300,
				.width = 400,
				.height = 100,
			},
			.text_color = WHITE,
			.rectangle_color = RED,
		},
	};
	uint8_t title_screen_buttons_count = sizeof title_screen_button_specifications / sizeof (ButtonDrawData);

	Vector2 game_state_tanks_path_points[] = {
		(Vector2) {0, 100},
		(Vector2) {1800, 100},
		(Vector2) {1800, 700},
		(Vector2) {1400, 700},
		(Vector2) {1400, 400},
		(Vector2) {800, 400},
		(Vector2) {800, 700},
		(Vector2) {500, 700},
		(Vector2) {500, 400},
		(Vector2) {100, 400},
		(Vector2) {100, 950},
		(Vector2) {1800, 950},
	};
	uint8_t game_state_tanks_path_points_count = sizeof game_state_tanks_path_points / sizeof (Vector2);
Texture2D texture_atlas = LoadTexture("assets/texture-atlas.png");




	MetaState meta_state = TITLE_SCREEN;

	TitleScreenState title_screen_state = {
		.tanks_velocity = {50, -50},
		.tanks_seconds_since_last_tick = alloca(TITLE_SCREEN_TANKS_COUNT * sizeof (float)),
		.buttons_original_rectangles = alloca(title_screen_buttons_count * sizeof (Rectangle)),
		.tanks_count = TITLE_SCREEN_TANKS_COUNT,
		.buttons_count = title_screen_buttons_count,
	};

	TitleScreenDrawData title_screen_draw_data = {
		.texture_atlas = texture_atlas,
		.background_color = DARKGREEN,
		.tanks_draw_data = alloca(TITLE_SCREEN_TANKS_COUNT * sizeof (TankDrawData)),
		.buttons_draw_data = title_screen_button_specifications,
		.tanks_count = TITLE_SCREEN_TANKS_COUNT,
		.buttons_count = title_screen_buttons_count,
	};

	GameLogic game_logic = {
		.tanks_logic = &(TankLogic) {
			.path_segment_index = 0,
		},
		.tanks_path_points = game_state_tanks_path_points,
		.tanks_count = 1,
		.tanks_path_points_count = game_state_tanks_path_points_count,
	};

	GamePhysics game_physics = {
		.tanks_physics = &(TankPhysics) {
			.position = {0, 100},
			.velocity = {1, 0},
		},
		.tanks_count = 1,
	};

	GameDrawData game_draw_data = {
		.texture_atlas = texture_atlas,
		.tanks_draw_data = &(TankDrawData) {
			.atlas_source_rectangle = RED_TANK_ATLAS_SOURCE_RECTANGLE,
			.destination_rectangle = {
				.width = GREEN_TANK_ATLAS_SOURCE_RECTANGLE.width * 2,
				.height = GREEN_TANK_ATLAS_SOURCE_RECTANGLE.height * 2,
			},
		},
		.tanks_path_points = game_state_tanks_path_points,
		.tanks_count = 1,
		.tanks_path_points_count = game_state_tanks_path_points_count,
	};



	srand(time(NULL));

	for (uint8_t i = 0; i < TITLE_SCREEN_TANKS_COUNT; i++) {
		Rectangle atlas_source_rectangle;
		switch (rand() % 3) {
		case 0:
			atlas_source_rectangle = GREEN_TANK_ATLAS_SOURCE_RECTANGLE;
			break;
		case 1:
			atlas_source_rectangle = BLUE_TANK_ATLAS_SOURCE_RECTANGLE;
			break;
		case 2:
			atlas_source_rectangle = RED_TANK_ATLAS_SOURCE_RECTANGLE;
			break;
		}

		title_screen_draw_data.tanks_draw_data[i] = (TankDrawData) {
			.atlas_source_rectangle = atlas_source_rectangle,
			.destination_rectangle = (Rectangle) {
				.width = atlas_source_rectangle.width * 2,
				.height = atlas_source_rectangle.height * 2,
			},
			.rotation = -135,
		};

respawn:
		title_screen_draw_data.tanks_draw_data[i].destination_rectangle.x = ((float) rand() / RAND_MAX) * (WINDOW_WIDTH + TITLE_SCREEN_SPAWN_PADDING * 2) - TITLE_SCREEN_SPAWN_PADDING;
		title_screen_draw_data.tanks_draw_data[i].destination_rectangle.y = ((float) rand() / RAND_MAX) * (WINDOW_HEIGHT + TITLE_SCREEN_SPAWN_PADDING * 2) - TITLE_SCREEN_SPAWN_PADDING;

		for (uint8_t j = 0; j < i; j++) {
			if (Vector2Distance(
				(Vector2) {
					title_screen_draw_data.tanks_draw_data[i].destination_rectangle.x,
					title_screen_draw_data.tanks_draw_data[i].destination_rectangle.y
				},
				(Vector2) {
					title_screen_draw_data.tanks_draw_data[j].destination_rectangle.x,
					title_screen_draw_data.tanks_draw_data[j].destination_rectangle.y
				}
			) < TITLE_SCREEN_SPAWN_PADDING)
				goto respawn;
		}
	}

	for (uint8_t i = 0; i < title_screen_buttons_count; i++)
		title_screen_state.buttons_original_rectangles[i] = title_screen_button_specifications[i].rectangle;




	while(!WindowShouldClose()) {
		BeginDrawing(); // OK to have updation code after this

		switch (meta_state) {
		case TITLE_SCREEN:
			updateMetaStateAndTitleScreen(&meta_state, &title_screen_state, &title_screen_draw_data);
			drawTitleScreen(&title_screen_draw_data);
			break;
		case GAME:
			updateGameLogic(&meta_state, &game_logic, &game_physics);
			updateGamePhysics(&game_physics, &game_logic);
			updateGameDrawData(&game_draw_data, &game_physics);
			drawGame(&game_draw_data);
			break;
		case QUIT:
			goto quit;
		}

		EndDrawing();
	}

quit:
	CloseWindow();
	return 0;
}
