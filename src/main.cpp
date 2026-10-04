#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

#include "./header/Shader.h"
#include "./header/Object.h"

// ============================================================================
// ICG 2026 HW1 - STUDENT STARTER CODE
//
// This file intentionally keeps the inherited main_ori.cpp skeleton style.
// Most graded parts are left as TODOs, similar to the previous HW starter.
//
// Provided infrastructure:
//   - GLFW / GLAD initialization
//   - render loop structure
//   - object loading
//   - drawModel(..., alpha)
//   - basic state / constants needed by the assignment
//
// Students should complete the TODO sections according to the slides.
// ============================================================================


// Settings
const int INITIAL_SCR_WIDTH = 800;
const int INITIAL_SCR_HEIGHT = 600;

int SCR_WIDTH = INITIAL_SCR_WIDTH;
int SCR_HEIGHT = INITIAL_SCR_HEIGHT;


// Global objects
Shader* shader = nullptr;
Object* cube = nullptr;
Object* bird = nullptr;
Object* flower = nullptr;
Object* magicBall = nullptr;


// ============================================================================
// Scene settings
// ============================================================================

const glm::vec3 GROUND_POSITION(0.0f, -0.5f, 0.0f);
const glm::vec3 GROUND_SCALE(70.0f, 1.0f, 40.0f);
const glm::vec3 GROUND_COLOR(0.25f, 0.65f, 0.20f);

const glm::vec3 BIRD_BASE_POSITION(0.0f, 10.0f, -5.0f);

// Keep this consistent with the slides.
const glm::vec3 BIRD_SCALE(0.20f);
// Reference values only.
// You may change the ellipse size and flying speed.
const float BIRD_RADIUS_X = 10.0f;
const float BIRD_RADIUS_Z = 4.5f;
const float BIRD_SPEED = 1.0f;

const glm::vec3 FLOWER_POSITIONS[3] = {
    glm::vec3(-7.0f, 0.0f,  4.5f),
    glm::vec3( 7.5f, 0.0f,  4.0f),
    glm::vec3( 5.0f, 0.0f, -6.0f)
};

const glm::vec3 FLOWER_SCALE(1.35f);


// ============================================================================
// Character state
// ============================================================================

struct Slime {
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    float angle = 0.0f;
    float speed = 5.0f;

    bool movingHorizontally = false;
    float hopTime = 0.0f;
} playerSlime;

const float totalHopTime = 1.0f;
const float maxHopHeight = 2.0f;

float buttomLength = 5.0f;



// ============================================================================
// Spell state
// ============================================================================

struct Spell {
    bool active = false;
    float elapsed = 0.0f;

    float mainSelfAngle = 0.0f;
    float smallSelfAngle = 0.0f;
    float orbitAngle = 0.0f;
} spell;

const float spelllastTime = 5.0f;

const glm::vec3 MAGIC_BALL_ABOVE_POS(0.0f, 5.6f, 0.0f);
const glm::vec3 MAGIC_BALL_FRONT_POS(0.0f, 2.4f, 4.5f);

const float MAIN_BALL_SCALE = 0.40f;
const float SMALL_BALL_SCALE = 0.14f;
const float ORBIT_RADIUS = 3.0f;


// ============================================================================
// Function declarations
// ============================================================================

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow* window, float deltaTime);

void drawModel(
    std::string type,
    const glm::mat4& model,
    const glm::mat4& view,
    const glm::mat4& projection,
    const glm::vec3& color,
    float alpha = 1.0f);

void drawSlime(
    const glm::mat4& view,
    const glm::mat4& projection);

void drawBird(
    float currentTime,
    const glm::mat4& view,
    const glm::mat4& projection);

void drawFlowers(
    const glm::mat4& view,
    const glm::mat4& projection);

void drawMagic(
    const glm::mat4& view,
    const glm::mat4& projection);

void updateSpell(float deltaTime);

void cleanup();
void init();


// ============================================================================
// main()
// Keep the inherited main_ori.cpp flow.
// ============================================================================

