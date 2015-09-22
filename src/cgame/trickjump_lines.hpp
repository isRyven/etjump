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
	typedef float vec4_t[4];

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
	void displayCurrentRoute(int x);

	bool getRecording() { return _recording; }
	int countRoute() { return _routes.size(); }
	
private:

	unsigned long gcd_ui(unsigned long x, unsigned long y);
	unsigned long binomial(unsigned long n, unsigned long k);

	void draw4VertexLine(vec3_t start, vec3_t end, float width, vec4_t color);

	void addTrickjumpRecursiveBezier(std::vector<std::array<float, 3> > points, vec4_t color, float width, int nbDivision);
	void addTrickjumpLines(std::vector<std::array<float, 3> > points, vec4_t color, float width);
	void addJumpIndicator(vec3_t point, vec4_t color, float quadSize);

	bool _recording;
	bool _existingTJL;
	Route _currentRoute;
	std::vector<Route> _routes;
	unsigned _nextRecording;
	int _nextAddTime;
};
#endif