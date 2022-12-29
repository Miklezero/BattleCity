#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

GLfloat point[] = {
    0.0f, 0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
   -0.5f, -0.5f, 0.0f,
};

GLfloat colors[] = {
    1.0f, 0.0f, 0.0f,
    0.0f,1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
};

//в текстовом режиме объявляем программу вертексного шейдера
const char* vertex_shader =
"#version 460\n"
"layout(location = 0) in vec3 vertex_position;"
"layout(location = 1) in vec3 vertex_color;"
"out vec3 color;"
"void main() {"
"   color = vertex_color;"
"   gl_Position = vec4(vertex_position, 1.0);"
"}";

// и программму фрагментного шейдера (конкретные пиксели между вершинами)
const char* fragment_shader =
"#version 460\n"
"in vec3 color;"
"out vec4 frag_color;"
"void main() {"
"   frag_color = vec4(color, 1.0);"
"}";

int g_windowSizeX = 640;
int g_windowSizeY = 480;

void glfwWindowSizeCallback(GLFWwindow* pWindow, int width, int height)
{
    g_windowSizeX = width;
    g_windowSizeY = height;
    glViewport(0, 0, g_windowSizeX, g_windowSizeY);
}

void glfwKeyCallback(GLFWwindow* pWindow, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(pWindow, GL_TRUE);
    }
}

int main(void)
{
    

    /* Initialize the library */
    if (!glfwInit())
    {
        std::cout << "glfwInit faild!" << std::endl;
        return -1;
    }
    //задание требований к версии opengl (4.6)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

    //не понял что за бодяга
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    GLFWwindow* pWindow = glfwCreateWindow(g_windowSizeX, g_windowSizeY, "Battle City", nullptr, nullptr);
    if (!pWindow)
    {
        std::cout << "glfwCreateWindow faild!" << std::endl;
        glfwTerminate();
        return -1;
    }
    //регистрация функции изменения размера окна
    glfwSetWindowSizeCallback(pWindow, glfwWindowSizeCallback);
    
    //регистрация функции реакции на клавишу
    glfwSetKeyCallback(pWindow, glfwKeyCallback);



    /* Make the window's context current */
    glfwMakeContextCurrent(pWindow);
	
	if(!gladLoadGL())
	{
		std::cout<<"Can't load GLAD"<<std::endl;
		return -1;
	}
	
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;


	glClearColor(0,0,0,1);

    //вышеобявленные программы шейдеров и фрагментов передаем видеокарте
    //для этого объявляем идентификатор шейдера
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    //далее этому шейдеру передаем вышеописанный исходный код(1- одна строка, поэтому nullptr - массива с длинами строк нет)
    glShaderSource(vs, 1, &vertex_shader, nullptr);
    //далее команда на компиляцию
    glCompileShader(vs);

    //то же самое для фрагментного шейдера
    GLint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader, nullptr);
    glCompileShader(fs);

    //генерируем и получаем идентификатор шейдерной программы
   GLint shader_program =  glCreateProgram();
   //аттачим шейдеры к этой программе
   glAttachShader(shader_program, vs);
   glAttachShader(shader_program, fs);
   //выполняем линковку шейдеров
   glLinkProgram(shader_program);
	
   //после этого шейдеры больше не нужны - удаляем их
   glDeleteShader(vs);
   glDeleteShader(fs);

   //далее надо передать эту лабуду в память видеокарты
   // создаем в памяти видеокарты два буфера (для координат и для цветов)
   //для этого потребуется vbo (vertex buffer object)
   GLuint points_vbo = 0;
   //нам потребуется один буфер, генерируем его
   glGenBuffers(1, &points_vbo);
   //подключаем буфер и делаем его текущим
   glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
   //заполняем этот буфер требуемой информацией (передаем данные из оперативной памяти в память видеокарты)
   //GL_STATIC_DRAW - указание для драйвера видеокарты, что данные не будут часто менятся, иначе GL_DYNAMIC_DRAW - данные пом в область приспособ для более быстрого изм
   glBufferData(GL_ARRAY_BUFFER, sizeof(point), point, GL_STATIC_DRAW);

   // повторяем для цвета
   GLuint colors_vbo = 0;
   glGenBuffers(1, &colors_vbo);
   glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

   //теперь надо указать видеокарте, что делать с этими данными
   //  для этого понадобится vao (vertex array object)
   //объявляем
   GLuint vao = 0;
   //генерируем
   glGenVertexArrays(1, &vao);
   //привязываем и делаем текущим
   glBindVertexArray(vao);

   //далее надо связать points_vbo и colors_vbo с позициями шейдеров (см. строку шейдерной программы )
   //по умолчанию позиции выключены, поэтому включаем
   glEnableVertexAttribArray(0); //включаем нулевую позицию в шейдере
   //снова делаем текущим буфер points_vbo
   glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
   //связываем данные 
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

   //повторяем для цвета
   glEnableVertexAttribArray(1); //включаем первую позицию в шейдере
  //снова делаем текущим буфер colors_vbo
   glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
   //связываем данные 
   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);


    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(pWindow))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        //после очистки кадра рисуем необходимое
        glUseProgram(shader_program);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        /* Swap front and back buffers */
        glfwSwapBuffers(pWindow);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}