int main() {
    // GLFW: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//openGL 3.3
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//core profile only

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW window creation
    GLFWwindow* window =
        glfwCreateWindow(
            SCR_WIDTH,
            SCR_HEIGHT,
            "ICG 2026 HW1",
            nullptr,
            nullptr);
            /* 
                . 
                . 
                .(string)  
                window mode(NULL) [FullScreen(glfwGetPrimaryMonitor)]
                not share(NULL) [GLFWwindow*]
            */

    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);//remember to maintain global screen size variables
    glfwSetKeyCallback(window, keyCallback);
    glfwSwapInterval(1);

    // GLAD: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }


    // TODO: Enable depth test and face culling.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);


    // Alpha support is provided by the starter code.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //this is BAD pratice, usually we enable blend after finishing drawing opaque things. 
    //the following are from Gemini:
    /*
        glDisable(GL_BLEND);（確保混色關閉）

        繪製所有「不透明」的物體。

        glEnable(GL_BLEND);（開啟混色）

        glDepthMask(GL_FALSE);（這步很關鍵：關閉深度寫入，確保透明物體彼此之間不會寫入深度互相遮擋，但依然會被第一步已經畫好的不透明物體遮擋）

        將所有「透明」的物體依據距離攝影機的遠近，由遠到近進行排序。

        繪製排序後的透明物體。

        glDepthMask(GL_TRUE);（恢復深度寫入，保持 OpenGL 狀態機乾淨）
    */ 

    // Initialize Object and Shader
    init();


    float lastFrame =
        static_cast<float>(glfwGetTime());


    while (!glfwWindowShouldClose(window)) {
        // Calculate delta time for animation
        float currentFrame =
            static_cast<float>(glfwGetTime());

        float deltaTime =
            currentFrame - lastFrame;

        lastFrame =
            currentFrame;


        // TODO: Update spell / animation state.
        updateSpell(deltaTime);


        // Render background
        glClearColor(
            0.65f,
            0.85f,
            1.0f,
            1.0f);

        glClear(
            GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT);


        shader->use();




        /*=================== Example of creating model matrix =======================
        1. translate
        glm::mat4 model(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, 1.0f, 0.0f));
        drawModel("cube", model, view, projection, glm::vec3(0.9f, 0.8f, 0.6f));

        2. scale
        glm::mat4 model(1.0f);
        model = glm::scale(model, glm::vec3(0.5f, 1.0f, 2.0f));
        drawModel("cube", model, view, projection, glm::vec3(0.9f, 0.8f, 0.6f));

        3. rotate
        glm::mat4 model(1.0f);
        model = glm::rotate(
            model,
            glm::radians(45.0f),
            glm::vec3(0.0f, 0.0f, 1.0f));
        drawModel("cube", model, view, projection, glm::vec3(0.9f, 0.8f, 0.6f));
        ==========================================================================*/


        // TODO: Create view matrix and perspective projection matrix.
        //
        // Camera:
        // Position = (0, 14, 23)
        // Target   = (0, 5, 0)
        // Up       = (0, 1, 0)
        // FOV      = 45
        // Near     = 0.1
        // Far      = 1000
        //
        // The identity matrices below are placeholders so the starter compiles.
        glm::mat4 view(1.0f);
        glm::mat4 projection(1.0f);
        

        view = glm::lookAt(
            glm::vec3(0.0f,14.0f,23.0f),
            glm::vec3(0.0f,5.0f,0.0f),
            glm::vec3(0.0f,1.0f,0.0f)
        );

        projection = glm::perspective(

            //原理：定義垂直夾角FOV，再根據視窗長寬比得出水平夾角
            glm::radians(45.0f),
            (float)SCR_WIDTH/(float)SCR_HEIGHT,
            0.1f,
            1000.0f
        );
        



        // TODO: Scene Setup
        // - Draw the ground cube using the fixed position / scale / color.
        // - Draw the bird.
        // - Draw exactly three flowers at the fixed positions / scale.
        //
        // Suggested calls after implementing each function:

        /*
            const glm::vec3 GROUND_POSITION(0.0f, -0.5f, 0.0f);
            const glm::vec3 GROUND_SCALE(70.0f, 1.0f, 40.0f);
            const glm::vec3 GROUND_COLOR(0.25f, 0.65f, 0.20f);
        */
        
        glm::mat4 groundModel(1.0f);
        groundModel = glm::translate(groundModel,GROUND_POSITION);
        groundModel = glm::scale(groundModel,GROUND_SCALE);
        drawModel("cube",groundModel,view,projection,GROUND_COLOR,1.0f);


        drawBird(currentFrame, view, projection);
        drawFlowers(view, projection);


        // TODO: Draw the main slime character.
        // - At least 3 cubes.
        // - Hierarchical transformation required.
        // - Shared root transformation.
        // - Facing direction should be visually distinguishable.
        //
        drawSlime(view, projection);


        // TODO: Draw / animate the magic spell.
        drawMagic(view, projection);


        // TODO: Implement input processing.
        processInput(window, deltaTime);


        glfwSwapBuffers(window);
        glfwPollEvents();
        //glfwWaitEvents() : wait until at least one event arrives, otherwise, sleep.
        //glfwWaitEventsTimeout(0.7) combines the both
        //glfwPostEmptyEvent() provides an empty event for main thread(if sleeping) to wake up.
        //Important concept: Don't assume WHEN will the callbacks happen.

    }


    cleanup();
    glfwTerminate();
    return 0;
}


