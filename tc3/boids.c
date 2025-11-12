#include <GLFW/glfw3.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

#define PI 3.14159265358979323846
// Valor modificavel para ajustar a janela
int SCREEN_WIDTH = 1280;
int SCREEN_HEIGHT = 720;

// boids
#define NUM_BOIDS 250
#define PERCEPTION_RADIUS 70.0f
#define MAX_SPEED 150.0f
#define MAX_FORCE 15.0f
#define MOUSE_REPEL_RADIUS 20.0f

#define SEPARATION_WEIGHT 20.0f
#define ALIGNMENT_WEIGHT  2.0f
#define COHESION_WEIGHT   0.3f
#define MOUSE_REPEL_WEIGHT 3.0f

typedef struct { float x, y; } Vector2;

Vector2 vec2_add(Vector2 v1, Vector2 v2) { return (Vector2){v1.x + v2.x, v1.y + v2.y}; }
Vector2 vec2_sub(Vector2 v1, Vector2 v2) { return (Vector2){v1.x - v2.x, v1.y - v2.y}; }
Vector2 vec2_mul_scalar(Vector2 v, float s) { return (Vector2){v.x * s, v.y * s}; }
float vec2_magnitude(Vector2 v) { return sqrt(v.x*v.x + v.y*v.y); }
Vector2 vec2_normalize(Vector2 v) {
    float mag = vec2_magnitude(v);
    if (mag > 0) return vec2_mul_scalar(v, 1.0f / mag);
    return (Vector2){0,0};
}
float vec2_distance(Vector2 v1, Vector2 v2) { return vec2_magnitude(vec2_sub(v1, v2)); }
Vector2 vec2_limit(Vector2 v, float max) {
    if (vec2_magnitude(v) > max) return vec2_mul_scalar(vec2_normalize(v), max);
    return v;
}

typedef struct {
    Vector2 position;
    Vector2 velocity;
    Vector2 acceleration;
    float maxSpeed;
    float maxForce;
    float r, g, b;
} Boid;

Boid boids[NUM_BOIDS];
Vector2 mouseWorldPos = {0,0};

// comportamento
Vector2 separation(Boid* current, Boid allBoids[], int currentIndex) {
    Vector2 steer = {0,0};
    int count = 0;
    for (int i = 0; i < NUM_BOIDS; i++) {
        if (i == currentIndex) continue;
        float d = vec2_distance(current->position, allBoids[i].position);
        if (d > 0 && d < PERCEPTION_RADIUS / 2.0) {
            Vector2 diff = vec2_sub(current->position, allBoids[i].position);
            diff = vec2_normalize(diff);
            diff = vec2_mul_scalar(diff, 1.0f / d);
            steer = vec2_add(steer, diff);
            count++;
        }
    }
    if (count > 0) steer = vec2_mul_scalar(steer, 1.0f / count);
    return steer;
}

Vector2 alignment(Boid* current, Boid allBoids[], int currentIndex) {
    Vector2 steer = {0,0};
    int count = 0;
    for (int i = 0; i < NUM_BOIDS; i++) {
        if (i == currentIndex) continue;
        float d = vec2_distance(current->position, allBoids[i].position);
        if (d > 0 && d < PERCEPTION_RADIUS) {
            steer = vec2_add(steer, allBoids[i].velocity);
            count++;
        }
    }
    if (count > 0) {
        steer = vec2_mul_scalar(steer, 1.0f / count);
        steer = vec2_normalize(steer);
        steer = vec2_mul_scalar(steer, current->maxSpeed);
        steer = vec2_sub(steer, current->velocity);
        steer = vec2_limit(steer, current->maxForce);
    }
    return steer;
}

Vector2 cohesion(Boid* current, Boid allBoids[], int currentIndex) {
    Vector2 steer = {0,0};
    int count = 0;
    for (int i = 0; i < NUM_BOIDS; i++) {
        if (i == currentIndex) continue;
        float d = vec2_distance(current->position, allBoids[i].position);
        if (d > 0 && d < PERCEPTION_RADIUS) {
            steer = vec2_add(steer, allBoids[i].position);
            count++;
        }
    }
    if (count > 0) {
        steer = vec2_mul_scalar(steer, 1.0f / count);
        steer = vec2_sub(steer, current->position);
        steer = vec2_normalize(steer);
        steer = vec2_mul_scalar(steer, current->maxSpeed);
        steer = vec2_sub(steer, current->velocity);
        steer = vec2_limit(steer, current->maxForce);
    }
    return steer;
}

Vector2 flee(Boid* b, Vector2 target) {
    Vector2 desired = vec2_sub(b->position, target);
    float distance = vec2_magnitude(desired);
    if (distance > MOUSE_REPEL_RADIUS) return (Vector2){0,0};
    
    desired = vec2_normalize(desired);
    desired = vec2_mul_scalar(desired, b->maxSpeed);
    Vector2 steer = vec2_sub(desired, b->velocity);
    steer = vec2_limit(steer, b->maxForce);
    if(distance > 0.1) steer = vec2_mul_scalar(steer, MOUSE_REPEL_RADIUS / distance);
    return steer;
}

// o resto

