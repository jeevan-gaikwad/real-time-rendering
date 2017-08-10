#include<GL/freeglut.h>

bool gbFullScreen = false;
static int shoulder = 0;
static int elbow = 0;
int main(int argc, char** argv) {
	int iScreenWidth = 0, iScreenHeight = 0;
	//function prototypes
	void display(void);
	void resize(int, int);
	void keyboard(unsigned char, int, int);
	void mouse(int, int, int, int);
	void initialize(void);
	void uninitialize(void);
	iScreenWidth = GetSystemMetrics(0);
	iScreenHeight = GetSystemMetrics(1);
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB| GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(((iScreenWidth / 2) - 400), ((iScreenHeight / 2) - 300));
	glutCreateWindow("Robo Arm using freeglut");
	initialize();

	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutCloseFunc(uninitialize);

	glutMainLoop();

	return (0);
}

void keyboard(unsigned char key, int x, int y) {
	void display();
	switch (key)
	{
	case 27: //Esc key
		glutLeaveMainLoop();
		break;
	case 'E':
		elbow = (elbow + 6) % 360;
		glutPostRedisplay();
		break;
	case 'e':
		elbow = (elbow - 6) % 360;
		glutPostRedisplay();
		break;
	case 'S':
		shoulder = (shoulder + 3) % 360;
		glutPostRedisplay();
		break;
	case 's':
		shoulder = (shoulder - 3) % 360;
		glutPostRedisplay();
		break;
	case 'F':
	case 'f':
		if (gbFullScreen == false) {
			glutFullScreen();
			gbFullScreen = true;
		}
		else {
			glutLeaveFullScreen();
			gbFullScreen = false;
		}
	default:
		break;
	}
}

void mouse(int button, int state, int x, int y) {
	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		break;
	default:
		break;
	}
}

void resize(int width, int height)
{
	//code
	if(height==0)
		height=1;
	glViewport(0,0,(GLsizei)width,(GLsizei)height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	
}
void initialize() {
	//code
	glShadeModel(GL_SMOOTH);
	// set background clearing color to black
	glClearColor(0.0f,0.0f,0.0f,0.0f);
	// set-up depth buffer
	glClearDepth(1.0f);
	// enable depth testing
	glEnable(GL_DEPTH_TEST);
	// depth test to do
	glDepthFunc(GL_LEQUAL);
	// set really nice percpective calculations ?
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
}
void uninitialize() {
	//code here
}


void display() {
	glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
	glTranslatef(0.0f,0.0f,-2.0f);
	glTranslatef(-1.0f,0.0f,0.0f);
	glRotatef((GLfloat)shoulder,0.0,0.0,1.0);
	glTranslatef(1.0f,0.0f,0.0f);
	glPushMatrix();
	glScalef(2.0f,0.4f,1.0f);
	glutWireCube(1.0);
	glPopMatrix();
	
	glTranslatef(1.0f,0.0f,0.0f);
	glRotatef((GLfloat)elbow,0.0f,0.0f,1.0f);
	glTranslatef(1.0f,0.0f,0.0f);
	glPushMatrix();
	glScalef(2.0f,0.4f,1.0f);
	glutWireCube(1.0);
	glPopMatrix();
	
	glPopMatrix();
	glutSwapBuffers();
	
}



