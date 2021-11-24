#version 330 core

// Atributos de fragmentos recebidos como entrada ("in") pelo Fragment Shader.
// Neste exemplo, este atributo foi gerado pelo rasterizador como a
// interpolação da posição global e a normal de cada vértice, definidas em
// "shader_vertex.glsl" e "main.cpp".
in vec4 position_world;
in vec4 normal;
in vec3 ball_gouraud_color;

// Matrizes computadas no código C++ e enviadas para a GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Identificador que define qual objeto está sendo desenhado no momento
#define BALL 0
#define FLOOR 1
#define WALL 2
#define PURPLE_CAR 3
#define ORANGE_CAR 4
uniform int object_id;

// O valor de saída ("out") de um Fragment Shader é a cor final do fragmento.
out vec3 color;

uniform sampler2D floor_color;
uniform sampler2D walls_color;

void main()
{
    // Obtemos a posição da câmera utilizando a inversa da matriz que define o
    // sistema de coordenadas da câmera.
    vec4 origin = vec4(0, 0, 0, 1);
    vec4 camera_position = inverse(view) * origin;

    // O fragmento atual é coberto por um ponto que percente à superfície de um
    // dos objetos virtuais da cena. Este ponto, p, possui uma posição no
    // sistema de coordenadas global (World coordinates). Esta posição é obtida
    // através da interpolação, feita pelo rasterizador, da posição de cada
    // vértice.
    vec4 p = position_world;

    // Normal do fragmento atual, interpolada pelo rasterizador a partir das
    // normais de cada vértice.
    vec4 n = normalize(normal);

    // Vetor que define o sentido da fonte de luz em relação ao ponto atual.
    vec4 l = normalize(vec4(1, 1, 0.5, 0));

    // Vetor que define o sentido da câmera em relação ao ponto atual.
    vec4 v = normalize(camera_position - p);

    // Vetor que define o sentido da reflexão especular ideal.
    vec4 r = 2*n*dot(n,l) - l; // PREENCHA AQUI o vetor de reflexão especular ideal

    // Parâmetros que definem as propriedades espectrais da superfície
    vec3 Kd; // Refletância difusa
    vec3 Ks; // Refletância especular
    vec3 Ka; // Refletância ambiente
    float q; // Expoente especular para o modelo de iluminação de Phong

    
    if (object_id == BALL) // Define a cor da bola
    {
        Kd = vec3(1, 1, 1);
        Ks = vec3(0.15, 0.15, 0.15);
        Ka = Kd/2;
        q = 50;
    }
    else if (object_id == FLOOR) // Define a cor do chão
    {
        Kd = texture(floor_color, vec2(p.x, p.z) / 20).rgb;
        Ks = vec3(0, 0, 0);
        Ka = Kd/4;
        q = 1;
    }
    else if (object_id == WALL) // Define a cor da parede
    {
        Kd = texture(walls_color, vec2(p.x + p.z, p.y) / 20).rgb;
        Ks = vec3(0.01, 0.01, 0.01);
        Ka = Kd/4;
        q = 1;
    }
    else if (object_id == PURPLE_CAR) //  Define a cor do carro roxo 
    {
        Kd = vec3(0.29, 0, 0.51);
        Ks = vec3(0.2, 0.2, 0.2);
        Ka = Kd/4;
        q = 40;
    }
    else if (object_id == ORANGE_CAR) //  Define a cor do carro laranja
    {
        Kd = vec3(1, 0.3, 0);
        Ks = vec3(0.2, 0.2, 0.2);
        Ka = Kd/4;
        q = 40;
    }
    else // Objeto desconhecido = preto
    {
        Kd = vec3(0, 0, 0);
        Ks = vec3(0, 0, 0);
        Ka = Kd/2;
        q = 1;
    }

    // Espectro da fonte de iluminação
    vec3 I = vec3(0.75, 0.75, 0.75); // PREENCH AQUI o espectro da fonte de luz

    // Espectro da luz ambiente
    vec3 Ia = vec3(0.2, 0.2, 0.2); // PREENCHA AQUI o espectro da luz ambiente

    // Termo difuso utilizando a lei dos cossenos de Lambert
    vec3 lambert_diffuse_term = Kd*I*max(0, dot(n,l)); // PREENCHA AQUI o termo difuso de Lambert

    // Termo ambiente
    vec3 ambient_term = Ka*Ia; // PREENCHA AQUI o termo ambiente

    // Termo especular utilizando o modelo de iluminação de Blinn-Phong
    vec3 blinn_phong_specular_term  = Ks * I * pow(max(0, dot(normalize(v + l), n)), q);

    // Cor final do fragmento calculada com uma combinação dos termos difuso,
    // especular, e ambiente. Veja slide 129 do documento Aula_17_e_18_Modelos_de_Iluminacao.pdf.
    color = lambert_diffuse_term + ambient_term + blinn_phong_specular_term;

    // Define a cor da bola, utilizando ~Gurá, Guaraná?~ ah, Gouraud
    if (object_id == BALL)
    {
        color = ball_gouraud_color;
    }

    // Cor final com correção gamma, considerando monitor sRGB.
    // Veja https://en.wikipedia.org/w/index.php?title=Gamma_correction&oldid=751281772#Windows.2C_Mac.2C_sRGB_and_TV.2Fvideo_standard_gammas
    color = pow(color, vec3(1, 1, 1) / 2.2);
} 

