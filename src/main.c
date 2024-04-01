#include <alloca.h>
#include <stdint.h>
#include <stdio.h> // Debugging
#include <stdlib.h> // rand()
#include <time.h>

#include <raylib.h>
#include <raymath.h>

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

typedef enum {
	SINGLE,
	DOULBE,
	PIERCE,
} TankType;

typedef struct {
	float health;
	TankType type;
	uint8_t path_segment_index;
} TankLogic;

typedef struct {
	Vector2 position;
	Vector2 velocity;
	Vector2 acceleration;
} TankPhysics;

typedef struct {
	Rectangle atlas_source_rectangle;
	Rectangle destination_rectangle;
	float angle;
} TankDrawData; // rename to TankDrawing or TankDrawingData

typedef enum {
	NONE,
} OutpostType;

typedef struct {
	float health;
	OutpostType type;
} OutpostLogic;

typedef struct {
	Vector2 position;
	Vector2 turret_direction;
} OutpostPhysics;

typedef struct {
	Rectangle base_destination_rectangle;
	Rectangle turret_atlas_source_rectangle;
	Rectangle turret_destination_rectangle;
	float turret_angle;
} OutpostDrawData;

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
	float tanks_seconds_since_last_tick; // move to TitleScreenDrawData
	Rectangle *buttons_original_rectangles;
	uint8_t tanks_count;
	uint8_t buttons_count;
} TitleScreenState;

typedef struct {
	Texture2D texture_atlas;
	Color background_color;
	TankDrawData *tanks_draw_data;
	ButtonDrawData *buttons_draw_data;
	uint16_t tanks_texture_x_offset;
	uint8_t tanks_count;
	uint8_t buttons_count;
} TitleScreenDrawData;




typedef struct {
	OutpostLogic *outposts_logic;
	TankLogic *tanks_logic;
	Vector2 *tanks_path_points;
	uint8_t outposts_count;
	uint8_t tanks_count;
	uint8_t tanks_path_points_count;
} GameLogic;

typedef struct {
	OutpostPhysics *outposts_physics;
	TankPhysics *tanks_physics;
	uint8_t outposts_count;
	uint8_t tanks_count;
} GamePhysics;

typedef struct {
	Texture2D texture_atlas;
	OutpostDrawData *outposts_draw_data;
	TankDrawData *tanks_draw_data;
	Vector2 *tanks_path_points;
	float tanks_seconds_since_last_tick;
	uint16_t tanks_texture_x_offset;
	uint8_t outposts_count;
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
		draw_data->angle,
		WHITE
	);
}

void enlargeButton(ButtonDrawData *draw_data, Rectangle const *original_rectangle)
{
#define BUTTON_ENLARGE_FACTOR 4.f / 3.f
	draw_data->rectangle.x = original_rectangle->x - original_rectangle->width * ((BUTTON_ENLARGE_FACTOR - 1) / 2);
	draw_data->rectangle.y = original_rectangle->y - original_rectangle->height * ((BUTTON_ENLARGE_FACTOR - 1) / 2);
	draw_data->rectangle.width = BUTTON_ENLARGE_FACTOR * original_rectangle->width;
	draw_data->rectangle.height = BUTTON_ENLARGE_FACTOR * original_rectangle->height;
#undef BUTTON_ENLARGE_FACTOR
}