// ============================================================================
// Window callback
// ============================================================================

void framebuffer_size_callback(
    GLFWwindow* window,
    int width,
    int height) {

    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}


// ============================================================================
// Input
// ============================================================================

void processInput(
    GLFWwindow* window,
    float deltaTime) {

    // We use processInput() in the display loop instead of relying only on
    // keyCallback() because continuous movement requires checking key state
    // every frame.


    // TODO:
    // Controls:
    // - W / S           : Move forward / backward in world space.
    // - A / D           : Move left / right in world space.
    // - SPACE / LSHIFT  : Move up / down.
    //
    // Behavior:
    // - Use deltaTime for frame-rate-independent movement.
    // - Slime facing direction follows horizontal movement direction.
    // - Horizontal movement on the ground should produce hopping.
    // - Hopping is not required while flying.
    // - The character must not move below the ground.

    /*
        
        struct Slime {
            glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
            float angle = 0.0f;
            float speed = 5.0f;

            bool movingHorizontally = false;
            float hopTime = 0.0f;
        } playerSlime;

    */
   playerSlime.movingHorizontally = false;


    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        playerSlime.movingHorizontally = true;
        playerSlime.angle = glm::radians(90.0f);
        playerSlime.position.z -= playerSlime.speed * deltaTime;
        playerSlime.hopTime += deltaTime;
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        playerSlime.movingHorizontally = true;
        playerSlime.angle = glm::radians(270.0f);
        playerSlime.position.z += playerSlime.speed * deltaTime;
        playerSlime.hopTime += deltaTime;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        playerSlime.movingHorizontally = true;
        playerSlime.angle = glm::radians(180.0f);
        playerSlime.position.x -= playerSlime.speed * deltaTime;
        playerSlime.hopTime += deltaTime;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        playerSlime.movingHorizontally = true;
        playerSlime.angle = glm::radians(0.0f);
        playerSlime.position.x += playerSlime.speed * deltaTime;
        playerSlime.hopTime += deltaTime;
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        playerSlime.movingHorizontally = false;
        playerSlime.position.y += playerSlime.speed * deltaTime;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        playerSlime.movingHorizontally = false;
        if(
            playerSlime.position.y - playerSlime.speed * deltaTime <=
            GROUND_POSITION.y + 0.5*GROUND_SCALE.y
        )   playerSlime.position.y = GROUND_POSITION.y + 0.5f *GROUND_SCALE.y;
        else playerSlime.position.y -= playerSlime.speed * deltaTime;
    }
    if(!playerSlime.movingHorizontally){
        playerSlime.hopTime = 0.0f;
    }

    // TODO: Ground boundary collision.
}


