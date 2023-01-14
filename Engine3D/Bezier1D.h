#pragma once

//#include "Mesh.h"
#include "../Engine3D/shape.h"

class Bezier1D : public Shape
{
	int segmentsNum;
	std::vector<glm::mat4> segments;
	int resT;
	 glm::mat4 M;
	 float MoveControlPoint(int segment, int indx, float dx, float dy, bool preserveC1);  //change the position of one control point. when preserveC1 is true it may affect other  control points 
public:
	Bezier1D(int segNum, int res,int mode, int viewport = 0);
	std::vector<glm::mat4> BuildSegments();
	IndexedModel GetLine() const;	//generates a model for MeshConstructor Constructor with resT vertices. See Mesh.h/cpp for a IndexedModel defenition and how to intialize it
	glm::vec4 GetControlPoint(int segment, int indx) const; //returns a control point in the requested segment. indx will be 0,1,2,3, for p0,p1,p2,p3
	
	glm::vec4 GetPointOnCurve(int segment, int t) const; //returns point on curve in the requested segment for the value of t
	glm::vec3 GetVelosity(int segment, int t); //returns the derivative of the curve in the requested segment for the value of t

	void AddSegment(glm::vec4 p1, glm::vec4 p2, glm::vec4 p3);  // adds a segment at the end of the curve
	void CurveUpdate(int pointIndx, float dx, float dy, bool preserveC1 = false);  //changes the line in by using ChangeLine function of MeshConstructor and MoveControlPoint 
	void ChangeSegment(int segIndx, glm::vec4 p1, glm::vec4 p2, glm::vec4 p3); // changes three control point of a segment
	void ResetCurve(int segNum); // to the init state
	inline int GetSegmentsNum() const { return segmentsNum; }
	~Bezier1D(void);
};

