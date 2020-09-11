#include <iostream>
#include <ncurses.h>
#include <panel.h>
#include <math.h>
#include <string>
#include "frame_stats.hpp"

#define WALL_LIGHT 1
#define WALL_MED 2
#define WALL_DARK 3
#define FLOOR_LIGHT 4
#define FLOOR_MED 5
#define FLOOR_DARK 6
#define CEILING_LIGHT 7
#define CEILING_MED 8
#define CEILING_DARK 9
#define STAT_COLOR 10

using namespace std;

void print_stat_win(WINDOW *win, chtype, frame_stats);
void print_to_mid(WINDOW *win, int, int, int, string, chtype);

// Player X, Y position and viewing angle
float player_x = 8.0f;
float player_y = 8.0f;
float player_a = 0.0f;

// Map design - # represents wall and '.' for empty space
int map_height = 16;
int map_width = 16;
float map_depth = 16.0;

// Field of view
float fov = 3.14159 / 4.0;

// Map
string map;

/*
 * main()
 */
int main() {
	
	// Initialize ncurses
	initscr();
	curs_set(0);
	cbreak();	// one char at a time
	noecho();	// don't echo char inputs
	clear();

	// Define colors
	// init_pair(short pair, short fg, short bg);
	start_color();
	use_default_colors();
	init_pair(WALL_LIGHT, COLOR_BLUE, COLOR_BLUE);
	init_pair(WALL_MED, COLOR_BLACK, COLOR_BLUE);
	init_pair(WALL_DARK, COLOR_BLUE, COLOR_BLACK);
	init_pair(CEILING_LIGHT, COLOR_WHITE, COLOR_WHITE);
	init_pair(CEILING_MED, COLOR_BLACK, COLOR_WHITE);
	init_pair(CEILING_DARK, COLOR_WHITE, COLOR_BLACK);
	init_pair(FLOOR_LIGHT, COLOR_WHITE, COLOR_WHITE);
	init_pair(FLOOR_MED, COLOR_BLACK, COLOR_WHITE);
	init_pair(FLOOR_DARK, COLOR_WHITE, COLOR_BLACK);
	init_pair(STAT_COLOR, COLOR_WHITE, COLOR_BLACK);

	// Window stats
	int max_rows, max_cols;
	max_rows = LINES - 1;
	max_cols = COLS - 1;

	// Write map layout
	map += "################";
	map += "#..............#";
	map += "#..............#";
	map += "#.....####.#####";
	map += "#........#.#...#";
	map += "#........#.#...#";
	map += "#........#.#####";
	map += "#........#.....#";
	map += "#........#######";
	map += "#..............#";
	map += "#..............#";
	map += "#..............#";
	map += "#..............#";
	map += "#..............#";
	map += "#..............#";
	map += "################";

	// mvaddch(int y, int x, const chtype ch);
	// row, column, char

	// Initialize stats panel
	// TODO: move this stuff
	WINDOW *stat_win = subwin(stdscr, 10, 30, 2, (max_cols - 35));		// newwin(lines, cols, begin_y, begin_x)
	wbkgd(stat_win, COLOR_PAIR(STAT_COLOR));		// set stat window BG to ceiling color
	PANEL *stat_panel = new_panel(stat_win);		// create the panel abstraction
	top_panel(stat_panel);

	// Game loop
	while (1) {

		// Get player input
		char input = getch();
		if (input == 'a') {
			player_a -= (0.1f); 
		} else if (input == 'd') {
			player_a += (0.1f);
		} else if (input == 'w') {
			player_x += sinf(player_a) * 0.2f;
			player_y += cosf(player_a) * 0.2f;
		} else if (input == 's') {
			player_x -= sinf(player_a) * 0.2f;
			player_y -= cosf(player_a) * 0.2f;
		}

		// Stat variables
		int ray_trace_count = 0;

		// For each column on the screen
		// Or, for each ray that needs to be drawn calculated from the players angle and FOV
		for (int cur_col = 0; cur_col < max_cols; cur_col++) {
			
			// For each column, calculate the projected ray angle into world space
			// ray_angle is calculated in radians since FOV and player angle are in rads
			// Calculates the starting angle, and then adds the offset angle by finding the point
			// in which we are in the possible view sweep, and converting to radians by * FOV. 
			float ray_angle = (player_a - fov / 2.0f) + ((float)cur_col / (float)max_cols) * fov;

			// Calculate the i and j unit vectors for this ray angle
			// Using sin and cos, we can find the missing triangle values to hit the 1 unit outwards
			// in the direction of our calculated ray angle.
			float x_vec = sinf(ray_angle);
			float y_vec = cosf(ray_angle);

			// Boundary conditions, increment distance to wall until wall is hit
			float d_wall = 0;
			bool hit_wall = false;

			while (!hit_wall && d_wall < map_depth) {

				d_wall += 0.0005f;
				
				int test_x = (int)(player_x + x_vec * d_wall);
				int test_y = (int)(player_y + y_vec * d_wall);

				// Stat count
				ray_trace_count++; 

				// Check to see if this ray is out of bounds
				if (test_x < 0 || test_x >= map_width || test_y < 0 || test_y >= map_height) {

					hit_wall = true;
					d_wall = map_depth;

				} else {

					// The ray is inbounds, test to see if the ray cell is a wall block
					// Convert test points to array index
					if (map[test_y * map_width + test_x] == '#') {
						hit_wall = true;
					}
				}
			}

			// Calculate distance to ceiling and floor
			// This is what determines the wall height, as the distance to wall grows/shortens, the amount of 
			// rows provided for the ceiling floor expands/subtracts by a factor of the wall distance
			int ceiling = (float)(max_rows / 2.0) - max_rows / ((float)d_wall);
			int floor = max_rows - ceiling;

			for (int cur_row = 0; cur_row < max_rows; cur_row++) {

				if (cur_row < ceiling) {

					// Must be ceiling
					
					// If scaled distance is high, the ceiling is farther away
					// If scaled distance is low, the ceiling is close
					float d_scaled_ceiling = 1.0f - ((((float)max_rows / 2.0f) - (float)cur_row) / ((float)max_rows / 2.0f));

					if (d_scaled_ceiling < 0.35) { 
						// ceiling is close, shade light
						attron(COLOR_PAIR(CEILING_LIGHT));
						mvaddch(cur_row, cur_col, ACS_CKBOARD);
						attroff(COLOR_PAIR(CEILING_LIGHT));
					} else if (d_scaled_ceiling < 0.65) {
						// medium ceiling shade
						attron(COLOR_PAIR(CEILING_MED));
						mvaddch(cur_row, cur_col, ACS_CKBOARD);
						attroff(COLOR_PAIR(CEILING_MED));
					} else {
						// dark ceiling shade
						attron(COLOR_PAIR(CEILING_DARK));
						mvaddch(cur_row, cur_col, ACS_CKBOARD);
						attroff(COLOR_PAIR(CEILING_DARK));
					}

						/*attron(COLOR_PAIR(CEILING_LIGHT));
						mvaddch(cur_row, cur_col, ACS_CKBOARD);
						attroff(COLOR_PAIR(CEILING_LIGHT));*/

				} else if (cur_row >= ceiling && cur_row <= floor) {
					
					// Must be a wall
					// TODO: must be a better way to write this
					if (d_wall < map_depth / 3.25f) {	
						attron(COLOR_PAIR(WALL_LIGHT));
						mvaddch(cur_row, cur_col, ACS_CKBOARD);
						attroff(COLOR_PAIR(WALL_LIGHT));
					} else if (d_wall < map_depth / 1.3f) {
						attron(COLOR_PAIR(WALL_MED));
						mvaddch(cur_row, cur_col, ACS_CKBOARD);
						attroff(COLOR_PAIR(WALL_MED));
					} else {
						attron(COLOR_PAIR(WALL_DARK));
						mvaddch(cur_row, cur_col, ACS_CKBOARD);
						attroff(COLOR_PAIR(WALL_DARK));
					}

				} else {

					// Neither ceiling nor wall, must be floor

					// If scaled distance is high, the floor is close
					// If scaled distance is low, the floor is far
					float d_scaled_floor = (((float)cur_row - ((float)max_rows / 2)) / ((float)max_rows / 2));

					if (d_scaled_floor < 0.35) {
						// floor is far, shade dark
						attron(COLOR_PAIR(FLOOR_DARK));
						mvaddch(cur_row, cur_col, ACS_CKBOARD);
						attroff(COLOR_PAIR(FLOOR_DARK));
					} else if (d_scaled_floor < 0.60) {
						// medium floor shade
						attron(COLOR_PAIR(FLOOR_MED));
						mvaddch(cur_row, cur_col, ACS_CKBOARD);
						attroff(COLOR_PAIR(FLOOR_MED));
					} else {
						// light floor shade
						attron(COLOR_PAIR(FLOOR_LIGHT));
						mvaddch(cur_row, cur_col, ACS_CKBOARD);
						attroff(COLOR_PAIR(FLOOR_LIGHT));
					}

				}
			}
		}

		// Frame stat update
		frame_stats stats(ray_trace_count, player_x, player_y, player_a);
		print_stat_win(stat_win, COLOR_PAIR(STAT_COLOR), stats);

		// Update graphics
		refresh();
	}

	return 0;
}

