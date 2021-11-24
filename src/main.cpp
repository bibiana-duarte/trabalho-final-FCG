//     Universidade Federal do Rio Grande do Sul
//             Instituto de Informática
//       Departamento de Informática Aplicada
//
//    INF01047 Fundamentos de Computação Gráfica
//               Prof. Eduardo Gastal
//
//             Bibiana Duarte - 00297635
//                 Trabalho Final
//
// Arquivos "headers" padrões de C podem ser incluídos em um
// programa C++, sendo necessário somente adicionar o caractere
// "c" antes de seu nome, e remover o sufixo ".h". Exemplo:
//    #include <stdio.h> // Em C
//  vira
//    #include <cstdio> // Em C++
//
#include <cmath>
#include <cstdio>
#include <cstdlib>

// Headers abaixo são específicos de C++
#include <map>
#include <stack>
#include <string>
#include <vector>
#include <limits>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <iostream>

// Headers das bibliotecas OpenGL
#include "../include/glad/glad.h"   // Criação de contexto OpenGL 3.3
#include "../include/GLFW/glfw3.h"  // Criação de janelas do sistema operacional

// Headers da biblioteca GLM: criação de matrizes e vetores.
#include "../include/glm/mat4x4.hpp"
#include "../include/glm/vec4.hpp"
#include "../include/glm/gtc/type_ptr.hpp"

// Headers da biblioteca para carregar modelos obj
#include "../include/tiny_obj_loader.h"

// Headers locais, definidos na pasta "include/"
#include "../include/utils.h"
#include "../include/matrices.h"
#include "../include/stb_image.h"

#include "../include/constants.hpp"
#include "../include/collisions.hpp"

// Estrutura que representa um modelo geométrico carregado a partir de um
// arquivo ".obj". Veja https://en.wikipedia.org/wiki/Wavefront_.obj_file .
struct ObjModel
{
    tinyobj::attrib_t                 attrib;
    std::vector<tinyobj::shape_t>     shapes;
    std::vector<tinyobj::material_t>  materials;

    // Este construtor lê o modelo de um arquivo utilizando a biblioteca tinyobjloader.
    // Veja: https://github.com/syoyo/tinyobjloader
    ObjModel(const char* filename, const char* basepath = NULL, bool triangulate = true)
    {
        printf("Carregando modelo \"%s\"... ", filename);

        std::string err;
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename, basepath, triangulate);

        if (!err.empty())
            fprintf(stderr, "\n%s\n", err.c_str());

        if (!ret)
            throw std::runtime_error("Erro ao carregar modelo.");

        printf("OK.\n");
    }
};


// Declaração de funções utilizadas para pilha de matrizes de modelagem.
void PushMatrix(glm::mat4 M);
void PopMatrix(glm::mat4& M);

// Declaração de várias funções utilizadas em main().  Essas estão definidas
// logo após a definição de main() neste arquivo.
void BuildTrianglesAndAddToVirtualScene(ObjModel*); // Constrói representação de um ObjModel como malha de triângulos para renderização
void ComputeNormals(ObjModel* model); // Computa normais de um ObjModel, caso não existam.
void LoadShadersFromFiles(); // Carrega os shaders de vértice e fragmento, criando um programa de GPU
void DrawVirtualObject(const char* object_name); // Desenha um objeto armazenado em g_VirtualScene
GLuint LoadShader_Vertex(const char* filename);   // Carrega um vertex shader
GLuint LoadShader_Fragment(const char* filename); // Carrega um fragment shader
void LoadShader(const char* filename, GLuint shader_id); // Função utilizada pelas duas acima
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id); // Cria um programa de GPU
void PrintObjModelInfo(ObjModel*); // Função para debugging

