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
	DOUBLE,
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
	OUTPOST_SIMPLE,
	OUTPOST_MORTAR,
	OUTPOST_PIERCE,
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
} TextButtonSpecification;

typedef enum {
	GAME,
	SETTINGS,
	QUIT,
	TITLE_SCREEN,
} MetaState;

typedef struct {
	Vector2 tanks_velocity;
	Rectangle *text_button_specifications_original_rectangles;
	float tanks_seconds_since_last_tick; // move to TitleScreenDrawData
	uint8_t tanks_count;
	uint8_t text_button_specifications_count;
} TitleScreenState;

typedef struct {
	Texture2D texture_atlas;
	Color background_color;
	TankDrawData *tanks_draw_data;
	TextButtonSpecification *text_button_specifications;
	char *highscore_text;
	float highscore_text_splash_time;
	uint16_t tanks_texture_x_offset;
	uint8_t tanks_count;
	uint8_t text_button_specifications_count;
} TitleScreenDrawData;




typedef struct {
	OutpostLogic *outposts_logic;
	TankLogic *tanks_logic;
	Vector2 *tanks_path_points;
	uint8_t outposts_count;
	uint8_t tanks_count;
	uint8_t tanks_path_points_count;
} GameplayLogic;

typedef struct {
	OutpostPhysics *outposts_physics;
	TankPhysics *tanks_physics;
	uint8_t outposts_count;
	uint8_t tanks_count;
} GameplayPhysics;

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
} GameplayDrawData;




