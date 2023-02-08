#pragma once

//#include "Mesh.h"
#include "../Engine3D/shape.h"

class Bezier1D : public Shape
{
	int segmentsNum;
	std::vector<glm::mat4> segments;
	int resT;
	 glm::mat4 M;
public:
	Bezier1D(int segNum, int res,int mode, int viewport = 0);
	std::vector<glm::mat4> BuildSegments(int resT, int segNum);
	IndexedModel GetLine() const;	//generates a model for MeshConstructor Constructor with resT vertices. See Mesh.h/cpp for a IndexedModel defenition and how to intialize it
	glm::vec4 GetControlPoint(int segment, int indx) const; //returns a control point in the requested segment. indx will be 0,1,2,3, for p0,p1,p2,p3
	
	glm::vec4 GetPointOnCurve(int segment, float t) const; //returns point on curve in the requested segment for the value of t
	glm::vec4 GetVelocity(int segment, float t); //returns the derivative of the curve in the requested segment for the value of t

	void AddSegments(int n);  // adds a segment at the end of the curve
	void CurveUpdate(int pointIndx, float dx, float dy, bool preserveC1 = false);  //changes the line in by using ChangeLine function of MeshConstructor and MoveControlPoint 
	void ChangeSegment(int segIndx, glm::vec4 p1, glm::vec4 p2, glm::vec4 p3); // changes three control point of a segment
	float MoveControlPoint(int segment, int indx, float dx, float dy, bool preserveC1);  //change the position of one control point. when preserveC1 is true it may affect other  control points 
	void ResetCurve(); // to the init state
    glm::vec4 Align(int segNum);
    inline int GetSegmentsNum() const { return segmentsNum; }
    // int GetSegmentsNum();
	// int GetResT();
	~Bezier1D(void);
};

