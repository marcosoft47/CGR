#include <GL/freeglut.h>
#include <math.h>

#define PI 3.1415
  
// Rotation
static GLfloat yRot = 0.0f;
static GLfloat xRot = 0.0f;
static GLfloat zRot = 0.0f;
static GLfloat castlex = 0.0f;
static GLfloat castley = -3.0f;
static GLfloat castlez = -17.0f;

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
  
    // Black blue background  
    float gray = 0.8;
    glClearColor(gray, gray, gray, 1.0f);  

}

// React to special keys
void SpecialKeys(int key, int x, int y){  

    if(key == GLUT_KEY_LEFT)  
        yRot += 5.0f;  
  
    if(key == GLUT_KEY_RIGHT)  
        yRot -= 5.0f;  
    
    if(key == GLUT_KEY_SHIFT_L)
        castley += 0.5f;
    
    if (key == GLUT_KEY_UP)
        xRot += 5.0f;
    if (key == GLUT_KEY_DOWN)
        xRot -= 5.0f;

    // if (key == GLUT_KEY_UP) {
    //     float yRotRad = yRot * PI / 180.0f;

    //     // Calcula o movimento nos eixos X e Z
    //     // sin(ângulo) dá o componente X do movimento
    //     // cos(ângulo) dá o componente Z do movimento
    //     xRot += cos(yRotRad) * 5.0f;
    //     zRot += sin(yRotRad) * 5.0f; // Usamos -= para Z porque o eixo Z aponta para "fora" da tela
    // }

    // if (key == GLUT_KEY_DOWN) {
    //     // Converte o ângulo de yRot para radianos
    //     float yRotRad = yRot * PI / 180.0f;

    //     // Move na direção oposta
    //     xRot += cos(yRotRad) * 5.0f;
    //     zRot += sin(yRotRad) * 5.0f;
    // }
    
                  
    yRot = fmod(yRot, 360.0f);
    xRot = fmod(xRot, 360.0f);
    zRot = fmod(zRot, 360.0f);

    // Refresh the Window  
    glutPostRedisplay();  
}
void NormalKeys(unsigned char key, int x, int y){

    if (key == 'd')
        castlex += 0.5f;
    if (key == 'a')
        castlex -= 0.5f;
    if (key == 'w')
        castlez += 0.5f;
    if (key == 's')
        castlez -= 0.5f;
    if (key == '0'){
        xRot = 0;
        yRot = 0;
        zRot = 0;
    }
    if (key == 32)
        castley -= 0.5f;
    if (key == 27)
        glutLeaveMainLoop();

    glutPostRedisplay();
}

void renderWindow(GLfloat r, GLfloat h, GLfloat angle){
    float wWidth = 0.4f;
    float wHeight = 0.5f;
    glPushMatrix();
            glTranslatef(0, h, 0);
            glRotatef(angle, 0, 1, 0);
            glTranslatef(0, 0, r + 0.001f); 
            glBegin(GL_QUADS);
                glVertex3f(-wWidth / 2, 0, 0.0f);
                glVertex3f( wWidth / 2, 0, 0.0f);
                glVertex3f( wWidth / 2, wHeight, 0.0f);
                glVertex3f(-wWidth / 2, wHeight, 0.0f);
            glEnd();
    glPopMatrix();
}

void renderTower(GLUquadricObj *pObj, GLfloat x, GLfloat y, GLfloat z){
    float h = 6;
    float r = 1.2;

    glPushMatrix();
    glTranslatef(x,y,z);

    glColor3f(0.5f,0.5f,0.5f);

    // Body
    glPushMatrix();
        glRotatef(-90,1,0,0);
        gluCylinder(pObj, r, r, h, 20, 20);
        glRotatef(180,1,0,0);
        glTranslatef(0,0.02,0);
        gluDisk(pObj, 0, r, 20, 20);
    glPopMatrix();

    // Hat (?)
    glColor3f(1.0f, 0.0f, 0.0f);
    glPushMatrix();
        glTranslatef(0, h, 0);
        glRotatef(90, 1, 0, 0);
        gluDisk(pObj, 0, r + 0.2f, 20, 20);
        glRotatef(-180, 1, 0, 0);
        gluCylinder(pObj, r + 0.2f, 0, 2, 20, 20);
    glPopMatrix();

    glColor3f(0,0,0);
    renderWindow(r, h-1, 0);   // +Z
    renderWindow(r, h-1, 90);  // +X
    renderWindow(r, h-1, 180); // -Z
    renderWindow(r, h-1, -90); // -X

    renderWindow(r, h-2, 0);   // +Z
    renderWindow(r, h-2, 90);  // +X
    renderWindow(r, h-2, 180); // -Z
    renderWindow(r, h-2, -90); // -X

    glPopMatrix();
}

