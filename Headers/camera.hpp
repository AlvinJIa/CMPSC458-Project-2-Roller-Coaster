#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <heightmap.hpp>
#include <track.hpp>
#include <vector>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 5.0f;
const float SENSITIVTY = 0.1f;
const float ZOOM = 45.0f;


// An abstract camera class that processes input and calculates the corresponding Eular Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
	// Camera Attributes
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

	glm::vec3 prevPosition;
	glm::vec3 prevFront;
	glm::vec3 prevUp;
	glm::vec3 prevRight;

	// Eular Angles
	float Yaw;
	float Pitch;
	// Camera options
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;
	// Our Parameters
	float currentpos = 1;  // Position you are on the track
	bool onTrack = false; // Whether or not you are following the track

	const float heightMax = 14.0f;
	const float gravity = 9.81f;
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 ft = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 rt = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f);

	// Constructor with vectors
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Zoom(ZOOM)
	{
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}
	// Constructor with scalar values
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Zoom(ZOOM)
	{
		Position = glm::vec3(posX, posY, posZ);
		WorldUp = glm::vec3(upX, upY, upZ);
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}

	// Returns the view matrix calculated using Eular Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(Position, Position + Front, Up);
	}

	// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(Camera_Movement direction, float deltaTime)
	{
		float velocity = MovementSpeed * deltaTime;
		if (direction == FORWARD)
			Position += Front * velocity;
		if (direction == BACKWARD)
			Position -= Front * velocity;
		if (direction == LEFT)
			Position -= Right * velocity;
		if (direction == RIGHT)
			Position += Right * velocity;
	}

	float get_distance(glm::vec3 PointA, glm::vec3 PointB) {
		float x = sqrt(pow((PointA.x - PointB.x), 2) + pow((PointA.y - PointB.y), 2) + pow((PointA.z - PointB.z), 2));
		return x;
	}

	//  Find the next camera position based on the amount of passed time, the track, and the track position s (defined in this class).  You can just use your code from the track function. 
	void ProcessTrackMovement(float deltaTime, Track &track)
	{
		//init
		if (onTrack == false)
		{
			Up = up;
			Front = ft;
			Right = rt;
			Position = pos;
			prevUp = up;
			prevFront = ft;
			prevRight = rt;
			prevPosition = pos;
			onTrack = true;
		}
		else 
		{
			prevUp = Up;
			prevFront = Front;
			prevRight = Right;
			prevPosition = Position;

			float V = 0.5 * sqrt(2.0f * gravity * (float(heightMax) - float(Position.y)));

			float D = V * deltaTime;

			while (D> 0) 
			{
				if (currentpos > track.controlPoints.size())
				{
					currentpos = 1;
					Up = up;
					Front = ft;
					Right = rt;
					Position = pos;
				}
				else
				{
					prevUp = Up;
					prevFront = Front;
					prevRight = Right;
					prevPosition = Position;

					currentpos += 0.001;
					Position = track.get_point(currentpos);

					Front = glm::normalize(Position - prevPosition);
					Right = glm::normalize(glm::cross(prevUp, Front));
					Up = glm::normalize(glm::cross(Front, Right));

					D -= get_distance(prevPosition, Position);
				}
			}

			if (currentpos > track.controlPoints.size() - 3) {
				currentpos = 1;
				Up = up;
				Front = ft;
				Right = rt;
				Position = pos;
			}
		}
	}

	// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
	{
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw += xoffset;
		Pitch += yoffset;
		 
		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		}

		// Update Front, Right and Up Vectors using the updated Eular angles
		updateCameraVectors();
	}

	// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	//    Not really necessary, you can use this for something else if you like
	void ProcessMouseScroll(float yoffset)
	{
		if (Zoom >= 1.0f && Zoom <= 45.0f)
			Zoom -= yoffset;
		if (Zoom <= 1.0f)
			Zoom = 1.0f;
		if (Zoom >= 45.0f)
			Zoom = 45.0f;
	}

private:
	// Calculates the front vector from the Camera's (updated) Eular Angles
	void updateCameraVectors()
	{
		// Calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);
		// Also re-calculate the Right and Up vector
		Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Front));
	}
};
#endif
