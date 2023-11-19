#include "engine.h"
#include "debug.h"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <ctime>

using namespace std;
using namespace std::chrono;

const color white(1, 1, 1);
const color black(0, 0, 0);
const color yellow (1,1,0);
const color gray (0.5,0.5,0.5);
const int MAX_NUMBER = 5;
int clickCounter = 0;
int colorCounter = 25;

enum state {start, play, over};
state screen;

Engine::Engine() : keys() {
    this->initWindow();
    this->initShaders();
    this->initShapes();
}

Engine::~Engine() {}

time_point<high_resolution_clock> startTime;
time_point<high_resolution_clock> endTime;

unsigned int Engine::initWindow(bool debug) {
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, false);

    // This creates the window using GLFW.
    // It's a C function, so we have to pass it a pointer to the window variable.
    window = glfwCreateWindow(width, height, "Lights Out", nullptr, nullptr);
    if (window == nullptr) {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    // This sets the OpenGL context to the window we just created.
    glfwMakeContextCurrent(window);

    // Glad is an OpenGL function loader. It loads all the OpenGL functions that are defined by the driver.
    // This is required because OpenGL is a specification, not an implementation.
    // The driver is the implementation of OpenGL that is installed on your computer.
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }

    // OpenGL configuration
    // This defines the size of the area OpenGL should render to.
    glViewport(0, 0, width, height);
    // This enables depth testing which prevents triangles from overlapping.
    glEnable(GL_BLEND);
    // Alpha blending allows for transparent backgrounds.
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glfwSwapInterval(1);
    return 0;
}

void Engine::initShaders() {
    // load shader manager
    shaderManager = make_unique<ShaderManager>();

    // Load shader into shader manager and retrieve it
    shapeShader = this->shaderManager->loadShader("../res/shaders/shape.vert",
                                                  "../res/shaders/shape.frag",
                                                  nullptr, "shape");

    textShader = shaderManager->loadShader("../res/shaders/text.vert", "../res/shaders/text.frag", nullptr, "text");
    fontRenderer = make_unique<FontRenderer>(shaderManager->getShader("text"), "../res/fonts/MxPlus_IBM_BIOS.ttf", 24);

    // Set uniforms
    textShader.use().setVector2f("vertex", vec4(100, 100, .5, .5));
    // Set uniforms that never change
    shapeShader.use().setMatrix4("projection", this->PROJECTION);
}

void Engine::initShapes() {
    int numRows = 5; // Define the number of rows
    int numCols = 5; // Define the number of columns
    int squareSize = 100; // Define the size of each square
    for (int i = 0; i < numRows; ++i) {
        for (int j = 0; j < numCols; ++j) {
            int x = 100 + j * squareSize;
            int y = 100 + i * squareSize;
            underLay.push_back(make_unique<Rect>(shapeShader, vec2(x, y), vec2(90, 90), color(yellow)));
        }
    }

    // Define the grid properties
    user = make_unique<Rect>(shapeShader, vec2(0, 0), vec2(10, 10), white);

    // Use current time as seed for random generator
    srand(time(0));

    for (int i = 0; i < numRows; ++i) {
        for (int j = 0; j < numCols; ++j) {
            int x = 100 + j * squareSize;
            int y = 100 + i * squareSize;
            // Create a square
            square = make_unique<Rect>(shapeShader, vec2(x, y), vec2(80, 80), color(gray));
            shapes.push_back(move(square));
        }
    }
    for(int i = 0; i < MAX_NUMBER; i++) {
        int randomNum = rand() % 25;
        unique_ptr<Rect> &b = shapes[randomNum];
        // If a square is under the mouse cursor, toggle its color
        if ((b->getColor4() == yellow.vec)) {
            b->setColor(color(0.5, 0.5, 0.5, 1)); // Turn yellow to gray
            colorCounter++;
        } else if ((b->getColor4() == gray.vec)) {
            b->setColor(color(1, 1, 0, 1)); // Turn gray to yellow
            colorCounter--;
        }

        // Calculate the row and column of the clicked square
        int row = (b->getPosY() - 100) / 100;
        int col = (b->getPosX() - 100) / 100;

        for (const unique_ptr<Rect> &neighbor : shapes) {
            int neighborRow = (neighbor->getPosY() - 100) / 100;
            int neighborCol = (neighbor->getPosX() - 100) / 100;

            if ((neighborRow == row - 1 && neighborCol == col) ||  // Above
                (neighborRow == row + 1 && neighborCol == col) ||  // Below
                (neighborRow == row && neighborCol == col + 1) ||  // Right
                (neighborRow == row && neighborCol == col - 1))    // Left
            {
                if (neighbor->getColor4() == yellow.vec) {
                    neighbor->setColor(color(0.5, 0.5, 0.5, 1)); // Turn yellow to gray
                    colorCounter++;
                } else if (neighbor->getColor4() == gray.vec) {
                    neighbor->setColor(color(1, 1, 0, 1)); // Turn gray to yellow
                    colorCounter--;
                }
            }
        }
    }
}

