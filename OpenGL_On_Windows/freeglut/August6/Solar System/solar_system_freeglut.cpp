#include<GL/freeglut.h>

bool gbFullScreen = false;
static int year = 0;
static int day = 0;
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
	glutCreateWindow("Solar System using freeglut");
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
	case 68:
		day = (day + 6) % 360;
		glutPostRedisplay();
		break;
	case 'd':
		day = (day - 6) % 360;
		glutPostRedisplay();
		break;
	case 'Y':
		year = (year + 3) % 360;
		glutPostRedisplay();
		break;
	case 'y':
		year = (year - 3) % 360;
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
	glColor3f(1.0f,1.0f,1.0f);
	glPushMatrix();
	glutWireSphere(1.0f,20,16);
	glRotatef((GLfloat)year,0.0,1.0,0.0);
	glTranslatef(2.0f,0.0f,0.0f);
	glRotatef((GLfloat)day,0.0f,1.0f,0.0f);
	glutWireSphere(0.2f,10,8);
	glPopMatrix();
	glutSwapBuffers();
	/* //Keeping native style with freeglut for reference. It works.
	GLUquadric *quadric = NULL;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -4.0f);	
	glPushMatrix();
	
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		quadric = gluNewQuadric();
		glColor3f(1.0f, 1.0f, 0.0f);
		gluSphere(quadric, 0.75f, 30, 30);

		glPushMatrix(); //earth
		glRotatef((GLfloat)year, 0.0f, 1.0f, 0.0f);
		glTranslatef(1.5f, 0.0f, 0.0f);
		glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
		glRotatef((GLfloat)day, 0.0f, 0.0f, 1.0f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		gluDeleteQuadric(quadric);
		quadric = gluNewQuadric();
		glColor3f(0.4f, 0.9f, 1.0f);
		gluSphere(quadric, 0.2, 20, 20);
		glPopMatrix();
	
	glPopMatrix();
	glutSwapBuffers();
	gluDeleteQuadric(quadric);
    */
}