void renderWall(float x, float y, float z, float length, float height, float thickness, float angleY) {
    float halfLength = length / 2.0f;
    float halfThickness = thickness / 2.0f;

    glPushMatrix();
        glTranslatef(x, y, z);
        glRotatef(angleY, 0.0f, 1.0f, 0.0f);

        glBegin(GL_QUADS);
            glColor3f(0.6f, 0.6f, 0.6f);
            // (+Z)
            glVertex3f(-halfLength, 0,      halfThickness);
            glVertex3f( halfLength, 0,      halfThickness);
            glVertex3f( halfLength, height, halfThickness);
            glVertex3f(-halfLength, height, halfThickness);

            // (-Z)
            glColor3f(0.5f, 0.5f, 0.5f);
            glVertex3f(-halfLength, 0,      -halfThickness);
            glVertex3f(-halfLength, height, -halfThickness);
            glVertex3f( halfLength, height, -halfThickness);
            glVertex3f( halfLength, 0,      -halfThickness);
            
            // (+Y)
            glColor3f(0.7f, 0.7f, 0.7f);
            glVertex3f(-halfLength, height, -halfThickness);
            glVertex3f(-halfLength, height,  halfThickness);
            glVertex3f( halfLength, height,  halfThickness);
            glVertex3f( halfLength, height, -halfThickness);
            
            // (-Y)
            glColor3f(0.4f, 0.4f, 0.4f);
            glVertex3f(-halfLength, 0, -halfThickness);
            glVertex3f( halfLength, 0, -halfThickness);
            glVertex3f( halfLength, 0,  halfThickness);
            glVertex3f(-halfLength, 0,  halfThickness);
            
            // (+X)
            glColor3f(0.55f, 0.55f, 0.55f);
            glVertex3f(halfLength, 0,      -halfThickness);
            glVertex3f(halfLength, height, -halfThickness);
            glVertex3f(halfLength, height,  halfThickness);
            glVertex3f(halfLength, 0,       halfThickness);

            // (-X)
            glColor3f(0.55f, 0.55f, 0.55f);
            glVertex3f(-halfLength, 0,      -halfThickness);
            glVertex3f(-halfLength, 0,       halfThickness);
            glVertex3f(-halfLength, height,  halfThickness);
            glVertex3f(-halfLength, height, -halfThickness);
        glEnd();

    glPopMatrix();
}

