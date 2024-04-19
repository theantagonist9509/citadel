#include <stdint.h>
#include <stdio.h> // Debugging
#include <stdlib.h> // rand()
#include <string.h>
#include <time.h>

#include <raylib.h>
#include <raymath.h>

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

typedef enum {
	TANK_SINGLE,
	TANK_DOUBLE,
	TANK_PIERCE,
} TankType;

typedef struct {
	float health;
	float seconds_since_last_shot;
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
	float seconds_since_last_shot;
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

typedef struct {
	float seconds_remaining;
	uint8_t outpost_index;
	uint8_t tank_index;
} ShotAnimation;




typedef enum {
	GAME,
	QUIT,
	TITLE_SCREEN,
} MetaState;

typedef struct {
	Vector2 tanks_velocity;
	Rectangle *text_button_specifications_original_rectangles;
	Rectangle music_toggle_button_specification_original_rectangle;
	Music background_music;
	float tanks_seconds_since_last_tick; // move to TitleScreenDrawData
	uint8_t tanks_count;
	uint8_t text_button_specifications_count;
} TitleScreenState;

typedef struct {
	Texture2D texture_atlas;
	Color background_color;
	TankDrawData *tanks_draw_data;
	TextButtonSpecification *text_button_specifications;
	TextButtonSpecification music_toggle_button_specification;
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
	ShotAnimation *outpost_shot_animations;
	ShotAnimation *tank_shot_animations;
	Vector2 *tanks_path_points;
	float tanks_seconds_since_last_tick;
	uint16_t tanks_texture_x_offset;
	uint8_t outposts_count;
	uint8_t outpost_shot_animations_count;
	uint8_t tanks_count;
	uint8_t tank_shot_animations_count;
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
		GOLD
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
			.y = 450,
			.width = SOYJAK_ATLAS_SOURCE_RECTANGLE.width * scale,
			.height = SOYJAK_ATLAS_SOURCE_RECTANGLE.height * scale,
		},
		(Vector2) {SOYJAK_ATLAS_SOURCE_RECTANGLE.width * scale / 2, SOYJAK_ATLAS_SOURCE_RECTANGLE.height * scale / 2},
		-30,
		(Color) {
			.r = 255,
			.g = 255,
			.b = 255,
			.a = 127,
		}
	);
#undef SOYJAK_ATLAS_SOURCE_RECTANGLE
}




