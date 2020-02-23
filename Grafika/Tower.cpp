#include "Tower.h"
#include "glm/gtc/matrix_transform.hpp"

Tower::Tower()
{
    timeHit = 0;
   
}


Tower::~Tower()
{
}

void Tower::SetCoordinates(float x, float z)
{
    Tower_x = x;
    Tower_z = z;
}
int Tower::GetTimeHit() {
    return timeHit;
}

void Tower::SetTimeHit(int time) {
    timeHit = time;
}

float Tower::GetPositionX()
{
    return Tower_x;
}

float Tower::GetPositionZ()
{
    return Tower_z;
}

glm::mat4 Tower::GetTransformationMatrix()
{
    return m_geometric_tower_transformation_matrix;
}

glm::mat4 Tower::GetNormalMatrix()
{
    return m_geometric_tower_transformation_normal_matrix;
}

void Tower::SetTransformationMatrix(glm::vec3 pos)
{
    m_geometric_tower_transformation_matrix = glm::translate(glm::mat4(1.f), glm::vec3(pos)) * glm::scale(glm::mat4(1.f), glm::vec3(0.4f));
}

void Tower::SetNormalMatrix(glm::mat4 a)
{
    m_geometric_tower_transformation_normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(a))));

}
