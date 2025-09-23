#include <GL/freeglut.h>
#include <math.h>

static GLfloat yRot = 0.0f;
static GLfloat xRot = 0.0f;
static GLfloat zRot = 0.0f;

static GLfloat xRotLeg = 0.0f;
static GLfloat yRotLeg = 0.0f;
static GLfloat zRotLeg = 0.0f;
static GLfloat xRotArm = 0.0f;
static GLfloat yRotArm = 0.0f;
static GLfloat zRotArm = 0.0f;

static GLfloat robotx = 0.0f;
static GLfloat roboty = -3.0f;
static GLfloat robotz = -50.0f;

// Change viewing volume and viewport. Called when window is resized  
void ChangeSize(int w, int h)  {  
    GLfloat fAspect;  
  
    // Prevent a divide by zero  
    if(h == 0)  
        h = 1;  
  
    // Set Viewport to window dimensions  
    glViewport(0, 0, w, h);  
  
    fAspect = (GLfloat)w/(GLfloat)h;  
  
    // Reset coordinate system  
    glMatrixMode(GL_PROJECTION);  
    glLoadIdentity();  
  
    // Produce the perspective projection  
    gluPerspective(35.0f, fAspect, 1.0, 70.0);  
  
    glMatrixMode(GL_MODELVIEW);  
    glLoadIdentity();  
}  