#define HEALTH_BAR_WIDTH 75.f
#define HEALTH_BAR_HEIGHT 10.f
void drawHealthBar(float health, float maximum_health, Vector2 position)
{
	float fraction = health / maximum_health;
	DrawRectangle(position.x, position.y, HEALTH_BAR_WIDTH * fraction, HEALTH_BAR_HEIGHT, GREEN);
	DrawRectangle(position.x + HEALTH_BAR_WIDTH * fraction, position.y, HEALTH_BAR_WIDTH * (1.f - fraction), HEALTH_BAR_HEIGHT, LIGHTGRAY);
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

void enlargeTextButton(TextButtonSpecification *draw_data, Rectangle const *original_rectangle)
{
#define BUTTON_ENLARGE_FACTOR 5.f / 4.f
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

	// Music toggle button
	if (CheckCollisionPointRec(GetMousePosition(), title_screen_state->music_toggle_button_specification_original_rectangle)) {
		if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
			if (IsMusicStreamPlaying(title_screen_state->background_music)) {
				StopMusicStream(title_screen_state->background_music);
				strcpy(title_screen_draw_data->music_toggle_button_specification.text, "Toggle Music [Off]");
			} else {
				PlayMusicStream(title_screen_state->background_music);
				strcpy(title_screen_draw_data->music_toggle_button_specification.text, "Toggle Music [On]");
			}
		}

		enlargeTextButton(&title_screen_draw_data->music_toggle_button_specification, &title_screen_state->music_toggle_button_specification_original_rectangle);
	} else {
		title_screen_draw_data->music_toggle_button_specification.rectangle = title_screen_state->music_toggle_button_specification_original_rectangle;
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
	drawTextButton(&title_screen_draw_data->music_toggle_button_specification);

	drawHighscore("Highscore: ", title_screen_draw_data->texture_atlas, 1 + 0.2f * sinf(HIGHSCORE_TEXT_SPLASH_FREQUENCY * 2 * M_PI * title_screen_draw_data->highscore_text_splash_time));
}




void evictElement(void *array, uint8_t length, size_t element_size, uint8_t index)
{
	uint8_t *byte_array = (uint8_t *) array;
	//memmove(byte_array + index * element_size, byte_array + (index + 1) * element_size, (length - (index + 1)) * element_size); // TODO why does this segfault?

	for (uint8_t i = index; i < length - 1; i++)
		memcpy(byte_array + index * element_size, byte_array + (index + 1) * element_size, element_size);
}

#define OUTPOST_RANGE 300.f
#define TANK_RANGE 200.f

#define OUTPOST_SHOT_COOLDOWN_SECONDS 0.5f
#define TANK_SHOT_COOLDOWN_SECONDS 0.75f

#define OUTPOST_MAXIMUM_HEALTH 100.f
#define TANK_MAXIMUM_HEALTH 1000.f

#define OUTPOST_SIMPLE_ANIMATION_DURATION_SECONDS 0.2f

void updateGameplayLogic(GameplayLogic *gameplay_logic, GameplayPhysics *gameplay_physics, GameplayDrawData *gameplay_draw_data)
{
	for (uint8_t i = 0; i < gameplay_logic->outposts_count; i++) {
		for (uint8_t j = 0; j < gameplay_logic->tanks_count; j++) {
			if (Vector2Distance(gameplay_physics->tanks_physics[j].position, gameplay_physics->outposts_physics[i].position) < OUTPOST_RANGE) {
				Vector2 difference = Vector2Subtract(gameplay_physics->tanks_physics[j].position, gameplay_physics->outposts_physics[i].position);
				gameplay_physics->outposts_physics[i].turret_direction = Vector2Normalize(Vector2Add(
					gameplay_physics->outposts_physics[i].turret_direction,
					Vector2Scale(difference, 0.025f * GetFrameTime())
				));

				if (gameplay_logic->outposts_logic[i].seconds_since_last_shot > OUTPOST_SHOT_COOLDOWN_SECONDS) { // can be separated out for more data orientation
					gameplay_logic->tanks_logic[j].health -= 10.f;
					gameplay_draw_data->outpost_shot_animations[gameplay_draw_data->outpost_shot_animations_count++] = (ShotAnimation) {
						.seconds_remaining = OUTPOST_SIMPLE_ANIMATION_DURATION_SECONDS,
						.outpost_index = i,
						.tank_index = j,
					};
					gameplay_logic->outposts_logic[i].seconds_since_last_shot = 0.f;
				}

				break;
			}
		}
	}

	for (uint8_t i = 0; i < gameplay_logic->tanks_count; i++) {
		if (gameplay_logic->tanks_logic[i].seconds_since_last_shot < TANK_SHOT_COOLDOWN_SECONDS)
			continue;

		for (uint8_t j = 0; j < gameplay_logic->outposts_count; j++) {
			if (Vector2Distance(gameplay_physics->outposts_physics[j].position, gameplay_physics->tanks_physics[i].position) < TANK_RANGE) {
				gameplay_logic->outposts_logic[j].health -= 15.f;
				gameplay_draw_data->tank_shot_animations[gameplay_draw_data->tank_shot_animations_count++] = (ShotAnimation) {
					.seconds_remaining = 0.2f,
					.tank_index = i,
					.outpost_index = j,
				};
				gameplay_logic->tanks_logic[i].seconds_since_last_shot = 0.f;

				break;
			}
		}
	}


	// evict zero health elements

	for (uint8_t i = 0; i < gameplay_logic->outposts_count; i++) {
		if (gameplay_logic->outposts_logic[i].health < 0.f) {
			evictElement(gameplay_logic->outposts_logic, gameplay_logic->outposts_count, sizeof (OutpostLogic), i);
			evictElement(gameplay_physics->outposts_physics, gameplay_logic->outposts_count, sizeof (OutpostPhysics), i);
			evictElement(gameplay_draw_data->outposts_draw_data, gameplay_logic->outposts_count, sizeof (OutpostDrawData), i);
			gameplay_logic->outposts_count--;
			gameplay_physics->outposts_count--;
			gameplay_draw_data->outposts_count--;
			break;
		}
	}

	for (uint8_t i = 0; i < gameplay_logic->tanks_count; i++) {
		if (gameplay_logic->tanks_logic[i].health < 0.f) {
			evictElement(gameplay_logic->tanks_logic, gameplay_logic->outposts_count, sizeof (TankLogic), i);
			evictElement(gameplay_physics->tanks_physics, gameplay_logic->outposts_count, sizeof (TankPhysics), i);
			evictElement(gameplay_draw_data->tanks_draw_data, gameplay_logic->outposts_count, sizeof (TankDrawData), i);
			gameplay_logic->tanks_count--;
			gameplay_physics->tanks_count--;
			gameplay_draw_data->tanks_count--;
			break;
		}
	}


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

	float frame_time = GetFrameTime();
	for (uint8_t i = 0; i < gameplay_logic->outposts_count; i++)
		gameplay_logic->outposts_logic[i].seconds_since_last_shot += frame_time;
	for (uint8_t i = 0; i < gameplay_logic->tanks_count; i++)
		gameplay_logic->tanks_logic[i].seconds_since_last_shot += frame_time;
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

void updateGameplayDrawData(GameplayDrawData *gameplay_draw_data, GameplayPhysics const *gameplay_physics)
{
	float frame_time = GetFrameTime();

	if (gameplay_draw_data->tanks_seconds_since_last_tick > 0.05f) {
		if (gameplay_draw_data->tanks_texture_x_offset == 0)
			gameplay_draw_data->tanks_texture_x_offset = 100;
		else
			gameplay_draw_data->tanks_texture_x_offset = 0;

		gameplay_draw_data->tanks_seconds_since_last_tick = 0.f;
	}
	gameplay_draw_data->tanks_seconds_since_last_tick += frame_time;

	// update animations (outpost and tank)

	for (uint8_t i = 0; i < gameplay_draw_data->outpost_shot_animations_count; i++)
		gameplay_draw_data->outpost_shot_animations[i].seconds_remaining -= frame_time;

	for (uint8_t i = 0; i < gameplay_draw_data->tank_shot_animations_count; i++)
		gameplay_draw_data->tank_shot_animations[i].seconds_remaining -= frame_time;


	// evict expired animations (outpost and tank)

	for (uint8_t i = 0; i < gameplay_draw_data->outpost_shot_animations_count; i++) {
		if (gameplay_draw_data->outpost_shot_animations[i].seconds_remaining < 0.f) {
			evictElement(gameplay_draw_data->outpost_shot_animations, gameplay_draw_data->outpost_shot_animations_count, sizeof (ShotAnimation), i);
			gameplay_draw_data->outpost_shot_animations_count--;
			break;
		}
	}

	for (uint8_t i = 0; i < gameplay_draw_data->tank_shot_animations_count; i++) {
		if (gameplay_draw_data->tank_shot_animations[i].seconds_remaining < 0.f) {
			evictElement(gameplay_draw_data->tank_shot_animations, gameplay_draw_data->tank_shot_animations_count, sizeof (ShotAnimation), i);
			gameplay_draw_data->tank_shot_animations_count--;
			break;
		}
	}


	for (uint8_t i = 0; i < gameplay_draw_data->outposts_count; i++)
		gameplay_draw_data->outposts_draw_data[i].turret_angle = atan2f(gameplay_physics->outposts_physics[i].turret_direction.y, gameplay_physics->outposts_physics[i].turret_direction.x) * 180.f / M_PI;

	for (uint8_t i = 0; i < gameplay_draw_data->tanks_count; i++) {
		if (Vector2Length(gameplay_physics->tanks_physics[i].velocity) > 1)
			gameplay_draw_data->tanks_draw_data[i].atlas_source_rectangle.x = gameplay_draw_data->tanks_texture_x_offset;

		gameplay_draw_data->tanks_draw_data[i].destination_rectangle.x = gameplay_physics->tanks_physics[i].position.x;
		gameplay_draw_data->tanks_draw_data[i].destination_rectangle.y = gameplay_physics->tanks_physics[i].position.y;

		gameplay_draw_data->tanks_draw_data[i].angle = atan2f(gameplay_physics->tanks_physics[i].velocity.y, gameplay_physics->tanks_physics[i].velocity.x) * 180.f / M_PI - 90.f;
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

void drawGameplay(GameplayLogic const *gameplay_logic, GameplayPhysics const *gameplay_physics, GameplayDrawData const *gameplay_draw_data)
{
	drawBackground(gameplay_draw_data->texture_atlas);

	for (uint8_t i = 0; i < gameplay_draw_data->tanks_path_points_count - 1; i++) { // Replace with baked background texture
		DrawLineEx(gameplay_draw_data->tanks_path_points[i], gameplay_draw_data->tanks_path_points[i + 1], TANKS_PATH_THICKNESS, BEIGE);
		DrawCircleV(gameplay_draw_data->tanks_path_points[i + 1], TANKS_PATH_THICKNESS / 2, BEIGE);
	}

	// draw animations (outpost and tank)

	for (uint8_t i = 0; i < gameplay_draw_data->outpost_shot_animations_count; i++) {
		switch (gameplay_logic->outposts_logic[gameplay_draw_data->outpost_shot_animations[i].outpost_index].type) {
		case OUTPOST_SIMPLE:
			uint8_t outpost_index = gameplay_draw_data->outpost_shot_animations[i].outpost_index;
			uint8_t tank_index = gameplay_draw_data->outpost_shot_animations[i].tank_index;

			Vector2 start = gameplay_physics->outposts_physics[outpost_index].position;
			Vector2 end = gameplay_physics->tanks_physics[tank_index].position;

			float distance = Vector2Distance(end, start);
			float cos = Vector2DotProduct(
				gameplay_physics->outposts_physics[outpost_index].turret_direction,
				Vector2Scale(Vector2Subtract(end, start), 1.f / distance)
			);
			if (cos < 0.5f) // To prevent excessively long Beziers
				cos = 0.5f;
			Vector2 control = Vector2Add(
				start,
				Vector2Scale(
					gameplay_physics->outposts_physics[outpost_index].turret_direction,
					distance / (cos * 2.f)
				)
			);

			DrawSplineBezierQuadratic(
				(Vector2 []) {start, control, end},
				3,
				5,
				(Color) {
					RED.r,
					RED.g,
					RED.b,
					255.f * gameplay_draw_data->outpost_shot_animations[i].seconds_remaining / OUTPOST_SIMPLE_ANIMATION_DURATION_SECONDS,
				}
			);
			break;
		case OUTPOST_MORTAR:
			break;
		case OUTPOST_PIERCE:
			break;
		}
	}

	for (uint8_t i = 0; i < gameplay_draw_data->outposts_count; i++) {
		drawOutpost(&gameplay_draw_data->outposts_draw_data[i], gameplay_draw_data->texture_atlas, WHITE);
		drawHealthBar(
			gameplay_logic->outposts_logic[i].health,
			OUTPOST_MAXIMUM_HEALTH,
			(Vector2) {
				.x = gameplay_draw_data->outposts_draw_data[i].base_destination_rectangle.x - HEALTH_BAR_WIDTH / 2,
				.y = gameplay_draw_data->outposts_draw_data[i].base_destination_rectangle.y - (gameplay_draw_data->outposts_draw_data[i].base_destination_rectangle.height / 2 + 1.5f * HEALTH_BAR_HEIGHT),
			}
		);
	}

	for (uint8_t i = 0; i < gameplay_draw_data->tanks_count; i++) {
		drawTank(&gameplay_draw_data->tanks_draw_data[i], gameplay_draw_data->texture_atlas);
		drawHealthBar(
			gameplay_logic->tanks_logic[i].health,
			TANK_MAXIMUM_HEALTH,
			(Vector2) {
				.x = gameplay_draw_data->tanks_draw_data[i].destination_rectangle.x - HEALTH_BAR_WIDTH / 2,
				.y = gameplay_draw_data->tanks_draw_data[i].destination_rectangle.y - (gameplay_draw_data->tanks_draw_data[i].destination_rectangle.height / 2 + 1.5f * HEALTH_BAR_HEIGHT),
			}
		);

	}
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

//CHECK HOW THIS FUNCTION NEEDS TO BE ALTERED AS PER REQUIREMENT
//void handleSettingsInput(Settings* settings) {
//    // Example: adjust sound volume with arrow keys
//    if (IsKeyPressed(KEY_UP)) {
//        settings->soundVolume += 0.1f;
//        if (settings->soundVolume > 1.0f) settings->soundVolume = 1.0f;
//    } else if (IsKeyPressed(KEY_DOWN)) {
//        settings->soundVolume -= 0.1f;
//        if (settings->soundVolume < 0.0f) settings->soundVolume = 0.0f;
//    }
//    // Add similar logic for other settings
//}



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

#define SQRT_2_F 1.414213f
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
				length * sqrt(1 - cos * cos) < TANKS_PATH_THICKNESS / 2 + 75.f / SQRT_2_F
			) ||
			Vector2Distance(position, path_points[i + 1]) < TANKS_PATH_THICKNESS / 2 + 75.f / SQRT_2_F
		)
			return false;
	}

	for (uint8_t i = 0; i < outposts_count; i++) {
		if (CheckCollisionRecs(
			(Rectangle) {
				.x = position.x,
				.y = position.y,
				.width = 75.f,
				.height = 75.f,
			},
			outposts_draw_data[i].base_destination_rectangle
		))
			return false;
	}

	return true;
}

#define SQRT_3_F 1.732050f
void placeOutpost(OutpostType type, Vector2 position, OutpostLogic *logic, OutpostPhysics *physics, OutpostDrawData *draw_data)
{
	*logic = (OutpostLogic) {
		.health = 100,
		.type = type,
	};

	*physics = (OutpostPhysics) {
		.position = position,
		.turret_direction = {SQRT_3_F / 2.f, -1.f / 2.f},
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
				if (canOutpostBePlaced(
					GetMousePosition(),
					gameplay_logic->tanks_path_points,
					gameplay_logic->tanks_path_points_count,
					gameplay_draw_data->outposts_draw_data,
					gameplay_logic->outposts_count
				)) {
					placeOutpost(
						game_ui_logic->selected_outpost,
						GetMousePosition(),
						gameplay_logic->outposts_logic + gameplay_logic->outposts_count,
						gameplay_physics->outposts_physics + gameplay_logic->outposts_count, 
						gameplay_draw_data->outposts_draw_data + gameplay_logic->outposts_count
					);
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
			if (canOutpostBePlaced(mouse_position, gameplay_logic->tanks_path_points, gameplay_logic->tanks_path_points_count, gameplay_draw_data->outposts_draw_data, gameplay_logic->outposts_count)) {
				DrawCircleV(
					mouse_position,
					OUTPOST_RANGE,
					(Color) {
						.r = GRAY.r,
						.g = GRAY.g,
						.b = GRAY.b,
						.a = 127,
					}
				);
				tint = GRAY;
			} else {
				tint = RED;
			}
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
	} else {
		for (uint8_t i = 0; i < gameplay_draw_data->outposts_count; i++) {
			Rectangle bounding_rectangle = gameplay_draw_data->outposts_draw_data[i].base_destination_rectangle;
			bounding_rectangle.x -= bounding_rectangle.width / 2;
			bounding_rectangle.y -= bounding_rectangle.height / 2;
			if (CheckCollisionPointRec(GetMousePosition(), bounding_rectangle)) {
				DrawCircle(
					gameplay_draw_data->outposts_draw_data[i].base_destination_rectangle.x,
					gameplay_draw_data->outposts_draw_data[i].base_destination_rectangle.y,
					OUTPOST_RANGE,
					(Color) {
						.r = GRAY.r,
						.g = GRAY.g,
						.b = GRAY.b,
						.a = 127,
					}
				);

				break;
			}
		}
	}
}

//MINOR ADJUSTMENTS REQUIRED, SLIDER NOT SLIDING
// Function to draw the settings page
//void DrawSettingsPage(Settings *settings) {
//    bool draggingSlider = false; // Flag to track if the slider handle is being dragged
//
//    while (!WindowShouldClose()) {
//        BeginDrawing();
//        ClearBackground(RAYWHITE);
//
//        // Draw volume adjustment slider
//        DrawText("Sound Volume", 100, 100, 20, BLACK);
//        DrawRectangle(100, 130, 200, 20, LIGHTGRAY);
//        
//        // Calculate the position of the slider handle based on the sound volume
//        int sliderHandlePosX = 100 + (int)(settings->soundVolume * 200);
//
//        // Check for mouse input
//        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
//            Vector2 mousePos = GetMousePosition();
//
//            // Check if the mouse is over the slider handle
//            if (CheckCollisionPointRec(mousePos, (Rectangle){sliderHandlePosX - 10, 130, 20, 20})) {
//                draggingSlider = true; // Start dragging the slider handle
//            }
//        } else {
//            draggingSlider = false; // Stop dragging the slider handle
//        }
//
//        // If the slider handle is being dragged, update the sound volume
//        if (draggingSlider) {
//            Vector2 mousePos = GetMousePosition();
//            // Ensure the slider handle stays within the slider bar bounds
//            settings->soundVolume = Clamp((mousePos.x - 100) / 200, 0.0f, 1.0f);
//        }
//
//        // Draw the slider handle
//        DrawRectangle(sliderHandlePosX - 10, 130, 20, 20, SKYBLUE);
//
//        EndDrawing();
//    }}

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

	InitAudioDevice();
	Music background_music = LoadMusicStream("assets/background-music.mp3"); // TODO currently broken
	PlayMusicStream(background_music);




#define TITLE_SCREEN_TANKS_COUNT 50
#define MAXIMUM_OUTPOSTS_COUNT 128
#define MAXIMUM_OUTPOST_SHOT_ANIMATIONS_COUNT 128
#define MAXIMUM_TANK_SHOT_ANIMATIONS_COUNT 128

	TextButtonSpecification title_screen_text_button_specifications[] = {
		(TextButtonSpecification) {
		        .text = "Play",
		        .rectangle = {
		        	.x = 100,
		        	.y = WINDOW_HEIGHT / 2,
		        	.width = 700,
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
				.width = 700,
				.height = 100,
			},
			.text_color = WHITE,
			.rectangle_color = RED,
		},
	};
	uint8_t title_screen_text_button_specifications_count = sizeof title_screen_text_button_specifications / sizeof (TextButtonSpecification);
	TextButtonSpecification title_screen_music_toggle_button_specification = {
	        .text = malloc(64 * sizeof (char)), // Trailing null to accomodate for Off condition
	        .rectangle = {
	        	.x = 100,
	        	.y = WINDOW_HEIGHT / 2 + 150,
	        	.width = 700,
	        	.height = 100,
	        },
	        .text_color = WHITE,
	        .rectangle_color = RED,
	};
	strcpy(title_screen_music_toggle_button_specification.text, "Toggle Music [On]");

	Texture2D texture_atlas = LoadTexture("assets/texture-atlas.png");

	Vector2 game_state_tanks_path_points[] = {
		(Vector2) {0, 200},
		(Vector2) {1000, 200},
		(Vector2) {1000, 550},
		(Vector2) {100, 550},
		(Vector2) {100, 900},
		(Vector2) {1300, 900},
		(Vector2) {1300, 100},
		(Vector2) {1650, 100},
		(Vector2) {1650, 1000},
		(Vector2) {1920, 1000},
	};
	uint8_t game_state_tanks_path_points_count = sizeof game_state_tanks_path_points / sizeof (Vector2);




	MetaState meta_state = TITLE_SCREEN;

	TitleScreenState title_screen_state = {
		.tanks_velocity = {50, -50},
		.text_button_specifications_original_rectangles = malloc(title_screen_text_button_specifications_count * sizeof (Rectangle)),
		.music_toggle_button_specification_original_rectangle = title_screen_music_toggle_button_specification.rectangle,
		.background_music = background_music,
		.tanks_count = TITLE_SCREEN_TANKS_COUNT,
		.text_button_specifications_count = title_screen_text_button_specifications_count,
	};

	TitleScreenDrawData title_screen_draw_data = {
		.texture_atlas = texture_atlas,
		.background_color = DARKGREEN,
		.tanks_draw_data = malloc(TITLE_SCREEN_TANKS_COUNT * sizeof (TankDrawData)),
		.text_button_specifications = title_screen_text_button_specifications,
		.music_toggle_button_specification = title_screen_music_toggle_button_specification,
		.tanks_count = TITLE_SCREEN_TANKS_COUNT,
		.text_button_specifications_count = title_screen_text_button_specifications_count,
	};

	GameplayLogic gameplay_logic = {
		.outposts_logic = malloc(MAXIMUM_OUTPOSTS_COUNT * sizeof (OutpostLogic)), // get rid of all alloca() calls? can this just be a reference to an rvalue?
		.tanks_logic = &(TankLogic) {
			.health = TANK_MAXIMUM_HEALTH,
		},
		.tanks_path_points = game_state_tanks_path_points,
		.tanks_count = 1,
		.tanks_path_points_count = game_state_tanks_path_points_count,
	};

	GameplayPhysics gameplay_physics = {
		.outposts_physics = malloc(MAXIMUM_OUTPOSTS_COUNT * sizeof (OutpostPhysics)),
		.tanks_physics = &(TankPhysics) {
			.position = {0, 200},
			.velocity = {1, 0},
		},
		.tanks_count = 1,
	};

	GameplayDrawData gameplay_draw_data = {
		.texture_atlas = texture_atlas,
		.outposts_draw_data = malloc(MAXIMUM_OUTPOSTS_COUNT * sizeof (OutpostDrawData)), // MALLOC
		.tanks_draw_data = &(TankDrawData) {
			.atlas_source_rectangle = RED_TANK_ATLAS_SOURCE_RECTANGLE,
			.destination_rectangle = {
				.width = GREEN_TANK_ATLAS_SOURCE_RECTANGLE.width,
				.height = GREEN_TANK_ATLAS_SOURCE_RECTANGLE.height,
			},
		},
		.outpost_shot_animations = malloc(MAXIMUM_OUTPOST_SHOT_ANIMATIONS_COUNT * sizeof (ShotAnimation)),
		.tank_shot_animations = malloc(MAXIMUM_TANK_SHOT_ANIMATIONS_COUNT * sizeof (ShotAnimation)),
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
		UpdateMusicStream(background_music);
		BeginDrawing(); // OK to have updation code after this

		switch (meta_state) {
		case TITLE_SCREEN:
			updateMetaStateAndTitleScreen(&meta_state, &title_screen_state, &title_screen_draw_data);
			drawTitleScreen(&title_screen_draw_data);
			break;
		case GAME:
			updateGameUiLogic(&game_ui_logic, &gameplay_logic, &gameplay_physics, &gameplay_draw_data);
			updateGameplayLogic(&gameplay_logic, &gameplay_physics, &gameplay_draw_data);
			updateGameplayPhysics(&gameplay_physics);

			updateGameplayDrawData(&gameplay_draw_data, &gameplay_physics);

			drawGameplay(&gameplay_logic, &gameplay_physics, &gameplay_draw_data);
			drawGameUi(&game_ui_logic, &gameplay_logic, &gameplay_draw_data, texture_atlas);
			break;
		case QUIT:
			goto quit;
		}

		EndDrawing();
	}
quit:
	UnloadMusicStream(background_music);
	CloseAudioDevice();
	CloseWindow();
	return 0;
}
