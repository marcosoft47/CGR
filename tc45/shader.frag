#version 330 core

// Saída final (a cor do pixel)
out vec4 FragColor;

// Entradas vindas do Vertex Shader
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

// Uniforms que vêm do seu C++
uniform sampler2D ourTexture; // A textura (brick.bmp, etc.)
uniform vec3 ourColor;        // O substituto do glColor3f()

void main()
{
    // Pega a cor da textura na coordenada UV
    vec4 texColor = texture(ourTexture, TexCoord);
    
    // Multiplica a cor da textura pela "cor de base" (o seu glColor3f)
    FragColor = texColor * vec4(ourColor, 1.0);
    
    // Se a textura não carregar, ela fica preta. 
    // Para depurar, você pode fazer:
    // FragColor = vec4(1.0, 0.0, 1.0, 1.0); // Pinta tudo de rosa-choque
}