// Declaração de funções auxiliares para renderizar texto dentro da janela
// OpenGL. Estas funções estão definidas no arquivo "textrendering.cpp".
void TextRendering_Init();
float TextRendering_LineHeight(GLFWwindow* window);
float TextRendering_CharWidth(GLFWwindow* window);
void TextRendering_PrintString(GLFWwindow* window, const std::string &str, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrix(GLFWwindow* window, glm::mat4 M, float x, float y, float scale = 1.0f);
void TextRendering_PrintVector(GLFWwindow* window, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProduct(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductMoreDigits(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductDivW(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);

// Funções abaixo renderizam como texto na janela OpenGL algumas matrizes e
// outras informações do programa. Definidas após main().
void TextRendering_ShowModelViewProjection(GLFWwindow* window, glm::mat4 projection, glm::mat4 view, glm::mat4 model, glm::vec4 p_model);
void TextRendering_ShowProjection(GLFWwindow* window);
void TextRendering_ShowFramesPerSecond(GLFWwindow* window);

// Funções callback para comunicação com o sistema operacional e interação do
// usuário. Veja mais comentários nas definições das mesmas, abaixo.
void ErrorCallback(int error, const char* description);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);

void LoadTextureImage(const char* filename);

// Definimos uma estrutura que armazenará dados necessários para renderizar
// cada objeto da cena virtual.
struct SceneObject
{
    std::string  name;        // Nome do objeto
    size_t       first_index; // Índice do primeiro vértice dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    size_t       num_indices; // Número de índices do objeto dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    GLenum       rendering_mode; // Modo de rasterização (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.)
    GLuint       vertex_array_object_id; // ID do VAO onde estão armazenados os atributos do modelo
};

// Abaixo definimos variáveis globais utilizadas em várias funções do código.

// A cena virtual é uma lista de objetos nomeados, guardados em um dicionário
// (map).  Veja dentro da função BuildTrianglesAndAddToVirtualScene() como que são incluídos
// objetos dentro da variável g_VirtualScene, e veja na função main() como
// estes são acessados.
std::map<std::string, SceneObject> g_VirtualScene;

// Pilha que guardará as matrizes de modelagem.
std::stack<glm::mat4>  g_MatrixStack;

// Razão de proporção da janela (largura/altura). Veja função FramebufferSizeCallback().
float g_ScreenRatio = 1.0f;

// Variáveis que definem a câmera em coordenadas esféricas, controladas pelo
// usuário através do mouse (veja função CursorPosCallback()). A posição
// efetiva da câmera é calculada dentro da função main(), dentro do loop de
// renderização.
float g_CameraDistance = 3.5f; // Distância da câmera para a origem

// Variável que controla se o texto informativo será mostrado na tela.
bool g_ShowInfoText = true;

// Variáveis que definem um programa de GPU (shaders). Veja função LoadShadersFromFiles().
GLuint vertex_shader_id;
GLuint fragment_shader_id;
GLuint program_id = 0;
GLint model_uniform;
GLint view_uniform;
GLint projection_uniform;
GLint object_id_uniform;

GLuint g_NumLoadedTextures = 0;

 // Variaveis de posição
glm::vec4 purple_car_position;
glm::vec4 orange_car_position;
glm::vec4 ball_position;

// Variaveis de velocidade
glm::vec4 purple_car_speed = ZERO;
glm::vec4 orange_car_speed = ZERO;
glm::vec4 ball_speed = ZERO;

// Inicialização da direção dos carros
glm::vec4 purple_car_direction = NORTH;
glm::vec4 orange_car_direction = SOUTH;

GLfloat purple_car_direction_angle;
GLfloat orange_car_direction_angle;

 
GLboolean is_purple_car_looking_at_ball = false;
GLboolean is_orange_car_looking_at_ball = false;

// Declaração e inicialização das variaveis de tempo
GLfloat last_frame_time;
GLfloat current_frame_time = glfwGetTime();
GLfloat time_between_frames;

GLboolean is_purple_car_moving_left = false;
GLboolean is_purple_car_moving_right = false;
GLboolean is_purple_car_moving_front = false;
GLboolean is_purple_car_moving_back = false;
GLboolean is_purple_camera_looking_back = false;

GLboolean is_orange_car_moving_left = false;
GLboolean is_orange_car_moving_right = false;
GLboolean is_orange_car_moving_front = false;
GLboolean is_orange_car_moving_back = false;
GLboolean is_orange_camera_looking_back = false;

GLboolean is_ball_returning = false;
GLfloat ball_returning_progress;
glm::vec4 ball_returning_point_1;
glm::vec4 ball_returning_point_2;
glm::vec4 ball_returning_point_3;
glm::vec4 ball_returning_point_4;

int main(int argc, char* argv[])
{
    // Inicializamos a biblioteca GLFW, utilizada para criar uma janela do
    // sistema operacional, onde poderemos renderizar com OpenGL.
    int success = glfwInit();
    if (!success)
    {
        fprintf(stderr, "ERROR: glfwInit() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos o callback para impressão de erros da GLFW no terminal
    glfwSetErrorCallback(ErrorCallback);

    // Pedimos para utilizar OpenGL versão 3.3 (ou superior)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // Pedimos para utilizar o perfil "core", isto é, utilizaremos somente as
    // funções modernas de OpenGL.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Criamos uma janela do sistema operacional, com 800 colunas e 600 linhas
    // de pixels, e com título "INF01047 ...".
    GLFWwindow* window;
    window = glfwCreateWindow(1920, 1080, "Carritos", glfwGetPrimaryMonitor(), NULL);
    g_ScreenRatio = 1920.0f / 1080.0f / 2;
    if (!window)
    {
        glfwTerminate();
        fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos a função de callback que será chamada sempre que o usuário
    // pressionar alguma tecla do teclado ...
    glfwSetKeyCallback(window, KeyCallback);

    // Indicamos que as chamadas OpenGL deverão renderizar nesta janela
    glfwMakeContextCurrent(window);

    // Carregamento de todas funções definidas por OpenGL 3.3, utilizando a
    // biblioteca GLAD.
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    // Imprimimos no terminal informações sobre a GPU do sistema
    const GLubyte *vendor      = glGetString(GL_VENDOR);
    const GLubyte *renderer    = glGetString(GL_RENDERER);
    const GLubyte *glversion   = glGetString(GL_VERSION);
    const GLubyte *glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);

    printf("GPU: %s, %s, OpenGL %s, GLSL %s\n", vendor, renderer, glversion, glslversion);

    // Carregamos os shaders de vértices e de fragmentos que serão utilizados
    // para renderização. Veja slides 176-196 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
    //
    LoadShadersFromFiles();

    LoadTextureImage("../../data/grass.jpg");
    LoadTextureImage("../../data/wall.jpg");

    // Construímos a representação de objetos geométricos através de malhas de triângulos
    ObjModel spheremodel("../../data/sphere.obj");
    ComputeNormals(&spheremodel);
    BuildTrianglesAndAddToVirtualScene(&spheremodel);

    ObjModel planemodel("../../data/plane.obj");
    ComputeNormals(&planemodel);
    BuildTrianglesAndAddToVirtualScene(&planemodel);

    ObjModel carritomodel("../../data/carrito.obj");
    ComputeNormals(&carritomodel);
    BuildTrianglesAndAddToVirtualScene(&carritomodel);

    if (argc > 1)
    {
        ObjModel model(argv[1]);
        BuildTrianglesAndAddToVirtualScene(&model);
    }

    // Inicializamos o código para renderização de texto.
    TextRendering_Init();

    // Habilitamos o Z-buffer. Veja slides 104-116 do documento Aula_09_Projecoes.pdf.
    glEnable(GL_DEPTH_TEST);

    // Habilitamos o Backface Culling. Veja slides 23-34 do documento Aula_13_Clipping_and_Culling.pdf.
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Variáveis auxiliares utilizadas para chamada à função
    // TextRendering_ShowModelViewProjection(), armazenando matrizes 4x4.
    glm::mat4 the_projection;
    glm::mat4 the_model;
    glm::mat4 the_view;

    // Inicialização da posição da bola e dos carros
    ball_position = glm::vec4(0, BALL_RADIUS, 0, 1); 
    purple_car_position = glm::vec4(0, CAR_HEIGHT / 2, CAR_TO_BALL_INITIAL_DISTANCE, 1);
    orange_car_position = glm::vec4(0, CAR_HEIGHT / 2, -CAR_TO_BALL_INITIAL_DISTANCE, 1);

    // Ficamos em loop, renderizando, até que o usuário feche a janela
    while (!glfwWindowShouldClose(window))
    {
        // Atualização do tempo passado entre os frames
        last_frame_time = current_frame_time;
        current_frame_time = glfwGetTime();
        time_between_frames = current_frame_time - last_frame_time;

        // Aqui executamos as operações de renderização

        // Definição da cor do céu 
        //           R     G     B      A
        glClearColor(0.6f, 0.8f, 0.95f, 1.0f);

        // "Pintamos" todos os pixels do framebuffer com a cor definida acima,
        // e também resetamos todos os pixels do Z-buffer (depth buffer).
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Pedimos para a GPU utilizar o programa de GPU criado acima (contendo
        // os shaders de vértice e fragmentos).
        glUseProgram(program_id);
        
        // As texturas são passadas para a GPU
        glUniform1i(glGetUniformLocation(program_id, "floor_color"), 0);
        glUniform1i(glGetUniformLocation(program_id, "walls_color"), 1);


        // Funções de atrito da bola e dos carros, se o carro estão de lado o atrito é maior 
        if (norm(purple_car_speed) > 0)
        {
            purple_car_speed *= std::pow(0.6, time_between_frames);
            purple_car_speed *= std::pow(0.02, time_between_frames * (1 - abs(purple_car_direction.x * purple_car_speed.x + purple_car_direction.z * purple_car_speed.z) / norm(purple_car_speed)));
        }
        if (norm(orange_car_speed) > 0)
        {
            orange_car_speed *= std::pow(0.6, time_between_frames);
            orange_car_speed *= std::pow(0.02, time_between_frames * (1 - abs(orange_car_direction.x * orange_car_speed.x + orange_car_direction.z * orange_car_speed.z) / norm(orange_car_speed)));
        }
        ball_speed *= std::pow(0.8, time_between_frames);
        
        // Criação de variaveis temporarias para possivel novo valor da posição, velocidade e direção do carro
        glm::vec4 purple_car_new_position = purple_car_position;
        glm::vec4 purple_car_new_speed = purple_car_speed;
        glm::vec4 purple_car_new_direction = purple_car_direction;


        // Atualiza a velocidade do carro, usando o tempo entre os frames. Se o carro está dando ré, a aceleração é menor
        if (is_purple_car_moving_front)
        {
            purple_car_new_speed += 30.0f * time_between_frames * purple_car_direction;
        }
        if (is_purple_car_moving_back)
        {
            purple_car_new_speed -= 20.0f * time_between_frames * purple_car_direction;
        }

        // Atualiza a posição do carro 
        purple_car_new_position += time_between_frames * purple_car_new_speed;

        // Atualiza a direção do carro, quanto maior a velocidade do carro, mais rápido ele gira
        if (is_purple_car_moving_right)
        {
            purple_car_new_direction = Matrix_Rotate_Y(-norm(purple_car_speed) / 8 * time_between_frames) * purple_car_new_direction;
        }
        if (is_purple_car_moving_left)
        {
            purple_car_new_direction = Matrix_Rotate_Y(+norm(purple_car_speed) / 8 * time_between_frames) * purple_car_new_direction;
        }

        // Verifica se há colisão do carro com o cenário, se tiver a velocidade do carro é zerada, senão, a posição, velocidade e direção são definitivamente atualizadas
        if (not is_colliding_car_to_scenario(purple_car_new_position, purple_car_new_direction))
        {
            purple_car_position = purple_car_new_position;
            purple_car_speed = purple_car_new_speed;
            purple_car_direction = purple_car_new_direction;
        }
        else
        {
            purple_car_speed = ZERO;
        }
        
        // Aqui as mesmas funções são implementadas, porém para o carro laranja

        glm::vec4 orange_car_new_position = orange_car_position;
        glm::vec4 orange_car_new_speed = orange_car_speed;
        glm::vec4 orange_car_new_direction = orange_car_direction;

        if (is_orange_car_moving_front)
        {
            orange_car_new_speed += 30.0f * time_between_frames * orange_car_direction;
        }
        if (is_orange_car_moving_back)
        {
            orange_car_new_speed -= 20.0f * time_between_frames * orange_car_direction;
        }

        orange_car_new_position += time_between_frames * orange_car_new_speed;

        if (is_orange_car_moving_right)
        {
            orange_car_new_direction = Matrix_Rotate_Y(-norm(orange_car_speed) / 8 * time_between_frames) * orange_car_new_direction;
        }
        if (is_orange_car_moving_left)
        {
            orange_car_new_direction = Matrix_Rotate_Y(+norm(orange_car_speed) / 8 * time_between_frames) * orange_car_new_direction;
        }

        if (not is_colliding_car_to_scenario(orange_car_new_position, orange_car_new_direction))
        {
            orange_car_position = orange_car_new_position;
            orange_car_speed = orange_car_new_speed;
            orange_car_direction = orange_car_new_direction;
        }
        else
        {
            orange_car_speed = ZERO;
        }

        // Define o angulo da direção
        purple_car_direction_angle = atan2(-purple_car_direction.x, -purple_car_direction.z);
        orange_car_direction_angle = atan2(-orange_car_direction.x, -orange_car_direction.z);

        // Funções de movimentação da bola
        // Se a bola está voltando do gol ou não
        if (is_ball_returning)
        {
            //  Se a bola está voltando do gol, é usado curvas de Bezier para fazer a movimentação da bola de volta ao centro. 
            ball_returning_progress += time_between_frames / 6;

            if (ball_returning_progress >= 1)
            {
                ball_returning_progress = 1;
                is_ball_returning = false;
                ball_speed = ZERO;
            }

            float p = ball_returning_progress * ball_returning_progress * ball_returning_progress;
            float q = 1 - p;

            ball_position = 
                1 * q*q*q * ball_returning_point_1 +
                3 * p*q*q * ball_returning_point_2 +
                3 * p*p*q * ball_returning_point_3 +
                1 * p*p*p * ball_returning_point_4;
            ball_position.w = 1;
        }
        else
        {
            // A possivel nova posição da bola é salva em uma variavel temporaria
            glm::vec4 ball_new_position = ball_position;

            ball_new_position += time_between_frames * ball_speed;

            // Se a bola colide com algum dos carros, ela vai na direção contrária do carro, e ambos, a bola e o carro, perdem velocidade, mas a bola ganha velocidade do impacto (quanto mais velocidade o carro possuia, mais veloz é o "retorno" da bola)
            if (is_colliding_ball_to_car(ball_new_position, purple_car_position, purple_car_direction))
            {
                ball_speed = (1.3f * norm(purple_car_speed) + norm(ball_speed) / 1.3f) * (ball_new_position - purple_car_position) / norm(ball_new_position - purple_car_position);
                ball_speed.y = 0;
                purple_car_speed /= 1.5f;
            }
            if (is_colliding_ball_to_car(ball_new_position, orange_car_position, orange_car_direction))
            {
                ball_speed = (1.3f * norm(orange_car_speed) + norm(ball_speed) / 1.3f) * (ball_new_position - orange_car_position) / norm(ball_new_position - orange_car_position);
                ball_speed.y = 0;
                orange_car_speed /= 1.5f;
            }

            // Se a bola colide com alguma parede, ela reflete 
            if (is_colliding_ball_to_north_wall(ball_new_position) or is_colliding_ball_to_south_wall(ball_new_position))
            {
                ball_speed.z = -ball_speed.z;
            }
            if (is_colliding_ball_to_east_wall(ball_new_position) or is_colliding_ball_to_west_wall(ball_new_position))
            {
                ball_speed.x = -ball_speed.x;
            }

            // Verica se a bola saiu do cenário, se sim, ela cai
            if (abs(ball_new_position.z) > FIELD_LENGTH / 2)
            {
                ball_speed.y -= std::pow(9.8f, time_between_frames); 
            }

            ball_position = ball_new_position;

            // Se a bola está abaixo de -5 metros, ela retorna ao centro
            if (ball_position.y <= -5)
            {
                is_ball_returning = true;
                ball_returning_progress = 0;

                if (ball_position.z > 0)
                {
                    ball_returning_point_1 = ball_position;
                    ball_returning_point_2 = glm::vec4(0, FIELD_HEIGHT + 5, FIELD_LENGTH / 2 + 10, 1);
                    ball_returning_point_3 = glm::vec4(0, FIELD_HEIGHT + 12, FIELD_LENGTH / 2, 1);
                    ball_returning_point_4 = glm::vec4(0, BALL_RADIUS, 0, 1);
                }
                else
                {
                    ball_returning_point_1 = ball_position;
                    ball_returning_point_2 = glm::vec4(0, FIELD_HEIGHT + 5, -FIELD_LENGTH / 2 - 10, 1);
                    ball_returning_point_3 = glm::vec4(0, FIELD_HEIGHT + 12, -FIELD_LENGTH / 2, 1);
                    ball_returning_point_4 = glm::vec4(0, BALL_RADIUS, 0, 1);
                }
            }
        }

        // Abaixo definimos as varáveis que efetivamente definem a câmera virtual.
        // Veja slides 195-227 e 229-234 do documento Aula_08_Sistemas_de_Coordenadas.pdf.
        glm::vec4 car_position;
        GLfloat car_direction_angle;
        GLboolean is_camera_looking_back;
        glm::vec4 camera_offset_to_car;
        glm::vec4 camera_position;
        glm::vec4 camera_view_vector;
        glm::vec4 camera_up_vector;
        glm::mat4 view;
        float field_of_view;
        glm::mat4 projection;

        // Note que, no sistema de coordenadas da câmera, os planos near e far
        // estão no sentido negativo! Veja slides 176-204 do documento Aula_09_Projecoes.pdf.
        float nearplane = -0.1f;  // Posição do "near plane"
        float farplane  = FIELD_WIDTH + FIELD_LENGTH + FIELD_HEIGHT; // Posição do "far plane"

        glm::mat4 model = Matrix_Identity(); // Transformação identidade de modelagem

        
        car_position = purple_car_position;
        car_direction_angle = purple_car_direction_angle;
        is_camera_looking_back = is_purple_camera_looking_back;

        // Definição da tela do carro roxo
        glViewport(0, 0, 1920/2, 1080);

        //  Posicionamento da cãmera, se o jogador estiver olhando para bola, a câmera á posicionada 1.8m acima do carro, caso contrário, a câmera é posicionada acima do carro ,e , se estiver olhando para a frente, um pouco para trás dele, caso o contrário, um pouco a frente dele 
        if (is_purple_car_looking_at_ball)
        {
            camera_offset_to_car = glm::vec4(0, 1.8, 0, 1);
        }
        else
        {
            camera_offset_to_car = glm::vec4(0, 1.8, (is_purple_camera_looking_back? -7 : 7), 1);
        }
        camera_position = Matrix_Translate(car_position.x, car_position.y, car_position.z) * Matrix_Rotate_Y(car_direction_angle) * camera_offset_to_car;
        if (is_purple_car_looking_at_ball)
        {
            camera_view_vector = ball_position - camera_position; // Câmera Lookat
        }
        else
        {
            camera_view_vector = Matrix_Rotate_Y((is_camera_looking_back? PI : 0) + car_direction_angle) * NORTH;
        }
        camera_up_vector = UP;

        view = Matrix_Camera_View(camera_position, camera_view_vector, camera_up_vector);

        // Projeção Perspectiva.
        // Para definição do field of view (FOV), veja slides 205-215 do documento Aula_09_Projecoes.pdf.
        field_of_view = PI * 2/5;
        projection = Matrix_Perspective(field_of_view, g_ScreenRatio, nearplane, farplane);

        // Enviamos as matrizes "view" e "projection" para a placa de vídeo
        // (GPU). Veja o arquivo "shader_vertex.glsl", onde estas são
        // efetivamente aplicadas em todos os pontos.
        glUniformMatrix4fv(view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
        glUniformMatrix4fv(projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));
        
        // Definição de constantes para os objetos 
        #define BALL 0
        #define FLOOR 1
        #define WALL 2
        #define PURPLE_CAR 3
        #define ORANGE_CAR 4

        // Desenho a bola
        model = Matrix_Translate(ball_position.x, ball_position.y, ball_position.z)
                * Matrix_Scale(BALL_DIAMETER / 2, BALL_DIAMETER / 2, BALL_DIAMETER / 2);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, BALL);
        DrawVirtualObject("sphere");

        // Desenho o chão
        model = Matrix_Translate(0, 0, 0)
                * Matrix_Scale(FIELD_WIDTH / 2, 1, FIELD_LENGTH / 2);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, FLOOR);
        DrawVirtualObject("plane");

        // Desenho as paredes laterais
        model = Matrix_Translate(FIELD_WIDTH / 2, FIELD_HEIGHT / 2, 0)
                * Matrix_Rotate_Z(PI / 2)
                * Matrix_Scale(FIELD_HEIGHT / 2, 1, FIELD_LENGTH / 2);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, WALL);
        DrawVirtualObject("plane");

        model = Matrix_Translate(-FIELD_WIDTH / 2, FIELD_HEIGHT / 2, 0)
                * Matrix_Rotate_Z(-PI / 2)
                * Matrix_Scale(FIELD_HEIGHT / 2, 1, FIELD_LENGTH / 2);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, WALL);
        DrawVirtualObject("plane");

        // Desenho as paredes traseiras
        // XXXXXXXXXXXXX
        // XXXXXXXXXXXXX
        // XXXXXXXXXXXXX
        // XXXXX   XXXXX
        // XXXXX   XXXXX

        model = Matrix_Translate(0, GOAL_HEIGHT + (FIELD_HEIGHT - GOAL_HEIGHT) / 2, FIELD_LENGTH / 2)
                * Matrix_Rotate_X(-PI / 2)
                * Matrix_Scale(FIELD_WIDTH / 2, 1, (FIELD_HEIGHT - GOAL_HEIGHT) / 2);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, WALL);
        DrawVirtualObject("plane");

        model = Matrix_Translate(GOAL_WIDTH / 2 + (FIELD_WIDTH - GOAL_WIDTH) / 4, GOAL_HEIGHT / 2, FIELD_LENGTH / 2)
                * Matrix_Rotate_X(-PI / 2)
                * Matrix_Scale((FIELD_WIDTH / 2 - GOAL_WIDTH / 2) / 2, 1, GOAL_HEIGHT / 2);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, WALL);
        DrawVirtualObject("plane");

        model = Matrix_Translate(-(GOAL_WIDTH / 2 + (FIELD_WIDTH - GOAL_WIDTH) / 4), GOAL_HEIGHT / 2, FIELD_LENGTH / 2)
                * Matrix_Rotate_X(-PI / 2)
                * Matrix_Scale((FIELD_WIDTH / 2 - GOAL_WIDTH / 2) / 2, 1, GOAL_HEIGHT / 2);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, WALL);
        DrawVirtualObject("plane");


        model = Matrix_Translate(0, GOAL_HEIGHT + (FIELD_HEIGHT - GOAL_HEIGHT) / 2, -FIELD_LENGTH / 2)
                * Matrix_Rotate_X(PI / 2)
                * Matrix_Scale(FIELD_WIDTH / 2, 1, (FIELD_HEIGHT - GOAL_HEIGHT) / 2);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, WALL);
        DrawVirtualObject("plane");

        model = Matrix_Translate(GOAL_WIDTH / 2 + (FIELD_WIDTH - GOAL_WIDTH) / 4, GOAL_HEIGHT / 2, -FIELD_LENGTH / 2)
                * Matrix_Rotate_X(PI / 2)
                * Matrix_Scale((FIELD_WIDTH / 2 - GOAL_WIDTH / 2) / 2, 1, GOAL_HEIGHT / 2);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, WALL);
        DrawVirtualObject("plane");

        model = Matrix_Translate(-(GOAL_WIDTH / 2 + (FIELD_WIDTH - GOAL_WIDTH) / 4), GOAL_HEIGHT / 2, -FIELD_LENGTH / 2)
                * Matrix_Rotate_X(PI / 2)
                * Matrix_Scale((FIELD_WIDTH / 2 - GOAL_WIDTH / 2) / 2, 1, GOAL_HEIGHT / 2);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, WALL);
        DrawVirtualObject("plane");

        // Desenho os carros

        model = Matrix_Translate(purple_car_position.x, purple_car_position.y, purple_car_position.z)
                * Matrix_Rotate_Y(purple_car_direction_angle + PI)
                * Matrix_Scale(CAR_WIDTH / 2, CAR_HEIGHT / 2, CAR_LENGTH / 2);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, PURPLE_CAR);
        DrawVirtualObject("carrito");

        model = Matrix_Translate(orange_car_position.x, orange_car_position.y, orange_car_position.z)
                * Matrix_Rotate_Y(orange_car_direction_angle + PI)
                * Matrix_Scale(CAR_WIDTH / 2, CAR_HEIGHT / 2, CAR_LENGTH / 2);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, ORANGE_CAR);
        DrawVirtualObject("carrito");

        car_position = orange_car_position;
        car_direction_angle = orange_car_direction_angle;
        is_camera_looking_back = is_orange_camera_looking_back;
        
        glViewport(1920/2, 0, 1920/2, 1080); // Definição da tela  do carro laranja


        // Aqui são os mesmos comandos, porém para o carro laranja  
        if (is_orange_car_looking_at_ball)
        {
            camera_offset_to_car = glm::vec4(0, 1.8, 0, 1);
        }
        else
        {
            camera_offset_to_car = glm::vec4(0, 1.8, (is_orange_camera_looking_back? -7 : 7), 1);
        }
        camera_position = Matrix_Translate(car_position.x, car_position.y, car_position.z) * Matrix_Rotate_Y(car_direction_angle) * camera_offset_to_car;
        if (is_orange_car_looking_at_ball)
        {
            camera_view_vector = ball_position - camera_position; // Câmera Lookat
        }
        else
        {
            camera_view_vector = Matrix_Rotate_Y((is_camera_looking_back? PI : 0) + car_direction_angle) * NORTH;
        }
        camera_up_vector = UP;

        view = Matrix_Camera_View(camera_position, camera_view_vector, camera_up_vector);

        // Projeção Perspectiva.
        // Para definição do field of view (FOV), veja slides 205-215 do documento Aula_09_Projecoes.pdf.
        field_of_view = PI * 2/5;
        projection = Matrix_Perspective(field_of_view, g_ScreenRatio, nearplane, farplane);

        // Enviamos as matrizes "view" e "projection" para a placa de vídeo
        // (GPU). Veja o arquivo "shader_vertex.glsl", onde estas são
        // efetivamente aplicadas em todos os pontos.
        glUniformMatrix4fv(view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
        glUniformMatrix4fv(projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));
        
        #define BALL 0
        #define FLOOR 1
        #define WALL 2
        #define PURPLE_CAR 3
        #define ORANGE_CAR 4

        // Desenho a bola
        model = Matrix_Translate(ball_position.x, ball_position.y, ball_position.z)
                * Matrix_Scale(BALL_DIAMETER / 2, BALL_DIAMETER / 2, BALL_DIAMETER / 2);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, BALL);
        DrawVirtualObject("sphere");

        // Desenho o chão
        model = Matrix_Translate(0, 0, 0)
                * Matrix_Scale(FIELD_WIDTH / 2, 1, FIELD_LENGTH / 2);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, FLOOR);
        DrawVirtualObject("plane");

        // Desenho as paredes laterais
        model = Matrix_Translate(FIELD_WIDTH / 2, FIELD_HEIGHT / 2, 0)
                * Matrix_Rotate_Z(PI / 2)
                * Matrix_Scale(FIELD_HEIGHT / 2, 1, FIELD_LENGTH / 2);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, WALL);
        DrawVirtualObject("plane");

        model = Matrix_Translate(-FIELD_WIDTH / 2, FIELD_HEIGHT / 2, 0)
                * Matrix_Rotate_Z(-PI / 2)
                * Matrix_Scale(FIELD_HEIGHT / 2, 1, FIELD_LENGTH / 2);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, WALL);
        DrawVirtualObject("plane");

        // Desenho as paredes traseiras
        // XXXXXXXXXXXXX
        // XXXXXXXXXXXXX
        // XXXXXXXXXXXXX
        // XXXXX   XXXXX
        // XXXXX   XXXXX

        model = Matrix_Translate(0, GOAL_HEIGHT + (FIELD_HEIGHT - GOAL_HEIGHT) / 2, FIELD_LENGTH / 2)
                * Matrix_Rotate_X(-PI / 2)
                * Matrix_Scale(FIELD_WIDTH / 2, 1, (FIELD_HEIGHT - GOAL_HEIGHT) / 2);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, WALL);
        DrawVirtualObject("plane");

        model = Matrix_Translate(GOAL_WIDTH / 2 + (FIELD_WIDTH - GOAL_WIDTH) / 4, GOAL_HEIGHT / 2, FIELD_LENGTH / 2)
                * Matrix_Rotate_X(-PI / 2)
                * Matrix_Scale((FIELD_WIDTH / 2 - GOAL_WIDTH / 2) / 2, 1, GOAL_HEIGHT / 2);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, WALL);
        DrawVirtualObject("plane");

        model = Matrix_Translate(-(GOAL_WIDTH / 2 + (FIELD_WIDTH - GOAL_WIDTH) / 4), GOAL_HEIGHT / 2, FIELD_LENGTH / 2)
                * Matrix_Rotate_X(-PI / 2)
                * Matrix_Scale((FIELD_WIDTH / 2 - GOAL_WIDTH / 2) / 2, 1, GOAL_HEIGHT / 2);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, WALL);
        DrawVirtualObject("plane");


        model = Matrix_Translate(0, GOAL_HEIGHT + (FIELD_HEIGHT - GOAL_HEIGHT) / 2, -FIELD_LENGTH / 2)
                * Matrix_Rotate_X(PI / 2)
                * Matrix_Scale(FIELD_WIDTH / 2, 1, (FIELD_HEIGHT - GOAL_HEIGHT) / 2);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, WALL);
        DrawVirtualObject("plane");

        model = Matrix_Translate(GOAL_WIDTH / 2 + (FIELD_WIDTH - GOAL_WIDTH) / 4, GOAL_HEIGHT / 2, -FIELD_LENGTH / 2)
                * Matrix_Rotate_X(PI / 2)
                * Matrix_Scale((FIELD_WIDTH / 2 - GOAL_WIDTH / 2) / 2, 1, GOAL_HEIGHT / 2);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, WALL);
        DrawVirtualObject("plane");

        model = Matrix_Translate(-(GOAL_WIDTH / 2 + (FIELD_WIDTH - GOAL_WIDTH) / 4), GOAL_HEIGHT / 2, -FIELD_LENGTH / 2)
                * Matrix_Rotate_X(PI / 2)
                * Matrix_Scale((FIELD_WIDTH / 2 - GOAL_WIDTH / 2) / 2, 1, GOAL_HEIGHT / 2);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, WALL);
        DrawVirtualObject("plane");

        // Desenho os carros

        model = Matrix_Translate(purple_car_position.x, purple_car_position.y, purple_car_position.z)
                * Matrix_Rotate_Y(purple_car_direction_angle + PI)
                * Matrix_Scale(CAR_WIDTH / 2, CAR_HEIGHT / 2, CAR_LENGTH / 2);
        glUniformMatrix4fv(model_uniform, 1, GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, PURPLE_CAR);
        DrawVirtualObject("carrito");

        model = Matrix_Translate(orange_car_position.x, orange_car_position.y, orange_car_position.z)
                * Matrix_Rotate_Y(orange_car_direction_angle + PI)
                * Matrix_Scale(CAR_WIDTH / 2, CAR_HEIGHT / 2, CAR_LENGTH / 2);
        glUniformMatrix4fv(model_uniform, 1, GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, ORANGE_CAR);
        DrawVirtualObject("carrito");

        // Imprimimos na tela informação sobre o número de quadros renderizados
        // por segundo (frames per second).
        TextRendering_ShowFramesPerSecond(window);

        // O framebuffer onde OpenGL executa as operações de renderização não
        // é o mesmo que está sendo mostrado para o usuário, caso contrário
        // seria possível ver artefatos conhecidos como "screen tearing". A
        // chamada abaixo faz a troca dos buffers, mostrando para o usuário
        // tudo que foi renderizado pelas funções acima.
        // Veja o link: Veja o link: https://en.wikipedia.org/w/index.php?title=Multiple_buffering&oldid=793452829#Double_buffering_in_computer_graphics
        glfwSwapBuffers(window);

        // Verificamos com o sistema operacional se houve alguma interação do
        // usuário (teclado, mouse, ...). Caso positivo, as funções de callback
        // definidas anteriormente usando glfwSet*Callback() serão chamadas
        // pela biblioteca GLFW.
        glfwPollEvents();
    }

    // Finalizamos o uso dos recursos do sistema operacional
    glfwTerminate();

    // Fim do programa
    return 0;
}