#define TITLE_SCREEN_SPAWN_PADDING 250
void updateMetaStateAndTitleScreen(MetaState *meta_state, TitleScreenState *title_screen_state, TitleScreenDrawData *title_screen_draw_data)
{
	if (title_screen_state->tanks_seconds_since_last_tick > 0.05f) {
		if (title_screen_draw_data->tanks_texture_x_offset == 0)
			title_screen_draw_data->tanks_texture_x_offset = 100;
		else
			title_screen_draw_data->tanks_texture_x_offset = 0;

		title_screen_state->tanks_seconds_since_last_tick = 0.f;
	}
	title_screen_state->tanks_seconds_since_last_tick += GetFrameTime();

	for (uint8_t i = 0; i < title_screen_state->tanks_count; i++) {
		title_screen_draw_data->tanks_draw_data[i].destination_rectangle.x += title_screen_state->tanks_velocity.x * GetFrameTime();
		title_screen_draw_data->tanks_draw_data[i].destination_rectangle.y += title_screen_state->tanks_velocity.y * GetFrameTime();

		if (title_screen_draw_data->tanks_draw_data[i].destination_rectangle.x > WINDOW_WIDTH + TITLE_SCREEN_SPAWN_PADDING)
			title_screen_draw_data->tanks_draw_data[i].destination_rectangle.x = -TITLE_SCREEN_SPAWN_PADDING;
		if (title_screen_draw_data->tanks_draw_data[i].destination_rectangle.y < -TITLE_SCREEN_SPAWN_PADDING)
			title_screen_draw_data->tanks_draw_data[i].destination_rectangle.y = WINDOW_HEIGHT + TITLE_SCREEN_SPAWN_PADDING;

		title_screen_draw_data->tanks_draw_data[i].atlas_source_rectangle.x = title_screen_draw_data->tanks_texture_x_offset;
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
#define BUTTON_TEXT_SCALE_FACTOR 2.f / 3.f
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
		if (Vector2Distance(game_physics->tanks_physics[i].position,  game_logic->tanks_path_points[game_logic->tanks_path_points_count - 1]) < 100.f) {
			Vector2 displacement = Vector2Subtract(game_physics->tanks_physics[i].position, game_logic->tanks_path_points[game_logic->tanks_path_points_count - 1]);
			game_physics->tanks_physics[i].acceleration = Vector2Scale(game_physics->tanks_physics[i].velocity, -5.f);
		} else {
			game_physics->tanks_physics[i].velocity = Vector2ClampValue(game_physics->tanks_physics[i].velocity, 150.f, 150.f);
		}

		if (Vector2Distance(game_physics->tanks_physics[i].position, game_logic->tanks_path_points[game_logic->tanks_logic[i].path_segment_index + 1]) < 120.f) {
			Vector2 difference = Vector2Subtract(
				game_logic->tanks_path_points[game_logic->tanks_logic[i].path_segment_index + 2],
				game_logic->tanks_path_points[game_logic->tanks_logic[i].path_segment_index + 1]
			);
			game_physics->tanks_physics[i].acceleration = Vector2Scale(difference, 300.f / Vector2Length(difference));

			game_logic->tanks_logic[i].path_segment_index++;
		}
	}
}

void updateGamePhysics(GamePhysics *game_physics)
{
	for (uint8_t i = 0; i < game_physics->tanks_count; i++) {
		game_physics->tanks_physics[i].velocity.x += game_physics->tanks_physics[i].acceleration.x * GetFrameTime();
		game_physics->tanks_physics[i].velocity.y += game_physics->tanks_physics[i].acceleration.y * GetFrameTime();

		game_physics->tanks_physics[i].position.x += game_physics->tanks_physics[i].velocity.x * GetFrameTime();
		game_physics->tanks_physics[i].position.y += game_physics->tanks_physics[i].velocity.y * GetFrameTime();
	}
}

void updateGameDrawData(GameDrawData *game_draw_data, GamePhysics const *game_physics)
{
	drawBackground(game_draw_data->texture_atlas);

	if (game_draw_data->tanks_seconds_since_last_tick > 0.05f) {
		if (game_draw_data->tanks_texture_x_offset == 0)
			game_draw_data->tanks_texture_x_offset = 100;
		else
			game_draw_data->tanks_texture_x_offset = 0;

		game_draw_data->tanks_seconds_since_last_tick = 0.f;
	}
	game_draw_data->tanks_seconds_since_last_tick += GetFrameTime();

	for (uint8_t i = 0; i < game_draw_data->tanks_count; i++) {
		if (Vector2Length(game_physics->tanks_physics[i].velocity) > 1)
			game_draw_data->tanks_draw_data[i].atlas_source_rectangle.x = game_draw_data->tanks_texture_x_offset;

		game_draw_data->tanks_draw_data[i].destination_rectangle.x = game_physics->tanks_physics[i].position.x;
		game_draw_data->tanks_draw_data[i].destination_rectangle.y = game_physics->tanks_physics[i].position.y;

		game_draw_data->tanks_draw_data[i].angle = atan2f(game_physics->tanks_physics[i].velocity.y, game_physics->tanks_physics[i].velocity.x) * 180.f / M_PI - 90.f;
	}
}

