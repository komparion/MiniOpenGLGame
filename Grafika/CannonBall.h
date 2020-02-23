#pragma once
#include "GLEW\glew.h"
#include "glm\glm.hpp"
#include "GeometryNode.h"
#include "Pirate.h"

class CannonBall
{

public:
    
    CannonBall();
    ~CannonBall();
    glm::mat4                                       GetTransformationMatrix();
    glm::mat4                                       GetNormalMatrix();
    void SetTransformationMatrix(glm::vec3 pos);
    void SetNormalMatrix(glm::mat4 a);
	void Shoot(float time);
	bool Getshooting();
	void Setshooting(bool a);
	bool Getcollision();
	void Setcollision(bool a);
	void SetLockedEnemy(Pirate* a,float time);
	Pirate* GetLockedEnemy();
	void SetEnemy(glm::vec3 a);
	void                                            SetCoordinates(float x, float z);
    float                                           GetPositionX();
    float                                           GetPositionZ();
	bool CannonBall::CheckCollided(glm::vec3 pos);
	bool collision;

private:

	
	float                                     cannonballRadius;
	float                                          shootTime;
	bool                                           shooting;
	Pirate*                                       enemy;
    glm::mat4										m_geometric_CannonBall_transformation_matrix;
    glm::mat4										m_geometric_CannonBall_transformation_normal_matrix;
    int                                             CannonBall_x, CannonBall_z;
};

