#include <GL/glut.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define MAX_PARTICLES 1000
#define PARTICLES_PER_FRAME 10

// Estrutura para representar uma única partícula
typedef struct {
    float x, y, z;          // Posição
    float vx, vy, vz;       // Velocidade
    float r, g, b, a;       // Cor e transparência
    float initialLifespan;  // Vida útil inicial
    float lifespan;         // Tempo de vida restante em segundos
    int active;             // Se a partícula está ativa
} Particle;

Particle particles[MAX_PARTICLES];

float camAngle = 0.0f;
float camDistance = 10.0f;
int lastTime = 0;

// Criar particula
void createParticle(Particle* p) {
    p->active = 1;
    
    p->x = (rand() % 20 - 10) / 20.0f; // -0.5 a 0.5
    p->y = -0.1f;
    p->z = (rand() % 20 - 10) / 20.0f; // -0.5 a 0.5

    p->vx = (rand() % 20 - 10) / 20.0f; // -0.5 a 0.5
    p->vy = (rand() % 100) / 100.0f + 1.5f; // 1.5 a 2.5
    p->vz = (rand() % 20 - 10) / 20.0f; // -0.5 a 0.5

    // Tempo de vida entre 1.0 e 2.0 segundos
    p->lifespan = (rand() % 100) / 100.0f + 1.0f;
    p->initialLifespan = p->lifespan;

    p->r = 1.0f;
    p->g = 0.5f + (rand() % 50) / 100.0f; // Variação no verde para tons de laranja
    p->b = 0.0f;
    p->a = 1.0f;
}

// Função de inicialização do OpenGL e do sistema de partículas
void init() {
    glClearColor(0.0f, 0.0f, 0.1f, 1.0f); // Fundo azul escuro

    glEnable(GL_DEPTH_TEST);    // Hidden surface removal  
    glFrontFace(GL_CCW);        // Counter clock-wise polygons face out  
    glEnable(GL_CULL_FACE);     // Do not calculate inside  
    glEnable(GL_LIGHTING);

    GLfloat  whiteLight[] = { 0.05f, 0.05f, 0.05f, 1.0f };  
    GLfloat  sourceLight[] = { 0.4f, 0.4f, 0.4f, 1.0f };  
    GLfloat  lightPos[] = { -10.f, 5.0f, 5.0f, 1.0f };  
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT,whiteLight);  
    glLightfv(GL_LIGHT0,GL_AMBIENT,sourceLight);  
    glLightfv(GL_LIGHT0,GL_DIFFUSE,sourceLight);  
    glLightfv(GL_LIGHT0,GL_POSITION,lightPos);  
    glEnable(GL_LIGHT0);  
    
    // Enable color tracking  
    glEnable(GL_COLOR_MATERIAL);  
      
    // Set Material properties to follow glColor values  
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);  


    // Inicializa o gerador de números aleatórios
    srand(time(NULL));

    // Inicializa todas as partículas como inativas
    for (int i = 0; i < MAX_PARTICLES; i++) {
        particles[i].active = 0;
    }
    
    // Pega o tempo inicial para o cálculo do deltaTime
    lastTime = glutGet(GLUT_ELAPSED_TIME);
}

