#ifndef TRICKJUMP_LINES_HPP
#define TRICKJUMP_LINES_HPP
#include <string>
#include <vector>
#include <array>

class TrickjumpLines
{
public:
	static const unsigned LINE_WIDTH = 1;

	typedef float vec3_t[3];

	struct Route
	{
		std::string name;
		std::vector<std::array<float, 3> > nodes;
		float width;
	};

	TrickjumpLines();
	~TrickjumpLines();

	/**
	 * Starts tjl recording
	 * @param name Name of the tjl file. Can be nullptr -> automatically generated filename
	 */
	void record(const char *name);
	/**
	 * Stops tjl recording and saves the current tjl
	 */
	void stopRecord();
	/**
	 * Adds current position to the currently recorded tjl (if we're recording)
	 * TODO: Should we also give speed as a param?
	 * @param pos Current position
	 */
	void addPosition(vec3_t pos);
	/**
	 * Displays the latest recorded route
	 */
	void displayCurrentRoute();
	
private:
	bool _recording;
	Route _currentRoute;
	std::vector<Route> _routes;
	unsigned _nextRecording;
	int _nextAddTime;
};
#endif