void drawBackground(Texture2D texture_atlas)
{
	DrawTexturePro(
		texture_atlas,
		(Rectangle) {
			.x = 0,
			.y = 300,
			.width = 400,
			.height = 400 * ((float) WINDOW_HEIGHT / WINDOW_WIDTH),
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




void drawHighscore(char highscore_text[], Texture2D texture_atlas, float scale)
{
	uint8_t font_size = 100 * scale;
	DrawTextPro(
		(Font) {},
		highscore_text,
		(Vector2) {1350, 400},
		(Vector2) {MeasureText(highscore_text, font_size) / 2,
		font_size / 2},
		-30,
		font_size,
		font_size / 10,
		BLACK
	);
#define SOYJAK_ATLAS_SOURCE_RECTANGLE\
	(Rectangle) {\
		.x = 0,\
		.y = 540,\
		.width = 260,\
		.height = 180,\
	}
	DrawTexturePro(
		texture_atlas,
		SOYJAK_ATLAS_SOURCE_RECTANGLE,
		(Rectangle) {
			.x = 1400,
			.y = 550,
			.width = SOYJAK_ATLAS_SOURCE_RECTANGLE.width * scale,
			.height = SOYJAK_ATLAS_SOURCE_RECTANGLE.height * scale,
		},
		(Vector2) {SOYJAK_ATLAS_SOURCE_RECTANGLE.width * scale / 2, SOYJAK_ATLAS_SOURCE_RECTANGLE.height * scale / 2},
		-30,
		(Color) {
			.r = 255,
			.g = 255,
			.b = 255,
			.a = 90,
		}
	);
#undef SOYJAK_ATLAS_SOURCE_RECTANGLE
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

void drawHealthBar(Vector2 position, float health) {
    // Define health bar properties
    float width = 50; // to be adjusted as needed
    float height = 10; // to be adjusted as needed
    Color healthColor = GREEN; // to be adjusted as needed

    // Calculate health bar width based on health percentage
    float currentWidth = width * (health / 100.0f);

    // Draw health bar background
    DrawRectangle(position.x - (width / 2), position.y - (height / 2), width, height, GRAY);
    // Draw health bar foreground
    DrawRectangle(position.x - (width / 2), position.y - (height / 2), currentWidth, height, healthColor); //drawing green over gray to show current health 
}

void enlargeTextButton(TextButtonSpecification *draw_data, Rectangle const *original_rectangle)
{
#define BUTTON_ENLARGE_FACTOR 4.f / 3.f
	draw_data->rectangle.x = original_rectangle->x - original_rectangle->width * ((BUTTON_ENLARGE_FACTOR - 1) / 2);
	draw_data->rectangle.y = original_rectangle->y - original_rectangle->height * ((BUTTON_ENLARGE_FACTOR - 1) / 2);
	draw_data->rectangle.width = BUTTON_ENLARGE_FACTOR * original_rectangle->width;
	draw_data->rectangle.height = BUTTON_ENLARGE_FACTOR * original_rectangle->height;
#undef BUTTON_ENLARGE_FACTOR
}

#define TITLE_SCREEN_SPAWN_PADDING 150
#define HIGHSCORE_TEXT_SPLASH_FREQUENCY 1.f
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

	for (uint8_t i = 0; i < title_screen_state->text_button_specifications_count; i++) {
		if (CheckCollisionPointRec(GetMousePosition(), title_screen_state->text_button_specifications_original_rectangles[i])) {
			if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
				*meta_state = i;

			enlargeTextButton(&title_screen_draw_data->text_button_specifications[i], &title_screen_state->text_button_specifications_original_rectangles[i]);
		} else {
			title_screen_draw_data->text_button_specifications[i].rectangle = title_screen_state->text_button_specifications_original_rectangles[i];
		}
	}

	if (title_screen_draw_data->highscore_text_splash_time > 1.f / HIGHSCORE_TEXT_SPLASH_FREQUENCY)
		title_screen_draw_data->highscore_text_splash_time -= 1.f / HIGHSCORE_TEXT_SPLASH_FREQUENCY;

	title_screen_draw_data->highscore_text_splash_time += GetFrameTime();
}

void drawTextButton(TextButtonSpecification const *draw_data)
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

	DrawText("Citadel", 100, 100, 350, WHITE);

	for (uint8_t i = 0; i < title_screen_draw_data->text_button_specifications_count; i++)
		drawTextButton(&title_screen_draw_data->text_button_specifications[i]);

	drawHighscore("Highscore: ", title_screen_draw_data->texture_atlas, 1 + 0.2f * sinf(HIGHSCORE_TEXT_SPLASH_FREQUENCY * 2 * M_PI * title_screen_draw_data->highscore_text_splash_time));
}




void updateGameplayLogic(GameplayLogic *gameplay_logic, GameplayPhysics const *gameplay_physics)
{
	for (uint8_t i = 0; i < gameplay_logic->tanks_count; i++) {
		if (Vector2Distance(gameplay_physics->tanks_physics[i].position,  gameplay_logic->tanks_path_points[gameplay_logic->tanks_path_points_count - 1]) < 60.f) {
			Vector2 displacement = Vector2Subtract(gameplay_physics->tanks_physics[i].position, gameplay_logic->tanks_path_points[gameplay_logic->tanks_path_points_count - 1]);
			gameplay_physics->tanks_physics[i].acceleration = Vector2Scale(gameplay_physics->tanks_physics[i].velocity, -5.f);
		} else {
			gameplay_physics->tanks_physics[i].velocity = Vector2ClampValue(gameplay_physics->tanks_physics[i].velocity, 150.f, 150.f);
		}

		if (Vector2Distance(gameplay_physics->tanks_physics[i].position, gameplay_logic->tanks_path_points[gameplay_logic->tanks_logic[i].path_segment_index + 1]) < 60.f) {
			Vector2 difference = Vector2Subtract(
				gameplay_logic->tanks_path_points[gameplay_logic->tanks_logic[i].path_segment_index + 2],
				gameplay_logic->tanks_path_points[gameplay_logic->tanks_logic[i].path_segment_index + 1]
			);
			gameplay_physics->tanks_physics[i].acceleration = Vector2Scale(difference, 600.f / Vector2Length(difference));

			gameplay_logic->tanks_logic[i].path_segment_index++;
		}
	}
}

void updateGameplayPhysics(GameplayPhysics *gameplay_physics)
{
	for (uint8_t i = 0; i < gameplay_physics->tanks_count; i++) {
		gameplay_physics->tanks_physics[i].velocity.x += gameplay_physics->tanks_physics[i].acceleration.x * GetFrameTime();
		gameplay_physics->tanks_physics[i].velocity.y += gameplay_physics->tanks_physics[i].acceleration.y * GetFrameTime();

		gameplay_physics->tanks_physics[i].position.x += gameplay_physics->tanks_physics[i].velocity.x * GetFrameTime();
		gameplay_physics->tanks_physics[i].position.y += gameplay_physics->tanks_physics[i].velocity.y * GetFrameTime();
	}
}

//Health bar implementation needs a bit of correction

void updateGameplayDrawData(GameplayDrawData *gameplay_draw_data, GameplayPhysics const *gameplay_physics)
{
	drawBackground(gameplay_draw_data->texture_atlas);

	if (gameplay_draw_data->tanks_seconds_since_last_tick > 0.05f) {
		if (gameplay_draw_data->tanks_texture_x_offset == 0)
			gameplay_draw_data->tanks_texture_x_offset = 100;
		else
			gameplay_draw_data->tanks_texture_x_offset = 0;

		gameplay_draw_data->tanks_seconds_since_last_tick = 0.f;
	}
	gameplay_draw_data->tanks_seconds_since_last_tick += GetFrameTime();

	for (uint8_t i = 0; i < gameplay_draw_data->tanks_count; i++) {
		if (Vector2Length(gameplay_physics->tanks_physics[i].velocity) > 1)
			gameplay_draw_data->tanks_draw_data[i].atlas_source_rectangle.x = gameplay_draw_data->tanks_texture_x_offset;

		gameplay_draw_data->tanks_draw_data[i].destination_rectangle.x = gameplay_physics->tanks_physics[i].position.x;
		gameplay_draw_data->tanks_draw_data[i].destination_rectangle.y = gameplay_physics->tanks_physics[i].position.y;

		gameplay_draw_data->tanks_draw_data[i].angle = atan2f(gameplay_physics->tanks_physics[i].velocity.y, gameplay_physics->tanks_physics[i].velocity.x) * 180.f / M_PI - 90.f;

		/*// Update health bar position based on tank position
        Vector2 healthBarPosition = {
        gameplay_physics->tanks_physics[i].position.x, // Adjust as needed
        gameplay_physics->tanks_physics[i].position.y - 20 // Adjust as needed
        };

        // Draw tank and health bar
        drawTank(&gameplay_draw_data->tanks_draw_data[i], gameplay_draw_data->texture_atlas);
        drawHealthBar(healthBarPosition, gameplay_logic->tanks_logic[i].health);*/
	}
}



void drawOutpost(OutpostDrawData const *draw_data, Texture2D texture_atlas, Color tint)
{
	DrawTexturePro(
		texture_atlas,
		(Rectangle) {
 			.x = 0,
			.y = 250,
			.width = 26,
			.height = 26,
		},
		draw_data->base_destination_rectangle,
		(Vector2) {
			draw_data->base_destination_rectangle.width / 2,
			draw_data->base_destination_rectangle.height / 2,
		},
		0,
		tint
	);
	DrawTexturePro(
		texture_atlas,
		draw_data->turret_atlas_source_rectangle,
		draw_data->turret_destination_rectangle,
		(Vector2) {
			draw_data->turret_destination_rectangle.width / 2,
			draw_data->turret_destination_rectangle.height / 2,
		},
		draw_data->turret_angle,
		tint
	);
}

#define TANKS_PATH_THICKNESS 75
void drawGameplay(GameplayDrawData const *gameplay_draw_data)
{
	for (int i = 0; i < gameplay_draw_data->outposts_count; i++)
		drawOutpost(&gameplay_draw_data->outposts_draw_data[i], gameplay_draw_data->texture_atlas, WHITE);

	for (uint8_t i = 0; i < gameplay_draw_data->tanks_path_points_count - 1; i++) { // Replace with baked background texture
		DrawLineEx(gameplay_draw_data->tanks_path_points[i], gameplay_draw_data->tanks_path_points[i + 1], TANKS_PATH_THICKNESS, BEIGE);
		DrawCircleV(gameplay_draw_data->tanks_path_points[i + 1], TANKS_PATH_THICKNESS / 2, BEIGE);
	}

	for (uint8_t i = 0; i < gameplay_draw_data->tanks_count; i++)
		drawTank(&gameplay_draw_data->tanks_draw_data[i], gameplay_draw_data->texture_atlas);
		
}




typedef struct {
	Rectangle atlas_source_rectangle;
	Rectangle destination_rectangle;
} TextureButtonSpecification;

typedef struct {
	bool is_ui_active;

	TextureButtonSpecification *outpost_texture_button_specifications;
	uint8_t outpost_texture_button_specifications_count;

	uint8_t selected_outpost;

	uint32_t score; // move to GameplayLogic?
	uint32_t coins;
} GameUiLogic; // merge with GameplayLogic?




bool isMouseHoveringOverTextureButton(Rectangle specification_destination_rectangle)
{
	return Vector2Distance(
		GetMousePosition(),
		(Vector2) {
			specification_destination_rectangle.x,
			specification_destination_rectangle.y,
		}
	) < specification_destination_rectangle.width / 2;
}

bool canOutpostBePlaced(Vector2 position, Vector2 *path_points, uint8_t path_points_count, OutpostDrawData *outposts_draw_data, uint8_t outposts_count)
{
	for (uint8_t i = 0; i < path_points_count - 1; i++) {
		float cos = Vector2DotProduct(
			Vector2Normalize(Vector2Subtract(position, path_points[i])),
			Vector2Normalize(Vector2Subtract(path_points[i + 1], path_points[i]))
		);
		float length = Vector2Length(Vector2Subtract(position, path_points[i]));
		if (
			(
				cos > 0 &&
				length * cos < Vector2Length(Vector2Subtract(path_points[i + 1], path_points[i])) &&
				length * sqrt(1 - cos * cos) < TANKS_PATH_THICKNESS / 2 + 40
			) ||
			Vector2Distance(position, path_points[i + 1]) < TANKS_PATH_THICKNESS / 2 + 40
		)
			return false;
	}

	for (uint8_t i = 0; i < outposts_count; i++) {
		if (CheckCollisionRecs(
			(Rectangle) {
				.x = position.x,
				.y = position.y,
				.width = 75,
				.height = 75,
			},
			outposts_draw_data[i].base_destination_rectangle
		))
			return false;
	}

	return true;
}

void placeOutpost(OutpostType type, Vector2 position, OutpostLogic *logic, OutpostPhysics *physics, OutpostDrawData *draw_data)
{
	*logic = (OutpostLogic) {
		.health = 100,
		.type = type,
	};

	*physics = (OutpostPhysics) {
		.position = position,
		.turret_direction = {sqrtf(3.f) / 2.f, -1.f / 2.f},
	};

	*draw_data = (OutpostDrawData) {
		.base_destination_rectangle = {
			.x = position.x,
			.y = position.y,
			.width = 75,
			.height = 75,
		},
		.turret_atlas_source_rectangle = {
			.x = 30 * type,
			.y = 280,
			.width = 30,
			.height = 11,
		},
		.turret_destination_rectangle = {
			.x = position.x,
			.y = position.y,
			.width = 75,
			.height = 30,
		},
		.turret_angle = -30,
	};
}

void updateGameUiLogic(GameUiLogic *game_ui_logic, GameplayLogic *gameplay_logic, GameplayPhysics *gameplay_physics, GameplayDrawData *gameplay_draw_data)
{
	game_ui_logic->is_ui_active = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);

	if (game_ui_logic->is_ui_active) {
		if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
			if (game_ui_logic->selected_outpost < game_ui_logic->outpost_texture_button_specifications_count) {
				if (canOutpostBePlaced(GetMousePosition(), gameplay_logic->tanks_path_points, gameplay_logic->tanks_path_points_count, gameplay_draw_data->outposts_draw_data, gameplay_logic->outposts_count)) {
					placeOutpost(game_ui_logic->selected_outpost, GetMousePosition(), gameplay_logic->outposts_logic + gameplay_logic->outposts_count, gameplay_physics->outposts_physics + gameplay_logic->outposts_count, gameplay_draw_data->outposts_draw_data + gameplay_logic->outposts_count);
					gameplay_logic->outposts_count++;
					gameplay_physics->outposts_count++;
					gameplay_draw_data->outposts_count++;
				}
			} else {
				for (uint8_t i = 0; i < game_ui_logic->outpost_texture_button_specifications_count; i++) {
					if (isMouseHoveringOverTextureButton(game_ui_logic->outpost_texture_button_specifications[i].destination_rectangle)) {
						game_ui_logic->selected_outpost = i;
						break;
					}
				}
			}
		}
	} else {
		game_ui_logic->selected_outpost = game_ui_logic->outpost_texture_button_specifications_count;
	}
}

