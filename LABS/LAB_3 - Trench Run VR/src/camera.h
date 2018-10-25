#pragma once



#ifndef LAB471_CAMERA_H_INCLUDED
#define LAB471_CAMERA_H_INCLUDED

#include <stack>
#include <memory>

#include "glm/glm.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"


//#define LEFTEYE 0
//#define RIGHTEYE 1
class camera
{
public:
	glm::vec3 pos;
	glm::vec3 rot;
	glm::vec3 viewrot;
	glm::vec3 camdir;
	glm::mat4 cameraPos;
	int w, a, s, d, c, q, e, z, x, space, reset;
	float going_up = 0.0;
	float floatAcceleration = 0;
	camera()
	{
		w = a = s = d = c = q = e = z = x = space, reset = 0;
		pos = rot = viewrot = glm::vec3(0, 0, 0);
		camdir = glm::vec3(0, 0, 1);
	}
	glm::mat4 process()
	{
		if (reset == 1) {
			rot.z = 0;
			rot.x = 0;
			rot.y = 0;
			going_up = 0;
			reset = 0;
		}

		float going_forward = 0.0;
		if (w == 1)
			going_forward = floatAcceleration += .005;
		if (w == 0 && s != 1)
			if (floatAcceleration != 0) {
				floatAcceleration -= 0.005;
			}
			if (floatAcceleration < 0) {
				floatAcceleration = 0.0;
			}
			going_forward = floatAcceleration;
		if (s == 1)
			going_forward = -.1;
		if (q == 1)
			rot.z += 0.1;
		if (e == 1)
			rot.z -= 0.1;
		if (z == 1)
			rot.x += 0.1;
		if (x == 1)
			rot.x -= 0.1;
		if (space == 1)
			going_up += .1;
		if (c == 1)
			going_up -= .1;
		if (a == 1)
			rot.y -= 0.1;
		if (d == 1)
			rot.y += 0.1;

		glm::mat4 roll = glm::rotate(glm::mat4(1), rot.z, glm::vec3(0, 0, 1));

		glm::mat4 pitch = glm::rotate(glm::mat4(1), rot.x, glm::vec3(1, 0, 0));

		glm::mat4 yaw = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));

		glm::vec4 rpos = glm::vec4(0, 0, going_forward, 1);

		glm::mat4 trans = glm::translate(glm::mat4(1), glm::vec3(0, -going_up, 0));

		rpos = roll * pitch * yaw *rpos;
		pos.x += -rpos.x;
		pos.z += rpos.z;

		glm::mat4 T = glm::translate(glm::mat4(1), glm::vec3(0, pos.y, pos.z));
		cameraPos = roll*pitch*yaw*T*trans;
		return cameraPos;
	}
	glm::mat4 getR()
	{
		glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		return R;
	}

	void offsetOrientation(float upAngle, float rightAngle) {
		rot.y += rightAngle;
		rot.x += upAngle;
		normalizeAngles();
	}

	void normalizeAngles() {
		static const float MaxVerticalAngle = 85.0f; //must be less than 90 to avoid gimbal lock
		rot.y = fmodf(rot.y, 360.0f);
		//fmodf can return negative values, but this will make them all positive
		if (rot.y < 0.0f)
			rot.y += 360.0f;

		if (rot.x > MaxVerticalAngle)
			rot.x = MaxVerticalAngle;
		else if (rot.x < -MaxVerticalAngle)
			rot.x = -MaxVerticalAngle;
	}
};








#endif // LAB471_CAMERA_H_INCLUDED