void keyCallback(//FIFO
    GLFWwindow* window,
    int key,
    int scancode,
    int action,
    int mods) {

    // The action is one of GLFW_PRESS, GLFW_REPEAT or GLFW_RELEASE.
    // https://www.glfw.org/docs/3.3/input_guide.html

    /*  
        key: key token !!!與US鍵盤綁定，如AZERTY鍵盤的A實際上對應到GLFW_KEY_Q!!!
        scancode: platform_specific regardless of whether it has key token or not, 
        can be requested by calling glfwGetKeyScancode( GLFW_KEY_XX ).
        action:GLFW_PRESS GLFW_REPEAT GLFW_RELEASE

    */

    /*

        當持續按壓某個按鍵時，因為GLFW_PRESS只會觸發一次，(0.5秒之後變為GLFW_REPEAT，每秒固定發30次\
        其實詳細數值由OS決定)，
        所以callback特別適合拿來做單觸發處理；
        反之，如果使用polling，只檢查按鍵狀態，則可以預期連續行為。
    
    */


    if (key == GLFW_KEY_ESCAPE &&
        action == GLFW_PRESS) {

        glfwSetWindowShouldClose(window, true);
    }


    // TODO: Press R to cast the spell.
    //
    // Pressing R while a spell is active should not restart the animation.

    if(key == GLFW_KEY_R && action == GLFW_PRESS){
        if(spell.active == false){
            spell.elapsed = 0.0f;
            spell.active = true;
        }
    }
}


// ============================================================================
// drawModel()
// alpha support is provided.
// ============================================================================

void drawModel(
    std::string type,
    const glm::mat4& model,
    const glm::mat4& view,
    const glm::mat4& projection,
    const glm::vec3& color,
    float alpha) {
    //BAD practice, we should not always pass view and projection.
    shader->set_uniform("projection", projection);
    shader->set_uniform("view", view);
    shader->set_uniform("model", model);
    shader->set_uniform("objectColor", color);
    shader->set_uniform("objectAlpha", alpha);

    if (type == "cube") {
        cube->draw();

    } else if (type == "bird") {
        bird->draw();

    } else if (type == "flower") {
        flower->draw();

    } else if (type == "magic_ball") {
        magicBall->draw();
    }
}


// ============================================================================
// init()
// ============================================================================

void init() {
#if defined(__linux__) || defined(__APPLE__)
    std::string dirShader = "shaders/";
    std::string dirAsset = "asset/";
#else
    std::string dirShader = "shaders\\";
    std::string dirAsset = "asset\\";
#endif

    shader =
        new Shader(
            (dirShader + "easy.vert").c_str(),
            (dirShader + "easy.frag").c_str());

    cube =
        new Object(dirAsset + "cube.obj");

    bird =
        new Object(dirAsset + "bird.obj");

    flower =
        new Object(dirAsset + "flower.obj");

    magicBall =
        new Object(dirAsset + "magic_ball.obj");
}


// ============================================================================
// cleanup()
// ============================================================================

void cleanup() {
    if (shader) {
        delete shader;
        shader = nullptr;
    }

    if (cube) {
        delete cube;
        cube = nullptr;
    }

    if (bird) {
        delete bird;
        bird = nullptr;
    }

    if (flower) {
        delete flower;
        flower = nullptr;
    }

    if (magicBall) {
        delete magicBall;
        magicBall = nullptr;
    }
}


// ============================================================================
// Main character
// ============================================================================