void drawGameUi(GameUiLogic const *game_ui_logic, GameplayLogic const *gameplay_logic, GameplayDrawData const *gameplay_draw_data, Texture2D texture_atlas)
{
	if (game_ui_logic->is_ui_active) {
		if (game_ui_logic->selected_outpost < game_ui_logic->outpost_texture_button_specifications_count) {
			Color color;
			switch (game_ui_logic->selected_outpost) {
			case OUTPOST_SIMPLE:
				color = LIGHTGRAY;
				break;
			case OUTPOST_MORTAR:
				color = GOLD;
				break;
			case OUTPOST_PIERCE:
				color = SKYBLUE;
				break;
			}

			Vector2 mouse_position= GetMousePosition();

			OutpostDrawData hovering_outpost_draw_data = {
				.base_destination_rectangle = {
					.x = mouse_position.x,
					.y = mouse_position.y,
					.width = 75,
					.height = 75,
				},
				.turret_atlas_source_rectangle = {
					.x = 30 * game_ui_logic->selected_outpost,
					.y = 280,
					.width = 30,
					.height = 11,
				},
				.turret_destination_rectangle = {
					.x = mouse_position.x,
					.y = mouse_position.y,
					.width = 75,
					.height = 30,
				},
				.turret_angle = -30,
			};

			Color tint;
			if (canOutpostBePlaced(mouse_position, gameplay_logic->tanks_path_points, gameplay_logic->tanks_path_points_count, gameplay_draw_data->outposts_draw_data, gameplay_logic->outposts_count))
				tint = GRAY;
			else
				tint = RED;
			drawOutpost(&hovering_outpost_draw_data, gameplay_draw_data->texture_atlas, tint);
		} else {
			DrawRectangle(
				WINDOW_WIDTH * 3 / 4,
				0,
				WINDOW_WIDTH / 4,
				WINDOW_HEIGHT,
				(Color) {
					.r = BEIGE.r,
					.g = BEIGE.g,
					.b = BEIGE.b,
					.a = 127,
				}
			);

			for (uint8_t i = 0; i < game_ui_logic->outpost_texture_button_specifications_count; i++) {
				Rectangle scaled_destination_rectangle = game_ui_logic->outpost_texture_button_specifications[i].destination_rectangle;
				if (
					!(game_ui_logic->selected_outpost < game_ui_logic->outpost_texture_button_specifications_count) &&
					isMouseHoveringOverTextureButton(game_ui_logic->outpost_texture_button_specifications[i].destination_rectangle)) {
					scaled_destination_rectangle.width *= 3.f / 2;
					scaled_destination_rectangle.height *= 3.f / 2;
				}

				DrawTexturePro(
					texture_atlas,
					game_ui_logic->outpost_texture_button_specifications[i].atlas_source_rectangle,
					scaled_destination_rectangle,
					(Vector2) {
						scaled_destination_rectangle.width / 2,
						scaled_destination_rectangle.height / 2,
					},
					-30,
					WHITE
				);
			}
		}
	}
}




