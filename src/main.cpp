#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Texture.h"
#include "Skybox.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

#include <Cube.h>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct SpotLight {
    glm::vec3 position;
    glm::vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

struct ProgramState {
    glm::vec3 clearColor = glm::vec3(0);
    bool ImGuiEnabled = true;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;
    glm::vec3 backpackPosition = glm::vec3(0.0f);
    float backpackScale = 1.0f;
    SpotLight spotLight;

    ProgramState()
            : camera(glm::vec3(0.0f, 0.0f, 3.0f)) {}

    void SaveToFile(std::string filename);

    void LoadFromFile(std::string filename);
};

void ProgramState::SaveToFile(std::string filename) {
    std::ofstream out(filename);
    out << clearColor.r << '\n'
        << clearColor.g << '\n'
        << clearColor.b << '\n'
        << ImGuiEnabled << '\n'
        << camera.Position.x << '\n'
        << camera.Position.y << '\n'
        << camera.Position.z << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z << '\n';
}

void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream in(filename);
    if (in) {
        in >> clearColor.r
           >> clearColor.g
           >> clearColor.b
           >> ImGuiEnabled
           >> camera.Position.x
           >> camera.Position.y
           >> camera.Position.z
           >> camera.Front.x
           >> camera.Front.y
           >> camera.Front.z;
    }
}

ProgramState *programState;

void DrawImGui(ProgramState *programState);

