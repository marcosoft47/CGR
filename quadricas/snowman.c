// gcc snowman_sample.c -lglut -lGL -lGLU -lm -o snowman && ./snowman

#include <GL/glut.h>
  
// Rotation
static GLfloat yRot = 0.0f;
static GLfloat xRot = 0.0f;

// Change viewing volume and viewport. Called when window is resized  
void ChangeSize(int w, int h)  
    {  
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
    gluPerspective(35.0f, fAspect, 1.0, 40.0);  
  
    glMatrixMode(GL_MODELVIEW);  
    glLoadIdentity();  
    }  
  
  
// This function does any needed initialization on the rendering context.  Here it sets up and initializes the lighting for the scene.  
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
  
    // Black blue background  
    glClearColor(0.25f, 0.25f, 0.50f, 1.0f);  

}  
  
// Respond to arrow keys (rotate snowman)
void SpecialKeys(int key, int x, int y){  

    if(key == GLUT_KEY_LEFT)  
        yRot -= 5.0f;  
  
    if(key == GLUT_KEY_RIGHT)  
        yRot += 5.0f;  
                  
    yRot = (GLfloat)((const int)yRot % 360);  
    if(key == GLUT_KEY_UP)  
        xRot -= 5.0f;  
  
    if(key == GLUT_KEY_DOWN)  
        xRot += 5.0f;  
                  
    xRot = (GLfloat)((const int)xRot % 360);  
  
    // Refresh the Window  
    glutPostRedisplay();  
}
// Mouth is a bunch of equal pebbles. Might as well.
void drawPebble(GLUquadricObj *pObj, GLfloat x, GLfloat y, GLfloat z){
    glPushMatrix();
        glTranslatef(x, y, z);
        gluSphere(pObj, 0.02f, 10, 10);
    glPopMatrix();
}