#define GREEN_TANK_ATLAS_SOURCE_RECTANGLE\
	((Rectangle) {\
	 	.x = 0,\
		.y = 0,\
		.width = 63,\
		.height = 83,\
	 })

#define BLUE_TANK_ATLAS_SOURCE_RECTANGLE\
	((Rectangle) {\
	 	.x = 0,\
		.y = 100,\
		.width = 62,\
		.height = 67,\
	 })

#define RED_TANK_ATLAS_SOURCE_RECTANGLE\
	((Rectangle) {\
	 	.x = 0,\
		.y = 182,\
		.width = 59,\
		.height = 68,\
	 })

int main(void)
{
	SetConfigFlags(FLAG_MSAA_4X_HINT); // Antialiasing (must be called before InitWindow())
	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Citadel");
	ToggleFullscreen();
	SetTargetFPS(60);




#define TITLE_SCREEN_TANKS_COUNT 50
#define MAXIMUM_OUTPOSTS_COUNT 128

	TextButtonSpecification title_screen_text_button_specifications[] = {
		(TextButtonSpecification) {
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
		(TextButtonSpecification) {
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
		(TextButtonSpecification) {
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
	uint8_t title_screen_text_button_specifications_count = sizeof title_screen_text_button_specifications / sizeof (TextButtonSpecification);

	Texture2D texture_atlas = LoadTexture("assets/texture-atlas.png");

	//OutpostDrawData game_outposts_draw_data[] = {
	//	(OutpostDrawData) {
	//		.base_destination_rectangle = {
	//			.x = 650,
	//			.y = 270,
	//		},
	//	},
	//	(OutpostDrawData) {
	//		.base_destination_rectangle = {
	//			.x = 1600,
	//			.y = 270,
	//		},
	//	},
	//	(OutpostDrawData) {
	//		.base_destination_rectangle = {
	//			.x = 300,
	//			.y = 700,
	//		},
	//	},
	//	(OutpostDrawData) {
	//		.base_destination_rectangle = {
	//			.x = 1100,
	//			.y = 600,
	//		},
	//	},
	//	(OutpostDrawData) {
	//		.base_destination_rectangle = {
	//			.x = 1100,
	//			.y = 800,
	//		},
	//	},
	//};
	//uint8_t game_outposts_count = sizeof game_outposts_draw_data / sizeof (OutpostDrawData);

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
		.text_button_specifications_original_rectangles = alloca(title_screen_text_button_specifications_count * sizeof (Rectangle)),
		.tanks_count = TITLE_SCREEN_TANKS_COUNT,
		.text_button_specifications_count = title_screen_text_button_specifications_count,
	};

	TitleScreenDrawData title_screen_draw_data = {
		.texture_atlas = texture_atlas,
		.background_color = DARKGREEN,
		.tanks_draw_data = alloca(TITLE_SCREEN_TANKS_COUNT * sizeof (TankDrawData)),
		.text_button_specifications = title_screen_text_button_specifications,
		.tanks_count = TITLE_SCREEN_TANKS_COUNT,
		.text_button_specifications_count = title_screen_text_button_specifications_count,
	};

	GameplayLogic gameplay_logic = {
		.outposts_logic = alloca(MAXIMUM_OUTPOSTS_COUNT * sizeof (OutpostLogic)), // get rid of all alloca() calls? can this just be a reference to an rvalue?
		.tanks_logic = &(TankLogic) {},
		.tanks_path_points = game_state_tanks_path_points,
		.tanks_count = 1,
		.tanks_path_points_count = game_state_tanks_path_points_count,
	};

	GameplayPhysics gameplay_physics = {
		.outposts_physics = alloca(MAXIMUM_OUTPOSTS_COUNT * sizeof (OutpostPhysics)),
		.tanks_physics = &(TankPhysics) {
			.position = {0, 100},
			.velocity = {1, 0},
		},
		.tanks_count = 1,
	};

	GameplayDrawData gameplay_draw_data = {
		.texture_atlas = texture_atlas,
		.outposts_draw_data = alloca(MAXIMUM_OUTPOSTS_COUNT * sizeof (OutpostDrawData)),
		.tanks_draw_data = &(TankDrawData) {
			.atlas_source_rectangle = RED_TANK_ATLAS_SOURCE_RECTANGLE,
			.destination_rectangle = {
				.width = GREEN_TANK_ATLAS_SOURCE_RECTANGLE.width,
				.height = GREEN_TANK_ATLAS_SOURCE_RECTANGLE.height,
			},
		},
		.tanks_path_points = game_state_tanks_path_points,
		.tanks_count = 1,
		.tanks_path_points_count = game_state_tanks_path_points_count,
	};




	TextureButtonSpecification outpost_texture_button_specifications[] = {
		(TextureButtonSpecification) {
			.atlas_source_rectangle = {
				.x = 0,
				.y = 280,
				.width = 30,
				.height = 11,
			},
			.destination_rectangle = {
				.x = WINDOW_WIDTH * 7 / 8,
				.y = WINDOW_HEIGHT / 4,
				.width = 180,
				.height = 66,
			},
		},
		(TextureButtonSpecification) {
			.atlas_source_rectangle = {
				.x = 30,
				.y = 280,
				.width = 30,
				.height = 11,
			},
			.destination_rectangle = {
				.x = WINDOW_WIDTH * 7 / 8,
				.y = WINDOW_HEIGHT * 1 / 2,
				.width = 180,
				.height = 66,
			},
		},
		(TextureButtonSpecification) {
			.atlas_source_rectangle = {
				.x = 60,
				.y = 280,
				.width = 30,
				.height = 11,
			},
			.destination_rectangle = {
				.x = WINDOW_WIDTH * 7 / 8,
				.y = WINDOW_HEIGHT * 3 / 4,
				.width = 180,
				.height = 66,
			},
		},
	};

	GameUiLogic game_ui_logic = {
		.outpost_texture_button_specifications = outpost_texture_button_specifications,
		.outpost_texture_button_specifications_count = sizeof outpost_texture_button_specifications / sizeof (TextureButtonSpecification),
	};




	srand(time(NULL));

	//for (uint8_t i = 0; i < game_outposts_count; i++) {
	//	gameplay_draw_data.outposts_draw_data[i].base_destination_rectangle.width = 125;
	//	gameplay_draw_data.outposts_draw_data[i].base_destination_rectangle.height = 125;

	//	gameplay_draw_data.outposts_draw_data[i].turret_atlas_source_rectangle = (Rectangle) {
	//		.x = 30 * (i % 3),
	//		.y = 280,
	//		.width = 30,
	//		.height = 11,
	//	};
	//	gameplay_draw_data.outposts_draw_data[i].turret_destination_rectangle = (Rectangle) {
	//		.x = gameplay_draw_data.outposts_draw_data[i].base_destination_rectangle.x,
	//		.y = gameplay_draw_data.outposts_draw_data[i].base_destination_rectangle.y,
	//		.width = 150,
	//		.height = 60,
	//	};

	//	gameplay_draw_data.outposts_draw_data[i].turret_angle = 360.f * ((float) rand() / RAND_MAX); // need this?
	//}

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
				.width = atlas_source_rectangle.width,
				.height = atlas_source_rectangle.height,
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

	for (uint8_t i = 0; i < title_screen_text_button_specifications_count; i++)
		title_screen_state.text_button_specifications_original_rectangles[i] = title_screen_text_button_specifications[i].rectangle;




	while(!WindowShouldClose()) {
		BeginDrawing(); // OK to have updation code after this

		switch (meta_state) {
		case TITLE_SCREEN:
			updateMetaStateAndTitleScreen(&meta_state, &title_screen_state, &title_screen_draw_data);
			drawTitleScreen(&title_screen_draw_data);
			break;
		case GAME:
			updateGameUiLogic(&game_ui_logic, &gameplay_logic, &gameplay_physics, &gameplay_draw_data);
			updateGameplayLogic(&gameplay_logic, &gameplay_physics);
			updateGameplayPhysics(&gameplay_physics);

			updateGameplayDrawData(&gameplay_draw_data, &gameplay_physics);

			drawGameplay(&gameplay_draw_data);
			drawGameUi(&game_ui_logic, &gameplay_logic, &gameplay_draw_data, texture_atlas);
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
