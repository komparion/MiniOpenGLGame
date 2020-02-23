#pragma once

#include "GeometryNode.h"
#include "Tools.h"

#include <algorithm>
#include "ShaderProgram.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "OBJLoader.h"
#include <iostream>
#include "Renderer.h"



class Pirate {
	//auto ElapsedTimeSinceSpawned = chrono::steady_clock::now();
	public:
		class GeometryNode* m_pirate_left_foot;
		glm::mat4										m_pirate_left_foot_transformation_matrix;
		glm::mat4										m_pirate_left_foot_transformation_normal_matrix;
		class GeometryNode* m_pirate_right_foot;
		glm::mat4										m_pirate_right_foot_transformation_matrix;
		glm::mat4										m_pirate_right_foot_transformation_normal_matrix;
		class GeometryNode* m_pirate_body;
		glm::mat4										m_pirate_body_transformation_matrix;
		glm::mat4										m_pirate_body_transformation_normal_matrix;
		class GeometryNode* m_pirate_arm;
		glm::mat4										m_pirate_arm_transformation_matrix;
		glm::mat4										m_pirate_arm_transformation_normal_matrix;
		
		bool target;

		ShaderProgram									m_geometry_rendering_program;
		ShaderProgram									m_postprocess_program;
		ShaderProgram									m_spot_light_shadow_map_program;
		

		float											pirate_continous_time;
		float												ElapsedSecSinceSpawned;
		int												Current_Tile_Index;
		//int TILES[29][2];
		glm::vec2										m_skeleton_movement;
		glm::vec3										m_skeleton_position;

		glm::vec2										Current_Tile_Position, Next_Tile_Position, direction, Pirate_Position;

		glm::vec3										center;
		float											radius;
		bool											collidedWithChest;
		Pirate(float dt);
		~Pirate();
		std::string vertex_shader_path, fragment_shader_path;
		void DrawGeometryPirate(float pirate_continous_time);
		void DrawShadowMapPirate();
		bool LoadPirateMeshes();
		void Update(float pirate_continous_time);
		bool InitRenderingTechniques();

		bool DetectCollisionWithChest(glm::vec3 center, float radius);

		/*glm::mat4										GetArmTransformationMatrix();
		glm::mat4                                       GetArmNormalMatrix();
		glm::mat4										GetBodyTransformationMatrix();
		glm::mat4                                       GetBodyNormalMatrix();
		glm::mat4										GetLeftFootTransformationMatrix();
		glm::mat4                                       GetLeftFootNormalMatrix();
		glm::mat4										GetRightFootTransformationMatrix();
		glm::mat4                                       GetRightFootNormalMatrix();

		void											SetArmTransformationMatrix(GeometryNode* arm);
		void											SetArmNormalMatrix(glm::mat4 a);
		void											SetBodyTransformationMatrix(GeometryNode* body);
		void											SetBodyNormalMatrix(glm::mat4 a);
		void											SetLeftFootTransformationMatrix(GeometryNode* left_foot);
		void											SetLeftFootNormalMatrix(glm::mat4 a);
		void											SetRightFootTransformationMatrix(GeometryNode* right_foot);
		void											SetRightFootNormalMatrix(glm::mat4 a);*/

		void SetArm(GeometryNode* arm);
		void SetBody(GeometryNode* body);
		void SetLeftFoot(GeometryNode* left_foot);
		void SetRightFoot(GeometryNode* right_foot);
};