// Função que desenha um objeto armazenado em g_VirtualScene. Veja definição
// dos objetos na função BuildTrianglesAndAddToVirtualScene().
void DrawVirtualObject(const char* object_name)
{
    // "Ligamos" o VAO. Informamos que queremos utilizar os atributos de
    // vértices apontados pelo VAO criado pela função BuildTrianglesAndAddToVirtualScene(). Veja
    // comentários detalhados dentro da definição de BuildTrianglesAndAddToVirtualScene().
    glBindVertexArray(g_VirtualScene[object_name].vertex_array_object_id);

    // Pedimos para a GPU rasterizar os vértices dos eixos XYZ
    // apontados pelo VAO como linhas. Veja a definição de
    // g_VirtualScene[""] dentro da função BuildTrianglesAndAddToVirtualScene(), e veja
    // a documentação da função glDrawElements() em
    // http://docs.gl/gl3/glDrawElements.
    glDrawElements(
        g_VirtualScene[object_name].rendering_mode,
        g_VirtualScene[object_name].num_indices,
        GL_UNSIGNED_INT,
        (void*)(g_VirtualScene[object_name].first_index * sizeof(GLuint))
    );

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
}

// Função que carrega os shaders de vértices e de fragmentos que serão
// utilizados para renderização. Veja slides 176-196 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
//
void LoadShadersFromFiles()
{
    // Note que o caminho para os arquivos "shader_vertex.glsl" e
    // "shader_fragment.glsl" estão fixados, sendo que assumimos a existência
    // da seguinte estrutura no sistema de arquivos:
    //
    //    + FCG_Lab_01/
    //    |
    //    +--+ bin/
    //    |  |
    //    |  +--+ Release/  (ou Debug/ ou Linux/)
    //    |     |
    //    |     o-- main.exe
    //    |
    //    +--+ src/
    //       |
    //       o-- shader_vertex.glsl
    //       |
    //       o-- shader_fragment.glsl
    //
    vertex_shader_id = LoadShader_Vertex("../../src/shader_vertex.glsl");
    fragment_shader_id = LoadShader_Fragment("../../src/shader_fragment.glsl");

    // Deletamos o programa de GPU anterior, caso ele exista.
    if ( program_id != 0 )
        glDeleteProgram(program_id);

    // Criamos um programa de GPU utilizando os shaders carregados acima.
    program_id = CreateGpuProgram(vertex_shader_id, fragment_shader_id);

    // Buscamos o endereço das variáveis definidas dentro do Vertex Shader.
    // Utilizaremos estas variáveis para enviar dados para a placa de vídeo
    // (GPU)! Veja arquivo "shader_vertex.glsl" e "shader_fragment.glsl".
    model_uniform           = glGetUniformLocation(program_id, "model"); // Variável da matriz "model"
    view_uniform            = glGetUniformLocation(program_id, "view"); // Variável da matriz "view" em shader_vertex.glsl
    projection_uniform      = glGetUniformLocation(program_id, "projection"); // Variável da matriz "projection" em shader_vertex.glsl
    object_id_uniform       = glGetUniformLocation(program_id, "object_id"); // Variável "object_id" em shader_fragment.glsl
}