void print_stat_win(WINDOW *win, chtype color, frame_stats stats) {

	// Get window specs
	int max_y;
	int max_x;
	getmaxyx(win, max_y, max_x);

	char buf[max_x-2];

	// Create outline box
	//wattron(win, color);		// set color attr for creating box lines

	box(win, 0, 0);

	mvwaddch(win, 2, 0, ACS_LTEE);
	mvwhline(win, 2, 1, ACS_HLINE, (max_x-2));
	mvwaddch(win, 2, (max_x-1), ACS_RTEE);

	print_to_mid(win, 1, 0, max_x, "Statistics", color);
	
	// Add statistics into box
	mvwprintw(win, 4, 2, "Ray traces: %d", stats.get_ray_trace_count());
	mvwprintw(win, 5, 2, "Player X: %0.2f", stats.get_player_x());
	mvwprintw(win, 6, 2, "Player Y: %0.2f", stats.get_player_y());
	mvwprintw(win, 7, 2, "Player A: %0.2f", stats.get_player_a());
	//wattroff(win, color);

	return;
}

void print_to_mid(WINDOW *win, int starty, int startx, int width, string text, chtype color) {

	int length, x, y;
	float temp;

	if (win == NULL) {
		win = stdscr;
	} 

	getyx(win, y, x);	// places current cursor position of the given window into y and x (row and column) 

	if (startx != 0)
		x = startx;
	if (starty != 0)
		y = starty;
	if (width == 0)
		width = 80;

	length = text.length();
	temp = (width - length) / 2;

	x = startx + (int)temp;		// offset cursor position to begin at correct point
	
	wattron(win, color);
	mvwprintw(win, y, x, "%s", text.c_str());
	wattroff(win, color);
}