void drawBranches(GLUquadricObj *pObj, GLfloat x, GLfloat y, GLfloat z, GLfloat xRot, GLfloat yRot){
    glPushMatrix();
		glTranslatef(x, y, z);
        glRotatef(xRot,0,1,0);
        glRotatef(yRot,1,0,0);
		gluCylinder(pObj, 0.03f, 0.01f, 0.45f, 26, 13);  
    glPopMatrix();
}
// Called to draw scene  
void RenderScene(void){  

    GLUquadricObj *pObj; // Quadric Object  
      
    // Clear the window with current clearing color  
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  
  
    // Save the matrix state and do the rotations  
    glPushMatrix();

	// Move object back and do in place rotation  
	glTranslatef(0.0f, -0.5f, -5.0f);  
	glRotatef(yRot, 0.0f, 1.0f, 0.0f);  
	glRotatef(xRot, 1.0f, 0.0f, 0.0f);  

	// Draw something  
	pObj = gluNewQuadric();  
	gluQuadricNormals(pObj, GLU_SMOOTH);  

	// white
	glColor3f(1.0f, 1.0f, 1.0f);  

	// Main Body  
    glPushMatrix();
	    gluSphere(pObj, 0.5f, 26, 13); 
    glPopMatrix();

	// Mid section
	glPushMatrix();
		glTranslatef(0.0f, 0.7f, 0.0f); 
		gluSphere(pObj, 0.33, 26, 13);
	glPopMatrix();

	// Head
	glPushMatrix();
		glTranslatef(0.0f, 1.2f, 0.0f);
		gluSphere(pObj, 0.24f, 26, 13);
	glPopMatrix();

	// Nose
	glColor3f(1.0f, 0.4f, 0.51f);  
	glPushMatrix();
		glTranslatef(0.0f, 1.2f, 0.2f);
		gluCylinder(pObj, 0.04f, 0.0f, 0.3f, 26, 13);  
	glPopMatrix();  

	// Eyes (black)
    glColor3f(0,0,0);
    glPushMatrix();
        glTranslatef(-0.1f, 1.25f, 0.2f);
        gluSphere(pObj, 0.05f, 10, 10);
    glPopMatrix();

    glPushMatrix();
        glTranslatef(0.1f, 1.25f, 0.2f);
        gluSphere(pObj, 0.05f, 10, 10);
    glPopMatrix();

    // Mouth
    drawPebble(pObj, 0.1f, 1.15f, 0.195f); //leftmost
    drawPebble(pObj, 0.05f, 1.14f, 0.21f);
    drawPebble(pObj, 0, 1.13f, 0.215f); // lower
    drawPebble(pObj, -0.05f, 1.14f, 0.21f);
    drawPebble(pObj, -0.1f, 1.15f, 0.195f); // rightmost

    // Tie
    // drawPebble(pObj, 0, 0.95, 0.21);
    drawPebble(pObj, 0, 0.9, 0.26);
    drawPebble(pObj, 0, 0.8, 0.31);
    drawPebble(pObj, 0, 0.7, 0.33);
    drawPebble(pObj, 0, 0.6, 0.31);
    // Branches
    glColor3f(0.2f, 0.1f, 0);
    drawBranches(pObj, 0.27f, 0.80f, 0.0f, 90, -30);
    drawBranches(pObj, -0.27f, 0.80f, 0.0f, -90, -30);

    glPushMatrix(); // Hands, gave up to do it in drawBranches
        glTranslatef(0.553, 0.96,0);
        glRotatef(90,0,1,0);
        glRotatef(10,1,0,0);
        gluCylinder(pObj, 0.02f, 0.01f, 0.15f, 26, 13);  
    glPopMatrix();

    glPushMatrix();
        glTranslatef(-0.553, 0.96,0);
        glRotatef(-90,0,1,0);
        glRotatef(10,1,0,0);
        gluCylinder(pObj, 0.02f, 0.01f, 0.15f, 26, 13);  
    glPopMatrix();



	// ----- Hat -----
    glColor3f(0,0,0);
    // Body
    glPushMatrix();
        glTranslatef(0.0f, 1.3f, 0.0f);
        glRotatef(-95,1,0,0);
        gluCylinder(pObj, 0.15f, 0.2f, 0.35f, 26, 13); 
    glPopMatrix();

	// Hat brim
    glPushMatrix();
        glTranslatef(0.0f, 1.31f, 0.0f);
        glRotatef(-95,1,0,0);
        gluCylinder(pObj, 0.30f, 0.30f, 0.05f, 26, 13);  
    glPopMatrix();  
    // Hat brim top cover
    glPushMatrix();
        glTranslatef(0.0f, 1.36f, 0.0f);
        glRotatef(-95,1,0,0);
        gluDisk(pObj, 0.0f, 0.3f, 26, 13);  
    glPopMatrix(); 
    // Hat top
    glPushMatrix();
        glTranslatef(0.0f, 1.65f, 0.0f);
        glRotatef(-95,1,0,0);
        // glRotatef(180,0,1,0);
        gluDisk(pObj, 0.0f, 0.2f, 26, 13);  
    glPopMatrix();

    // Hat bottom
    glPushMatrix();
        glTranslatef(0.0f, 1.31f, 0.0f);
        glRotatef(-95,1,0,0);
        glRotatef(180,0,1,0);
        gluDisk(pObj, 0.0f, 0.3f, 26, 13);  
    glPopMatrix(); 

    // Ribbon
    glColor3f(1.0f, 0.08f, 0.58f);  
    glPushMatrix();
            glTranslatef(0.0f, 1.35f, 0.0f);
            glRotatef(-95,1,0,0);
            gluCylinder(pObj, 0.2f, 0.16f, 0.06f, 26, 13);  
    glPopMatrix(); 

    // Restore the matrix state  
    glPopMatrix();  
  
    // Buffer swap  
    glutSwapBuffers();  

}


int main(int argc, char *argv[]){

    glutInit(&argc, argv);  
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);  
    glutInitWindowSize(800, 600);  
    glutCreateWindow("Boneco de neve");  
    glutReshapeFunc(ChangeSize);  
    glutSpecialFunc(SpecialKeys);  
    glutDisplayFunc(RenderScene);  
    SetupRC();  
    glutMainLoop();  
      
    return 0; 
}

