
#include "GeometryNode.h"
#include "Pirate.h"
#include "Renderer.h"
#include "Tools.h"
#include <chrono>
#include <algorithm>
#include "ShaderProgram.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "OBJLoader.h"
#include <iostream>
#include <math.h>


#define PI 3.14159265



Pirate::Pirate(float dt) {

	m_pirate_left_foot = nullptr;
	m_pirate_body = nullptr;
	m_pirate_arm = nullptr;
	m_pirate_right_foot = nullptr;
	bool initialized = /*LoadPirateMeshes() && */ InitRenderingTechniques();
	//If there was any errors
	if (Tools::CheckGLError() != GL_NO_ERROR)
	{
		printf("Exiting with error at Renderer::Init\n");
		initialized = false;
	}
	ElapsedSecSinceSpawned =dt;
	pirate_continous_time =dt;

	target = false;

	
	m_skeleton_position = glm::vec3(0, 0, -2);
	Current_Tile_Index = 0;
	Pirate_Position = glm::vec2(0, 0);

	center = glm::vec3(-0.5957 * 0.09, 11.683 * 0.09, -4.274 * 0.09);
	radius = 12.87075 * 0.09;
	collidedWithChest = false;
}

Pirate::~Pirate() {
	//std::cout << "Inside destr ";
	//delete m_pirate_left_foot;
	//delete m_pirate_body;
	//delete m_pirate_arm;
	//delete m_pirate_right_foot;
}

void Pirate::Update(float continous_time) {
	pirate_continous_time += continous_time;
	//std::cout << pirate_continous_time << " ";
	int TILES[30][2] = { {0,0} ,{0,4}, {0,8} ,{0,12}, {4,12}, {4,16}, {4,20}, {4,24}, {4,28}, {8,28}, {8,32}, {12,32}, {16,32}, {20,32},
	{24,32}, {24,28}, {24,24}, {28,24}, {28,20}, {28,16}, {28,12}, {32,12}, {36,12}, {36,8}, {36,4}, {32,4}, {28,4}, {24,4}, {24,0}, { 24, -4 } };
	//std::cout << pirate_continous_time;
	//std::cout << ElapsedSecSinceSpawned;

	glm::mat4 rotation = glm::mat4(1.0);
	glm::vec2 Pirate_Positionx = glm::vec2(0, 0);
	//std::cout << "Sec "<< ElapsedSecSinceSpawned << " ";
	if (Current_Tile_Index < 29) {
		
		Current_Tile_Position = glm::vec2(TILES[Current_Tile_Index][0], TILES[Current_Tile_Index][1]);

		Next_Tile_Position = glm::vec2(TILES[Current_Tile_Index + 1][0], TILES[Current_Tile_Index + 1][1]);

		direction = glm::normalize(Next_Tile_Position - Current_Tile_Position);

		float position = (pirate_continous_time - ElapsedSecSinceSpawned);
		if (position > 1) {
			position = 1;
		}
		//std::cout << position;
		//std::cout << "";
		//std::cout << (int) Next_Tile_Position.y;

		Pirate_Position = glm::mix(Current_Tile_Position, Next_Tile_Position, position);
		float rotation_angle = atan2(direction.y, -direction.x);
		rotation *= glm::rotate(glm::mat4(1.0), rotation_angle + glm::radians(90.0f), glm::vec3(0, 1, 0));

		m_skeleton_position = { Pirate_Position.x, 0, Pirate_Position.y };
		
		glm::mat4 tran = glm::translate(glm::mat4(1.f), m_skeleton_position);

		//std::cout << m_skeleton_position.z;
		//std::cout << "";
		//std::cout << Next_Tile_Position.y;

		m_pirate_body_transformation_matrix = tran * rotation * glm::scale(glm::mat4(1.0), glm::vec3(0.09f));
		m_pirate_body_transformation_normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(m_pirate_body_transformation_matrix))));

		m_pirate_arm_transformation_matrix = tran * rotation * glm::scale(glm::mat4(1.0), glm::vec3(0.09f));
		m_pirate_arm_transformation_normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(m_pirate_arm_transformation_matrix))));

		m_pirate_left_foot_transformation_matrix = tran * rotation * glm::scale(glm::mat4(1.0), glm::vec3(0.09f));
		m_pirate_left_foot_transformation_normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(m_pirate_left_foot_transformation_matrix))));

		m_pirate_right_foot_transformation_matrix = tran * rotation * glm::scale(glm::mat4(1.0), glm::vec3(0.09f));
		m_pirate_right_foot_transformation_normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(m_pirate_right_foot_transformation_matrix))));

		if (Next_Tile_Position.x == Current_Tile_Position.x) {
			if (Next_Tile_Position.y == m_skeleton_position.z) {
				Current_Tile_Index++;
				ElapsedSecSinceSpawned++;
				
			}
		}
		if (Next_Tile_Position.y == Current_Tile_Position.y) {
			if (Next_Tile_Position.x == m_skeleton_position.x) {
				Current_Tile_Index++;
				ElapsedSecSinceSpawned++;
				
			}
		}
	}
}
bool Pirate::LoadPirateMeshes() {
	bool initialized = true;
	OBJLoader loader;
	// load pirate body
	auto mesh = loader.load("../Data/Pirate/pirate_body.obj");
	if (mesh != nullptr)
	{
		m_pirate_body = new GeometryNode();
		m_pirate_body->Init(mesh);
	}
	else
		initialized = false;

	// load pirate arm
	mesh = loader.load("../Data/Pirate/pirate_arm.obj");
	if (mesh != nullptr)
	{
		m_pirate_arm = new GeometryNode();
		m_pirate_arm->Init(mesh);
	}
	else
		initialized = false;
	//load left foot
	mesh = loader.load("../Data/Pirate/pirate_left_foot.obj");
	if (mesh != nullptr)
	{
		m_pirate_left_foot = new GeometryNode();
		m_pirate_left_foot->Init(mesh);
	}
	else
		initialized = false;
	//load right foot
	mesh = loader.load("../Data/Pirate/pirate_right_foot.obj");
	if (mesh != nullptr)
	{
		m_pirate_right_foot = new GeometryNode();
		m_pirate_right_foot->Init(mesh);
	}
	else
		initialized = false;

	return initialized;
}

