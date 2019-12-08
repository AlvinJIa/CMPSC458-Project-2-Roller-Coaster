#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <iostream>

#include <shader.hpp>
#include <rc_spline.h>

struct Orientation {
	// Front
	glm::vec3 Front;
	// Up
	glm::vec3 Up;
	// Right
	glm::vec3 Right;
	// position
	glm::vec3 position;
};

class Track
{
public:

	// VAO
	unsigned int VAO;

	// Control Points Loading Class for loading from File
	rc_Spline g_Track;

	// Vector of control points
	std::vector<glm::vec3> controlPoints;

	// Track data
	std::vector<Vertex> vertices;
	std::vector<Orientation> camera;

	// indices for EBO
	std::vector<unsigned int> indices;

	// hmax for camera
	float hmax = 0.0f;

	// constructor, just use same VBO as before, 
	Track(const char* trackPath)
	{
		// load Track data
		load_track(trackPath);

		create_track();

		setup_track();
	}

	// render the mesh
	void Draw(Shader shader, unsigned int textureID)
	{
		/*
			Draw the objects here.
		*/
		// You must:
		// -  active proper texture unit before binding
		// -  bind the texture
		// -  draw mesh (using GL_TRIANGLES is the most reliable way)
		glm::mat4 model_track;

		// Bind new textures to both texture positions (do both since it has 2 textures in the vertex shader)
		glActiveTexture(GL_TEXTURE0);

		glBindTexture(GL_TEXTURE_2D, textureID);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// Set model in shaders


		shader.setMat4("model", model_track);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());

		// always good practice to set everything back to defaults once configured.
		glActiveTexture(GL_TEXTURE0);
	}

	// given an s float, find the point
	//  S is defined as the distance on the spline, so s=1.5 is the at the halfway point between the 1st and 2nd control point
	glm::vec3 get_point(float s)
	{
		float sDecimal = (float)s - (int)s;
		int pA = ((int)s) - 1;
		int pB = ((int)s);
		int pC = ((int)s) + 1;
		int pD = ((int)s) + 2;

		return interpolate(controlPoints[pA], controlPoints[pB], controlPoints[pC], controlPoints[pD], 0.5f, sDecimal);
	}


	void delete_buffers()
	{
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}

