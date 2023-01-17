#include "MovableGLM.h"
#include <stdio.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

const float pi = 3.14159265358979323846;


static void printMat(const glm::mat4 mat)
{
	printf(" matrix: \n");
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
			printf("%f ", mat[j][i]);
		printf("\n");
	}
}

MovableGLM::MovableGLM()
{
	ZeroTrans();
}

glm::mat4 MovableGLM::MakeTrans(glm::mat4 &prevTransformations) const
{
	return prevTransformations * MakeTrans();
}

glm::mat4 MovableGLM::MakeTrans() const
{
	return   trans * rot * scl ;
}

void MovableGLM::MyTranslate(glm::vec3 delta,int mode)
{
	if (mode == 1) {
		trans = glm::mat4(1); //return to origin
	} else {
		trans = glm::translate(trans,delta);
	}
}

void  MovableGLM::MyRotate(float angle,glm::vec3 &vec,int mode)
{
	if (mode == 2) {
		glm::vec4 y4 = rot * glm::vec4(0, 1, 0, 1);
		glm::vec3 yAxis = glm::normalize(glm::vec3(y4.x, y4.y, y4.z)); // The current y-axis of the cube
		glm::vec3 v = glm::normalize(vec); // normalize the target vector

		glm::vec3 rotationAxis = glm::normalize(glm::cross(yAxis, v)); // The axis to rotate around
		float alpha = acos(glm::dot(yAxis, v)); 
		alpha *= 360/(2 * pi); // The angle to rotate by

		rot = glm::rotate(rot, alpha, rotationAxis);
	} else {
		rot = glm::rotate(rot,angle,vec);
	}
}
	
void  MovableGLM::MyScale(glm::vec3 scale)
{
	scl = glm::scale(scl,scale);
}

void MovableGLM::ZeroTrans()
{
	trans = glm::mat4(1);
	rot = glm::mat4(1);
	scl = glm::mat4(1);
}