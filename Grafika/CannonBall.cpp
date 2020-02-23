#include "CannonBall.h"
#include "glm/gtc/matrix_transform.hpp"
#include <iostream>
#include "Pirate.h"


CannonBall::CannonBall()
{
	cannonballRadius = 1.0*0.1;

}


CannonBall::~CannonBall()
{
}

void CannonBall::SetLockedEnemy(Pirate* a,float time) {
	enemy = a;
	shooting = true;
	shootTime = time;
}

Pirate* CannonBall::GetLockedEnemy() {
	return enemy;
}

bool CannonBall::Getshooting() {
	return shooting;
}

void CannonBall::Setshooting(bool a) {
	shooting = a;
}

bool CannonBall::Getcollision() {
	return collision;
}

void CannonBall::Setcollision(bool a) {
	collision = a;
}

void CannonBall::SetCoordinates(float x, float z)
{
    CannonBall_x = x;
    CannonBall_z = z;
}

float CannonBall::GetPositionX()
{
    return CannonBall_x;
}

float CannonBall::GetPositionZ()
{
    return CannonBall_z;
}

glm::mat4 CannonBall::GetTransformationMatrix()
{
    return m_geometric_CannonBall_transformation_matrix;
}

glm::mat4 CannonBall::GetNormalMatrix()
{
    return m_geometric_CannonBall_transformation_normal_matrix;
}

void CannonBall::SetTransformationMatrix(glm::vec3 pos)
{
    m_geometric_CannonBall_transformation_matrix = glm::translate(glm::mat4(1.f), glm::vec3(pos.x - 0.0101*0.4,pos.y+9.5*0.4,pos.z - 0.0758*0.4)) * glm::scale(glm::mat4(1.f), glm::vec3(0.1f));
}

void CannonBall::SetNormalMatrix(glm::mat4 a)
{
    m_geometric_CannonBall_transformation_normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(a))));
}

void CannonBall::Shoot(float time) {

	
	glm::vec3 cannonBall = { CannonBall_x, 9.5*0.4, CannonBall_z };
	glm::vec3 CannonBall_Position = glm::mix(cannonBall, enemy->m_skeleton_position + enemy->center, (time - shootTime)/0.4);
	m_geometric_CannonBall_transformation_matrix = glm::translate(glm::mat4(1.f), glm::vec3(CannonBall_Position)) * glm::scale(glm::mat4(1.f), glm::vec3(0.1f));
	m_geometric_CannonBall_transformation_normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(m_geometric_CannonBall_transformation_matrix))));
	if (CheckCollided(CannonBall_Position) == true){
	
		shooting = false;
		m_geometric_CannonBall_transformation_matrix = glm::translate(glm::mat4(1.f), glm::vec3(CannonBall_x - 0.0101*0.4, 9.5*0.4, CannonBall_z - 0.0758*0.4)) * glm::scale(glm::mat4(1.f), glm::vec3(0.1f));
		m_geometric_CannonBall_transformation_normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(m_geometric_CannonBall_transformation_matrix))));
		collision = true;
		enemy->collidedWithChest = true;
	}
}

bool CannonBall::CheckCollided(glm::vec3 pos){
	float distance = glm::distance(pos, enemy->m_skeleton_position + enemy->center);
	if (distance < cannonballRadius + 12.87075 * 0.09) {
		return true;
	
	}
	
	return false;
}