// Função que pega a matriz M e guarda a mesma no topo da pilha
void PushMatrix(glm::mat4 M)
{
    g_MatrixStack.push(M);
}

// Função que remove a matriz atualmente no topo da pilha e armazena a mesma na variável M
void PopMatrix(glm::mat4& M)
{
    if ( g_MatrixStack.empty() )
    {
        M = Matrix_Identity();
    }
    else
    {
        M = g_MatrixStack.top();
        g_MatrixStack.pop();
    }
}

// Função que computa as normais de um ObjModel, caso elas não tenham sido
// especificadas dentro do arquivo ".obj"
void ComputeNormals(ObjModel* model)
{
    if ( !model->attrib.normals.empty() )
        return;

    // Primeiro computamos as normais para todos os TRIÂNGULOS.
    // Segundo, computamos as normais dos VÉRTICES através do método proposto
    // por Gouraud, onde a normal de cada vértice vai ser a média das normais de
    // todas as faces que compartilham este vértice.

    size_t num_vertices = model->attrib.vertices.size() / 3;

    std::vector<int> num_triangles_per_vertex(num_vertices, 0);
    std::vector<glm::vec4> vertex_normals(num_vertices, ZERO);

    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);

            glm::vec4  vertices[3];
            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                vertices[vertex] = glm::vec4(vx,vy,vz,1.0);
            }

            const glm::vec4  a = vertices[0];
            const glm::vec4  b = vertices[1];
            const glm::vec4  c = vertices[2];

            // PREENCHA AQUI o cálculo da normal de um triângulo cujos vértices
            // estão nos pontos "a", "b", e "c", definidos no sentido anti-horário.
            const glm::vec4 u = c-a;
            const glm::vec4 v = b-a;
            const glm::vec4  n = crossproduct(v,u);

            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                num_triangles_per_vertex[idx.vertex_index] += 1;
                vertex_normals[idx.vertex_index] += n;
                model->shapes[shape].mesh.indices[3*triangle + vertex].normal_index = idx.vertex_index;
            }
        }
    }

    model->attrib.normals.resize( 3*num_vertices );

    for (size_t i = 0; i < vertex_normals.size(); ++i)
    {
        glm::vec4 n = vertex_normals[i] / (float)num_triangles_per_vertex[i];
        n /= norm(n);
        model->attrib.normals[3*i + 0] = n.x;
        model->attrib.normals[3*i + 1] = n.y;
        model->attrib.normals[3*i + 2] = n.z;
    }
}

