#version 330 core

// -----------------------------------------------------------------
// ESTES 'layout (location = X)' DEVEM BATER EXATAMENTE
// com os seus 'glVertexAttribPointer'
// -----------------------------------------------------------------
layout (location = 0) in vec3 aPos;     // Posição do Vértice
layout (location = 1) in vec3 aNormal;  // Normal do Vértice (para iluminação)
layout (location = 2) in vec2 aTexCoord; // Coordenada de Textura (UV)

// Saídas para o Fragment Shader
out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos; // Posição no "mundo" (para iluminação)

// Matrizes que vêm do seu C++ (substitutas do glMatrixMode)
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Calcula a posição final na tela
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    
    // Passa os dados para o fragment shader
    TexCoord = aTexCoord;
    
    // Cálculos para iluminação
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
}