void drawSlime(
    const glm::mat4& view,
    const glm::mat4& projection) {

    glm::mat4 slimeModel(1.0f);


        
    // TODO: Build the shared root transformation.
    //
    // The root should contain the character's world position and facing
    // rotation. All slime parts should be children of this root.
    
    slimeModel = glm::translate(slimeModel,playerSlime.position);
    slimeModel = glm::rotate(slimeModel,playerSlime.angle,glm::vec3(0.0f,1.0f,0.0f));


    // TODO: Slime hopping.
    //
    // Suggested idea:
    // yOffset = H * abs(sin(w * t))
    //
    // Only apply hopping when the slime is moving horizontally on the ground.
    
    if(playerSlime.movingHorizontally == true){
        if(playerSlime.hopTime >= totalHopTime ) playerSlime.hopTime -= totalHopTime;
        float currHopTime = playerSlime.hopTime;
        
        float hopFrac = currHopTime / totalHopTime;
    

        float t = hopFrac * glm::radians(180.0f);

        //playerSlime.position.y = maxHopHeight * glm::sin(t);
        float y_dis = maxHopHeight * glm::sin(t);
        slimeModel = glm::translate(slimeModel , glm::vec3(0.0f,y_dis,0.0f));

    }
        


       

    // TODO: Draw the slime using at least 3 cubes.
    //
    // Reuse the same root matrix for all children.
    // The facing direction must be visually distinguishable.


    //cube [-0.5,0.5]^3

    //body

    glm::mat4 body = slimeModel;
    float scaleFactor = 3.0f; 
    const glm::vec3 body_size(scaleFactor);
    buttomLength = 0.5f *scaleFactor;
    body = glm::translate(body,glm::vec3(0.0f,buttomLength,0.0f));
    body = glm::scale(body,body_size);
    drawModel("cube",body,view,projection,glm::vec3(float(128)/float(255), float(255)/float(255), 0),0.9f);



    return;

}


// ============================================================================
// Bird
// ============================================================================

void drawBird(
    float currentTime,
    const glm::mat4& view,
    const glm::mat4& projection) {

    // TODO: Bird elliptical flight.
    //
    // Base position is the ellipse center:
    //     BIRD_BASE_POSITION
    //
    // Position:
    //     x = cx + rx * cos(t)
    //     z = cz + rz * sin(t)
    //
    // Facing:
    //     vx = -rx * sin(t)
    //     vz =  rz * cos(t)
    //     angle = atan2(vx, vz) ... 代表鳥初始朝+x (atan2特性，順序對調)
    /*
        // You may change the ellipse size and flying speed.
        const float BIRD_RADIUS_X = 10.0f;
        const float BIRD_RADIUS_Z = 4.5f;
        const float BIRD_SPEED = 0.45f;
        
    */
    
    currentTime *= BIRD_SPEED;
    

    float b_x = BIRD_BASE_POSITION.x + BIRD_RADIUS_X/2 * glm::cos(currentTime);
    float b_z = BIRD_BASE_POSITION.z + BIRD_RADIUS_Z * glm::sin(currentTime);
    float b_y = BIRD_BASE_POSITION.y;
    glm::vec3 b_pos = glm::vec3(b_x,b_y,b_z); 

    glm::mat4 birdModel = glm::mat4(1.0f);
    birdModel = glm::translate(birdModel,b_pos);

    float v_x = (-1.0f) * BIRD_RADIUS_X * glm::sin(currentTime);
    float v_z = (1.0f) * BIRD_RADIUS_Z * glm::cos(currentTime);
    
    float angle = atan2(v_x,v_z);

    birdModel = glm::rotate(birdModel,angle,glm::vec3(0.0f,1.0f,0.0f));
    birdModel = glm::scale(birdModel,BIRD_SCALE);

    drawModel("bird",birdModel,view,projection,glm::vec3(1.0f,1.0f,1.0f),1.0f);
    return;
}


// ============================================================================
// Flowers
// ============================================================================