// Constrói triângulos para futura renderização a partir de um ObjModel.
void BuildTrianglesAndAddToVirtualScene(ObjModel* model)
{
    GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);
    glBindVertexArray(vertex_array_object_id);

    std::vector<GLuint> indices;
    std::vector<float>  model_coefficients;
    std::vector<float>  normal_coefficients;
    std::vector<float>  texture_coefficients;

    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t first_index = indices.size();
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);

            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];

                indices.push_back(first_index + 3*triangle + vertex);

                const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                //printf("tri %d vert %d = (%.2f, %.2f, %.2f)\n", (int)triangle, (int)vertex, vx, vy, vz);
                model_coefficients.push_back( vx ); // X
                model_coefficients.push_back( vy ); // Y
                model_coefficients.push_back( vz ); // Z
                model_coefficients.push_back( 1.0f ); // W

                // Inspecionando o código da tinyobjloader, o aluno Bernardo
                // Sulzbach (2017/1) apontou que a maneira correta de testar se
                // existem normais e coordenadas de textura no ObjModel é
                // comparando se o índice retornado é -1. Fazemos isso abaixo.

                if ( idx.normal_index != -1 )
                {
                    const float nx = model->attrib.normals[3*idx.normal_index + 0];
                    const float ny = model->attrib.normals[3*idx.normal_index + 1];
                    const float nz = model->attrib.normals[3*idx.normal_index + 2];
                    normal_coefficients.push_back( nx ); // X
                    normal_coefficients.push_back( ny ); // Y
                    normal_coefficients.push_back( nz ); // Z
                    normal_coefficients.push_back( 0.0f ); // W
                }

                if ( idx.texcoord_index != -1 )
                {
                    const float u = model->attrib.texcoords[2*idx.texcoord_index + 0];
                    const float v = model->attrib.texcoords[2*idx.texcoord_index + 1];
                    texture_coefficients.push_back( u );
                    texture_coefficients.push_back( v );
                }
            }
        }

        size_t last_index = indices.size() - 1;

        SceneObject theobject;
        theobject.name           = model->shapes[shape].name;
        theobject.first_index    = first_index; // Primeiro índice
        theobject.num_indices    = last_index - first_index + 1; // Número de indices
        theobject.rendering_mode = GL_TRIANGLES;       // Índices correspondem ao tipo de rasterização GL_TRIANGLES.
        theobject.vertex_array_object_id = vertex_array_object_id;

        g_VirtualScene[model->shapes[shape].name] = theobject;
    }

    GLuint VBO_model_coefficients_id;
    glGenBuffers(1, &VBO_model_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_model_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, model_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, model_coefficients.size() * sizeof(float), model_coefficients.data());
    GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
    GLint  number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if ( !normal_coefficients.empty() )
    {
        GLuint VBO_normal_coefficients_id;
        glGenBuffers(1, &VBO_normal_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_normal_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, normal_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, normal_coefficients.size() * sizeof(float), normal_coefficients.data());
        location = 1; // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if ( !texture_coefficients.empty() )
    {
        GLuint VBO_texture_coefficients_id;
        glGenBuffers(1, &VBO_texture_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_texture_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, texture_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, texture_coefficients.size() * sizeof(float), texture_coefficients.data());
        location = 2; // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 2; // vec2 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    GLuint indices_id;
    glGenBuffers(1, &indices_id);

    // "Ligamos" o buffer. Note que o tipo agora é GL_ELEMENT_ARRAY_BUFFER.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(GLuint), indices.data());
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // XXX Errado!
    //

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
}

// Carrega um Vertex Shader de um arquivo GLSL. Veja definição de LoadShader() abaixo.
GLuint LoadShader_Vertex(const char* filename)
{
    // Criamos um identificador (ID) para este shader, informando que o mesmo
    // será aplicado nos vértices.
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);

    // Carregamos e compilamos o shader
    LoadShader(filename, vertex_shader_id);

    // Retorna o ID gerado acima
    return vertex_shader_id;
}