// Função para desenhar as toras da fogueira
void drawLogs() {
    glColor3f(0.4f, 0.2f, 0.0f);

    GLUquadricObj *pObj = gluNewQuadric();
    GLfloat radius = 0.3f;
    
    glTranslatef(0.0f, 1.5f, 0.0f);
    // Tora 1
    glPushMatrix();
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(40.0f, 1.0f, 0.0f, 0.0f);
        glTranslatef(0.0f,0.0f,-0.3f);
        gluCylinder(pObj, radius, radius+0.1f, 4.0, 10, 2);
    glPopMatrix();
    
    // Tora 2
    glPushMatrix();
        glRotatef(-30.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(40.0f, 1.0f, 0.0f, 0.0f);
        glTranslatef(0.0f,0.0f,-0.3f);
        gluCylinder(pObj, radius, radius+0.1f, 4.0, 10, 2);
    glPopMatrix();

    // Tora 3
     glPushMatrix();
        glRotatef(210.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(40.0f, 1.0f, 0.0f, 0.0f);
        glTranslatef(0.0f,0.0f,-0.3f);
        gluCylinder(pObj, radius, radius+0.1f, 4.0, 10, 2);
    glPopMatrix();

    gluDeleteQuadric(pObj);
}

// Função principal de renderização
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Configura a câmera orbital
    gluLookAt(camDistance * cos(camAngle), 4.0, camDistance * sin(camAngle),
              0.0, 1.5, 0.0,
              0.0, 1.0, 0.0);
    
    // chão
    glColor3f(0.1f, 0.4f, 0.1f);
    glBegin(GL_QUADS);
        glNormal3f(0,1,0);
        glVertex3f(-10.0f, 0.0f, -10.0f);
        glVertex3f(-10.0f, 0.0f,  10.0f);
        glVertex3f( 10.0f, 0.0f,  10.0f);
        glVertex3f( 10.0f, 0.0f, -10.0f);
    glEnd();

    // Desenha as toras
    drawLogs();

    // Começo particulas
    glPushMatrix();
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPointSize(8.0f); // Tamanho particulas
    glBegin(GL_POINTS);
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            Particle* p = &particles[i];
            
            float lifeFactor = p->lifespan / p->initialLifespan;
            
            // Mudar para laranja escuro
            float r = 1.0f;
            float g = p->g * lifeFactor;
            float b = p->b;
            float a = p->a * lifeFactor;
            
            glColor4f(r, g, b, a);
            glVertex3f(p->x, p->y, p->z);
        }
    }
    glEnd();

    glDisable(GL_BLEND);
    
    glPopMatrix();
    glutSwapBuffers();
}

// Função de atualização da lógica (física das partículas)
void update(int value) {
    // Calcula o delta time para animação suave
    int currentTime = glutGet(GLUT_ELAPSED_TIME);
    float deltaTime = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;

    // Atualiza cada partícula
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            Particle* p = &particles[i];

            // Atualiza a posição
            p->x += p->vx * deltaTime;
            p->y += p->vy * deltaTime;
            p->z += p->vz * deltaTime;

            p->vy -= 1.0f * deltaTime; // Desaceleração

            // Diminui o tempo de vida
            p->lifespan -= deltaTime;
            if (p->lifespan <= 0.0f) {
                p->active = 0;
            }
        }
    }

    // Cria novas partículas para substituir as que morreram
    int newParticlesCount = 0;
    for (int i = 0; i < MAX_PARTICLES && newParticlesCount < PARTICLES_PER_FRAME; i++) {
        if (!particles[i].active) {
            createParticle(&particles[i]);
            newParticlesCount++;
        }
    }

    glutPostRedisplay();
    
    glutTimerFunc(16, update, 0);
}

// Callback de redimensionamento da janela
void reshape(int w, int h) {
    if (h == 0) h = 1;
    float ratio = 1.0f * w / h;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, w, h);
    gluPerspective(45, ratio, 1, 1000);
    glMatrixMode(GL_MODELVIEW);
}

// Callback de teclas especiais para controlar a câmera
void specialKeys(int key, int x, int y) {
    switch(key) {
        case GLUT_KEY_LEFT:
            camAngle -= 0.1f;
            break;
        case GLUT_KEY_RIGHT:
            camAngle += 0.1f;
            break;
        case GLUT_KEY_UP:
            camDistance -= 0.5f;
            break;
        case GLUT_KEY_DOWN:
            camDistance += 0.5f;
            break;
    }
}

// Função Main
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    glutCreateWindow("Fireplace");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(specialKeys);
    
    // Inicia o loop de atualização
    glutTimerFunc(0, update, 0);

    glutMainLoop();
    return 0;
}