void applyBehaviors(Boid* b, int boidIndex) {
    Vector2 sep = separation(b, boids, boidIndex);
    Vector2 ali = alignment(b, boids, boidIndex);
    Vector2 coh = cohesion(b, boids, boidIndex);
    Vector2 fleeForce = flee(b, mouseWorldPos);

    sep = vec2_mul_scalar(sep, SEPARATION_WEIGHT);
    ali = vec2_mul_scalar(ali, ALIGNMENT_WEIGHT);
    coh = vec2_mul_scalar(coh, COHESION_WEIGHT);
    fleeForce = vec2_mul_scalar(fleeForce, MOUSE_REPEL_WEIGHT);

    b->acceleration = vec2_add(b->acceleration, sep);
    b->acceleration = vec2_add(b->acceleration, ali);
    b->acceleration = vec2_add(b->acceleration, coh);
    b->acceleration = vec2_add(b->acceleration, fleeForce);
}

void updateBoids(float deltaTime) {
    for (int i = 0; i < NUM_BOIDS; i++) {
        Boid* b = &boids[i];
        b->acceleration = (Vector2){0,0};
        applyBehaviors(b, i);

        b->velocity = vec2_add(b->velocity, b->acceleration);
        b->velocity = vec2_limit(b->velocity, b->maxSpeed);
        // Aplica a velocidade multiplicada pelo tempo para um movimento consistente
        b->position = vec2_add(b->position, vec2_mul_scalar(b->velocity, deltaTime));

        // Wraparound (peixes que saem de um lado aparecem do outro)
        if (b->position.x > SCREEN_WIDTH) b->position.x = 0;
        if (b->position.x < 0) b->position.x = SCREEN_WIDTH;
        if (b->position.y > SCREEN_HEIGHT) b->position.y = 0;
        if (b->position.y < 0) b->position.y = SCREEN_HEIGHT;
    }
}

void initBoids() {
    for (int i = 0; i < NUM_BOIDS; i++) {
        boids[i].position.x = (float)(rand() % SCREEN_WIDTH);
        boids[i].position.y = (float)(rand() % SCREEN_HEIGHT);
        boids[i].velocity.x = (float)(rand() % 200 - 100);
        boids[i].velocity.y = (float)(rand() % 200 - 100);
        boids[i].acceleration = (Vector2){0,0};
        boids[i].maxSpeed = MAX_SPEED;
        boids[i].maxForce = MAX_FORCE;
        boids[i].r = 0.5f + (float)(rand() % 50) / 100.0f;
        boids[i].g = 0.8f + (float)(rand() % 20) / 100.0f;
        boids[i].b = 1.0f;
    }
}

void drawBoid(Boid* b) {
    glPushMatrix();
    glTranslatef(b->position.x, b->position.y, 0.0f);

    // Calcula o ângulo a partir da velocidade para orientar o peixe
    float angle = atan2(b->velocity.y, b->velocity.x) * 180.0f / PI;
    glRotatef(angle, 0.0f, 0.0f, 1.0f);

    glColor3f(b->r, b->g, b->b);
    
    // Desenha um triângulo simples como peixe, apontando para a direita (eixo +X)
    glBegin(GL_TRIANGLES);
        glVertex2f(10.0f, 0.0f);   // Ponta (nariz)
        glVertex2f(-5.0f, 5.0f);  // Esquerda traseira
        glVertex2f(-5.0f, -5.0f); // Direita traseira
    glEnd();

    glPopMatrix();
}

// --- CALLBACKS E MAIN ---
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    // As coordenadas do mundo 2D são as mesmas da tela com o gluOrtho2D
    mouseWorldPos.x = (float)xpos;
    mouseWorldPos.y = (float)SCREEN_HEIGHT - (float)ypos; // Inverte Y
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    SCREEN_WIDTH = width;
    SCREEN_HEIGHT = height;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Projeção 2D, onde as coordenadas do mundo são as mesmas dos pixels da janela
    gluOrtho2D(0.0, (double)width, 0.0, (double)height);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main(void) {
    if (!glfwInit()) return -1;

    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Boids 2D - Peixes Desviando do Mouse", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSwapInterval(1); // V-Sync

    srand(time(NULL));
    initBoids();

    double lastTime = glfwGetTime();

    framebuffer_size_callback(window, SCREEN_WIDTH, SCREEN_HEIGHT); // Configura projeção inicial

    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        float deltaTime = (float)(currentTime - lastTime);
        lastTime = currentTime;

        // --- LÓGICA ---
        updateBoids(deltaTime);
        
        // --- RENDERIZAÇÃO ---
        glClearColor(0.1f, 0.2f, 0.4f, 1.0f); // Cor de água
        glClear(GL_COLOR_BUFFER_BIT);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // Desenha os peixes
        for (int i = 0; i < NUM_BOIDS; i++) {
            drawBoid(&boids[i]);
        }
        
        // Desenha o cursor "predador"
        glColor4f(1.0f, 0.0f, 0.0f, 0.5f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBegin(GL_TRIANGLE_FAN);
            glVertex2f(mouseWorldPos.x, mouseWorldPos.y);
            for(int i = 0; i <= 20; i++) {
                float angle = i * (2.0f * PI / 20.0f);
                glVertex2f(mouseWorldPos.x + cos(angle) * MOUSE_REPEL_RADIUS, 
                           mouseWorldPos.y + sin(angle) * MOUSE_REPEL_RADIUS);
            }
        glEnd();
        glDisable(GL_BLEND);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}