// Carrega um Fragment Shader de um arquivo GLSL . Veja definição de LoadShader() abaixo.
GLuint LoadShader_Fragment(const char* filename)
{
    // Criamos um identificador (ID) para este shader, informando que o mesmo
    // será aplicado nos fragmentos.
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

    // Carregamos e compilamos o shader
    LoadShader(filename, fragment_shader_id);

    // Retorna o ID gerado acima
    return fragment_shader_id;
}

// Função auxilar, utilizada pelas duas funções acima. Carrega código de GPU de
// um arquivo GLSL e faz sua compilação.
void LoadShader(const char* filename, GLuint shader_id)
{
    // Lemos o arquivo de texto indicado pela variável "filename"
    // e colocamos seu conteúdo em memória, apontado pela variável
    // "shader_string".
    std::ifstream file;
    try {
        file.exceptions(std::ifstream::failbit);
        file.open(filename);
    } catch ( std::exception& e ) {
        fprintf(stderr, "ERROR: Cannot open file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }
    std::stringstream shader;
    shader << file.rdbuf();
    std::string str = shader.str();
    const GLchar* shader_string = str.c_str();
    const GLint   shader_string_length = static_cast<GLint>( str.length() );

    // Define o código do shader GLSL, contido na string "shader_string"
    glShaderSource(shader_id, 1, &shader_string, &shader_string_length);

    // Compila o código do shader GLSL (em tempo de execução)
    glCompileShader(shader_id);

    // Verificamos se ocorreu algum erro ou "warning" durante a compilação
    GLint compiled_ok;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled_ok);

    GLint log_length = 0;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);

    // Alocamos memória para guardar o log de compilação.
    // A chamada "new" em C++ é equivalente ao "malloc()" do C.
    GLchar* log = new GLchar[log_length];
    glGetShaderInfoLog(shader_id, log_length, &log_length, log);

    // Imprime no terminal qualquer erro ou "warning" de compilação
    if ( log_length != 0 )
    {
        std::string  output;

        if ( !compiled_ok )
        {
            output += "ERROR: OpenGL compilation of \"";
            output += filename;
            output += "\" failed.\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }
        else
        {
            output += "WARNING: OpenGL compilation of \"";
            output += filename;
            output += "\".\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }

        fprintf(stderr, "%s", output.c_str());
    }

    // A chamada "delete" em C++ é equivalente ao "free()" do C
    delete [] log;
}


void LoadTextureImage(const char* filename)
{
    printf("Carregando imagem \"%s\"... ", filename);

    // Primeiro fazemos a leitura da imagem do disco
    stbi_set_flip_vertically_on_load(true);
    int width;
    int height;
    int channels;
    unsigned char *data = stbi_load(filename, &width, &height, &channels, 3);

    if ( data == NULL )
    {
        fprintf(stderr, "ERROR: Cannot open image file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }

    printf("OK (%dx%d).\n", width, height);

    // Agora criamos objetos na GPU com OpenGL para armazenar a textura
    GLuint texture_id;
    GLuint sampler_id;
    glGenTextures(1, &texture_id);
    glGenSamplers(1, &sampler_id);

    // Veja slides 95-96 do documento Aula_20_Mapeamento_de_Texturas.pdf
    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    // Parâmetros de amostragem da textura.
    glSamplerParameteri(sampler_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(sampler_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Agora enviamos a imagem lida do disco para a GPU
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

    GLuint textureunit = g_NumLoadedTextures;
    glActiveTexture(GL_TEXTURE0 + textureunit);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindSampler(textureunit, sampler_id);

    stbi_image_free(data);

    g_NumLoadedTextures += 1;
}

// Esta função cria um programa de GPU, o qual contém obrigatoriamente um
// Vertex Shader e um Fragment Shader.
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id)
{
    // Criamos um identificador (ID) para este programa de GPU
    GLuint program_id = glCreateProgram();

    // Definição dos dois shaders GLSL que devem ser executados pelo programa
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);

    // Linkagem dos shaders acima ao programa
    glLinkProgram(program_id);

    // Verificamos se ocorreu algum erro durante a linkagem
    GLint linked_ok = GL_FALSE;
    glGetProgramiv(program_id, GL_LINK_STATUS, &linked_ok);

    // Imprime no terminal qualquer erro de linkagem
    if ( linked_ok == GL_FALSE )
    {
        GLint log_length = 0;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);

        // Alocamos memória para guardar o log de compilação.
        // A chamada "new" em C++ é equivalente ao "malloc()" do C.
        GLchar* log = new GLchar[log_length];

        glGetProgramInfoLog(program_id, log_length, &log_length, log);

        std::string output;

        output += "ERROR: OpenGL linking of program failed.\n";
        output += "== Start of link log\n";
        output += log;
        output += "\n== End of link log\n";

        // A chamada "delete" em C++ é equivalente ao "free()" do C
        delete [] log;

        fprintf(stderr, "%s", output.c_str());
    }

    // Os "Shader Objects" podem ser marcados para deleção após serem linkados
    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    // Retornamos o ID gerado acima
    return program_id;
}

// Definição da função que será chamada sempre que o usuário pressionar alguma
// tecla do teclado. Veja http://www.glfw.org/docs/latest/input_guide.html#input_key
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod)
{
    // Se o usuário pressionar a tecla ESC, fechamos a janela.
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // Se o usuário apertar a tecla W, o carro anda pra frente.
    if (key == GLFW_KEY_W && action == GLFW_PRESS)
    {
        is_purple_car_moving_front = true;
    }
    if (key == GLFW_KEY_W && action == GLFW_RELEASE)
    {
        is_purple_car_moving_front = false;
    }

    // Se o usuário apertar a tecla S, o carro anda pra trás.
    if (key == GLFW_KEY_S && action == GLFW_PRESS)
    {
        is_purple_car_moving_back = true;
    }
    if (key == GLFW_KEY_S && action == GLFW_RELEASE)
    {
        is_purple_car_moving_back = false;
    }

    // Se o usuário apertar a tecla A, giramos o carro pra esquerda.
    if (key == GLFW_KEY_A && action == GLFW_PRESS)
    {
        is_purple_car_moving_left = true;
    }
    if (key == GLFW_KEY_A && action == GLFW_RELEASE)
    {
        is_purple_car_moving_left = false;
    }

    // Se o usuário apertar a tecla D, giramos o carro pra direita.
    if (key == GLFW_KEY_D && action == GLFW_PRESS)
    {
        is_purple_car_moving_right = true;
    }
    if (key == GLFW_KEY_D && action == GLFW_RELEASE)
    {
        is_purple_car_moving_right = false;
    }

    // Se o usuário apertar a tecla LEFT CONTROL, mudamos o tipo de câmera yay \o/
    if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS)
    {
        is_purple_car_looking_at_ball = !is_purple_car_looking_at_ball;
    }

    // Se o usuário apertar a tecla LEFT SHIFT, a câmera olha na direção oposta 
    if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS)
    {
        is_purple_camera_looking_back = true;
    }
    if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE)
    {
        is_purple_camera_looking_back = false;
    }

    // Se o usuário apertar a tecla UP, o carro anda pra frente.
    if (key == GLFW_KEY_UP && action == GLFW_PRESS)
    {
        is_orange_car_moving_front = true;
    }
    if (key == GLFW_KEY_UP && action == GLFW_RELEASE)
    {
        is_orange_car_moving_front = false;
    }

    // Se o usuário apertar a tecla DOWN, o carro anda pra trás.
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
    {
        is_orange_car_moving_back = true;
    }
    if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE)
    {
        is_orange_car_moving_back = false;
    }

    // Se o usuário apertar a tecla LEFT, giramos o carro pra esquerda.
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
    {
        is_orange_car_moving_left = true;
    }
    if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE)
    {
        is_orange_car_moving_left = false;
    }

    // Se o usuário apertar a tecla RIGHT, giramos o carro pra direita.
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
    {
        is_orange_car_moving_right = true;
    }
    if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE)
    {
        is_orange_car_moving_right = false;
    }

    // Se o usuário apertar a tecla RIGHT CONTROL, mudamos o tipo de câmera yay \o/
    if (key == GLFW_KEY_RIGHT_CONTROL && action == GLFW_PRESS)
    {
        is_orange_car_looking_at_ball = !is_orange_car_looking_at_ball;
    }
    
    // Se o usuário apertar a tecla RIGHT SHIFT, a câmera olha na direção oposta 
    if (key == GLFW_KEY_RIGHT_SHIFT && action == GLFW_PRESS)
    {
        is_orange_camera_looking_back = true;
    }
    if (key == GLFW_KEY_RIGHT_SHIFT && action == GLFW_RELEASE)
    {
        is_orange_camera_looking_back = false;
    }
}

