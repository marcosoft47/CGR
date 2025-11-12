// g++ praca.cpp -lGLFW -lglad -o praca
// #include <GL/glut.h>  // Header File For The GLUT Library 
// #include <GL/gl.h>	  // Header File For The OpenGL32 Library
// #include <GL/glu.h>	  // Header File For The GLu32 Library
#include "glad/glad.h" 
#include "GLFW/glfw3.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // glm::perspective; glm::lookAt
#include <glm/gtc/type_ptr.hpp>         // glm::value_ptr
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define BENCH_PATH "banco.obj"
#define TEXTURE_BRICK   0
#define TEXTURE_FLOOR   1
#define TEXTURE_CEILING 2
#define TEXTURE_COUNT   3
GLuint  textures[TEXTURE_COUNT];
const char *szTextureFiles[TEXTURE_COUNT] = { "brick.bmp", "floor.bmp", "ceiling.bmp" };

glm::mat4 projectionMatrix;
glm::mat4 viewMatrix;
glm::mat4 modelMatrix;

GLuint planeVAO;
GLuint planeVBO;
GLuint shaderProgramID;

GLint modelLoc;
GLint viewLoc;
GLint projLoc;
GLint colorLoc;

// read shader file and return as a string
std::string readShaderFile(const char* filePath) {
    std::ifstream shaderFile;
    std::stringstream shaderStream;

    shaderFile.open(filePath);
    if (!shaderFile.is_open()) {
        std::cerr << "Erro: Nao foi possivel abrir o arquivo de shader: " << filePath << std::endl;
        return "";
    }
    
    shaderStream << shaderFile.rdbuf();
    shaderFile.close();
    
    return shaderStream.str();
}

// Do shader bullshit and link files
GLuint createShaderProgram(const char* vertPath, const char* fragPath) {
    // 1. read shader files
    std::string vertSource = readShaderFile(vertPath);
    std::string fragSource = readShaderFile(fragPath);
    
    if (vertSource.empty() || fragSource.empty()) {
        return 0;
    }

    const char* vShaderCode = vertSource.c_str();
    const char* fShaderCode = fragSource.c_str();

    GLuint vertexShader, fragmentShader;
    int success;
    char infoLog[512];

    // 2. Compile Vertex Shader
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vShaderCode, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERRO::SHADER::VERTEX::COMPILACAO_FALHOU\n" << infoLog << std::endl;
        return 0;
    }

    // 3. Compile Fragment Shader
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERRO::SHADER::FRAGMENT::COMPILACAO_FALHOU\n" << infoLog << std::endl;
        return 0;
    }

    // 4. Link shaders
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERRO::SHADER::PROGRAM::LINKAGEM_FALHOU\n" << infoLog << std::endl;
        return 0;
    }

    // 5. Free files
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    std::cout << "Shaders compilados e linkados com sucesso!" << std::endl;
    
    return shaderProgram; // returns shader program ID
}
// Change viewing volume and viewport. Called when window is resized
void ChangeSize(GLFWwindow* window, int w, int h)
    {
    GLfloat fAspect;

    // Prevent a divide by zero
    if(h == 0)
        h = 1;

    // Set Viewport to window dimensions
    glViewport(0, 0, w, h);

    fAspect = (GLfloat)w/(GLfloat)h;

    projectionMatrix = glm::perspective(
        glm::radians(90.0f),    // FOV
        fAspect,                // Aspect Ratio
        1.0f,                   // near plane
        120.0f                  // far plane
    );
    }

void setMatrices(){
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f); // Câmera 5 unidades "para fora"
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f); // Olhando para a origem
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);   // "Cima" é o Y positivo

    viewMatrix = glm::lookAt(cameraPos, cameraTarget, cameraUp);
    modelMatrix = glm::mat4(1.0f);
}
    // Deal with a bunch of tinyobj bullshit
