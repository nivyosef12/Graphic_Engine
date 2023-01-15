#include "Bezier1D.h"
#include "MeshConstructor.h"
#include <iostream>
#include <cstdio>

//parameters:
int points_per_segment = 20;

/*
Bezier1D::Bezier1D(int segNum,int res,int mode, int viewport): 
    //printf("1\n");
    segmentsNum(segNum),
    resT(res),
    M(
        glm::mat4(
            -1, 3, -3, 1,
            3, -6, 3, 0,
            -3, 3, 0, 0,
            1, 0, 0, 0
        )
    ),
    segments(BuildSegments()), //build the segments
    Shape(new MeshConstructor(GetLine(), false), 1) //build the mesh constructor
{ 
    int x = 2;
}
*/
// TODO: diff between segNum and res
Bezier1D::Bezier1D(int segNum, int res, int mode, int viewport) : Shape(1)
{
    this->segmentsNum = segNum;   
    this->resT = res; 
    this->M = glm::mat4(
            -1, 3, -3, 1,
            3, -6, 3, 0,
            -3, 3, 0, 0,
            1, 0, 0, 0
        );
    this->segments = BuildSegments(); //build the segments
    this->setMesh(new MeshConstructor(GetLine(), false));

}

std::vector<glm::mat4> Bezier1D::BuildSegments()
{
    std::vector<glm::mat4> segments;
    //first segment
    glm::vec4 p0(0, 0, 0, 0);
    glm::vec4 p1(0, 1, 0, 0);
    glm::vec4 p2(1, 2, 0, 0);
    glm::vec4 p3(2, 2, 0, 0);
    glm::mat4 seg1(p0, p1, p2, p3);
    segments.push_back(seg1);
    
    // throw exption for resT = 2
    for (int i = 0; i < resT - 2; i++) {
        //ith segment
        p0 = segments.back()[3];
        p1 = p0 + glm::vec4(1, 0, 0, 0);
        p2 = p1 + glm::vec4(1, 0, 0, 0);
        p3 = p2 + glm::vec4(1, 0, 0, 0);
        glm::mat4 segI(p0, p1, p2, p3);
        segments.push_back(segI);
    }

    //last segment
    p0 = segments.back()[3];
    p1 = p0 + glm::vec4(1, 0, 0, 0);
    p2 = p1 + glm::vec4(1, -1, 0, 0);
    p3 = p2 + glm::vec4(0, -1, 0, 0);
    glm::mat4 segN(p0, p1, p2, p3);
    segments.push_back(segN);

    return segments;
}

IndexedModel Bezier1D::GetLine() const
{
    IndexedModel model;

    std::vector<LineVertex> axisVertices;
    for (int i = 0; i < segmentsNum; i++) {
        for (int t = 0; t < points_per_segment; t++) {
            glm::vec4 point = GetPointOnCurve(i, t); //calculate le position of the point on the curve
            model.positions.push_back(glm::vec3(point.x,point.y,point.z)); //add it to the model
            model.colors.push_back(glm::vec3(1,1,1)); //make the point black
            if (t != 0) {
                model.indices.push_back(i*points_per_segment + t - 1);
                model.indices.push_back(i*points_per_segment + t); //add the previous point and current point to indices (this makes sure all points are connected in the scene)  
            } else {
                model.indices.push_back(i*points_per_segment + t); //if it's the first point, add only it  
            }
        }
    }

    return model;
}

glm::vec4 Bezier1D::GetControlPoint(int segment, int indx) const
{
    if (segment < segmentsNum)
        return segments[segment][indx];
    return segments[segmentsNum - 1][3];
}

glm::vec4 Bezier1D::GetPointOnCurve(int segment, int t) const
{
    t/=points_per_segment;
    glm::vec4 T(std::pow(t, 3), std::pow(t, 2), t, 1);
    glm::vec4 point = T * M * segments[segment];
    return point;
}

glm::vec3 Bezier1D::GetVelosity(int segment, int t)
{
    return glm::vec3();
}

// void Bezier1D::SplitSegment(int segment, int t)
// {
// }

void Bezier1D::AddSegment(glm::vec4 p1, glm::vec4 p2, glm::vec4 p3)
{
    glm::vec4 p0 = segments.back()[3];
    segments.push_back(glm::mat4(p0, p1, p2, p3));
}

void Bezier1D::ChangeSegment(int segIndx,glm::vec4 p1, glm::vec4 p2, glm::vec4 p3)
{
    glm::vec4 p0 = segments[segIndx-1][3];
    segments[segIndx] = glm::mat4(p0, p1, p2, p3);
}

float Bezier1D::MoveControlPoint(int segment, int indx, float dx,float dy,bool preserveC1)
{
    return 0; //not suppose to reach here
}

void Bezier1D::CurveUpdate(int pointIndx, float dx, float dy, bool preserveC1)
{
    //TODO: here the IndexModel will be updated, hopefully this will show in the scene, if not we need to find a way to make the scene change
}

void Bezier1D::ResetCurve(int segNum)
{

}

Bezier1D::~Bezier1D(void)
{

}