// Definimos o callback para impressão de erros da GLFW no terminal
void ErrorCallback(int error, const char* description)
{
    fprintf(stderr, "ERROR: GLFW: %s\n", description);
}

// Esta função recebe um vértice com coordenadas de modelo p_model e passa o
// mesmo por todos os sistemas de coordenadas armazenados nas matrizes model,
// view, e projection; e escreve na tela as matrizes e pontos resultantes
// dessas transformações.
void TextRendering_ShowModelViewProjection(
    GLFWwindow* window,
    glm::mat4 projection,
    glm::mat4 view,
    glm::mat4 model,
    glm::vec4 p_model
)
{
    if ( !g_ShowInfoText )
        return;

    glm::vec4 p_world = model*p_model;
    glm::vec4 p_camera = view*p_world;
    glm::vec4 p_clip = projection*p_camera;
    glm::vec4 p_ndc = p_clip / p_clip.w;

    float pad = TextRendering_LineHeight(window);

    TextRendering_PrintString(window, " Model matrix             Model     In World Coords.", -1.0f, 1.0f-pad, 1.0f);
    TextRendering_PrintMatrixVectorProduct(window, model, p_model, -1.0f, 1.0f-2*pad, 1.0f);

    TextRendering_PrintString(window, "                                        |  ", -1.0f, 1.0f-6*pad, 1.0f);
    TextRendering_PrintString(window, "                            .-----------'  ", -1.0f, 1.0f-7*pad, 1.0f);
    TextRendering_PrintString(window, "                            V              ", -1.0f, 1.0f-8*pad, 1.0f);

    TextRendering_PrintString(window, " View matrix              World     In Camera Coords.", -1.0f, 1.0f-9*pad, 1.0f);
    TextRendering_PrintMatrixVectorProduct(window, view, p_world, -1.0f, 1.0f-10*pad, 1.0f);

    TextRendering_PrintString(window, "                                        |  ", -1.0f, 1.0f-14*pad, 1.0f);
    TextRendering_PrintString(window, "                            .-----------'  ", -1.0f, 1.0f-15*pad, 1.0f);
    TextRendering_PrintString(window, "                            V              ", -1.0f, 1.0f-16*pad, 1.0f);

    TextRendering_PrintString(window, " Projection matrix        Camera                    In NDC", -1.0f, 1.0f-17*pad, 1.0f);
    TextRendering_PrintMatrixVectorProductDivW(window, projection, p_camera, -1.0f, 1.0f-18*pad, 1.0f);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    glm::vec2 a = glm::vec2(-1, -1);
    glm::vec2 b = glm::vec2(+1, +1);
    glm::vec2 p = glm::vec2( 0,  0);
    glm::vec2 q = glm::vec2(width, height);

    glm::mat4 viewport_mapping = Matrix(
        (q.x - p.x)/(b.x-a.x), 0.0f, 0.0f, (b.x*p.x - a.x*q.x)/(b.x-a.x),
        0.0f, (q.y - p.y)/(b.y-a.y), 0.0f, (b.y*p.y - a.y*q.y)/(b.y-a.y),
        0.0f , 0.0f , 1.0f , 0.0f ,
        0.0f , 0.0f , 0.0f , 1.0f
    );

    TextRendering_PrintString(window, "                                                       |  ", -1.0f, 1.0f-22*pad, 1.0f);
    TextRendering_PrintString(window, "                            .--------------------------'  ", -1.0f, 1.0f-23*pad, 1.0f);
    TextRendering_PrintString(window, "                            V                           ", -1.0f, 1.0f-24*pad, 1.0f);

    TextRendering_PrintString(window, " Viewport matrix           NDC      In Pixel Coords.", -1.0f, 1.0f-25*pad, 1.0f);
    TextRendering_PrintMatrixVectorProductMoreDigits(window, viewport_mapping, p_ndc, -1.0f, 1.0f-26*pad, 1.0f);
}