bool loadModel(const char* filepath, std::vector<float>& openglVertices) {
    
    // 2. Estruturas de dados do tinyobj
    tinyobj::attrib_t attrib; // Guarda todos os vértices, normais e UVs
    std::vector<tinyobj::shape_t> shapes; // Guarda os "objetos" ou "partes" do modelo
    std::vector<tinyobj::material_t> materials; // (Não usaremos agora)
    
    std::string warn;
    std::string err;

    // 2. Actually load the object
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath);

    // Check for errors
    if (!warn.empty()) {
        std::cout << "AVISO (tinyobj): " << warn << std::endl;
    }
    if (!err.empty()) {
        std::cerr << "ERRO (tinyobj): " << err << std::endl;
        return false;
    }
    if (!ret) {
        return false;
    }

    std::cout << "Modelo carregado com sucesso!" << std::endl;
    std::cout << "  Vértices: " << (attrib.vertices.size() / 3) << std::endl;
    std::cout << "  Normais:  " << (attrib.normals.size() / 3) << std::endl;
    std::cout << "  UVs:      " << (attrib.texcoords.size() / 2) << std::endl;
    std::cout << "  Formas:   " << shapes.size() << std::endl;

    // Turn the vertex into a VBO
    
    openglVertices.clear();

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            // Posição do vértice (x, y, z)
            float posX = attrib.vertices[3 * index.vertex_index + 0];
            float posY = attrib.vertices[3 * index.vertex_index + 1];
            float posZ = attrib.vertices[3 * index.vertex_index + 2];

            // Normal do vértice (Nx, Ny, Nz)
            // (Verifique se as normais existem)
            float normX = 0, normY = 0, normZ = 0;
            if (index.normal_index >= 0) {
                normX = attrib.normals[3 * index.normal_index + 0];
                normY = attrib.normals[3 * index.normal_index + 1];
                normZ = attrib.normals[3 * index.normal_index + 2];
            }

            // Coordenada de textura (U, V)
            // (Verifique se as UVs existem)
            float texU = 0, texV = 0;
            if (index.texcoord_index >= 0) {
                texU = attrib.texcoords[2 * index.texcoord_index + 0];
                texV = attrib.texcoords[2 * index.texcoord_index + 1];
            }

            // Adiciona os dados intercalados ao nosso vetor
            openglVertices.push_back(posX);
            openglVertices.push_back(posY);
            openglVertices.push_back(posZ);
            
            openglVertices.push_back(normX);
            openglVertices.push_back(normY);
            openglVertices.push_back(normZ);
            
            openglVertices.push_back(texU);
            openglVertices.push_back(texV);
        }
    }

    return true;
}

void setPlane(GLfloat x, GLfloat y, GLfloat z){
    // Formato: [Pos_x, Pos_y, Pos_z,  UV_u, UV_v]
    float planeVertices[] = {
        // Triângulo 1
        -x, y,  z,   0.0f, 0.0f, // Canto 1
        x, y,  z,   1.0f, 0.0f, // Canto 2
        x, y, -z,   1.0f, 1.0f, // Canto 3
        // Triângulo 2
        x, y, -z,   1.0f, 1.0f, // Canto 3
        -x, y, -z,   0.0f, 1.0f, // Canto 4
        -x, y,  z,   0.0f, 0.0f  // Canto 1
    };

    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);

    glBindVertexArray(planeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

    int stride = 5 * sizeof(float);

    // Atributo 0: Posição (layout = 0 no shader)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    // Atributo 2: Coordenadas de Textura (layout = 2 no shader)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void renderPlane(GLFWwindow* window){
    glUniform3f(colorLoc, 0.0f, 1.0f, 0.0f);
    glm::mat4 planeModel = glm::mat4(1.0f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(planeModel));
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
// void renderBench(){
//     std::vector<float> myModelData;
//     if (loadModel("meu_objeto.obj", myModelData)) {
        
//         // C. Próximo Passo (OpenGL):
//         // Agora 'myModelData' está pronto para a GPU.
        
//         // 1. Gerar VBO e VAO
//         GLuint vao, vbo;
//         glGenVertexArrays(1, &vao);
//         glGenBuffers(1, &vbo);
        
//         // 2. Enviar os dados
//         glBindVertexArray(vao);
//         glBindBuffer(GL_ARRAY_BUFFER, vbo);
//         glBufferData(GL_ARRAY_BUFFER, myModelData.size() * sizeof(float), myModelData.data(), GL_STATIC_DRAW);
        
//         // 3. Configurar Atributos (crucial!)
//         // Stride (passo) total é 8 floats (3 pos + 3 norm + 2 uv)
//         int stride = 8 * sizeof(float);
        
//         // Atributo 0: Posição (layout = 0)
//         glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
//         glEnableVertexAttribArray(0);
        
//         // Atributo 1: Normais (layout = 1)
//         glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
//         glEnableVertexAttribArray(1);

//         // Atributo 2: UVs (layout = 2)
//         glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
//         glEnableVertexAttribArray(2);
//     }
// }

// Respond to arrow keys, move the viewpoint back and forth
void readKeys(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
	// if(key == GLUT_KEY_UP)
	// 	zPos += 1.0f;

	// if(key == GLUT_KEY_DOWN)
	// 	zPos -= 1.0f;

	// Refresh the Window
	}


    // Program entry point
int main(int argc, char *argv[]){

    if (!glfwInit()) {
        std::cerr << "Falha ao inicializar GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Praca", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Falha ao criar janela GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Falha ao inicializar o GLAD" << std::endl;
        return -1;
    }

    // Deal with shader
    shaderProgramID = createShaderProgram("shader.vert", "shader.frag");
    if (shaderProgramID == 0) {
        std::cerr << "Falha ao criar shaders. Saindo." << std::endl;
        glfwTerminate();
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glfwSetFramebufferSizeCallback(window, ChangeSize); 
    glfwSetKeyCallback(window, readKeys);

    setPlane(10.0f, -0.5f, 5.0f);

    modelLoc = glGetUniformLocation(shaderProgramID, "model");
    viewLoc = glGetUniformLocation(shaderProgramID, "view");
    projLoc = glGetUniformLocation(shaderProgramID, "projection");
    colorLoc = glGetUniformLocation(shaderProgramID, "ourColor");

    // Mainloop
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgramID);
        setMatrices();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));


        
        // renderScene(window);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
    }