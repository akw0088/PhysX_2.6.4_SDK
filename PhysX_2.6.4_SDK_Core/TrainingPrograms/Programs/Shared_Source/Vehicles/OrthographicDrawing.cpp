#include "OrthographicDrawing.h"

#include <assert.h>

LARGE_INTEGER OrthographicDrawing::_performance_frequency;

void OrthographicDrawing::reset() {
	_messages.clear();
}

void OrthographicDrawing::renderSpacedBitmapString(float x, float y, int spacing, void *font, const std::string& string) {
	int x1=x;
	for (unsigned int i = 0; i < string.size(); i++) {
		char c = string[i];
		glRasterPos2f(x1,y);
		glutBitmapCharacter(font, c);
		x1 = x1 + glutBitmapWidth(font,c) + spacing;
	}
}

void OrthographicDrawing::renderSpacedBitmapStringWithNewline(float x, float y, int spacing, void *font, const std::string& string) {
	int x1=x;
	for (unsigned int i = 0; i < string.size(); i++) {
		char c = string[i];
		if (c == '\n') {
			x1 = x;
			y += 20;
		} else {
			glRasterPos2f(x1,y);
			glutBitmapCharacter(font, c);
			x1 = x1 + glutBitmapWidth(font,c) + spacing;
		}
	}
}

void OrthographicDrawing::setOrthographicProjection(float w, float h) {
	// switch to projection mode
	glMatrixMode(GL_PROJECTION);
	// save previous matrix which contains the 
	//settings for the perspective projection
	glPushMatrix();
	// reset matrix
	glLoadIdentity();
	// set a 2D orthographic projection
	gluOrtho2D(0, w, 0, h);
	// invert the y axis, down is positive
	glScalef(1, -1, 1);
	// mover the origin from the bottom left corner
	// to the upper left corner
	glTranslatef(0, -h, 0);
	glMatrixMode(GL_MODELVIEW);
}

void OrthographicDrawing::resetPerspectiveProjection() {
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void OrthographicDrawing::render(bool setOrtho) {
	glDisable(GL_LIGHTING);
	if (setOrtho) {
		int width = glutGet(GLUT_WINDOW_WIDTH);
		int height= glutGet(GLUT_WINDOW_HEIGHT);
		setOrthographicProjection(width,height);
		glPushMatrix();
		glLoadIdentity();
	}
	float _now = now();
	glColor4f(1,1,1,1);
	//int line = 20;
	for (unsigned int i = 0; i < _messages.size(); i++) {
		if (_now > _messages[i].endTime || _messages[i].frames <= 0) {
			_messages[i] = _messages[_messages.size()-1];
			_messages.pop_back();
			//_messages.replaceWithLast(i);
			i--;
		} else {
			_textMessage& cur = _messages[i];
			cur.frames--;
			//renderSpacedBitmapString(5,line,0,GLUT_BITMAP_HELVETICA_18,_messages[i].message);
			//line += 20;
			renderSpacedBitmapStringWithNewline(cur.position.x, cur.position.y, 0, GLUT_BITMAP_HELVETICA_18, cur.message);
		}
	}
	if (setOrtho) {
		glPopMatrix();
		resetPerspectiveProjection();
		glEnable(GL_LIGHTING);
	}
}

void OrthographicDrawing::drawCircle(float posX, float posY, float radius, float fromDegree, float toDegree, float stepDegree, int bigMarks, int smallMarks) {
	assert(stepDegree > 0.1f);
	glBegin(GL_LINE_STRIP);
	float fromRad = NxMath::degToRad(fromDegree);
	float toRad = NxMath::degToRad(toDegree);
	float stepRad = NxMath::degToRad(stepDegree);
	for (float angle = fromRad; angle <= toRad; angle += stepRad) {
		float Sin, Cos;
		NxMath::sinCos(angle, Sin, Cos);
		glVertex3f(posX + Cos*radius, posY - Sin*radius, 0);
	}
	glEnd();

	glBegin(GL_LINES);
	if (bigMarks > 0) {
		stepRad = (toRad - fromRad) / (float)bigMarks;
		float bigRadius = radius * 1.2f;
		float smallRadius = radius * 0.8f;
		float toRad2 = toRad + 0.01f;
		for (float angle = fromRad; angle <= toRad2; angle += stepRad) {
			float Sin, Cos;
			NxMath::sinCos(angle, Sin, Cos);
			glVertex3f(posX + Cos*bigRadius, posY - Sin*bigRadius, 0);
			glVertex3f(posX + Cos*smallRadius, posY - Sin*smallRadius, 0);
		}
	}
	if (smallMarks > 0) {
		stepRad = (toRad - fromRad) / (float)smallMarks;
		float bigRadius = radius * 1.1f;
		float smallRadius = radius * 0.9f;
		float toRad2 = toRad + 0.01f;
		for (float angle = fromRad; angle <= toRad2; angle += stepRad) {
			float Sin, Cos;
			NxMath::sinCos(angle, Sin, Cos);
			glVertex3f(posX + Cos*bigRadius, posY - Sin*bigRadius, 0);
			glVertex3f(posX + Cos*smallRadius, posY - Sin*smallRadius, 0);
		}
	}
	glEnd();
}

void OrthographicDrawing::drawLine(float x1, float y1, float x2, float y2) {
	glBegin(GL_LINES);
	glVertex3f(x1, y1, 0);
	glVertex3f(x2, y2, 0);
	glEnd();
}

void OrthographicDrawing::drawText(int xpos, int ypos, float seconds, int frames, const std::string& text) {
	// http://n.ethz.ch/student/hattp/man/glut
	/*NxU32 nbLineBreaks = 0;
	size_t offset = 0;

	float _now = now();
	while((offset = text.find('\n', offset)) != std::string::npos) {
		nbLineBreaks++;
		offset++;
	}
	offset = 0;
	for (NxU32 i = 0; i <= nbLineBreaks; i++) {
		size_t endpos = text.find('\n', offset);
		_textMessage curmessage;
		curmessage.message.clear();
		if (endpos == std::string::npos)
			endpos = text.size();
		for (size_t pos = offset; pos < endpos; pos++)
			curmessage.message.push_back(text[pos]);
		curmessage.position.x = xpos;
		curmessage.position.y = ypos;
		curmessage.endTime = _now + seconds;
		curmessage.frames = frames;
		if (curmessage.message.size() > 0)
			_messages.push_back(curmessage);
		ypos += 20;
		offset = endpos+1;
	}*/
	//printf("%d\n", nbLineBreaks);
	_textMessage curMessage;
	curMessage.message = text;
	curMessage.position.x = xpos;
	curMessage.position.y = ypos;
	curMessage.frames = frames;
	curMessage.endTime = now() + seconds;
	_messages.push_back(curMessage);
}
