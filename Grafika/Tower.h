#pragma once
#include "GLEW\glew.h"
#include "glm\glm.hpp"
#include "GeometryNode.h"


class Tower
{
public:
    Tower(glm::vec3 sud);
    Tower();
    ~Tower();
    glm::mat4                                       GetTransformationMatrix();
    glm::mat4                                       GetNormalMatrix();
    void SetTransformationMatrix(glm::vec3 pos);
    void SetNormalMatrix(glm::mat4 a);
    void                                            SetCoordinates(float x, float z);
    int GetTimeHit();
    void SetTimeHit(int time);
    float                                           GetPositionX();
    float                                           GetPositionZ();

private:

    glm::mat4										m_geometric_tower_transformation_matrix;
    glm::mat4										m_geometric_tower_transformation_normal_matrix;
    int                                             timeHit;
    bool                                            CanHit;
    int                                             Tower_x,Tower_z;
};



