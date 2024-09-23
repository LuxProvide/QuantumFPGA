#ifndef BLOCHSPHERE
#define BLOCHSPHERE
#include <GL/glut.h>
#include <cmath>

extern float angleX ;
extern float angleY ;
extern float angleZ ;
extern int lastMouseX, lastMouseY;
extern bool isDragging;

extern float theta;
extern float phi;

void drawWireSphere(float radius, int slices, int stacks); 
void drawText(const char* text);
void drawAxes(); 
void drawAxisLabels(); 
void drawBoldArrow(float theta, float phi); 
void renderScene();
void keyboard(unsigned char key, int x, int y); 
void mouseMotion(int x, int y);
void mouseClick(int button, int state, int x, int y); 
void initOpenGL();
void bloch_sphere(int argc, char** argv); 
#endif
