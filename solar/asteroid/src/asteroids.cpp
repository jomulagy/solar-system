#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 55.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader planetShader("shader/10.3.planet.vs", "shader/10.3.planet.fs");
    Shader asteroidShader("shader/10.3.asteroids.vs", "shader/10.3.asteroids.fs");

    // load models
    // -----------
    Model star("resources/objects/star/planet.obj");
    Model rock("resources/objects/rock/rock.obj");                                                                                  // 소행성 model
    Model sun("resources/objects/sun/planet.obj");                                                                                  // 태양 model
    vector<Model> planet = { Model("resources/objects/mer/planet.obj"), Model("resources/objects/venus/planet.obj"),                // 행성 model
                             Model("resources/objects/earth/planet.obj"), Model("resources/objects/mars/planet.obj"), 
                             Model("resources/objects/jupyter/planet.obj"), Model("resources/objects/saturn/planet.obj"), 
                             Model("resources/objects/uranus/planet.obj"), Model("resources/objects/neptune/planet.obj") };

    // generate a large list of semi-random model transformation matrices
    // ------------------------------------------------------------------
    unsigned int amount = 8;
    glm::mat4* modelMatrices;
    modelMatrices = new glm::mat4[amount];
    srand(static_cast<unsigned int>(glfwGetTime())); // initialize random seed
    float offset = 2.5f;
    vector< float >radius = { 60.0, 75.0, 100.0, 115.0, 280.0, 495.0, 980.0, 1525.0 };    // 중심으로부터 거리
    vector<glm::vec3> size = { glm::vec3(0.8f, 0.8f, 0.8f), glm::vec3(1.8f, 1.8f, 1.8f), glm::vec3(2.0f, 2.0f, 2.0f),  glm::vec3(1.0f, 1.0f, 0.5f),  glm::vec3(4.0f, 4.0f, 4.0f),
                         glm::vec3(3.5f, 3.5f, 3.5f),  glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(1.9f, 1.9f, 1.9f) };           // 행성별 크기
    vector<float> angle_planet = { 10.0f, 90.0f, 30.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f };
    vector<float> speed_planet = { 0.2f, 0.3f, 0.5f, 0.6f, 0.8f, 0.9f, 1.1f, 1.2f };
   
    // 별 구현
    unsigned int amount_star = 3000;
    glm::mat4* modelMatrices_star;
    modelMatrices_star = new glm::mat4[amount_star];
    srand(static_cast<unsigned int>(glfwGetTime())); // initialize random seed
    float radius_star = 300.0;
    for (unsigned int i = 0; i < amount_star; i++)
    {
        glm::mat4 model_star = glm::mat4(1.0f);
        // 1. translation: displace along circle with 'radius' in range [-offset, offset]
        float angle = (float)i / (float)amount_star * 360.0f;
        float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float x = sin(angle) * radius_star + displacement;
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float y = displacement * 200.0f; // keep height of asteroid field smaller compared to width of x and z
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float z = cos(angle) * radius_star + displacement;
        model_star = glm::translate(model_star, glm::vec3(x, y, z));

        // 2. scale: Scale between 0.05 and 0.25f
        float scale = static_cast<float>((rand() % 20) / 100.0 + 0.05);
        model_star = glm::scale(model_star, glm::vec3(scale));

        // 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
        float rotAngle = static_cast<float>((rand() % 360));
        model_star = glm::rotate(model_star, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

        // 4. now add to list of matrices
        modelMatrices_star[i] = model_star;
    }



    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // configure transformation matrices
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();;
        asteroidShader.use();
        asteroidShader.setMat4("projection", projection);
        asteroidShader.setMat4("view", view);

        planetShader.use();
        planetShader.setMat4("projection", projection);
        planetShader.setMat4("view", view);

        planetShader.use();
        planetShader.setInt("texture_diffuse1", 0);

        // draw sun
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(7.0f, 7.0f, 7.0f));
        float angle_sun = 10.0f;
        model = glm::rotate(model, (float)glfwGetTime() * glm::radians(angle_sun), glm::vec3(0.0f, 1.0f, 0.0f));        // 자전
        planetShader.setMat4("model", model);
        sun.Draw(planetShader);

        // draw planet
        for (unsigned int i = 0; i < amount; i++)       
        {
            glm::mat4 model_planet = glm::mat4(1.0f);
            // 1. translation: displace along circle with 'radius' in range [-offset, offset] ( 행성 위치 및 공전 궤도 설정)
            float displacement = (2 * offset * 100) / 100.0f - offset;
            float x = sin(speed_planet[i] * glfwGetTime()) * radius[i];
            displacement = (2 * offset * 100) / 100.0f - offset;
            float y = 0.4f; // keep height of asteroid field smaller compared to width of x and z (고정)
            displacement = (2 * offset * 100) / 100.0f - offset;
            float z = cos(speed_planet[i] * glfwGetTime()) * radius[i];
            model_planet = glm::translate(model_planet, glm::vec3(x, y, z));

            // 2. scale: Scale between 0.05 and 0.25f ( 행성 크기 설정)
            model_planet = glm::scale(model_planet, size[i]);

            // 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector ( 행성 자전 설정)
            model_planet = glm::rotate(model_planet, (float)glfwGetTime() * glm::radians(angle_planet[i]), glm::vec3(0.0f, 1.0f, 0.0f));        

            // 4. now add to list of matrices
            modelMatrices[i] = model_planet;

            planetShader.setMat4("model", modelMatrices[i]);
            planet[i].Draw(planetShader);
        }

        //소행성
        unsigned int amount_astr = 3000;
        glm::mat4* modelMatrices_astr;
        modelMatrices_astr = new glm::mat4[amount_astr];
        srand(static_cast<unsigned int>(glfwGetTime())); // initialize random seed
        float radius_astr = 150.0;
        for (unsigned int i = 0; i < amount_astr; i++)
        {
            glm::mat4 model_astr = glm::mat4(1.0f);
            // 1. translation: displace along circle with 'radius' in range [-offset, offset]
            float angle = (float)i / (float)amount_astr * 360.0f;
            float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
            float x = sin(0.001 * angle * glfwGetTime()) * radius_astr + displacement;
            displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
            float y = displacement * 2.0f; // keep height of asteroid field smaller compared to width of x and z
            displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
            float z = cos(0.001 * angle * glfwGetTime()) * radius_astr + displacement;
            model_astr = glm::translate(model_astr, glm::vec3(x, y, z));

            // 2. scale: Scale between 0.05 and 0.25f
            float scale = static_cast<float>((rand() % 20) / 100.0 + 0.05);
            model_astr = glm::scale(model_astr, glm::vec3(scale));

            // 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
            float rotAngle = static_cast<float>((rand() % 360));
            model_astr = glm::rotate(model_astr, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

            // 4. now add to list of matrices
            modelMatrices_astr[i] = model_astr;
        }

        // configure instanced array
        // -------------------------
        unsigned int buffer;
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, amount_astr * sizeof(glm::mat4), &modelMatrices_astr[0], GL_STATIC_DRAW);

        // set transformation matrices as an instance vertex attribute (with divisor 1)
        // note: we're cheating a little by taking the, now publicly declared, VAO of the model's mesh(es) and adding new vertexAttribPointers
        // normally you'd want to do this in a more organized fashion, but for learning purposes this will do.
        // -----------------------------------------------------------------------------------------------------------------------------------
        for (unsigned int i = 0; i < rock.meshes.size(); i++)
        {
            unsigned int VAO = rock.meshes[i].VAO;
            glBindVertexArray(VAO);
            // set attribute pointers for matrix (4 times vec4)
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
            glEnableVertexAttribArray(4);
            glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
            glEnableVertexAttribArray(5);
            glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
            glEnableVertexAttribArray(6);
            glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

            glVertexAttribDivisor(3, 1);
            glVertexAttribDivisor(4, 1);
            glVertexAttribDivisor(5, 1);
            glVertexAttribDivisor(6, 1);

            glBindVertexArray(0);
        }

        // draw meteorites
        asteroidShader.use();
        asteroidShader.setInt("texture_diffuse1", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rock.textures_loaded[0].id);
        // note: we also made the textures_loaded vector public (instead of private) from the model class.
        for (unsigned int i = 0; i < rock.meshes.size(); i++)
        {
            glBindVertexArray(rock.meshes[i].VAO);
            glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(rock.meshes[i].indices.size()), GL_UNSIGNED_INT, 0, amount_astr);
            glBindVertexArray(0);
        }
        // configure instanced array
        // -------------------------
        unsigned int buffer2;
        glGenBuffers(1, &buffer2);
        glBindBuffer(GL_ARRAY_BUFFER, buffer2);
        glBufferData(GL_ARRAY_BUFFER, amount_star * sizeof(glm::mat4), &modelMatrices_star[0], GL_STATIC_DRAW);

        // set transformation matrices as an instance vertex attribute (with divisor 1)
        // note: we're cheating a little by taking the, now publicly declared, VAO of the model's mesh(es) and adding new vertexAttribPointers
        // normally you'd want to do this in a more organized fashion, but for learning purposes this will do.
        // -----------------------------------------------------------------------------------------------------------------------------------
        for (unsigned int i = 0; i < star.meshes.size(); i++)
        {
            unsigned int VAO = star.meshes[i].VAO;
            glBindVertexArray(VAO);
            // set attribute pointers for matrix (4 times vec4)
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
            glEnableVertexAttribArray(4);
            glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
            glEnableVertexAttribArray(5);
            glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
            glEnableVertexAttribArray(6);
            glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

            glVertexAttribDivisor(3, 1);
            glVertexAttribDivisor(4, 1);
            glVertexAttribDivisor(5, 1);
            glVertexAttribDivisor(6, 1);

            glBindVertexArray(0);
        }
        // draw star
        asteroidShader.use();
        asteroidShader.setInt("texture_diffuse1", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, star.textures_loaded[0].id); // note: we also made the textures_loaded vector public (instead of private) from the model class.
        for (unsigned int i = 0; i < star.meshes.size(); i++)
        {
            glBindVertexArray(star.meshes[i].VAO);
            glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(star.meshes[i].indices.size()), GL_UNSIGNED_INT, 0, amount_star);
            glBindVertexArray(0);
        }
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
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