/**
 * Simulação de Tecido com OpenGL e C++
 * g++ main.cpp glad/glad.c -o cloth_sim -lglfw -lGL -ldl -I.
 * * Controles:
 * - Q: Sair
 * - W: Ativar/Desativar Vento
 * - Z: Zoom out
 * - X: Zoom in
 * - Seta cima/baixo: Ajustar vento
 * - Seta esquerda/direita: Girar tecido
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Define STB_IMAGE_IMPLEMENTATION apenas em um arquivo .cpp
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <vector>
#include <iostream>
#include <cmath>

// --- Configurações ---
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;
const int CLOTH_WIDTH = 30;  // Número de partículas na largura
const int CLOTH_HEIGHT = 20; // Número de partículas na altura
const float SPACING = 0.3f;  // Distância entre partículas

// Variáveis de Controle Global
float windStrength = 15.0f; // Força inicial do vento
float cameraAngle = 0.0f;   // Ângulo de rotação da câmera
float cameraRadius = 15.0f; // Distância da câmera

// --- Shaders (Embutidos para facilitar arquivo único) ---
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoord = aTexCoord;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture1;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main() {
    // Luz Ambiente
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0);
  
    // Textura
    vec4 texColor = texture(texture1, TexCoord);
    
    // Iluminação simples (frente e verso)
    // Se gl_FrontFacing for false, estamos vendo o verso do triangulo, entao invertemos a normal
    vec3 normalFixed = gl_FrontFacing ? Normal : -Normal;
    vec3 norm = normalize(normalFixed);
    vec3 lightDir = normalize(lightPos - FragPos);
    
    // max(..., 0.3) garante um pouco de luz mesmo contra a luz para não ficar totalmente preto
    float diff = max(dot(norm, lightDir), 0.3); 
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);
    
    vec3 result = (ambient + diffuse) * texColor.rgb;
    FragColor = vec4(result, 1.0);
}
)";

// --- Classes de Física ---

struct Particle {
    glm::vec3 pos;
    glm::vec3 oldPos;
    glm::vec3 acceleration;
    glm::vec2 texCoord;
    glm::vec3 normal;
    float mass;
    bool movable;

    Particle(glm::vec3 p, float m, glm::vec2 uv) 
        : pos(p), oldPos(p), acceleration(0.0f), mass(m), movable(true), texCoord(uv), normal(0.0f) {}

    void addForce(glm::vec3 f) {
        if (!movable) return;
        acceleration += f / mass;
    }

    void timeStep(float dt) {
        if (!movable) return;
        glm::vec3 temp = pos;
        // Integração de Verlet: pos = pos + (pos - oldPos) * (1.0 - damping) + acc * dt * dt
        pos = pos + (pos - oldPos) * 0.99f + acceleration * dt * dt;
        oldPos = temp;
        acceleration = glm::vec3(0.0f); // Resetar aceleração
    }
};

struct Constraint {
    Particle* p1;
    Particle* p2;
    float restDistance;

    Constraint(Particle* pi1, Particle* pi2) : p1(pi1), p2(pi2) {
        restDistance = glm::length(p1->pos - p2->pos);
    }

    void satisfy() {
        glm::vec3 delta = p2->pos - p1->pos;
        float currentDistance = glm::length(delta);
        if (currentDistance == 0.0f) return; // Evitar divisão por zero

        float correction = (currentDistance - restDistance) / currentDistance;
        glm::vec3 correctionVector = delta * 0.5f * correction;

        if (p1->movable && p2->movable) {
            p1->pos += correctionVector;
            p2->pos -= correctionVector;
        } else if (p1->movable) {
            p1->pos += delta * correction;
        } else if (p2->movable) {
            p2->pos -= delta * correction;
        }
    }
};

class Cloth {
public:
    std::vector<Particle> particles;
    std::vector<Constraint> constraints;
    std::vector<unsigned int> indices;
    int width, height;

    Cloth(int w, int h) : width(w), height(h) {
        particles.reserve(w * h);
        
        // Criar Partículas
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                // Ajuste de altura: - 6.0f para baixar o tecido (topo começa em Y=0 em vez de Y=6)
                glm::vec3 pos((x - w/2) * SPACING, (h - y) * SPACING - 6.0f, 0.0f);
                
                // Correção UV: Inverter o eixo Y (1.0 - y) para alinhar o topo da textura com o topo do tecido
                glm::vec2 uv((float)x / (w - 1), 1.0f - (float)y / (h - 1)); 
                
                // Mola superior (varal) tem massa infinita (movable = false)
                Particle p(pos, 1.0f, uv);
                if (y == 0) { 
                    p.movable = false; // Pinos no varal
                }
                particles.push_back(p);
            }
        }

        // Criar Restrições (Molas)
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                if (x < w - 1) makeConstraint(getParticle(x, y), getParticle(x + 1, y)); // Horizontal
                if (y < h - 1) makeConstraint(getParticle(x, y), getParticle(x, y + 1)); // Vertical
                if (x < w - 1 && y < h - 1) makeConstraint(getParticle(x, y), getParticle(x + 1, y + 1)); // Cisalhamento A
                if (x > 0 && y < h - 1) makeConstraint(getParticle(x, y), getParticle(x - 1, y + 1)); // Cisalhamento B
            }
        }

        // Gerar índices para triângulos
        for (int y = 0; y < h - 1; y++) {
            for (int x = 0; x < w - 1; x++) {
                unsigned int topLeft = y * w + x;
                unsigned int topRight = topLeft + 1;
                unsigned int bottomLeft = (y + 1) * w + x;
                unsigned int bottomRight = bottomLeft + 1;

                indices.push_back(topLeft);
                indices.push_back(bottomLeft);
                indices.push_back(topRight);

                indices.push_back(topRight);
                indices.push_back(bottomLeft);
                indices.push_back(bottomRight);
            }
        }
    }

    Particle* getParticle(int x, int y) {
        return &particles[y * width + x];
    }

    void makeConstraint(Particle* p1, Particle* p2) {
        constraints.push_back(Constraint(p1, p2));
    }

    void calculateNormals() {
        // Resetar normais
        for (auto& p : particles) p.normal = glm::vec3(0.0f);

        // Calcular normais por triângulo e acumular nos vértices
        for (size_t i = 0; i < indices.size(); i += 3) {
            Particle& p1 = particles[indices[i]];
            Particle& p2 = particles[indices[i+1]];
            Particle& p3 = particles[indices[i+2]];

            glm::vec3 v1 = p2.pos - p1.pos;
            glm::vec3 v2 = p3.pos - p1.pos;
            glm::vec3 normal = glm::cross(v1, v2);

            p1.normal += normal;
            p2.normal += normal;
            p3.normal += normal;
        }

        // Normalizar
        for (auto& p : particles) p.normal = glm::normalize(p.normal);
    }

    void update(float dt, glm::vec3 windDir, bool windEnabled) {
        // 1. Acumular Forças
        for (auto& p : particles) {
            p.addForce(glm::vec3(0.0f, -9.8f, 0.0f)); // Gravidade

            if (windEnabled) {
                // Vento simples: Adiciona ruído baseado no tempo e posição
                float noise = sin(glfwGetTime() * 3.0f + p.pos.x * 0.5f + p.pos.y * 0.5f);
                // Vento aplica força variável
                glm::vec3 windForce = windDir * (0.5f + 0.5f * noise);
                
                // Produto escalar para aplicar vento baseado na orientação da face
                p.addForce(windForce); 
            }
        }

        // 2. Passo de Tempo (Verlet)
        for (auto& p : particles) {
            p.timeStep(dt);
        }

        // 3. Satisfazer Restrições (Relaxamento)
        // Executar várias vezes para deixar o tecido mais rígido
        for (int i = 0; i < 5; i++) {
            for (auto& c : constraints) {
                c.satisfy();
            }
        }
        
        calculateNormals();
    }
};

// --- Globais para OpenGL ---
unsigned int VAO, VBO, EBO, textureID;
Cloth* cloth;
bool windEnabled = true;

// Helper para gerar textura xadrez se não houver imagem
unsigned int createCheckerboardTexture() {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    const int w = 64, h = 64;
    unsigned char data[w * h * 3];
    for(int i = 0; i < w * h; ++i) {
        int x = i % w;
        int y = i / w;
        bool white = ((x / 8) + (y / 8)) % 2 == 0;
        data[i*3] = white ? 255 : 50;   // R
        data[i*3+1] = white ? 255 : 50; // G
        data[i*3+2] = white ? 255 : 150; // B (Azulado)
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return texture;
}

unsigned int loadTexture(const char* path) {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    // Configurações de wrapping/filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); 
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        std::cout << "Textura carregada: " << path << std::endl;
    } else {
        // std::cout << "Falha ao carregar textura. Usando padrao xadrez." << std::endl
        return createCheckerboardTexture();
    }
    stbi_image_free(data);
    return texture;
}

unsigned int compileShader(unsigned int type, const char* source) {
    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &source, NULL);
    glCompileShader(id);
    
    int success;
    char infoLog[512];
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(id, 512, NULL, infoLog);
        std::cout << "ERRO COMPILACAO SHADER: " << infoLog << std::endl;
    }
    return id;
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    // Controle do Vento (W = Toggle)
    static bool wPressed = false;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        if (!wPressed) {
            windEnabled = !windEnabled;
            std::cout << "Vento: " << (windEnabled ? "LIGADO" : "DESLIGADO") << std::endl;
            wPressed = true;
        }
    } else {
        wPressed = false;
    }

    // Controle Força do Vento (Cima/Baixo)
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        windStrength += 0.2f;
        std::cout << "Forca Vento: " << windStrength << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        windStrength -= 0.2f;
        if (windStrength < 0) windStrength = 0;
        std::cout << "Forca Vento: " << windStrength << std::endl;
    }

    // Controle Camera (Esquerda/Direita/Z/X)
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        cameraAngle -= 0.05f;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        cameraAngle += 0.05f;
    }
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
        cameraRadius -= 0.1f;
        if(cameraRadius < 2.0f) cameraRadius = 2.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        cameraRadius += 0.1f;
    }
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Tecido", NULL, NULL);
    if (window == NULL) {
        std::cout << "Falha ao criar janela GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Falha ao inicializar GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // --- Shader Compilation ---
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // --- Init Cloth ---
    cloth = new Cloth(CLOTH_WIDTH, CLOTH_HEIGHT);
    textureID = loadTexture("brasil.png"); // Tenta carregar textura ou cria xadrez

    // --- Buffers ---
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Tamanho inicial: vertices * (pos + normal + uv) -> 3 + 3 + 2 = 8 floats
    glBufferData(GL_ARRAY_BUFFER, cloth->particles.size() * 8 * sizeof(float), NULL, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cloth->indices.size() * sizeof(unsigned int), cloth->indices.data(), GL_STATIC_DRAW);

    // Pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // TexCoord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // --- Loop ---
    float lastFrame = 0.0f;
    
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Limitar Delta Time para evitar explosão da física em lags
        if (deltaTime > 0.05f) deltaTime = 0.05f;

        processInput(window);

        // Atualizar Física
        // Vento vem de trás (-Z) para frente (+Z) com força variável
        cloth->update(deltaTime, glm::vec3(0.0f, 0.0f, windStrength), windEnabled);

        // Renderização
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

        // --- Camera Orbit ---
        float camX = sin(cameraAngle) * cameraRadius;
        float camZ = cos(cameraAngle) * cameraRadius;
        
        // Centralizar o foco verticalmente no meio do tecido
        // O tecido vai de Y=0 (topo) até aprox Y=-6.0 (base). Centro ~ -3.0
        glm::vec3 target(0.0f, -3.0f, 0.0f); 
        glm::vec3 camPos = glm::vec3(camX, -3.0f, camZ);

        glm::mat4 view = glm::lookAt(camPos, target, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 model = glm::mat4(1.0f); // Modelo na origem (o tecido já é criado nas coordenadas certas)

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        
        glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), camPos.x, camPos.y + 5.0f, camPos.z); // Luz segue a câmera

        // Atualizar VBO com novos dados das partículas
        std::vector<float> vertexData;
        for (const auto& p : cloth->particles) {
            vertexData.push_back(p.pos.x);
            vertexData.push_back(p.pos.y);
            vertexData.push_back(p.pos.z);
            vertexData.push_back(p.normal.x);
            vertexData.push_back(p.normal.y);
            vertexData.push_back(p.normal.z);
            vertexData.push_back(p.texCoord.x);
            vertexData.push_back(p.texCoord.y);
        }

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertexData.size() * sizeof(float), vertexData.data());

        glBindVertexArray(VAO);
        // Desenhar como triângulos (preenchido)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Mude para GL_LINE para ver a malha
        glDrawElements(GL_TRIANGLES, cloth->indices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    delete cloth;

    glfwTerminate();
    return 0;
}