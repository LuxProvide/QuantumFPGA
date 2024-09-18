#include "blochSphere.hpp"


// Function to plot the sphere (wired)
void drawWireSphere(float radius, int slices, int stacks) {
    GLUquadric* quad = gluNewQuadric();
    gluQuadricDrawStyle(quad, GLU_LINE); // Set wireframe style
    gluSphere(quad, radius, slices, stacks);
    gluDeleteQuadric(quad);
}

void drawText(const char* text) {
    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *text);
        text++;
    }
}

// Function to draw the axes
void drawAxes() {
    glBegin(GL_LINES);
    
    // X-axis (red)
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(1.0f, 0.0f, 0.0f);
    
    // Y-axis (green)
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, -1.0f, 0.0f);
    glVertex3f(0.0f, 1.0f, 0.0f);
    
    // Z-axis (blue)
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, -1.0f);
    glVertex3f(0.0f, 0.0f, 1.0f);
    
    glEnd();
}

// Function to label the axes
void drawAxisLabels() {
    // X-axis label
    glColor3f(1.0f, 0.0f, 0.0f);  // Red for X-axis
    glRasterPos3f(1.1f, 0.0f, 0.0f);  // Position for X label
    drawText("|+>");

    glColor3f(1.0f, 0.0f, 0.0f);  // Red for X-axis
    glRasterPos3f(-1.1f, 0.0f, 0.0f);  // Position for X label
    drawText("|->");
    
    // Y-axis label
    glColor3f(0.0f, 1.0f, 0.0f);  // Green for Y-axis
    glRasterPos3f(0.0f, 1.1f, 0.0f);  // Position for Y label
    glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 'Y');
    
    // Z-axis label
    glColor3f(0.0f, 0.0f, 1.0f);  // Blue for Z-axis
    glRasterPos3f(0.0f, 0.0f, 1.1f);  // Position for Z label
    drawText("|0>");
    
    // Z-axis label
    glColor3f(0.0f, 0.0f, 1.0f);  // Blue for Z-axis
    glRasterPos3f(0.0f, 0.0f, -1.1f);  // Position for Z label
    drawText("|1>");
}

// Function to draw a bold black arrow on the Bloch sphere
void drawBoldArrow(float theta, float phi) {
    // Convert spherical coordinates to Cartesian for arrow tip
    float x = sin(theta) * cos(phi);
    float y = sin(theta) * sin(phi);
    float z = cos(theta);

    // Set color to black for the arrow
    glColor3f(0.0f, 0.0f, 0.0f);
    
    // Make the arrow bold by increasing line width
    glLineWidth(3.0f);
    
    // Draw the arrow from origin (0, 0, 0) to (x, y, z)
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);  // Arrow start (origin)
    glVertex3f(x, y, z);            // Arrow end (tip)
    glEnd();
    
    // Optionally, draw the arrow head
    glPointSize(10.0f);
    glBegin(GL_POINTS);
    glVertex3f(x, y, z);  // Draw point at arrow tip to make it more visible
    glEnd();
}

// Rendering function
void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    // Move the scene back
    glTranslatef(0.0f, 0.0f, -5.0f);
    
    // Apply the rotation from the mouse interaction
    glRotatef(angleX, 1.0f, 0.0f, 0.0f);
    glRotatef(angleY, 0.0f, 1.0f, 0.0f);
    glRotatef(angleZ, 0.0f, 0.0f, 1.0f);
    
    // Draw the axes
    drawAxes();
    
    // Label the axes
    drawAxisLabels();
    
    // Draw the Bloch sphere (wireframe) with thin lines
    glColor3f(0.0f, 0.0f, 0.0f);  // Black for wireframe sphere
    glLineWidth(1.0f);  // Thin line for the wireframe
    drawWireSphere(1.0f, 20, 20);
    
    // Theta and phi are global variables
    drawBoldArrow(theta, phi);
    
    glutSwapBuffers();
}

// Function to handle keyboard input (for rotating the sphere)
void keyboard(unsigned char key, int x, int y) {
    const float angleStep = 5.0f;
    if (key == 'w') angleX -= angleStep;  // Rotate upward
    if (key == 's') angleX += angleStep;  // Rotate downward
    if (key == 'a') angleY -= angleStep;  // Rotate left
    if (key == 'd') angleY += angleStep;  // Rotate right
    glutPostRedisplay();  // Redraw the scene
}

// Mouse motion callback
void mouseMotion(int x, int y) {
    if (isDragging) {
        angleY += (x - lastMouseX) * 0.2f;
        angleX += (y - lastMouseY) * 0.2f;
        lastMouseX = x;
        lastMouseY = y;
        glutPostRedisplay();  // Redraw the scene
    }
}

// Mouse click callback
void mouseClick(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        isDragging = true;
        lastMouseX = x;
        lastMouseY = y;
    } else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        isDragging = false;
    }
}

// Initialize OpenGL
void initOpenGL() {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.97f, 0.91f, 0.81f, 1.0f);  // Champagne background
    glMatrixMode(GL_PROJECTION);
    gluPerspective(45.0, 1.0, 1.0, 10.0);
    glMatrixMode(GL_MODELVIEW);
}

void bloch_sphere(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 800);
    glutCreateWindow("Movable Wireframe Bloch Sphere with Arrow and Labels");
    initOpenGL();
    glutDisplayFunc(renderScene);
    glutKeyboardFunc(keyboard);  // Set keyboard callback
    glutMouseFunc(mouseClick);   // Set mouse click callback
    glutMotionFunc(mouseMotion); // Set mouse motion callback
    glutMainLoop();
}

