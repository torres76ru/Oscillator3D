#include "App.h"

Camera camera(glm::vec3(0.0f, 30.0f, 120.f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

App::App()
{
    this->initVariables();
    this->initWindow();
    this->initBuffers();
    this->initMatrices();
    this->initShaders();
    this->initAudio();
    
}

App::~App()
{
    delete va;
    delete vb;
    delete ib;
    delete shader;
    audio->stopAudio();
    glfwTerminate();
}

void App::processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, dt);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, dt);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, dt);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, dt);


    // music
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        audio->pause();
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        audio->play();
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        this->drawZDiagram = false;
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
        this->drawZDiagram = true;
}

void App::updateDt()
{
    crntTimeS = glfwGetTime();
    dt = crntTimeS - prevTimeS;
    counterS++;
    if (dt >= 1.0 / 30.0)
    {
        std::string FPS = std::to_string((1.0 / dt) * counterS);
        std::string ms = std::to_string((dt / counterS) * 1000);
        std::string newTitle = APP_NAME + FPS + "fps / " + ms + "ms";
        glfwSetWindowTitle(window, newTitle.c_str());
        prevTimeS = crntTimeS;
        counterS = 0;
    }
}

void App::update()
{

    shader->Bind();
    va->Bind();
    ib->Bind();

    // pass projection matrix to shader (note that in this case it could change every frame)
  
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 300.0f);
    this->shader->SetUniformMat4f("projection", projection);

    // camera/view transformation
    glm::mat4 view = camera.GetViewMatrix();
    this->shader->SetUniformMat4f("view", view);
}

void App::renderGraph()
{
#define FREQ(i) ((i) * (SAMPLE_RATE / SAMPLES) + (i))

    int freq1, freq2;
    double magnitude;
    shader->SetUniform4f("standart_color", 255 / 255.f, 201.f / 255.f, 14.f / 255.f, 1.f);
    shader->SetUniform1f("height", 20);
    for (int i = 0; i < BARS; i++)
    {
        freq1 = FREQ(i);
        freq2 = FREQ(i + 1);


        magnitude = audio->getMagnitude(freq1, freq2);
        magnitude = magnitude > 2.0 ? log(magnitude) : magnitude; // 0.0 - 2.0

        //std::cout << magnitude;
        if (i + BARS < BARS * BARS_LENGTH)
        {
            cubePositions[i + BARS].y = magnitude * 10;
        }
        model = glm::mat4(1.0f);
        cubePositions[i].y = magnitude * 10;
        model = glm::translate(model, cubePositions[i]);

        shader->SetUniformMat4f("model", model);
        

        GLCall(glDrawElements(GL_TRIANGLES, ib->GetCount(), GL_UNSIGNED_INT, nullptr));
    }
    if (this->drawZDiagram)
    {
        shader->SetUniform4f("standart_color", 13 / 255.f, 151 / 255.f, 250.f / 255.f, 1.f);
        for (int i = 1; i < BARS_LENGTH; i++)
        {
            for (int j = 0; j < BARS; j++)
            {
                model = glm::mat4(1.0f);
                model = glm::translate(model, cubePositions[i * BARS + j]);

                shader->SetUniformMat4f("model", model);
                shader->SetUniform1f("height", cubePositions[i * BARS + j].y);

                GLCall(glDrawElements(GL_TRIANGLES, ib->GetCount(), GL_UNSIGNED_INT, nullptr));
            }
        }
    }
    if (this->audio->isPaused() == false)
    {
        for (int i = BARS_LENGTH - 1; i > 1; i--)
        {
            for (int j = 0; j < BARS; j++)
            {
                cubePositions[BARS * i + j].y = cubePositions[BARS * (i - 1) + j].y;
            }
        }
    }

#undef FREQ
}


void App::render()
{
    this->renderGraph();
    //this->renderSin();
}

void App::renderSin()
{
    for (unsigned int i = 0; i < sizeof(cubePositions) / sizeof(glm::vec3); i++)
    {
        model = glm::mat4(1.0f);
        model = glm::translate(model, cubePositions[i]);
        model = glm::translate(model, glm::vec3(0.0f, glm::sin((float)glfwGetTime() + i) * 5, glm::cos((float)glfwGetTime() + i) * 5));

        this->shader->SetUniformMat4f("model", model);

        GLCall(glDrawElements(GL_TRIANGLES, ib->GetCount(), GL_UNSIGNED_INT, nullptr));
    }
}