private:


	/*  Render data  */
	unsigned int VBO, EBO;
	unsigned int VBOplank, EBOplank;

	void load_track(const char* trackPath)
	{
		// Set folder path for our projects (easier than repeatedly defining it)
		g_Track.folder = "../Project_2/Media/";

		// Load the control points
		g_Track.loadSplineFrom(trackPath);

	}

	// Implement the Catmull-Rom Spline here
	//     Given 4 points, a tau and the u value 
	//     Since you can just use linear algebra from glm, just make the vectors and matrices and multiply them.  
	//     This should not be a very complicated function
	glm::vec3 interpolate(glm::vec3 pointA, glm::vec3 pointB, glm::vec3 pointC, glm::vec3 pointD, float tau, float u)
	{
		glm::mat4 catmull(
			0, -tau, 2 * tau, -tau,
			1, 0, tau - 3, 2 - tau,
			0, tau, 3 - (2 * tau), tau - 2,
			0, 0, -tau, tau);

		glm::mat4 pointABCD(
			pointA.x, pointB.x, pointC.x, pointD.x,
			pointA.y, pointB.y, pointC.y, pointD.y,
			pointA.z, pointB.z, pointC.z, pointD.z,
			0, 0, 0, 0);

		glm::vec4 vecU(1, u, u*u, u*u*u);

		glm::vec3 result = vecU * catmull * pointABCD;

		// Just returning the first point at the moment, you need to return the interpolated point.  
		return result;
	}

	// Here is the class where you will make the vertices or positions of the necessary objects of the track (calling subfunctions)
	//  For example, to make a basic roller coster:
	//    First, make the vertices for each rail here (and indices for the EBO if you do it that way).  
	//        You need the XYZ world coordinates, the Normal Coordinates, and the texture coordinates.
	//        The normal coordinates are necessary for the lighting to work.  
	//    Second, make vector of transformations for the planks across the rails
	void create_track()
	{
		// Create the vertices and indices (optional) for the rails
		//    One trick in creating these is to move along the spline and 
		//    shift left and right (from the forward direction of the spline) 
		//     to find the 3D coordinates of the rails.

		// Create the plank transformations or just creating the planks vertices
		//   Remember, you have to make planks be on the rails and in the same rotational direction 
		//       (look at the pictures from the project description to give you ideas).  

		// Here is just visualizing of using the control points to set the box transformatins with boxes. 
		//       You can take this code out for your rollercoster, this is just showing you how to access the control points

		glm::vec3 currentpos = glm::vec3(0.0f, -1.0f, 5.0f);
		/* iterate throught  the points	g_Track.points() returns the vector containing all the control points */
		for (pointVectorIter ptsiter = g_Track.points().begin(); ptsiter != g_Track.points().end(); ptsiter++)
		{
			/* get the next point from the iterator */
			glm::vec3 pt(*ptsiter);

			/* now just the uninteresting code that is no use at all for this project */
			currentpos += pt;
			//  Mutliplying by two and translating (in initialization) just to move the boxes further apart.  
			controlPoints.push_back(currentpos*2.0f);
		}
		Orientation Prev_P;
		Orientation Current_p;
		Current_p.position = controlPoints[1];
		Current_p.Up = glm::vec3(0.0f, 1.0f, 0.0f);
		Current_p.Front = glm::vec3(0.0f, 0.0f, 1.0f);
		Current_p.Right = glm::vec3(1.0f, 0.0f, 0.0f);

		for (int i = 1; i < (controlPoints.size() - 3); i++) {
			for (float u = 0; u < 1; u += 0.05f) {
				Prev_P.position = Current_p.position;
				Prev_P.Up = Current_p.Up;
				Prev_P.Front = Current_p.Front;
				Prev_P.Right = Current_p.Right;

				Current_p.position = interpolate(controlPoints[i], controlPoints[i + 1], controlPoints[i + 2], controlPoints[i + 3], 0.5f, u);
				Current_p.Front = glm::normalize((Current_p.position - Prev_P.position));
				Current_p.Right = glm::normalize(glm::cross(Prev_P.Up, Current_p.Front));
				Current_p.Up = glm::normalize(glm::cross(Current_p.Front, Current_p.Right));

				makeRailPart(Prev_P, Current_p, glm::vec2(0, 0));
			}
		}
	}

	Vertex make_vertex(glm::vec3 myPoint, int index)
	{
		Vertex ret;

		ret.Position.x = myPoint.x;
		ret.Position.y = myPoint.y;
		ret.Position.z = myPoint.z;

		if (index == 0) {
			ret.TexCoords.x = 0;
			ret.TexCoords.y = 1;
		}
		else if (index == 1) {
			ret.TexCoords.x = 0;
			ret.TexCoords.y = 0;
		}
		else {
			ret.TexCoords.x = 1;
			ret.TexCoords.y = 0;
		}
		return ret;
	}

	// Given 3 Points, create a triangle and push it into vertices (and EBO if you are using one)
		// Optional boolean to flip the normal if you need to
	void make_triangle(glm::vec3 pointA, glm::vec3 pointB, glm::vec3 pointC, bool flipNormal)
	{
		Vertex vertexA = make_vertex(pointA, 0);
		Vertex vertexB = make_vertex(pointB, 1);
		Vertex vertexC = make_vertex(pointC, 2);

		set_normals(vertexA, vertexB, vertexC);
		if (flipNormal) {
			vertexA.Normal = -vertexA.Normal;
			vertexB.Normal = -vertexB.Normal;
			vertexC.Normal = -vertexC.Normal;
		}

		vertices.push_back(vertexA);
		vertices.push_back(vertexB);
		vertices.push_back(vertexC);
	}

	// Given two orintations, create the rail between them.  Offset can be useful if you want to call this for more than for multiple rails
	void makeRailPart(Orientation Prev_P, Orientation Current_p, glm::vec2 offset)
	{
		glm::vec3 A = Prev_P.position -(0.1f*Prev_P.Up) - (0.2f*Prev_P.Right);
		glm::vec3 B = Current_p.position - (0.1f*Current_p.Up) - (0.2f*Current_p.Right);
		glm::vec3 C = Current_p.position - (0.1f*Current_p.Up) + (0.2f*Current_p.Right);
		glm::vec3 D = Prev_P.position - (0.1f*Prev_P.Up) + (0.2f*Prev_P.Right);

		glm::vec3 E = Prev_P.position - (0.3f*Prev_P.Up) -  (0.5f*Prev_P.Right);
		glm::vec3 F = Current_p.position - (0.3f*Current_p.Up) - (0.5f*Current_p.Right);
		glm::vec3 G = Current_p.position - (0.3f*Current_p.Up) + (0.5f*Current_p.Right);
		glm::vec3 H = Prev_P.position - (0.3f*Prev_P.Up) + (0.5f*Prev_P.Right);

		//bottom
		make_triangle(A, B, D, true);
		make_triangle(D, C, B, true);
		//top
		make_triangle(E, F, H, false);
		make_triangle(H, G, F, true);
		////left
		make_triangle(A, B, E, false);
		make_triangle(E, F, B, true);
		////right
		make_triangle(C, D, H, false);
		make_triangle(H, G, C, false);
	}

	// Find the normal for each triangle uisng the cross product and then add it to all three vertices of the triangle.  
	//   The normalization of all the triangles happens in the shader which averages all norms of adjacent triangles.   
	//   Order of the triangles matters here since you want to normal facing out of the object.  
	void set_normals(Vertex &p1, Vertex &p2, Vertex &p3)
	{
		glm::vec3 normal = glm::cross(p2.Position - p1.Position, p3.Position - p1.Position);
		p1.Normal += normal;
		p2.Normal += normal;
		p3.Normal += normal;
	}

	void setup_track()
	{
		// Like the heightmap project, this will create the buffers and send the information to OpenGL
		 //  1.  Create ID / Generate Buffers and for Vertex Buffer Object (VBO),
		 //      Vertex Array Buffer (VAO), and the Element Buffer Objects (EBO)

		 // 2. Bind Vertex Array Object
		glGenVertexArrays(1, &VAO);

		//  Bind the Vertex Buffer
		glGenBuffers(1, &VBO);
		//glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		// 3. Copy our vertices array in a vertex buffer for OpenGL to use
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		// 4. Copy our indices array in a vertex buffer for OpenGL to use
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);

		// 5.  Position attribute for the 3D Position Coordinates and link to position 0
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glEnableVertexAttribArray(0);
		// 6.  TexCoord attribute for the 2d Texture Coordinates and link to position 2
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		// 6.  TexCoord attribute for the 2d Texture Coordinates and link to position 2
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);
	}
};