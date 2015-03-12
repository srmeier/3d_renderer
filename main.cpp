/* Prototype
g++ main.cpp -o test.exe -I./include -L./lib -lmingw32 -lglew32 -lopengl32 -lSDL2main -lSDL2
running on GCC 4.8.1, SDL 2.0.1, GLEW 1.10.0, and GLM 0.9.6.1
*/

//-----------------------------------------------------------------------------
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/vector_angle.hpp"
#include "glm/gtx/perpendicular.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "GL/glew.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_opengl.h"

#include "auxiliary.h"
#include "mesh.h"

//-----------------------------------------------------------------------------
int SDL_main(int argc, char *argv[]) {
	startGame();

	/* TESTING */
	// ========================================================================

	float cBoxW = 500.0f;

	int swing_frame = 0;
	SDL_bool swinging = SDL_FALSE;
	SDL_bool swing_forward = SDL_TRUE;

	glm::vec3 m_position(0.0f, 0.0f, 0.0f);
	glm::vec3 m_direction(0.0f, 0.0f, 1.0f);

	glm::mat4 model;
	float angle = (float) M_PI/100.0f;
	model = glm::translate(model, glm::vec3(0.0f, -15.0f, 0.0f));

	GLint uniModel = glGetUniformLocation(shaderProgram0, "model");
	glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

	float pDist = 0.0f;
	glm::mat4 view = glm::lookAt(
		glm::vec3(pDist, pDist, pDist),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	GLint uniView = glGetUniformLocation(shaderProgram0, "view");
	glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

	glm::mat4 proj = glm::perspective(45.0f, 800.0f/600.0f, 0.5f, 500.0f);

	GLint uniProj = glGetUniformLocation(shaderProgram0, "proj");
	glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

	// NOTE: test room
	Mesh* testRoom[9];

	testRoom[0] = new Mesh("floor.obj", "tex03.bmp");
	testRoom[0]->glLoad();

	// NOTE: sword mesh
	Mesh* swordMesh = new Mesh("sword.obj", "tex00.bmp");
	swordMesh->glLoad();

	/* END TESTING */
	// ========================================================================

	// NOTE: start running the game
	running = SDL_TRUE;

	while(running) {
		pollInput();

		// NOTE: clear the screen buffer
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		/* TESTING */
		// ====================================================================

		// NOTE: move around with left dpad
		if(pl_input.up) {
			m_position += m_direction;

			if((m_position.x>cBoxW||m_position.x<-cBoxW) || (m_position.z>cBoxW||m_position.z<-cBoxW))
				m_position -= m_direction;
		}

		if(pl_input.down) {
			m_position -= m_direction;

			if((m_position.x>cBoxW||m_position.x<-cBoxW) || (m_position.z>cBoxW||m_position.z<-cBoxW))
				m_position += m_direction;
		}

		if(pl_input.left) {
			m_direction = glm::rotate(m_direction, angle, glm::vec3(0.0f, 1.0f, 0.0f));
		}

		if(pl_input.right) {
			m_direction = glm::rotate(m_direction, -angle, glm::vec3(0.0f, 1.0f, 0.0f));
		}

		// NOTE: close on start button
		uint8_t start_bnt = SDL_GameControllerGetButton(p1_controller, SDL_CONTROLLER_BUTTON_START);
		if(start_bnt) running = SDL_FALSE;

		/*
		// NOTE: swing sword on Y button
		uint8_t y_bnt = SDL_GameControllerGetButton(p1_controller, SDL_CONTROLLER_BUTTON_Y);
		if(y_bnt && !swinging) {
			swing_frame = 0;
			swinging = SDL_TRUE;
			swing_forward = SDL_TRUE;
		}
		*/

		uint8_t r_trig = SDL_GameControllerGetButton(p1_controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
		if(r_trig && !swinging) {
			swing_frame = 0;
			swinging = SDL_TRUE;
			swing_forward = SDL_TRUE;
		}

		/*
		// NOTE: strafe on left trigger
		uint8_t l_trig = SDL_GameControllerGetButton(p1_controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
		if(l_trig) {
			m_position -= glm::cross(m_direction, glm::vec3(0.0f, 1.0f, 0.0f));

			if((m_position.x>cBoxW||m_position.x<-cBoxW) || (m_position.z>cBoxW||m_position.z<-cBoxW))
				m_position += glm::cross(m_direction, glm::vec3(0.0f, 1.0f, 0.0f));
		}

		// NOTE: strafe on right trigger
		uint8_t r_trig = SDL_GameControllerGetButton(p1_controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
		if(r_trig) {
			m_position += glm::cross(m_direction, glm::vec3(0.0f, 1.0f, 0.0f));

			if((m_position.x>cBoxW||m_position.x<-cBoxW) || (m_position.z>cBoxW||m_position.z<-cBoxW))
				m_position -= glm::cross(m_direction, glm::vec3(0.0f, 1.0f, 0.0f));
		}
		*/

		// NOTE: get axis numbers
		int x_axisl = SDL_GameControllerGetAxis(p1_controller, SDL_CONTROLLER_AXIS_LEFTX);
		int y_axisl = SDL_GameControllerGetAxis(p1_controller, SDL_CONTROLLER_AXIS_LEFTY);

		int x_axisr = SDL_GameControllerGetAxis(p1_controller, SDL_CONTROLLER_AXIS_RIGHTX);
		int y_axisr = SDL_GameControllerGetAxis(p1_controller, SDL_CONTROLLER_AXIS_RIGHTY);

		// NOTE: easy movement with the left stick
		if(abs(x_axisl)>10000) {
			//m_direction = glm::rotate(m_direction, (float) -x_axisl/1000000.0f, glm::vec3(0.0f, 1.0f, 0.0f));

			if(x_axisl>0) {
				m_position += glm::cross(m_direction, glm::vec3(0.0f, 1.0f, 0.0f));

				if((m_position.x>cBoxW||m_position.x<-cBoxW) || (m_position.z>cBoxW||m_position.z<-cBoxW))
					m_position -= glm::cross(m_direction, glm::vec3(0.0f, 1.0f, 0.0f));
			}

			if(x_axisl<0) {
				m_position -= glm::cross(m_direction, glm::vec3(0.0f, 1.0f, 0.0f));

				if((m_position.x>cBoxW||m_position.x<-cBoxW) || (m_position.z>cBoxW||m_position.z<-cBoxW))
					m_position += glm::cross(m_direction, glm::vec3(0.0f, 1.0f, 0.0f));
			}
		}

		if(abs(y_axisl)>10000) {
			if(y_axisl>0) {
				m_position -= m_direction;

				if((m_position.x>cBoxW||m_position.x<-cBoxW) || (m_position.z>cBoxW||m_position.z<-cBoxW))
					m_position += m_direction;
			}

			if(y_axisl<0) {
				m_position += m_direction;

				if((m_position.x>cBoxW||m_position.x<-cBoxW) || (m_position.z>cBoxW||m_position.z<-cBoxW))
					m_position -= m_direction;
			}
		}

		// NOTE: right axis
		if(abs(x_axisr)>10000) {
			m_direction = glm::rotate(m_direction, (float) -x_axisr/1000000.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		}

		static glm::vec3 tempV;
		static float tempAng = 0.0f;

		if(abs(y_axisr)>10000) {
			tempAng += (float) -y_axisr/1000000.0f;
		} tempV = glm::rotate(m_direction, tempAng, glm::cross(m_direction, glm::vec3(0.0f, 1.0f, 0.0f)));

		/*
		- for lock-on I think I'll have to manually set the m_direction vector
		- this is probably the simplest way of doing lock-on
		*/

		// NOTE: set the view matrix
		view = glm::lookAt(m_position, m_position+tempV, glm::vec3(0.0f, 1.0f, 0.0f));

		// NOTE: sword transform
		if((swinging&&swing_forward) && swing_frame<60) {
			swing_frame+=2;
		} else if((swinging&&!swing_forward) && swing_frame>0) {
			swing_frame-=2;
		} else {
			if(swing_forward) {
				swing_forward = SDL_FALSE;
			} else {
				swing_frame = 0;
				swinging = SDL_FALSE;
				swing_forward = SDL_TRUE;
			}
		}

		glm::mat4 smodel;
		smodel = glm::translate(smodel, m_position);
		float sangle = glm::sign(m_direction.x)*glm::angle(glm::normalize(m_direction), glm::vec3(0.0f, 0.0f, 1.0f));
		glm::vec3 spos = glm::rotate(glm::vec3(-5.0f+(float)swing_frame/10.0f, -1.5f-(float)swing_frame/20.0f, 8.0f), sangle, glm::vec3(0.0f, 1.0f, 0.0f));
		smodel = glm::translate(smodel, spos);
		smodel = glm::rotate(smodel, (float) sangle+(float) M_PI/2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		smodel = glm::rotate(smodel, (float) -M_PI/2.0f+(float) (swing_frame/180.0f)*(float)M_PI, glm::vec3(0.0f, 0.0f, 1.0f));

		// NOTE: draw sword
		swordMesh->glBind();

		glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(smodel));
		glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

		swordMesh->render();

		// testing
		int w = 4;
		int h = 4;
		int walls[4][4] = {
			{0,0,0,0},
			{0,0,0,0},
			{0,0,0,0},
			{0,0,0,0}
		};

		int i, j;
		for(j=0; j<h; j++) {
			for(i=0; i<w; i++) {
				int ind = walls[j][i];

				testRoom[ind]->glBind();
				glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
				glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
				glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));
				testRoom[ind]->render();
			}
		}

		/* END TESTING */
		// ====================================================================

		// NOTE: swap the front and back buffers
		SDL_GL_SwapWindow(window);
	}

	/* TESTING */
	// ========================================================================

	swordMesh->glUnload();

	endGame();
	return 0;
}