void Engine::processInput() {
    // Set keys to true if pressed, false if released
    for (int key = 0; key < 1024; ++key) {
        if (glfwGetKey(window, key) == GLFW_PRESS)
            keys[key] = true;
        else if (glfwGetKey(window, key) == GLFW_RELEASE)
            keys[key] = false;
    }

    if (screen == start && keys[GLFW_KEY_S])
        screen = play;

    // Close window if escape key is pressed
    if (keys[GLFW_KEY_ESCAPE])
        glfwSetWindowShouldClose(window, true);

    // Mouse position saved to check for collisions
    glfwGetCursorPos(window, &MouseX, &MouseY);
    // Update mouse rect to follow mouse
    MouseY = height - MouseY; // make sure mouse y-axd
    bool mousePressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    if (screen == play && mousePressedLastFrame && !mousePressed) {
        for (const unique_ptr<Rect> &b : shapes) {
            if (b->isOverlapping(*user)) {
                // If a square is under the mouse cursor, toggle its color
                if ((b->getColor4() == yellow.vec)) {
                    b->setColor(color(0.5, 0.5, 0.5, 1)); // Turn yellow to gray
                    ++colorCounter;
                } else if ((b->getColor4() == gray.vec)) {
                    b->setColor(color(1, 1, 0, 1)); // Turn gray to yellow
                    --colorCounter;
                }

                // Calculate the row and column of the clicked square
                int row = (b->getPosY() - 100) / 100;
                int col = (b->getPosX() - 100) / 100;

                // getting number of mouse clicks
                ++clickCounter;

                // Check and change the color of squares above, below, to the right, and to the left
                for (const unique_ptr<Rect> &neighbor : shapes) {
                    int neighborRow = (neighbor->getPosY() - 100) / 100;
                    int neighborCol = (neighbor->getPosX() - 100) / 100;
                    if ((neighborRow == row - 1 && neighborCol == col) ||  // Above
                        (neighborRow == row + 1 && neighborCol == col) ||  // Below
                        (neighborRow == row && neighborCol == col + 1) ||  // Right
                        (neighborRow == row && neighborCol == col - 1)){   // Left
                        if (neighbor->getColor4() == yellow.vec) {
                            neighbor->setColor(color(0.5, 0.5, 0.5, 1)); // Turn yellow to gray
                            colorCounter++;
                        } else if (neighbor->getColor4() == gray.vec) {
                            neighbor->setColor(color(1, 1, 0, 1)); // Turn gray to yellow
                            colorCounter--;
                        }
                    }
                }
            }
        }
    }

    // Save mousePressed for next frame
    mousePressedLastFrame = mousePressed;

    user->setPosX(MouseX);
    user->setPosY(MouseY);

    for (const unique_ptr<Rect>& b : underLay) {
        if (b->isOverlapping(*user)) {
            b->setColor(color(1,0,0,1));
        } else {
            b->setColor(color(1,1,0,0));
        }
    }

    // Calculate delta time
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // Close window if escape key is pressed
    if (keys[GLFW_KEY_ESCAPE])
        glfwSetWindowShouldClose(window, true);

    // Mouse position saved to check for collisions
    glfwGetCursorPos(window, &MouseX, &MouseY);

    // Update mouse rect to follow mouse
    MouseY = height - MouseY; // make sure mouse y-axis isn't flipped

    user->setPosX(MouseX);
    user->setPosY(MouseY);

    if (colorCounter == 25) {
        screen = over;
    }
}

void Engine::update() {
    // Calculate delta time
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // This function polls for events like keyboard input and mouse movement
    // It needs to be called every frame
    // Without this function, the window will freeze and become unresponsive
    glfwPollEvents();

    glfwPollEvents();
}

void Engine::render() {
    //changes background
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Set shader to use for all shapes
    shapeShader.use();

    // Render differently depending on screen
    switch (screen) {
        case start: {
            string message = "Press s to start";
            string message2 = "Rules:";
            string message3 = "Game consists 5x5 grid of lights. ";
            string message4 = "Pressing any lights will toggle";
            string message5 =  "four adjacent lights.";
            string message6 = "Goal is to switch all lights off";
            // (12 * message.length()) is the offset to center text.
            // 12 pixels is the width of each character scaled by 1.
            this->fontRenderer->renderText(message, width/2 - (5 * message.length()) , height/2, 1, vec3{1, 1, 1});
            this->fontRenderer->renderText(message2, 10, height/2.5, 0.75, vec3{1, 1, 1});
            this->fontRenderer->renderText(message3, 10, height/3, 0.75, vec3{1, 1, 1});
            this->fontRenderer->renderText(message4, 10, height/3.5, 0.75, vec3{1, 1, 1});
            this->fontRenderer->renderText(message5, 10, height/4, 0.75, vec3{1, 1, 1});
            this->fontRenderer->renderText(message6, 10, height/6, 0.75, vec3{1, 1, 1});

            startTime = high_resolution_clock::now();
            break;
        }
        case play: {
            string message = "Number Of Clicks: " + to_string(clickCounter);
            for (const unique_ptr<Rect>& b : underLay) {
                b->setUniforms();
                b ->draw();
            }
            for (const unique_ptr<Rect>& s : shapes) {
                s->setUniforms();
                s->draw();
            }

            this->fontRenderer->renderText(message, width/2 - (5 * message.length()) , 545, 1, vec3{1, 1, 1});
            endTime = high_resolution_clock::now();
            break;
        }
        case over: {
            auto duration = duration_cast<seconds>(endTime - startTime).count();
            string strDuration = "Time: " + to_string(duration) + " Seconds";
            this->fontRenderer->renderText(strDuration, width/2 - (5 * strDuration.length()) , height/4, 1, vec3{1, 1, 1});
            string message = "You win!";
            // Display the message on the screen
            this->fontRenderer->renderText(message, width/2 - (12 * message.length()), height/2 - 50, 1, vec3{1, 1, 1});
            break;
        }
    }
    glfwSwapBuffers(window);
}

bool Engine::shouldClose() {
    return glfwWindowShouldClose(window);
};