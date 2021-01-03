map_test:
	g++ -std=c++11 main.cpp frame_stats.cpp -lncurses -lpanel -o map_test

clean:
	rm map_test