void SetupRC(){  
    // Light values and coordinates  
    GLfloat  whiteLight[] = { 0.05f, 0.05f, 0.05f, 1.0f };  
    GLfloat  sourceLight[] = { 0.4f, 0.4f, 0.4f, 1.0f };  
    GLfloat  lightPos[] = { -10.f, 5.0f, 5.0f, 1.0f };  
  
    glEnable(GL_DEPTH_TEST);    // Hidden surface removal  
    glFrontFace(GL_CCW);        // Counter clock-wise polygons face out  
    glEnable(GL_CULL_FACE);     // Do not calculate inside  
  
    // Enable lighting  
    glEnable(GL_LIGHTING);  
  
    // Setup and enable light 0  
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT,whiteLight);  
    glLightfv(GL_LIGHT0,GL_AMBIENT,sourceLight);  
    glLightfv(GL_LIGHT0,GL_DIFFUSE,sourceLight);  
    glLightfv(GL_LIGHT0,GL_POSITION,lightPos);  
    glEnable(GL_LIGHT0);  
  
    // Enable color tracking  
    glEnable(GL_COLOR_MATERIAL);  
      
    // Set Material properties to follow glColor values  
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);  
  
    float gray = 0.6;
    glClearColor(gray, gray, gray, 1.0f);  
}
// React to special keys
void SpecialKeys(int key, int x, int y){  

    if(key == GLUT_KEY_LEFT)  
        yRot += 5.0f;  
  
    if(key == GLUT_KEY_RIGHT)  
        yRot -= 5.0f;  
    
    if(key == GLUT_KEY_SHIFT_L)
        roboty += 0.5f;
    
    if (key == GLUT_KEY_UP)
        xRot += 5.0f;

    if (key == GLUT_KEY_DOWN)
        xRot -= 5.0f;
    
                  
    yRot = fmod(yRot, 360.0f);
    xRot = fmod(xRot, 360.0f);
    zRot = fmod(zRot, 360.0f);

    // Refresh the Window  
    glutPostRedisplay();  
}
void NormalKeys(unsigned char key, int x, int y){

    if (key == 'd')
        robotx += 0.5f;
    if (key == 'a')
        robotx -= 0.5f;
    if (key == 'w')
        robotz += 0.5f;
    if (key == 's')
        robotz -= 0.5f;
    if (key == '0'){
        xRot = 0;
        yRot = 0;
        zRot = 0;

        xRotLeg = 0.0f;
        yRotLeg = 0.0f;
        zRotLeg = 0.0f;
        xRotArm = 0.0f;
        yRotArm = 0.0f;
        zRotArm = 0.0f;
    }

    if (key == 'k')
        xRotLeg += 5.0f;
    if (key == 'j')
        xRotLeg -= 5.0f;
    if (key == '8')
        zRotArm += 5.0f;
    if (key == '2')
        zRotArm -= 5.0f;
    if (key == '4')
        xRotArm += 5.0f;
    if (key == '6')
        xRotArm -= 5.0f;

    if (key == 32)
        roboty -= 0.5f;
    if (key == 27)
        glutLeaveMainLoop();
    
    xRotLeg = fmod(xRotLeg, 360);

    glutPostRedisplay();
}
void renderLimb(GLUquadricObj *pObj, GLfloat xRotLimb, GLfloat yRotLimb, GLfloat zRotLimb){
    float length = 5;
    float r = 0.7;
    glPushMatrix();

    glPushMatrix();

        glRotatef(xRotLimb,1,0,0);
        glRotatef(180,0,1,0);
        glRotatef(zRotLimb,0,0,1);

        glRotatef(-90,1,0,0);
        gluDisk(pObj,0,r,20,20);
        glRotatef(90,1,0,0);

        glRotatef(90,1,0,0);
        gluCylinder(pObj, r, r, length, 20, 4);
        glRotatef(-90,1,0,0);
        glTranslatef(0,-length,0);
        gluSphere(pObj,r+0.2f,20,20);
        glRotatef(90,1,0,0);
        gluCylinder(pObj, r, r, length, 20, 4);
        glRotatef(-90,1,0,0);
        glTranslatef(0,-length,0);
        gluSphere(pObj,r+0.4f,20,20);
    glPopMatrix();

    glPopMatrix();
}
void renderBody(GLfloat width, GLfloat length, GLfloat height){
    float halfLength = width / 2.0f;
    float halfThickness = length / 2.0f;

    glPushMatrix();
        glRotatef(0, 0.0f, 1.0f, 0.0f);

        glBegin(GL_QUADS);
            // (+Z)
            glVertex3f(-halfLength, 0,      halfThickness);
            glVertex3f( halfLength, 0,      halfThickness);
            glVertex3f( halfLength, height, halfThickness);
            glVertex3f(-halfLength, height, halfThickness);

            // (-Z)
            glVertex3f(-halfLength, 0,      -halfThickness);
            glVertex3f(-halfLength, height, -halfThickness);
            glVertex3f( halfLength, height, -halfThickness);
            glVertex3f( halfLength, 0,      -halfThickness);
            
            // (+Y)
            glVertex3f(-halfLength, height, -halfThickness);
            glVertex3f(-halfLength, height,  halfThickness);
            glVertex3f( halfLength, height,  halfThickness);
            glVertex3f( halfLength, height, -halfThickness);
            
            // (-Y)
            glVertex3f(-halfLength, 0, -halfThickness);
            glVertex3f( halfLength, 0, -halfThickness);
            glVertex3f( halfLength, 0,  halfThickness);
            glVertex3f(-halfLength, 0,  halfThickness);
            
            // (+X)
            glVertex3f(halfLength, 0,      -halfThickness);
            glVertex3f(halfLength, height, -halfThickness);
            glVertex3f(halfLength, height,  halfThickness);
            glVertex3f(halfLength, 0,       halfThickness);

            // (-X)
            glVertex3f(-halfLength, 0,      -halfThickness);
            glVertex3f(-halfLength, 0,       halfThickness);
            glVertex3f(-halfLength, height,  halfThickness);
            glVertex3f(-halfLength, height, -halfThickness);
        glEnd();

    glPopMatrix();
}
void renderRobot(void){

    GLUquadricObj *pObj; // Quadric Object  
      
    // Clear the window with current clearing color  
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glPushMatrix();	
	glTranslatef(robotx, roboty, robotz); 
    // Move object back and do in place rotation  
	glRotatef(xRot, 1.0f, 0.0f, 0.0f);  
	glRotatef(yRot, 0.0f, 1.0f, 0.0f);  
	glRotatef(zRot, 0.0f, 0.0f, 1.0f); 

	pObj = gluNewQuadric();  
	gluQuadricNormals(pObj, GLU_SMOOTH);  
    

    // Legs
    glColor3f(1,1,1);
    glPushMatrix();
        glTranslatef(-2,0,0);
        renderLimb(pObj,xRotLeg,yRotLeg,zRotLeg);
    glPopMatrix();

    glPushMatrix();
        glColor3f(0,0,1);
        glTranslatef(2,0,0);
        renderLimb(pObj,-xRotLeg,-yRotLeg,-zRotLeg);
    glPopMatrix();

    // Arms
    glPushMatrix();
        glColor3f(1,0,0);
        glTranslatef(-2,6,0);
        glRotatef(-90,0,0,1);
        renderLimb(pObj,xRotArm,yRotArm,zRotArm);
    glPopMatrix();

    glPushMatrix();
        glColor3f(0,1,0);
        glTranslatef(2,6,0);
        glRotatef(90,0,0,1);
        renderLimb(pObj,xRotArm,-yRotArm,-zRotArm);
    glPopMatrix();

    // Body
    glPushMatrix();
        glTranslatef(0,0,0);
        glColor3f(0.5f,0,0.8f);
        renderBody(6,2.5,8);
    glPopMatrix();

    // Head
    glPushMatrix();
        glTranslatef(0,12,0);
        glColor3f(1,1,0);
        gluSphere(pObj, 4, 20, 20);
    glPopMatrix();

    glPushMatrix();
        glColor3f(0,0,0);
		glTranslatef(0.0f, 12.0f, 3.0f);
		gluCylinder(pObj, 1.0f, 0.0f, 2.0f, 26, 13);  
	glPopMatrix();  

    // End drawing
    glPopMatrix();
    // Buffer swap  
    glutSwapBuffers(); 
}

int main(int argc, char *argv[]){

    glutInit(&argc, argv);  
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);  
    glutInitWindowSize(800, 600);  
    glutCreateWindow("Robot");  
    glutReshapeFunc(ChangeSize);  
    glutSpecialFunc(SpecialKeys);  
    glutKeyboardFunc(NormalKeys);
    glutDisplayFunc(renderRobot);  
    SetupRC();  
    glutMainLoop();  
      
    return 0; 
}