void Pirate::DrawGeometryPirate(float pirate_continous_time) {

	//glUniform3f(m_geometry_rendering_program["uniform_model_matrix"], center.r, center.g, center.b);
	//draw left foot
	glBindVertexArray(m_pirate_left_foot->m_vao);
	glUniformMatrix4fv(m_geometry_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(m_pirate_left_foot_transformation_matrix));
	glUniformMatrix4fv(m_geometry_rendering_program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(m_pirate_left_foot_transformation_normal_matrix));
	for (int j = 0; j < m_pirate_left_foot->parts.size(); j++)
	{
		glm::vec3 diffuseColor = m_pirate_left_foot->parts[j].diffuseColor;
		glm::vec3 specularColor = m_pirate_left_foot->parts[j].specularColor;
		float shininess = m_pirate_left_foot->parts[j].shininess;

		glm::mat4 tran = glm::translate(glm::mat4(1.0), glm::vec3(-4, 0, -2));
		glm::mat4 matrix = m_pirate_left_foot_transformation_matrix * /*glm::inverse(tran) **/ glm::rotate(glm::mat4(1.0f), 0.3f * sin(-3.0f * pirate_continous_time), glm::vec3(1, 0, 0)) * tran;
		glm::mat4 norm = glm::mat4(glm::transpose(glm::inverse(glm::mat3(matrix))));

		glUniformMatrix4fv(m_geometry_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(matrix));
		glUniformMatrix4fv(m_geometry_rendering_program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(norm));
		glUniform3f(m_geometry_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
		glUniform3f(m_geometry_rendering_program["uniform_specular"], specularColor.r, specularColor.g, specularColor.b);
		glUniform1f(m_geometry_rendering_program["uniform_shininess"], shininess);
		glUniform1f(m_geometry_rendering_program["uniform_has_texture"], (m_pirate_left_foot->parts[j].textureID > 0) ? 1.0f : 0.0f);
		glBindTexture(GL_TEXTURE_2D, m_pirate_left_foot->parts[j].textureID);

		glDrawArrays(GL_TRIANGLES, m_pirate_left_foot->parts[j].start_offset, m_pirate_left_foot->parts[j].count);
	}
	//draw right foot
	glBindVertexArray(m_pirate_right_foot->m_vao);
	glUniformMatrix4fv(m_geometry_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(m_pirate_right_foot_transformation_matrix));
	glUniformMatrix4fv(m_geometry_rendering_program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(m_pirate_right_foot_transformation_normal_matrix));
	for (int j = 0; j < m_pirate_right_foot->parts.size(); j++)
	{
		glm::vec3 diffuseColor = m_pirate_right_foot->parts[j].diffuseColor;
		glm::vec3 specularColor = m_pirate_right_foot->parts[j].specularColor;
		float shininess = m_pirate_right_foot->parts[j].shininess;

		glm::mat4 tran = glm::translate(glm::mat4(1.0), glm::vec3(4, 0, -2));
		glm::mat4 matrix = m_pirate_right_foot_transformation_matrix */* glm::inverse(tran) * */ glm::rotate(glm::mat4(1.0f), 0.3f * sin(3.0f * pirate_continous_time), glm::vec3(1, 0, 0)) * tran;
		glm::mat4 norm = glm::mat4(glm::transpose(glm::inverse(glm::mat3(matrix))));

		glUniformMatrix4fv(m_geometry_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(matrix));
		glUniformMatrix4fv(m_geometry_rendering_program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(norm));
		glUniform3f(m_geometry_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
		glUniform3f(m_geometry_rendering_program["uniform_specular"], specularColor.r, specularColor.g, specularColor.b);
		glUniform1f(m_geometry_rendering_program["uniform_shininess"], shininess);
		glUniform1f(m_geometry_rendering_program["uniform_has_texture"], (m_pirate_right_foot->parts[j].textureID > 0) ? 1.0f : 0.0f);
		glBindTexture(GL_TEXTURE_2D, m_pirate_right_foot->parts[j].textureID);

		glDrawArrays(GL_TRIANGLES, m_pirate_right_foot->parts[j].start_offset, m_pirate_right_foot->parts[j].count);
	}

	//draw pirate body
	glBindVertexArray(m_pirate_body->m_vao);
	glUniformMatrix4fv(m_geometry_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(m_pirate_body_transformation_matrix));
	glUniformMatrix4fv(m_geometry_rendering_program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(m_pirate_body_transformation_normal_matrix));
	for (int j = 0; j < m_pirate_body->parts.size(); j++)
	{
		glm::vec3 diffuseColor = m_pirate_body->parts[j].diffuseColor;
		glm::vec3 specularColor = m_pirate_body->parts[j].specularColor;
		float shininess = m_pirate_body->parts[j].shininess;
		//0 = olos o skeletos
		//1 = skeletos xwris proswpo
		//2 = skeletos xwris kefali
		//3 = xeri kai mpantana
		//4 = mpantana
		//5 = mlkies
		//6 = mia mlkia mono
		if (j == 0) {
			//std::cout << m_pirate_body->parts.size();
			glm::mat4 tran = glm::translate(glm::mat4(1.0), glm::vec3(0, 0, 0));
			glm::mat4 matrix = m_pirate_body_transformation_matrix * glm::inverse(tran) * glm::rotate(glm::mat4(1.0f), 0.1f * sin(3.0f * pirate_continous_time), glm::vec3(0, 0, 1)) * tran;
			glm::mat4 norm = glm::mat4(glm::transpose(glm::inverse(glm::mat3(matrix))));

			glUniformMatrix4fv(m_geometry_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(matrix));
			glUniformMatrix4fv(m_geometry_rendering_program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(norm));
		}
		glUniform3f(m_geometry_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
		glUniform3f(m_geometry_rendering_program["uniform_specular"], specularColor.r, specularColor.g, specularColor.b);
		glUniform1f(m_geometry_rendering_program["uniform_shininess"], shininess);
		glUniform1f(m_geometry_rendering_program["uniform_has_texture"], (m_pirate_body->parts[j].textureID > 0) ? 1.0f : 0.0f);
		glBindTexture(GL_TEXTURE_2D, m_pirate_body->parts[j].textureID);

		glDrawArrays(GL_TRIANGLES, m_pirate_body->parts[j].start_offset, m_pirate_body->parts[j].count);
	}
	// draw pirate arm
	// ..
	glUniformMatrix4fv(m_geometry_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(m_pirate_arm_transformation_matrix));
	glUniformMatrix4fv(m_geometry_rendering_program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(m_pirate_arm_transformation_normal_matrix));
	glBindVertexArray(m_pirate_arm->m_vao);
	for (int j = 0; j < m_pirate_arm->parts.size(); j++)
	{
		glm::vec3 diffuseColor = m_pirate_arm->parts[j].diffuseColor;
		glm::vec3 specularColor = m_pirate_arm->parts[j].specularColor;
		float shininess = m_pirate_arm->parts[j].shininess;

		// Animate hand 
		if (j == 0) {
			glm::mat4 tran = glm::translate(glm::mat4(1.0), glm::vec3(4.5, 12, 0));
			glm::mat4 matrix = m_pirate_arm_transformation_matrix /* * glm::inverse(tran) */ * glm::rotate(glm::mat4(1.0f), 0.3f * sin(3.0f * pirate_continous_time), glm::vec3(1, 0, 0)) * tran;
			glm::mat4 norm = glm::mat4(glm::transpose(glm::inverse(glm::mat3(matrix))));

			glUniformMatrix4fv(m_geometry_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(matrix));
			glUniformMatrix4fv(m_geometry_rendering_program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(norm));
		}
		// Animate sword 
		else if (j == 1) {
			glm::mat4 tran = glm::translate(glm::mat4(1.0), glm::vec3(4.5, 12, 0));
			glm::mat4 matrix = m_pirate_arm_transformation_matrix */* glm::inverse(tran) * */glm::rotate(glm::mat4(1.0f), 0.3f * sin(3.0f * pirate_continous_time), glm::vec3(1, 0, 0)) * tran;
			glm::mat4 norm = glm::mat4(glm::transpose(glm::inverse(glm::mat3(matrix))));

			glUniformMatrix4fv(m_geometry_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(matrix));
			glUniformMatrix4fv(m_geometry_rendering_program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(norm));
		}
		else {
			glUniformMatrix4fv(m_geometry_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(m_pirate_arm_transformation_matrix));
			glUniformMatrix4fv(m_geometry_rendering_program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(m_pirate_arm_transformation_normal_matrix));
		}

		glUniform3f(m_geometry_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
		if (j == 7)
			glUniform3f(m_geometry_rendering_program["uniform_diffuse"], 0, 0, 1);
		glUniform3f(m_geometry_rendering_program["uniform_specular"], specularColor.r, specularColor.g, specularColor.b);
		glUniform1f(m_geometry_rendering_program["uniform_shininess"], shininess);
		glUniform1f(m_geometry_rendering_program["uniform_has_texture"], (m_pirate_arm->parts[j].textureID > 0) ? 1.0f : 0.0f);
		glBindTexture(GL_TEXTURE_2D, m_pirate_arm->parts[j].textureID);

		glDrawArrays(GL_TRIANGLES, m_pirate_arm->parts[j].start_offset, m_pirate_arm->parts[j].count);
	}
}



void Pirate::DrawShadowMapPirate() {
	glBindVertexArray(m_pirate_arm->m_vao);
	glUniformMatrix4fv(m_spot_light_shadow_map_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(m_pirate_arm_transformation_matrix));
	for (int j = 0; j < m_pirate_arm->parts.size(); j++)
	{
		glDrawArrays(GL_TRIANGLES, m_pirate_arm->parts[j].start_offset, m_pirate_arm->parts[j].count);
	}

	glBindVertexArray(m_pirate_body->m_vao);
	glUniformMatrix4fv(m_spot_light_shadow_map_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(m_pirate_body_transformation_matrix));
	for (int j = 0; j < m_pirate_body->parts.size(); j++)
	{
		glDrawArrays(GL_TRIANGLES, m_pirate_body->parts[j].start_offset, m_pirate_body->parts[j].count);
	}

	glBindVertexArray(m_pirate_left_foot->m_vao);
	glUniformMatrix4fv(m_spot_light_shadow_map_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(m_pirate_left_foot_transformation_matrix));
	for (int j = 0; j < m_pirate_left_foot->parts.size(); j++)
	{
		glDrawArrays(GL_TRIANGLES, m_pirate_left_foot->parts[j].start_offset, m_pirate_left_foot->parts[j].count);
	}

	glBindVertexArray(m_pirate_right_foot->m_vao);
	glUniformMatrix4fv(m_spot_light_shadow_map_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(m_pirate_right_foot_transformation_matrix));
	for (int j = 0; j < m_pirate_right_foot->parts.size(); j++)
	{
		glDrawArrays(GL_TRIANGLES, m_pirate_right_foot->parts[j].start_offset, m_pirate_right_foot->parts[j].count);
	}
}

bool Pirate::InitRenderingTechniques()
{

	bool initialized = true;

	vertex_shader_path = "../Data/Shaders/basic_rendering.vert";
	fragment_shader_path = "../Data/Shaders/basic_rendering.frag";
	m_geometry_rendering_program.LoadVertexShaderFromFile(vertex_shader_path.c_str());
	m_geometry_rendering_program.LoadFragmentShaderFromFile(fragment_shader_path.c_str());
	m_geometry_rendering_program.CreateProgram();
	m_geometry_rendering_program.LoadUniform("uniform_projection_matrix");
	m_geometry_rendering_program.LoadUniform("uniform_view_matrix");
	m_geometry_rendering_program.LoadUniform("uniform_model_matrix");
	m_geometry_rendering_program.LoadUniform("uniform_normal_matrix");
	m_geometry_rendering_program.LoadUniform("uniform_diffuse");
	m_geometry_rendering_program.LoadUniform("uniform_specular");
	m_geometry_rendering_program.LoadUniform("uniform_shininess");
	m_geometry_rendering_program.LoadUniform("uniform_has_texture");
	m_geometry_rendering_program.LoadUniform("diffuse_texture");
	m_geometry_rendering_program.LoadUniform("uniform_camera_position");
	// Light Source Uniforms
	m_geometry_rendering_program.LoadUniform("uniform_light_position");
	m_geometry_rendering_program.LoadUniform("uniform_light_direction");
	m_geometry_rendering_program.LoadUniform("uniform_light_color");
	m_geometry_rendering_program.LoadUniform("uniform_light_umbra");
	m_geometry_rendering_program.LoadUniform("uniform_light_penumbra");

	// Create and Compile Particle Shader
	//vertex_shader_path = "../Data/Shaders/particle_rendering.vert";
	//fragment_shader_path = "../Data/Shaders/particle_rendering.frag";
	//m_particle_rendering_program.LoadVertexShaderFromFile(vertex_shader_path.c_str());
	//m_particle_rendering_program.LoadFragmentShaderFromFile(fragment_shader_path.c_str());
	//initialized = initialized && m_particle_rendering_program.CreateProgram();
	//m_particle_rendering_program.LoadUniform("uniform_view_matrix");
	//m_particle_rendering_program.LoadUniform("uniform_projection_matrix");

	// Post Processing Program
	vertex_shader_path = "../Data/Shaders/postproc.vert";
	fragment_shader_path = "../Data/Shaders/postproc.frag";
	m_postprocess_program.LoadVertexShaderFromFile(vertex_shader_path.c_str());
	m_postprocess_program.LoadFragmentShaderFromFile(fragment_shader_path.c_str());
	initialized = initialized && m_postprocess_program.CreateProgram();
	m_postprocess_program.LoadUniform("uniform_texture");
	m_postprocess_program.LoadUniform("uniform_time");
	m_postprocess_program.LoadUniform("uniform_depth");
	m_postprocess_program.LoadUniform("uniform_projection_inverse_matrix");

	// Shadow mapping Program
	vertex_shader_path = "../Data/Shaders/shadow_map_rendering.vert";
	fragment_shader_path = "../Data/Shaders/shadow_map_rendering.frag";
	m_spot_light_shadow_map_program.LoadVertexShaderFromFile(vertex_shader_path.c_str());
	m_spot_light_shadow_map_program.LoadFragmentShaderFromFile(fragment_shader_path.c_str());
	initialized = initialized && m_spot_light_shadow_map_program.CreateProgram();
	m_spot_light_shadow_map_program.LoadUniform("uniform_projection_matrix");
	m_spot_light_shadow_map_program.LoadUniform("uniform_view_matrix");
	m_spot_light_shadow_map_program.LoadUniform("uniform_model_matrix");

	return initialized;
}

bool Pirate::DetectCollisionWithChest(glm::vec3 chest_center, float chest_radius) {

	float distance = glm::distance(center + m_skeleton_position, chest_center);
	if (distance < chest_radius + radius) {
		return true;
	}
	
	return false;
}



void Pirate::SetArm(GeometryNode* arm) {
	m_pirate_arm = arm;
}

void Pirate::SetBody(GeometryNode* body) {
	m_pirate_body = body;
}

void Pirate::SetLeftFoot(GeometryNode* left_foot) {
	m_pirate_left_foot = left_foot;
}

void Pirate::SetRightFoot(GeometryNode* right_foot) {
	m_pirate_right_foot = right_foot;
}