int main() {
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
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    programState = new ProgramState;
    programState->LoadFromFile("resources/program_state.txt");
    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    // Init Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;


    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    //BLENDING
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //FACE CULLING
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);



    // build and compile shaders
    // -------------------------
    Shader advancedLightingShader("resources/shaders/advanced_lighting.vs", "resources/shaders/advanced_lighting.fs");
    Shader skyboxShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    Shader lightSource("resources/shaders/light_cube.vs", "resources/shaders/light_cube.fs");



    // load models
    // -----------
    Model moai("resources/objects/moai/moai.obj");
    moai.SetShaderTextureNamePrefix("material.");

    Model lucy("resources/objects/lucy/Stanford's Lucy Angel.obj");
    lucy.SetShaderTextureNamePrefix("material.");

    Model venus("resources/objects/venus/venus.obj");
    venus.SetShaderTextureNamePrefix("material.");

    Model spotlightObj("resources/objects/Spotlight.obj");
    spotlightObj.SetShaderTextureNamePrefix("material.");

    Model ceilingLamp("resources/objects/CeilingLamp.obj");
    ceilingLamp.SetShaderTextureNamePrefix("material.");








    // configure cube VAOs (and VBOs)
    unsigned int cubeVBO, cubeVAO;

    glGenBuffers(1, &cubeVBO);
    glGenVertexArrays(1, &cubeVAO);

    //skybox
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);



    // load textures (we now use a utility function to keep the code more organized)
    // -----------------------------------------------------------------------------
    unsigned int floorDiffuseMap = loadTexture(FileSystem::getPath("resources/textures/floor.jpg").c_str());
    unsigned int floorSpecularMap = loadTexture(FileSystem::getPath("resources/textures/floor_specular.png").c_str());
    unsigned int wallDiffuseMap = loadTexture(FileSystem::getPath("resources/textures/marble.jpg").c_str());
    unsigned int glassDiffuseMap = loadTexture(FileSystem::getPath("resources/textures/glass3.png").c_str());
    unsigned int glassSpecularMap = loadTexture(FileSystem::getPath("resources/textures/glass_specular.png").c_str());


    stbi_set_flip_vertically_on_load(false);
    unsigned int cubemapTexture = loadCubemap(faces);


    // shader configuration
    // --------------------
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    //Dodaj prozore <pozicija, velicina> kako bi mogli u render petlji da se sortiraju
    vector<std::pair<glm::vec3, glm::vec3>> prozori = {
            {glm::vec3(9.75f, 2.5, 0),   glm::vec3(0.5f, 5.0f, 8.0f)},
            {glm::vec3(-9.75f, 2.5, 0),  glm::vec3(0.5f, 5.0f, 8.0f)},
            {glm::vec3(0, 2.5f, 4.75f),  glm::vec3(18.0f, 5.0f, 0.5f)},
            {glm::vec3(0, 2.5f, -4.75f), glm::vec3(18.0f, 5.0f, 0.5f)}
    };

    advancedLightingShader.use();
    advancedLightingShader.setInt("blinn", 1);

    //Directional light
    advancedLightingShader.setVec3("dirLight.direction", glm::vec3(-0.4, -0.5f, -0.075));
    advancedLightingShader.setVec3("dirLight.ambient", glm::vec3(0.025f, 0.05f, 0.025f));
    advancedLightingShader.setVec3("dirLight.diffuse", glm::vec3(0.075, 0.275, 0.175));
    advancedLightingShader.setVec3("dirLight.specular", glm::vec3(0.05, 0.15, 0.05));

    //Point light
    advancedLightingShader.setVec3("pointLight.position", glm::vec3(-4, 4.2f, 0));
    advancedLightingShader.setVec3("pointLight.ambient", glm::vec3(0.1f, 0.1f, 0.05f));
    advancedLightingShader.setVec3("pointLight.diffuse", glm::vec3(0.4f, 0.35f, 0));
    advancedLightingShader.setVec3("pointLight.specular", glm::vec3(0.5f, 0.3f, 0));
    advancedLightingShader.setFloat("pointLight.constant", 1.0f);
    advancedLightingShader.setFloat("pointLight.linear", 0.045f);
    advancedLightingShader.setFloat("pointLight.quadratic", 0.0075f);




    //Spotlights
    advancedLightingShader.setVec3("spotLights[0].position", glm::vec3(8, 5.5f, -3));
    advancedLightingShader.setVec3("spotLights[1].position", glm::vec3(8, 5.5f, 3));
    advancedLightingShader.setVec3("spotLights[2].position", glm::vec3(8, 5.5f, 0));


    for (int i = 0; i < 4; i++) {
        string spotLightIt = "spotLights[" + to_string(i) + "]";
        advancedLightingShader.setVec3(spotLightIt + ".direction", glm::vec3(0, -1, 0));
        advancedLightingShader.setVec3(spotLightIt + ".ambient", glm::vec3(0.25f, 0.25f, 0.5f));
        advancedLightingShader.setVec3(spotLightIt + ".diffuse", glm::vec3(1));
        advancedLightingShader.setVec3(spotLightIt + ".specular", glm::vec3(1));
        advancedLightingShader.setFloat(spotLightIt + ".constant", 1.0f);
        advancedLightingShader.setFloat(spotLightIt + ".linear", 0.045f);
        advancedLightingShader.setFloat(spotLightIt + ".quadratic", 0.0075f);
        advancedLightingShader.setFloat(spotLightIt + ".cutOff", glm::cos(glm::radians(10.0f)));
        advancedLightingShader.setFloat(spotLightIt + ".outerCutOff", glm::cos(glm::radians(25.0f)));
    }



    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
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
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();


        //model se postavlja u pomocnoj funkciji
        advancedLightingShader.use();

        if (programState->ImGuiEnabled) {
            advancedLightingShader.setVec3("spotLight.ambient", programState->spotLight.ambient);
            advancedLightingShader.setVec3("spotLight.diffuse", programState->spotLight.diffuse);
            advancedLightingShader.setVec3("spotLight.specular", programState->spotLight.specular);
            advancedLightingShader.setFloat("spotLight.constant", programState->spotLight.constant);
            advancedLightingShader.setFloat("spotLight.linear", programState->spotLight.linear);
            advancedLightingShader.setFloat("spotLight.quadratic", programState->spotLight.quadratic);
            advancedLightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(programState->spotLight.cutOff)));
            advancedLightingShader.setFloat("spotLight.outerCutOff",
                                            glm::cos(glm::radians(programState->spotLight.outerCutOff)));
        }

        advancedLightingShader.setInt("material.diffuseMap",0);    //ova 2 moraju u petlji jer ce za neke kocke koje se crtaju posle
        advancedLightingShader.setInt("material.specularMap", 1);   //specularMap biti postavljeno na 0
        advancedLightingShader.setMat4("projection", projection);
        advancedLightingShader.setMat4("view", view);
        advancedLightingShader.setVec3("viewPos", programState->camera.Position);
        //advancedLightingShader.setVec3("lightPos", glm::vec3(0, 3, 0));


        //DRAW
        //---------

        //Modeli
        //----------------

        advancedLightingShader.setInt("blending", 0);

        //MOAI
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(8, 0.75f + sin(currentFrame) * 0.5, 0));
        model = glm::rotate(model, glm::radians(50 * cos(currentFrame * 0.01f) * 360), glm::vec3(0, 1, 0));
        model = glm::scale(model, glm::vec3(0.0375f));
        advancedLightingShader.setMat4("model", model);
        moai.Draw(advancedLightingShader);


        //LUCY
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(8, 0, 3.0f));
        model = glm::rotate(model, glm::radians(25 * cos(15 + currentFrame * 0.01f) * 360), glm::vec3(0, 1, 0));
        model = glm::scale(model, glm::vec3(0.025f));
        advancedLightingShader.setMat4("model", model);
        lucy.Draw(advancedLightingShader);

        //VENUS
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(8, 0.1f, -3));
        model = glm::rotate(model, glm::radians(-10 * cos(45 + currentFrame * 0.01f) * 360), glm::vec3(0, 1, 0));
        model = glm::scale(model, glm::vec3(0.0185f));
        advancedLightingShader.setMat4("model", model);
        venus.Draw(advancedLightingShader);



        //Spot light models
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(8, 4.75f, -3));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1, 0, 0));
        model = glm::scale(model, glm::vec3(3));
        advancedLightingShader.setMat4("model", model);
        spotlightObj.Draw(advancedLightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(8, 4.75f, 0));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1, 0, 0));
        model = glm::scale(model, glm::vec3(3));
        advancedLightingShader.setMat4("model", model);
        spotlightObj.Draw(advancedLightingShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(8, 4.75f, 3));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1, 0, 0));
        model = glm::scale(model, glm::vec3(3));
        advancedLightingShader.setMat4("model", model);
        spotlightObj.Draw(advancedLightingShader);

        //Ceiling lamp
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-4, 2.5f, 0));
        model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1, 0, 0));
        model = glm::scale(model, glm::vec3(0.5));
        advancedLightingShader.setMat4("model", model);
        ceilingLamp.Draw(advancedLightingShader);

        //Light source for ceiling lamp
        ConfigureVAO(cubeVAO, cubeVBO, cubeVertices, sizeof(cubeVertices));
        lightSource.use();
        lightSource.setMat4("projection", projection);
        lightSource.setMat4("view", view);

        lightSource.setVec3("color", glm::vec3(1, 1, 0));

        //Draw cube
        glBindVertexArray(cubeVAO);
        model = glm::mat4(1.0f);
        //Mora u ovom redosledu transformacije ------------------------------------------------
        model = glm::translate(model, glm::vec3(-4, 4.125f, 0));
        //rotate
        model = glm::scale(model, glm::vec3(0.2f, 0.75f, 0.2f));
        lightSource.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);



        //Floor
        ConfigureVAO(cubeVAO, cubeVBO, cubeVerticesTiled, sizeof(cubeVerticesTiled));
        SpawnCube(&advancedLightingShader, &floorDiffuseMap, &cubeVAO, glm::vec3(0), glm::vec3(20.0f, 0.25f, 10.0f),
                  &floorSpecularMap);

        //Roof
        SpawnCube(&advancedLightingShader, &wallDiffuseMap, &cubeVAO, glm::vec3(0, 5.0f, 0),
                  glm::vec3(20.0f, 0.25f, 10.0f));


        //Pillars
        ConfigureVAO(cubeVAO, cubeVBO, cubeVertices, sizeof(cubeVertices));
        SpawnCube(&advancedLightingShader, &wallDiffuseMap, &cubeVAO, glm::vec3(9.5f, 2.5f, 4.5f),
                  glm::vec3(1.0f, 5.0f, 1.0f));
        SpawnCube(&advancedLightingShader, &wallDiffuseMap, &cubeVAO, glm::vec3(9.5f, 2.5f, -4.5f),
                  glm::vec3(1.0f, 5.0f, 1.0f));
        SpawnCube(&advancedLightingShader, &wallDiffuseMap, &cubeVAO, glm::vec3(-9.5f, 2.5f, -4.5f),
                  glm::vec3(1.0f, 5.0f, 1.0f));
        SpawnCube(&advancedLightingShader, &wallDiffuseMap, &cubeVAO, glm::vec3(-9.5f, 2.5f, 4.5f),
                  glm::vec3(1.0f, 5.0f, 1.0f));


        glDepthFunc(
                GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(programState->camera.GetViewMatrix())); // remove translation from the view matrix
        view = glm::translate(view, glm::vec3(0, -0.5f, 0));    //prikazi skybox malo nize
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default


        advancedLightingShader.use();
        advancedLightingShader.setInt("blending", 1);

        //Prozori idu posle ostalih objekata zbog blendinga
        //Sortiraj prozore
        std::map<float, pair<glm::vec3, glm::vec3>> sorted;
        for (unsigned int i = 0; i < prozori.size(); i++) {
            float distance = glm::length(programState->camera.Position - prozori[i].first);
            sorted[distance] = prozori[i];
        }


        //Crtaj pocevsi od najblizeg
        for (std::map<float, pair<glm::vec3, glm::vec3>>::reverse_iterator it = sorted.rbegin();
             it != sorted.rend(); ++it) {
            SpawnCube(&advancedLightingShader, &glassDiffuseMap, &cubeVAO, it->second.first, it->second.second,
                      &glassSpecularMap);
        }


        if (programState->ImGuiEnabled)
            DrawImGui(programState);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }



    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &skyboxVBO);

    programState->SaveToFile("resources/program_state.txt");
    delete programState;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();



    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float speed = 2.0;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(FORWARD, deltaTime * speed);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(BACKWARD, deltaTime * speed);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(LEFT, deltaTime * speed);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(RIGHT, deltaTime * speed);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (programState->CameraMouseMovementUpdateEnabled)
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    programState->camera.ProcessMouseScroll(yoffset);
}