void App::run()
{
    
    audio->play();
	while (!glfwWindowShouldClose(window))
	{
        this->processInput(window);
        this->updateDt();

        GLCall(glClearColor(4 / 255.f, 7 / 255.f, 41 / 255.f, 16 / 255.f));
        GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		this->update();
		this->render();

        /* Swap front and back buffers | Poll for and process events*/
        glfwSwapBuffers(window);
        glfwPollEvents();
	}
}

void App::initVariables()
{
    prevTimeS = 0.0;
    crntTimeS = 0.0;
    dt = 0.0;
    counterS = 0;

    drawZDiagram = true;


}

void App::initWindow()
{
    /* Initialize the library */
    if (!glfwInit())
        exit(1);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Oscillator", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(1);
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);

    if (glewInit() != GLEW_OK)
        std::cout << "ERROR::GLEW_INIT" << std::endl;

    std::cout << glGetString(GL_VERSION) << std::endl;

    GLCall(glEnable(GL_DEPTH_TEST));
}

void App::initBuffers()
{
    float vertices[] = {
           0.2f,  0.2f, -0.2f,
           0.2f, -0.2f, -0.2f,
          -0.2f, -0.2f, -0.2f,
          -0.2f,  0.2f, -0.2f,

          -0.2f, -0.2f,  0.2f,
           0.2f, -0.2f,  0.2f,
           0.2f,  0.2f,  0.2f,
          -0.2f,  0.2f,  0.2f,

          -0.2f,  0.2f,  0.2f,
          -0.2f,  0.2f, -0.2f,
          -0.2f, -0.2f, -0.2f,
          -0.2f, -0.2f,  0.2f,

           0.2f,  0.2f,  0.2f,
           0.2f,  0.2f, -0.2f,
           0.2f, -0.2f, -0.2f,
           0.2f, -0.2f,  0.2f,

          -0.2f, -0.2f, -0.2f,
           0.2f, -0.2f, -0.2f,
           0.2f, -0.2f,  0.2f,
          -0.2f, -0.2f,  0.2f,

          -0.2f,  0.2f, -0.2f,
           0.2f,  0.2f, -0.2f,
           0.2f,  0.2f,  0.2f,
          -0.2f,  0.2f,  0.2f,
    };

    unsigned int indices[] = {
        0, 1, 2,
        0, 2, 3,

        4, 5, 6,
        4, 6, 7,

        8, 9, 10,
        8, 10, 11,

        12, 13, 14,
        12, 14, 15,

        16, 17, 18,
        16, 18, 19,

        20, 21, 22,
        20, 22, 23,

    };
    for (int j = 0; j < BARS_LENGTH; j++)
    {
        for (int i = 0; i < BARS; i++)
        {
            cubePositions[BARS * j + i] = glm::vec3(static_cast<float>(i * MARGIN - BARS / 2 * MARGIN) + MARGIN / 2.0f, 0.0f, -j);
        }
    }
    

    va = new VertexArray();
    vb = new VertexBuffer(vertices, sizeof(vertices));

    VertexBufferLayout layout;
    layout.Push<float>(3);
    va->AddBuffer(*vb, layout);

    ib = new IndexBuffer(indices, sizeof(indices) / sizeof(float));

    va->Unbind();
    vb->Unbind();
    ib->Unbind();
}

void App::initMatrices()
{
    // matrices
    model = glm::mat4(1.0f);

    view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -250.0f));

    projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 400.0f);
}

void App::initShaders()
{
    shader = new Shader(PATH_TO_SHADER);
    shader->Bind();
    shader->SetUniformMat4f("model", model);
    shader->SetUniformMat4f("view", view);
    shader->SetUniformMat4f("projection", projection);
    shader->SetUniform1f("height", 0.0f);
    shader->SetUniform4f("standart_color", 13 / 255.f, 151 / 255.f, 250.f / 255.f, 1.f);
    shader->Unbind();
}

void App::initAudio()
{   
    audio = new MyAudio();
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}