void drawFlowers(
    const glm::mat4& view,
    const glm::mat4& projection) {

    // TODO: Render exactly three flowers.
    //
    // Requirements:
    // - Use FLOWER_POSITIONS.
    // - Use the fixed FLOWER_SCALE.
    // - Use visibly different colors.
    // - When the spell is cast, gradually tilt / fall.
    // - During recovery, gradually return upright.
    // - Rotate around the bottom / base of flower.obj.
    // - Falling direction depends on:
    //
    //       FLOWER_POSITIONS[i] - playerSlime.position
    //
    // - Ignore the vertical component when deciding falling direction.
    // - Each flower should generally fall away from the slime.
    
    glm::vec3 FLOWER_leanArrows[3];
    glm::vec3 FLOWER_rotateAxis[3];

    glm::mat4 modelFlowers[3];

    float currFrac = spell.elapsed / spelllastTime;
    // 0.3 0.3 0.4
    // down : 0.3 ~ (0.3 + 0.06 * 2)
    // up : 0.6 ~ 0.9 
    float angle = 0.0f;

    if(currFrac >= 0.3f && currFrac <= 0.48f){
        angle = (currFrac - 0.3f) * glm::radians(50.0f) / 0.18f;
    }else if(currFrac >= 0.6 && currFrac <= 0.9){
        angle = glm::radians(50.0f)* (1 - (currFrac - 0.6f) / 0.3f);
    }else if(currFrac > 0.48f && currFrac < 0.6f){
        angle = glm::radians(50.0f);
    }

    glm::vec3 colors[3] = {
        glm::vec3(1.0f,0.0f,0.0f),
        glm::vec3(0.0f,1.0f,0.0f),
        glm::vec3(0.0f,0.0f,1.0f)
    };

    for(int i=0;i<3;i++){
        FLOWER_leanArrows[i] = FLOWER_POSITIONS[i]-playerSlime.position;
        FLOWER_rotateAxis[i] = glm::cross(glm::vec3(0.0f,1.0f,0.0f),FLOWER_leanArrows[i]);
        
        modelFlowers[i] = glm::mat4(1.0f);
        modelFlowers[i] = glm::translate(modelFlowers[i],FLOWER_POSITIONS[i]);

        modelFlowers[i] = glm::rotate(modelFlowers[i],angle,FLOWER_rotateAxis[i]);
        modelFlowers[i] = glm::scale(modelFlowers[i],FLOWER_SCALE);
        drawModel("flower",modelFlowers[i],view,projection,colors[i],1.0f);
    }
    
    return;
    
        
}


// ============================================================================
// Spell update
// ============================================================================

void updateSpell(
    float deltaTime) {
    //const float lastTime = 5.0f;
    const float mainOmega = 1.0f;
    const float revolveOmega = 1.0f;
    const float smallOmega = 1.0f;
    // TODO: Update the spell animation state.
    //
    // Required behavior:
    // - Main magic ball self rotation.
    // - Small magic balls self rotation.
    // - Small magic balls orbit around the main ball.
    // - Animation speed / timing are up to you.
    // - End / reset the spell so R can cast again later.

    if(spell.active == true){
        if(spell.elapsed >= spelllastTime){
            spell.active = false;
            spell.elapsed = 0.0f;
            spell.mainSelfAngle = 0.0f;
            spell.orbitAngle = 0.0f;
            spell.smallSelfAngle = 0.0f;
            return;
        }
        spell.elapsed += deltaTime;
        spell.mainSelfAngle += (mainOmega * deltaTime);
        spell.orbitAngle += (revolveOmega * deltaTime);
        spell.smallSelfAngle += (smallOmega * deltaTime);
    }
    return;
}


// ============================================================================
// Magic Casting
// ============================================================================

void drawMagic(
    const glm::mat4& view,
    const glm::mat4& projection) {

    // TODO: Magic Ball appearance.
    //
    // - Press R to cast.
    // - Main + four small magic balls appear above the slime.
    // - alpha: 0 -> 1.
    // - Main ball self-rotates.
    // - Each small ball self-rotates.
    // - Four small balls orbit around the main ball.
    // - Four small balls should use visibly different colors.


    // TODO: Hierarchical transformation.
    //
    // The four small balls must be children of the main magic hierarchy.
    //
    // Suggested structure:
    //
    // M_small =
    //     M_mainParent
    //     * R_orbit
    //     * T_offset
    //     * R_self
    //     * S
    //
    // Do NOT calculate small-ball positions independently in world space.


    // TODO: Move to Front.
    //
    // Before firing:
    // - Move the entire magic-ball hierarchy from above the slime
    //   to the front of the slime.
    // - Main + all four small balls move together.
    // - Rotate the entire hierarchy from horizontal to vertical.
    // - Therefore the small-ball orbit plane changes from
    //   horizontal to vertical.


    // TODO: Magic Beam.
    //
    // - Fire only after the magic hierarchy reaches the front.
    // - Use a cube.
    // - Start from the center of the main magic ball.
    // - Beam forward direction should align with slime facing direction.
    // - Beam length gradually increases.
    // - Use translation + scale.
}
