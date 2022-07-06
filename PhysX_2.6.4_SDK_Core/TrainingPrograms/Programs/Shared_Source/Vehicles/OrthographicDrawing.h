#ifndef ORTHOGRAPHIC_DRAWING_H
#define ORTHOGRAPHIC_DRAWING_H

#include <vector>

#include <string>
#include <NxMath.h>
#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include <GL/glut.h>

// the drawing of the fonts is mostly just copied from the lighthouse tutorial
// (http://www.lighthouse3d.com/opengl/glut/index.php3?bmpfontortho)
class OrthographicDrawing {
private:


	LARGE_INTEGER _start_time;
	static LARGE_INTEGER _performance_frequency;

	void renderSpacedBitmapString(float x, float y, int spacing, void *font, const std::string& string);
	void renderSpacedBitmapStringWithNewline(float x, float y, int spacing, void *font, const std::string& string);

	struct _2dpos {
		int x,y;
	};

	struct _textMessage {
		float endTime;
		int frames;
		_2dpos position;
		std::string message;
	};
	std::vector<_textMessage> _messages;

public:
	OrthographicDrawing() {
		QueryPerformanceCounter(&_start_time);
		QueryPerformanceFrequency(&_performance_frequency);
	}
	void print(const std::string& message, float time);
	void reset();
	void setOrthographicProjection(float w, float h);
	void resetPerspectiveProjection();
	void render(bool setOrtho);

	void drawCircle(float posX, float posY, float radius, float fromDegree = 0, float toDegree = 360, float stepDegree = 10, int bigMarks = 0, int smallMarks = 0);
	void drawLine(float x1, float y1, float x2, float y2);
	void drawText(int xpos, int ypos, float seconds, int frames, const std::string& text);

	float now()	{
		LARGE_INTEGER current_time;
		QueryPerformanceCounter(&current_time);
		return (float)(current_time.QuadPart - _start_time.QuadPart) / (float)_performance_frequency.QuadPart;
	}
};

#endif
