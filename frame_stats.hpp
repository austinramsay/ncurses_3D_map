#ifndef STATS_H_
#define STATS_H_

class frame_stats { 

	private:
		float ray_trace_count; 
		float player_x;
		float player_y;
		float player_a;

	public:
		frame_stats(int ray_trace_count, float player_x, float player_y, float player_a);
		int get_ray_trace_count();
		float get_player_x();
		float get_player_y();
		float get_player_a();

};

#endif
