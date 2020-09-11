#include "frame_stats.hpp"

frame_stats::frame_stats(
	int ray_trace_count, 
	float player_x, 
	float player_y, 
	float player_a) :
	ray_trace_count(ray_trace_count), 
	player_x(player_x),
	player_y(player_y),
	player_a(player_a) {}

int frame_stats::get_ray_trace_count() {
	return ray_trace_count;
}

float frame_stats::get_player_x() {
	return player_x;
}

float frame_stats::get_player_y() {
	return player_y;
}

float frame_stats::get_player_a() {
	return player_a;
}