void DrawImGui(ProgramState *programState) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    {
        static float f = 0.0f;
        ImGui::Begin("Spot light");

        ImGui::DragFloat3("spotLight.ambient", (float *) &programState->spotLight.ambient);
        ImGui::DragFloat3("spotLight.diffuse", (float *) &programState->spotLight.diffuse);
        ImGui::DragFloat3("spotLight.specular", (float *) &programState->spotLight.specular);


        ImGui::DragFloat("spotLight.cutOff", &programState->spotLight.cutOff, 0.05, 0.0, 180.0);
        ImGui::DragFloat("spotLight.outerCutOff", &programState->spotLight.outerCutOff, 0.05, 0.0, 180.0);

        ImGui::DragFloat("spotLight.constant", &programState->spotLight.constant, 0.05, 0.0, 1.0);
        ImGui::DragFloat("spotLight.linear", &programState->spotLight.linear, 0.05, 0.0, 1.0);
        ImGui::DragFloat("spotLight.quadratic", &programState->spotLight.quadratic, 0.05, 0.0, 1.0);
        ImGui::End();
    }

    {
        ImGui::Begin("Camera info");
        const Camera &c = programState->camera;
        ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
        ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
        ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
        ImGui::Checkbox("Camera mouse update", &programState->CameraMouseMovementUpdateEnabled);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        if (programState->ImGuiEnabled) {
            programState->CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}