void drawGame(GameDrawData const *game_draw_data)
{
	for (int i = 0; i < game_draw_data->outposts_count; i++) {
		DrawTexturePro(
			game_draw_data->texture_atlas,
			(Rectangle) {
	 			.x = 0,
				.y = 250,
				.width = 26,
				.height = 26,
			},
			game_draw_data->outposts_draw_data[i].base_destination_rectangle,
			(Vector2) {
				game_draw_data->outposts_draw_data[i].base_destination_rectangle.width / 2,
				game_draw_data->outposts_draw_data[i].base_destination_rectangle.height / 2,
			},
			0,
			WHITE
		);
		DrawTexturePro(
			game_draw_data->texture_atlas,
			game_draw_data->outposts_draw_data[i].turret_atlas_source_rectangle,
			game_draw_data->outposts_draw_data[i].turret_destination_rectangle,
			(Vector2) {
				game_draw_data->outposts_draw_data[i].turret_destination_rectangle.width / 2,
				game_draw_data->outposts_draw_data[i].turret_destination_rectangle.height / 2,
			},
			game_draw_data->outposts_draw_data[i].turret_angle,
			WHITE
		);
	}

#define TANKS_PATH_THICKNESS 150
	for (uint8_t i = 0; i < game_draw_data->tanks_path_points_count - 1; i++) { // Replace with baked background texture
		DrawLineEx(game_draw_data->tanks_path_points[i], game_draw_data->tanks_path_points[i + 1], TANKS_PATH_THICKNESS, BEIGE);
		DrawCircleV(game_draw_data->tanks_path_points[i + 1], TANKS_PATH_THICKNESS / 2, BEIGE);
	}
#undef TANKS_PATH_THICKNESS

	for (uint8_t i = 0; i < game_draw_data->tanks_count; i++)
		drawTank(&game_draw_data->tanks_draw_data[i], game_draw_data->texture_atlas);
}

#define GREEN_TANK_ATLAS_SOURCE_RECTANGLE\
	((Rectangle) {\
	 	.x = 0,\
		.y = 0,\
		.width = 63,\
		.height = 83,\
	 })\

#define BLUE_TANK_ATLAS_SOURCE_RECTANGLE\
	((Rectangle) {\
	 	.x = 0,\
		.y = 100,\
		.width = 62,\
		.height = 67,\
	 })\

#define RED_TANK_ATLAS_SOURCE_RECTANGLE\
	((Rectangle) {\
	 	.x = 0,\
		.y = 182,\
		.width = 59,\
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

	Texture2D texture_atlas = LoadTexture("assets/texture-atlas.png");

	OutpostDrawData game_outposts_draw_data[] = {
		(OutpostDrawData) {
			.base_destination_rectangle = {
				.x = 650,
				.y = 270,
			},
		},
		(OutpostDrawData) {
			.base_destination_rectangle = {
				.x = 1600,
				.y = 270,
			},
		},
		(OutpostDrawData) {
			.base_destination_rectangle = {
				.x = 300,
				.y = 700,
			},
		},
		(OutpostDrawData) {
			.base_destination_rectangle = {
				.x = 1100,
				.y = 600,
			},
		},
		(OutpostDrawData) {
			.base_destination_rectangle = {
				.x = 1100,
				.y = 800,
			},
		},
	};
	uint8_t game_outposts_count = sizeof game_outposts_draw_data / sizeof (OutpostDrawData);

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
		(Vector2) {100, 980},
		(Vector2) {1400, 980},
	};
	uint8_t game_state_tanks_path_points_count = sizeof game_state_tanks_path_points / sizeof (Vector2);




	MetaState meta_state = TITLE_SCREEN;

	TitleScreenState title_screen_state = {
		.tanks_velocity = {50, -50},
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
		.tanks_logic = &(TankLogic) {},
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
		.outposts_draw_data = game_outposts_draw_data,
		.tanks_draw_data = &(TankDrawData) {
			.atlas_source_rectangle = RED_TANK_ATLAS_SOURCE_RECTANGLE,
			.destination_rectangle = {
				.width = GREEN_TANK_ATLAS_SOURCE_RECTANGLE.width * 2,
				.height = GREEN_TANK_ATLAS_SOURCE_RECTANGLE.height * 2,
			},
		},
		.tanks_path_points = game_state_tanks_path_points,
		.outposts_count = game_outposts_count,
		.tanks_count = 1,
		.tanks_path_points_count = game_state_tanks_path_points_count,
	};




	srand(time(NULL));

	for (uint8_t i = 0; i < game_outposts_count; i++) {
		game_draw_data.outposts_draw_data[i].base_destination_rectangle.width = 125;
		game_draw_data.outposts_draw_data[i].base_destination_rectangle.height = 125;

		game_draw_data.outposts_draw_data[i].turret_atlas_source_rectangle = (Rectangle) {
			.x = 30 * (i % 3),
			.y = 280,
			.width = 30,
			.height = 11,
		};
		game_draw_data.outposts_draw_data[i].turret_destination_rectangle = (Rectangle) {
			.x = game_draw_data.outposts_draw_data[i].base_destination_rectangle.x,
			.y = game_draw_data.outposts_draw_data[i].base_destination_rectangle.y,
			.width = 150,
			.height = 60,
		};

		game_draw_data.outposts_draw_data[i].turret_angle = 360.f * ((float) rand() / RAND_MAX); // need this?
	}

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
			.angle = -135,
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
			updateGamePhysics(&game_physics);
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