// Escrevemos na tela o número de quadros renderizados por segundo (frames per
// second).
void TextRendering_ShowFramesPerSecond(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    // Variáveis estáticas (static) mantém seus valores entre chamadas
    // subsequentes da função!
    static float old_seconds = (float)glfwGetTime();
    static int   ellapsed_frames = 0;
    static char  buffer[20] = "?? fps";
    static int   numchars = 7;

    ellapsed_frames += 1;

    // Recuperamos o número de segundos que passou desde a execução do programa
    float seconds = (float)glfwGetTime();

    // Número de segundos desde o último cálculo do fps
    float ellapsed_seconds = seconds - old_seconds;

    if ( ellapsed_seconds > 1.0f )
    {
        numchars = snprintf(buffer, 20, "%.2f fps", ellapsed_frames / ellapsed_seconds);

        old_seconds = seconds;
        ellapsed_frames = 0;
    }

    float lineheight = TextRendering_LineHeight(window);
    float charwidth = TextRendering_CharWidth(window);

    TextRendering_PrintString(window, buffer, 1.0f-(numchars + 1)*charwidth, 1.0f-lineheight, 1.0f);
}

// Função para debugging: imprime no terminal todas informações de um modelo
// geométrico carregado de um arquivo ".obj".
// Veja: https://github.com/syoyo/tinyobjloader/blob/22883def8db9ef1f3ffb9b404318e7dd25fdbb51/loader_example.cc#L98
void PrintObjModelInfo(ObjModel* model)
{
  const tinyobj::attrib_t                & attrib    = model->attrib;
  const std::vector<tinyobj::shape_t>    & shapes    = model->shapes;
  const std::vector<tinyobj::material_t> & materials = model->materials;

  printf("# of vertices  : %d\n", (int)(attrib.vertices.size() / 3));
  printf("# of normals   : %d\n", (int)(attrib.normals.size() / 3));
  printf("# of texcoords : %d\n", (int)(attrib.texcoords.size() / 2));
  printf("# of shapes    : %d\n", (int)shapes.size());
  printf("# of materials : %d\n", (int)materials.size());

  for (size_t v = 0; v < attrib.vertices.size() / 3; v++) {
    printf("  v[%ld] = (%f, %f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.vertices[3 * v + 0]),
           static_cast<const double>(attrib.vertices[3 * v + 1]),
           static_cast<const double>(attrib.vertices[3 * v + 2]));
  }

  for (size_t v = 0; v < attrib.normals.size() / 3; v++) {
    printf("  n[%ld] = (%f, %f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.normals[3 * v + 0]),
           static_cast<const double>(attrib.normals[3 * v + 1]),
           static_cast<const double>(attrib.normals[3 * v + 2]));
  }

  for (size_t v = 0; v < attrib.texcoords.size() / 2; v++) {
    printf("  uv[%ld] = (%f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.texcoords[2 * v + 0]),
           static_cast<const double>(attrib.texcoords[2 * v + 1]));
  }

  // For each shape
  for (size_t i = 0; i < shapes.size(); i++) {
    printf("shape[%ld].name = %s\n", static_cast<long>(i),
           shapes[i].name.c_str());
    printf("Size of shape[%ld].indices: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.indices.size()));

    size_t index_offset = 0;

    assert(shapes[i].mesh.num_face_vertices.size() ==
           shapes[i].mesh.material_ids.size());

    printf("shape[%ld].num_faces: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.num_face_vertices.size()));

    // For each face
    for (size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
      size_t fnum = shapes[i].mesh.num_face_vertices[f];

      printf("  face[%ld].fnum = %ld\n", static_cast<long>(f),
             static_cast<unsigned long>(fnum));

      // For each vertex in the face
      for (size_t v = 0; v < fnum; v++) {
        tinyobj::index_t idx = shapes[i].mesh.indices[index_offset + v];
        printf("    face[%ld].v[%ld].idx = %d/%d/%d\n", static_cast<long>(f),
               static_cast<long>(v), idx.vertex_index, idx.normal_index,
               idx.texcoord_index);
      }

      printf("  face[%ld].material_id = %d\n", static_cast<long>(f),
             shapes[i].mesh.material_ids[f]);

      index_offset += fnum;
    }

    printf("shape[%ld].num_tags: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.tags.size()));
    for (size_t t = 0; t < shapes[i].mesh.tags.size(); t++) {
      printf("  tag[%ld] = %s ", static_cast<long>(t),
             shapes[i].mesh.tags[t].name.c_str());
      printf(" ints: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].intValues.size(); ++j) {
        printf("%ld", static_cast<long>(shapes[i].mesh.tags[t].intValues[j]));
        if (j < (shapes[i].mesh.tags[t].intValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");

      printf(" floats: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].floatValues.size(); ++j) {
        printf("%f", static_cast<const double>(
                         shapes[i].mesh.tags[t].floatValues[j]));
        if (j < (shapes[i].mesh.tags[t].floatValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");

      printf(" strings: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].stringValues.size(); ++j) {
        printf("%s", shapes[i].mesh.tags[t].stringValues[j].c_str());
        if (j < (shapes[i].mesh.tags[t].stringValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");
      printf("\n");
    }
  }

  for (size_t i = 0; i < materials.size(); i++) {
    printf("material[%ld].name = %s\n", static_cast<long>(i),
           materials[i].name.c_str());
    printf("  material.Ka = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].ambient[0]),
           static_cast<const double>(materials[i].ambient[1]),
           static_cast<const double>(materials[i].ambient[2]));
    printf("  material.Kd = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].diffuse[0]),
           static_cast<const double>(materials[i].diffuse[1]),
           static_cast<const double>(materials[i].diffuse[2]));
    printf("  material.Ks = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].specular[0]),
           static_cast<const double>(materials[i].specular[1]),
           static_cast<const double>(materials[i].specular[2]));
    printf("  material.Tr = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].transmittance[0]),
           static_cast<const double>(materials[i].transmittance[1]),
           static_cast<const double>(materials[i].transmittance[2]));
    printf("  material.Ke = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].emission[0]),
           static_cast<const double>(materials[i].emission[1]),
           static_cast<const double>(materials[i].emission[2]));
    printf("  material.Ns = %f\n",
           static_cast<const double>(materials[i].shininess));
    printf("  material.Ni = %f\n", static_cast<const double>(materials[i].ior));
    printf("  material.dissolve = %f\n",
           static_cast<const double>(materials[i].dissolve));
    printf("  material.illum = %d\n", materials[i].illum);
    printf("  material.map_Ka = %s\n", materials[i].ambient_texname.c_str());
    printf("  material.map_Kd = %s\n", materials[i].diffuse_texname.c_str());
    printf("  material.map_Ks = %s\n", materials[i].specular_texname.c_str());
    printf("  material.map_Ns = %s\n",
           materials[i].specular_highlight_texname.c_str());
    printf("  material.map_bump = %s\n", materials[i].bump_texname.c_str());
    printf("  material.map_d = %s\n", materials[i].alpha_texname.c_str());
    printf("  material.disp = %s\n", materials[i].displacement_texname.c_str());
    printf("  <<PBR>>\n");
    printf("  material.Pr     = %f\n", materials[i].roughness);
    printf("  material.Pm     = %f\n", materials[i].metallic);
    printf("  material.Ps     = %f\n", materials[i].sheen);
    printf("  material.Pc     = %f\n", materials[i].clearcoat_thickness);
    printf("  material.Pcr    = %f\n", materials[i].clearcoat_thickness);
    printf("  material.aniso  = %f\n", materials[i].anisotropy);
    printf("  material.anisor = %f\n", materials[i].anisotropy_rotation);
    printf("  material.map_Ke = %s\n", materials[i].emissive_texname.c_str());
    printf("  material.map_Pr = %s\n", materials[i].roughness_texname.c_str());
    printf("  material.map_Pm = %s\n", materials[i].metallic_texname.c_str());
    printf("  material.map_Ps = %s\n", materials[i].sheen_texname.c_str());
    printf("  material.norm   = %s\n", materials[i].normal_texname.c_str());
    std::map<std::string, std::string>::const_iterator it(
        materials[i].unknown_parameter.begin());
    std::map<std::string, std::string>::const_iterator itEnd(
        materials[i].unknown_parameter.end());

    for (; it != itEnd; it++) {
      printf("  material.%s = %s\n", it->first.c_str(), it->second.c_str());
    }
    printf("\n");
  }
}

// set makeprg=cd\ ..\ &&\ make\ run\ >/dev/null
// vim: set spell spelllang=pt_br :