void renderCastle(void){
    float castleSizeX = 5;
    float castleSizeZ = 5;
    GLUquadricObj *pObj; // Quadric Object  
      
    // Clear the window with current clearing color  
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  
    // gluLookAt(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    // Save the matrix state and do the rotations  
    glPushMatrix();	
	glTranslatef(castlex, castley, castlez);  
    
    // Move object back and do in place rotation  
	glRotatef(xRot, 1.0f, 0.0f, 0.0f);  
	glRotatef(yRot, 0.0f, 1.0f, 0.0f);  
	glRotatef(zRot, 0.0f, 0.0f, 1.0f);  

	// Draw something  
	pObj = gluNewQuadric();  
	gluQuadricNormals(pObj, GLU_SMOOTH); 

    renderTower(pObj, castleSizeX,0,castleSizeZ);
    renderTower(pObj, castleSizeX,0,-castleSizeZ);
    renderTower(pObj,-castleSizeX,0,castleSizeZ);
    renderTower(pObj,-castleSizeX,0,-castleSizeZ);

    // Base
    glPushMatrix();
        glColor3f(0.13f, 0.33f, 0);
        glBegin(GL_QUADS);
            glVertex3f( castleSizeX, 0, castleSizeZ);
            glVertex3f( castleSizeX, 0,-castleSizeZ);
            glVertex3f(-castleSizeX, 0,-castleSizeZ);
            glVertex3f(-castleSizeX, 0, castleSizeZ);
        glEnd();
        glRotatef(180,1,0,0);
        glBegin(GL_QUADS);
            glVertex3f( castleSizeX, 0, castleSizeZ);
            glVertex3f( castleSizeX, 0,-castleSizeZ);
            glVertex3f(-castleSizeX, 0,-castleSizeZ);
            glVertex3f(-castleSizeX, 0, castleSizeZ);
        glEnd();
    glPopMatrix();

    // Wall
    GLfloat wallSize = 4;
    renderWall(            0, 0,  castleSizeZ, castleSizeZ*2, 5.0f, 0.5f,   0.0f);
    renderWall(            0, 0, -castleSizeZ, castleSizeZ*2, 5.0f, 0.5f, 180.0f);
    renderWall(  castleSizeX, 0,            0, castleSizeX*2, 5.0f, 0.5f,  90.0f);
    renderWall( -castleSizeX, 0,            0, castleSizeX*2, 5.0f, 0.5f, -90.0f);

    // Door
    float doorLocation = castleSizeZ+0.251f;
    float doorSize = 2;
    glColor3f(0.475f, 0.302f, 0.243f);  
    glPushMatrix();
        glTranslatef(0.0f, doorSize, doorLocation);
        gluDisk(pObj, 0.0f, doorSize, 20, 20); 
    glPopMatrix();

    glPushMatrix();
        glTranslatef(0.0f, 0, doorLocation);  // X centralizado
        glColor3f(0.475f, 0.302f, 0.243f);
        glBegin(GL_QUADS);   
            glVertex3f(-doorSize,     0.0f, 0.0f);  
            glVertex3f( doorSize,     0.0f, 0.0f);  
            glVertex3f( doorSize, doorSize, 0.0f); 
            glVertex3f(-doorSize, doorSize, 0.0f); 
        glEnd();  
    glPopMatrix();

    // Field
    float fieldSize = 10;
    glPushMatrix();
        glColor3f(0.13f, 0.33f, 0);
        glBegin(GL_QUADS);
            glVertex3f( castleSizeX+fieldSize, -0.1, castleSizeZ+fieldSize);
            glVertex3f( castleSizeX+fieldSize, -0.1,-castleSizeZ-fieldSize);
            glVertex3f(-castleSizeX-fieldSize, -0.1,-castleSizeZ-fieldSize);
            glVertex3f(-castleSizeX-fieldSize, -0.1, castleSizeZ+fieldSize);
        glEnd();
        glRotatef(180,1,0,0);
        glBegin(GL_QUADS);
            glVertex3f( castleSizeX+fieldSize, -0.1, castleSizeZ+fieldSize);
            glVertex3f( castleSizeX+fieldSize, -0.1,-castleSizeZ-fieldSize);
            glVertex3f(-castleSizeX-fieldSize, -0.1,-castleSizeZ-fieldSize);
            glVertex3f(-castleSizeX-fieldSize, -0.1, castleSizeZ+fieldSize);
        glEnd();
    glPopMatrix();

    glPopMatrix();  
    // Buffer swap  
    glutSwapBuffers(); 
}

int main(int argc, char *argv[]){

    glutInit(&argc, argv);  
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);  
    glutInitWindowSize(800, 600);  
    glutCreateWindow("Castelo");  
    glutReshapeFunc(ChangeSize);  
    glutSpecialFunc(SpecialKeys);  
    glutKeyboardFunc(NormalKeys);
    glutDisplayFunc(renderCastle);  
    SetupRC();  
    glutMainLoop();  
      
    return 0; 
}