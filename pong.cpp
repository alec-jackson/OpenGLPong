// Use C++ standard libaries
#include <cstdlib>
#include <iostream>
#include <time.h>
#include <chrono>
using namespace std;

// Use glew
#include <GL/glew.h>

// Use SDL for windowing
#include <SDL2/SDL.h>
#include "SDL2/SDL_mixer.h"

// Use glm for matrix
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

// Add loadshaders file
#include "common/loadshaders.h"

// Begin Structs
struct attributes {
    GLfloat coord3d[3];
    GLfloat v_color[3];
};

// Begin Globals
struct attributes triangle_attributes[] = {
  {{ 0.8,  0.8, 0.0}, {1.0, 1.0, 1.0}},
  {{-0.8, -0.8, 0.0}, {1.0, 1.0, 1.0}},
  {{ 0.8, -0.8, 0.0}, {1.0, 1.0, 1.0}},
  {{ 0.8,  0.8, 0.0}, {1.0, 1.0, 1.0}},
  {{-0.8, -0.8, 0.0}, {1.0, 1.0, 1.0}},
  {{-0.8,  0.8, 0.0}, {1.0, 1.0, 1.0}}
};

typedef struct ballStats
{
	GLfloat xpos = 0;
	GLfloat ypos = 0;
	GLfloat xSpeed = 0;
	GLfloat ySpeed = 0;
}ballStats;

//Analog joystick dead zone
const int JOYSTICK_DEAD_ZONE = 8000;

const char* waveFileNames[] =
{
"AudioResources/Kick-Drum-1.wav",
"AudioResources/Electronic-Tom-1.wav",
};

Mix_Chunk* sound[2];

glm::mat4 View;
glm::mat4 Projection;
glm::mat4 Model;

mat4 MVP;
mat4 MVP2;
mat4 MVP3;

GLuint MatrixID;

mat4 p1translateMatrix;
mat4 p2translateMatrix;
mat4 balltranslateMatrix;
mat4 paddleScaleMatrix;
mat4 ballScaleMatrix;


GLfloat p1VertPosition = 0;
GLfloat p2VertPosition = 0;

int gameState = 0;

// Begin GL and SDL
class pongGraphics{
  public:
    void run(){
      initOGL();
      graphicsLoop();
      cleanup();
    }

  private:
    SDL_Window* window;
    GLenum glew_status;
    GLuint program;

    //Game Controller 1 handler
    SDL_GameController* gGameController1 = NULL;
    SDL_GameController* gGameController2 = NULL;

    GLuint paddleLeft, paddleRight;
    GLuint vboBall, vboBallColors;
    GLint attribute_coord3d, attribute_v_color;

    GLint uniform_fade;

    SDL_Event input;

    int audioID;

    bool runFlag;
    bool holdFlag;
    bool isTwoPlayer;
    timespec* delayTime;

    ballStats *ball;
    int p1Score;
    int p2Score;

    void initOGL(){
      initSDL();
      initController();
      initGlew();
      initAudio();
      initTransparency();
      initShaders();
      createVertexBuffers();
      //initUniformVariables();
      initGlobals();
    }

    void initSDL(){
      /* SDL-related initialising functions */
    	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK);
    	window = SDL_CreateWindow("Pong - Square Color Test",
    		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    		960, 720,
    		SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);

      if(window == NULL) {
        cerr << "Error: cannot create instance of window: " << SDL_GetError() << '\n';
        exit(1);
      }

      SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
      //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
      SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 1);
    	if (SDL_GL_CreateContext(window) == NULL) {
    		cerr << "Error: SDL_GL_CreateContext: " << SDL_GetError() << endl;
    		exit(1);
    	}

    	SDL_GL_CreateContext(window);
    }

    void initController(){
      //Check for joysticks
      isTwoPlayer = false;
      int joyFlag = SDL_NumJoysticks();
      printf("Number of Joysticks: %d\n", joyFlag);
      if( joyFlag < 1)
      {
          printf( "Warning: No joysticks connected!\n" );
      } else if (joyFlag > 1){
        gGameController1 = SDL_GameControllerOpen( 0 );
        gGameController2 = SDL_GameControllerOpen( 1 );
        if(gGameController1 == 0 || gGameController2 == 0){
          printf("Could not load multiple controllers\n");
          if(gGameController1 || gGameController2){
            if(gGameController2){
              gGameController1 = gGameController2;
            }
            printf("Reverting to single player mode\n" );
          } else {
            exit(1);
          }
        } else {
          isTwoPlayer = true;
        }
      }else {
        //Load joystick
        for(int i = 0; i < joyFlag; i++){
            if(SDL_IsGameController(i)){
              gGameController1 = SDL_GameControllerOpen( i );
              if( gGameController1 == NULL )
              {
                  printf( "Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError() );
              } else { printf("Enabled Gamepad\n"); return;}
            }
          }
          printf("Only JoySticks Connected, No Supported Mapping\n");
        }
    }

    void initGlew(){
      // Extension wrangler initialising
      glew_status = glewInit();
      if (glew_status != GLEW_OK && !GLEW_VERSION_2_0) {
      		cerr << "Error: glewInit: " << glewGetErrorString(glew_status) << endl;
      		exit(EXIT_FAILURE);
      	}
    }

    void initAudio(){
      // Set up the audio stream
      audioID = Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 512);
      if( audioID < 0 )
      {
          fprintf(stderr, "Unable to open audio: %s\n", SDL_GetError());
          exit(-1);
      }

      audioID = Mix_AllocateChannels(4);
      if( audioID < 0 )
      {
          fprintf(stderr, "Unable to allocate mixing channels: %s\n", SDL_GetError());
          exit(-1);
      }

      // Load waveforms
      for( int i = 0; i < (int)(sizeof(sound)/sizeof(sound[0])); i++ )
      {
          sound[i] = Mix_LoadWAV(waveFileNames[i]);
          if( sound[i] == NULL )
          {
              fprintf(stderr, "Unable to load wave file: %s\n", waveFileNames[i]);
          }
      }

    }

    void initShaders(){
      GLuint vs, fs;
      vs = glCreateShader(GL_VERTEX_SHADER);

      if ((vs = create_shader("shaders/vertexshader", GL_VERTEX_SHADER))   == 0) exit(1);
    	if ((fs = create_shader("shaders/fragmentshader", GL_FRAGMENT_SHADER)) == 0) exit(1);

      GLint link_ok = GL_FALSE;
      program = glCreateProgram();
    	glAttachShader(program, vs);
    	glAttachShader(program, fs);
    	glLinkProgram(program);
    	glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
    	if (!link_ok) {
    		cerr << "Error in glLinkProgram" << endl;
        print_log(program);
    		exit(1);
    	}

      const char* attribute_name = "coord3d";
    	attribute_coord3d = glGetAttribLocation(program, attribute_name);
    	if (attribute_coord3d == -1) {
    		cerr << "Could not bind attribute " << attribute_name << endl;
    		exit(1);
    	}
    }

    void createVertexBuffers(){
      glGenBuffers(1, &vboBall);
    	glBindBuffer(GL_ARRAY_BUFFER, vboBall);

      glGenBuffers(1, &paddleLeft);
    	glBindBuffer(GL_ARRAY_BUFFER, paddleLeft);

      glGenBuffers(1, &paddleRight);
    	glBindBuffer(GL_ARRAY_BUFFER, paddleRight);

      glGenBuffers(1, &vboBallColors);
      glBindBuffer(GL_ARRAY_BUFFER, vboBallColors);
      glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_attributes), triangle_attributes, GL_STATIC_DRAW);

      const char* attribute_name2 = "v_color";
      attribute_v_color = glGetAttribLocation(program, attribute_name2);
      if (attribute_v_color == -1) {
        cerr << "Could not bind attribute " << attribute_name2 << endl;
        exit(1);
      }
    }

    void initTransparency(){
      // Enable Alpha
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void initUniformVariables(){
      const char* uniform_name;
      uniform_name = "fade";
      uniform_fade = glGetUniformLocation(program, uniform_name);
      if (uniform_fade == -1) {
        cerr << "Could not bind uniform_fade " << uniform_name << endl;
        exit(1);
      }
      uniform_fade = 1.0f;
    }

    void initGlobals(){
      View = glm::lookAt(
    		glm::vec3(0,0,5), // Camera is at (4,3,3), in World Space
    		glm::vec3(0,0,0), // and looks at the origin
    		glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
    	);
      // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
      Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
      Model = glm::mat4(1.0f);

      ball = (ballStats *)malloc(sizeof(ballStats));

      ball -> xpos = 0;
    	ball -> ypos = 0;
    	ball -> xSpeed = 0;
    	ball -> ySpeed = 0;

      p1Score = 0;
      p2Score = 0;

      holdFlag = 0;
      delayTime = (timespec *)malloc(sizeof(timespec));
      delayTime -> tv_sec = 0;
      delayTime -> tv_nsec = 400000000;

      MatrixID = glGetUniformLocation(program, "MVP");
    }

    void cleanup(){
      glDeleteProgram(program);
      SDL_GameControllerClose( gGameController1 );
      glDeleteBuffers(1, &vboBall);
      glDeleteBuffers(1, &paddleLeft);
      glDeleteBuffers(1, &paddleRight);
      Mix_CloseAudio();
      free(delayTime);
      free(ball);
    }

    void render(){


      glEnableVertexAttribArray(attribute_v_color);

      // Clear Background to white;
      glClearColor(0.0, 0.0, 0.0, 1.0);
    	glClear(GL_COLOR_BUFFER_BIT);
      glUseProgram(program);




      // Begin Color Buffering
  		glBindBuffer(GL_ARRAY_BUFFER, vboBallColors);

      glVertexAttribPointer(
        attribute_v_color, // attribute
        3,                 // number of elements per vertex, here (r,g,b)
        GL_FLOAT,          // the type of each element
        GL_FALSE,          // take our values as-is
        sizeof(struct attributes), // no extra data between each position
        (GLvoid*) offsetof(struct attributes, v_color) // offset of first element
      );

      glEnableVertexAttribArray(attribute_coord3d);
      glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
      glBindBuffer(GL_ARRAY_BUFFER, vboBall);
      glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_attributes), triangle_attributes, GL_STATIC_DRAW);

      // Describe our vertices array to OpenGL
      glVertexAttribPointer(
    		attribute_coord3d, // attribute
    		3,                 // number of elements per vertex, here (x,y)
    		GL_FLOAT,          // the type of each element
    		GL_FALSE,          // take our values as-is
    		sizeof(struct attributes), // no extra data between each position
    		0  // pointer to the C
      );

      //Push each element in buffer_vertices to the vertex shader
      glDrawArrays(GL_TRIANGLES, 0, 3 * 2);

      glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP3[0][0]);

      glBindBuffer(GL_ARRAY_BUFFER, paddleLeft);
      glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_attributes), triangle_attributes, GL_STATIC_DRAW);

      // Describe our vertices array to OpenGL
      glVertexAttribPointer(
    		attribute_coord3d, // attribute
    		3,                 // number of elements per vertex, here (x,y)
    		GL_FLOAT,          // the type of each element
    		GL_FALSE,          // take our values as-is
    		sizeof(struct attributes), // no extra data between each position
    		0  // pointer to the C
      );

      //Push each element in buffer_vertices to the vertex shader
      glDrawArrays(GL_TRIANGLES, 0, 3 * 2);

      glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP2[0][0]);

      glBindBuffer(GL_ARRAY_BUFFER, paddleRight);
      glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_attributes), triangle_attributes, GL_STATIC_DRAW);

      // Describe our vertices array to OpenGL
      glVertexAttribPointer(
    		attribute_coord3d, // attribute
    		3,                 // number of elements per vertex, here (x,y)
    		GL_FLOAT,          // the type of each element
    		GL_FALSE,          // take our values as-is
    		sizeof(struct attributes), // no extra data between each position
    		0  // pointer to the C
      );

      //Push each element in buffer_vertices to the vertex shader
      glDrawArrays(GL_TRIANGLES, 0, 3 * 2);

      glDisableVertexAttribArray(attribute_coord3d);
      glDisableVertexAttribArray(attribute_v_color);

      // Send off to SDL
      SDL_GL_SwapWindow(window);
    }

    void logic() {

      if(gameState == 0){
        p1translateMatrix = translate(glm::mat4(1.0f), glm::vec3(-2.4f, 0.0f, 0.0f));
        p2translateMatrix = translate(glm::mat4(1.0f), glm::vec3(2.4f, 0.0f, 0.0f));
        balltranslateMatrix = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f));
        paddleScaleMatrix = scale(mat4(1.0f), vec3(0.05f, 0.2f, 1.0f));
        ballScaleMatrix = scale(mat4(1.0f), vec3(0.05f, 0.05f, 1.0f));

        const Uint8* keystate = SDL_GetKeyboardState(NULL);
        Uint8 p1StartCheck = SDL_GameControllerGetButton(gGameController1, SDL_CONTROLLER_BUTTON_A );
        Uint8 p2StartCheck = 0;
        if(isTwoPlayer){p2StartCheck = SDL_GameControllerGetButton(gGameController2, SDL_CONTROLLER_BUTTON_A );}

        if(keystate[SDL_SCANCODE_SPACE] || p1StartCheck || p2StartCheck){
          gameState = 1;
          printf("Starting Round\n" );
        }

      } else if(gameState == 1){
        ballControl();
        p1translateMatrix = translate(glm::mat4(1.0f), glm::vec3(-2.4f, p1VertPosition, 0.0f));
        p2translateMatrix = translate(glm::mat4(1.0f), glm::vec3(2.4f, p2VertPosition, 0.0f));
        balltranslateMatrix = translate(mat4(1.0f), vec3(ball -> xpos, ball -> ypos, 0.0f));
        paddleScaleMatrix = scale(mat4(1.0f), vec3(0.05f, 0.2f, 1.0f));
        ballScaleMatrix = scale(mat4(1.0f), vec3(0.05f, 0.05f, 1.0f));
      }

      MVP = Projection * Model * (View * balltranslateMatrix * ballScaleMatrix);
      MVP2 = Projection * Model * (View * p1translateMatrix * paddleScaleMatrix);
      MVP3 = Projection * Model * (View * p2translateMatrix * paddleScaleMatrix);

    }

    void getInput(){
      const Uint8* keystate = SDL_GetKeyboardState(NULL);

      Sint16 controllerLeftState = SDL_GameControllerGetAxis(gGameController1, SDL_CONTROLLER_AXIS_LEFTY );

      GLfloat maxdistance = 1.85;

      if(isTwoPlayer){
        Sint16 controller2State = SDL_GameControllerGetAxis(gGameController2, SDL_CONTROLLER_AXIS_LEFTY );

        if((keystate[SDL_SCANCODE_W] || controllerLeftState < -JOYSTICK_DEAD_ZONE ) && p1VertPosition < maxdistance){p1VertPosition += 0.05;}
        if((keystate[SDL_SCANCODE_S] || controllerLeftState > JOYSTICK_DEAD_ZONE) && p1VertPosition > maxdistance * -1.0){p1VertPosition -= 0.05;}
        if((keystate[SDL_SCANCODE_UP] || controller2State < -JOYSTICK_DEAD_ZONE) && p2VertPosition < maxdistance * 1.0){p2VertPosition += 0.05;}
        if((keystate[SDL_SCANCODE_DOWN] || controller2State > JOYSTICK_DEAD_ZONE ) && p2VertPosition > maxdistance * -1.0){p2VertPosition -= 0.05;}
        if(keystate[SDL_SCANCODE_ESCAPE]){runFlag = false;}
      } else {
        Sint16 controllerRightState = SDL_GameControllerGetAxis(gGameController1, SDL_CONTROLLER_AXIS_RIGHTY );

        if((keystate[SDL_SCANCODE_W] || controllerLeftState < -JOYSTICK_DEAD_ZONE ) && p1VertPosition < maxdistance){p1VertPosition += 0.05;}
        if((keystate[SDL_SCANCODE_S] || controllerLeftState > JOYSTICK_DEAD_ZONE) && p1VertPosition > maxdistance * -1.0){p1VertPosition -= 0.05;}
        if((keystate[SDL_SCANCODE_UP] || controllerRightState < -JOYSTICK_DEAD_ZONE) && p2VertPosition < maxdistance * 1.0){p2VertPosition += 0.05;}
        if((keystate[SDL_SCANCODE_DOWN] || controllerRightState > JOYSTICK_DEAD_ZONE ) && p2VertPosition > maxdistance * -1.0){p2VertPosition -= 0.05;}
        if(keystate[SDL_SCANCODE_ESCAPE]){runFlag = false;}
      }

    }

    void ballControl(){
      if(p1Score > 4 || p2Score > 4){
        return;
      }
    	if(ball -> xSpeed){
    		if((ball -> ypos > 1.85f) | (ball -> ypos < -1.85f)){
    			ball -> ySpeed *= -1.0f;
    		}
    		if(ball -> xpos > 2.35f && ball -> xSpeed > 0){
    			if(ball -> xpos > 2.55f){
    				p1Score += 1;
    				ballReset();
            Mix_PlayChannel(-1, sound[1], 0);
            return;
    			} else if (ball -> ypos < p2VertPosition + 0.2f && ball -> ypos > p2VertPosition - 0.2f){
            float scuffedDirection = 0.3 * (float)(rand() % 15) / 50;
            ball -> xSpeed = (ball -> xSpeed * -1.0f) - 0.001f;
            ball -> ySpeed = scuffedDirection;
            Mix_PlayChannel(-1, sound[0], 0);
    				//fprintf(stderr, "%f\n", ball -> xSpeed);
    			} else {
    				ball -> xpos += ball -> xSpeed;
    				ball -> ypos += ball -> ySpeed;
    			}
    		} else if(ball -> xpos < -2.35f && ball -> xSpeed < 0) {
    			if(ball -> xpos < -2.55f){
    				p2Score += 1;
    				ballReset();
            Mix_PlayChannel(-1, sound[1], 0);
    			} else if (ball -> ypos < p1VertPosition + 0.2f && ball -> ypos > p1VertPosition - 0.2f){
            float scuffedDirection = 0.3 * (float)(rand() % 15) / 50;
    				ball -> xSpeed = (ball -> xSpeed * -1.0f) + 0.001f;
            ball -> ySpeed = scuffedDirection;
            Mix_PlayChannel(-1, sound[0], 0);
    				fprintf(stderr, "%f\n", ball -> xSpeed);
    			} else {
    				ball -> xpos += ball -> xSpeed;
    				ball -> ypos += ball -> ySpeed;
    			}
    		} else {
    			ball -> xpos += ball -> xSpeed;
    			ball -> ypos += ball -> ySpeed;
    		}
    	} else {
    		int scuffedDirection = rand() % 2;
    		if(scuffedDirection){
    			ball -> xSpeed = 0.03f + (0.005f * (float)(p1Score + p2Score));
    		} else {
    			ball -> xSpeed = -0.03f - (0.005f * (float)(p1Score + p2Score));
    		}
    		scuffedDirection = rand() % 10;
    		printf("%d\n", scuffedDirection);
    		ball -> ySpeed = (float)scuffedDirection / 200;
    		printf("%f\n", ball -> ySpeed);
    		if(rand() % 2){
    			ball -> ySpeed *= -1.0f;
    		}
    	}
    }

    void ballReset(){
    	if(p1Score < 5 && p2Score < 5){
    		ball -> xpos = 0;
    		ball -> ypos = 0;
    		ball -> xSpeed = 0;
    		ball -> ySpeed = 0;
        p1VertPosition = 0;
        p2VertPosition = 0;
        holdFlag = 1;
    	} else {
    		if(p1Score > 4){
    			printf("Player 1 Wins\n");
    		} else {
    			printf("Player 2 Wins\n");
    		}
        gameState = 0;
        p1Score = 0;
        p2Score = 0;
    	}
    }

    void graphicsLoop(){
      runFlag = true;
      gameState = 0;
      do{
        logic();
        render();
        getInput();
        SDL_PollEvent(&input);
      }while(input.type != SDL_QUIT && runFlag);
    }
};

int main(int argc, char* argv[]) {
  pongGraphics app;

  app.run();
  return 0;
}
