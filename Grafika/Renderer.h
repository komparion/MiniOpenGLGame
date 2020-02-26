#ifndef BIM_ENGINE_RENDERER_H
#define BIM_ENGINE_RENDERER_H

#include "GLEW\glew.h"
#include "glm\glm.hpp"
#include <vector>
#include "ShaderProgram.h"
#include "SpotlightNode.h"

class Renderer
{
public:
	enum RENDERING_MODE
	{
		TRIANGLES,
		LINES,
		POINTS
	};

protected:
	int												m_screen_width, m_screen_height,road;
	glm::mat4										m_view_matrix;
	glm::mat4										m_projection_matrix;
	glm::vec3										m_camera_position;
	glm::vec3										m_camera_target_position;
	glm::vec3										m_camera_up_vector;
	glm::vec2										m_camera_movement;
	glm::vec2										m_camera_look_angle_destination;
	
	glm::vec2										m_skeleton_movement;

	glm::vec2										m_tile_movement;
	glm::vec3										m_tile_position;
	// Geometry Rendering Intermediate Buffer
	GLuint m_fbo;
	GLuint m_fbo_depth_texture;
	GLuint m_fbo_texture;
	GLuint m_nearest_sampler, m_linear_sampler, m_trilinear_sampler;
	GLuint m_wrapping_samplers[3];
	GLuint m_vao_fbo, m_vbo_fbo_vertices;

	float m_continous_time;

	// Rendering Mode
	RENDERING_MODE m_rendering_mode;

	// Lights
	SpotLightNode m_spotlight_node;

	// Meshes
	class GeometryNode*								m_geometric_object1;
	glm::mat4										m_geometric_object1_transformation_matrix;
	glm::mat4										m_geometric_object1_transformation_normal_matrix;
	class GeometryNode*								m_geometric_object2;
	glm::mat4										m_geometric_object2_transformation_matrix;
	glm::mat4										m_geometric_object2_transformation_normal_matrix;
	class GeometryNode*								m_geometric_redplane;
	glm::mat4										m_geometric_redplane_transformation_matrix;
	glm::mat4										m_geometric_redplane_transformation_normal_matrix;
	class GeometryNode*								m_geometric_tower;
	glm::mat4										m_geometric_tower_transformation_matrix;
	glm::mat4										m_geometric_tower_transformation_normal_matrix;
	class GeometryNode*								m_geometric_greenplane;
	glm::mat4										m_geometric_greenplane_transformation_matrix;
	glm::mat4										m_geometric_greenplane_transformation_normal_matrix;
	class GeometryNode*								m_geometric_cannonball;
	class GeometryNode*								m_geometric_chest;
	glm::mat4										m_geometric_chest_transformation_matrix;
	glm::mat4										m_geometric_chest_transformation_normal_matrix;
	class GeometryNode*                             m_pirate_body;
	glm::mat4										m_pirate_body_transformation_matrix;
	glm::mat4										m_pirate_body_transformation_normal_matrix;
	class GeometryNode*                             m_pirate_arm;
	glm::mat4										m_pirate_arm_transformation_matrix;
	glm::mat4										m_pirate_arm_transformation_normal_matrix;
	class GeometryNode*                             m_pirate_left_foot;
	glm::mat4										m_pirate_left_foot_transformation_matrix;
	glm::mat4										m_pirate_left_foot_transformation_normal_matrix;
	class GeometryNode*                             m_pirate_right_foot;
	glm::mat4										m_pirate_right_foot_transformation_matrix;
	glm::mat4										m_pirate_right_foot_transformation_normal_matrix;
	class GeometryNode* m_geometric_enemy;
	glm::mat4										m_geometric_enemy_transformation_matrix;
	glm::mat4										m_geometric_enemy_transformation_normal_matrix;
	

	// Protected Functions
	bool InitRenderingTechniques();
	bool InitIntermediateShaderBuffers();
	bool InitCommonItems();
	bool InitLightSources();
	bool InitGeometricMeshes();

	void DrawGeometryNode(class GeometryNode* node, glm::mat4 model_matrix, glm::mat4 normal_matrix);
	void DrawGeometryNodeToShadowMap(class GeometryNode* node, glm::mat4 model_matrix, glm::mat4 normal_matrix);
	ShaderProgram								m_geometry_rendering_program;
	ShaderProgram								m_postprocess_program;
	ShaderProgram								m_spot_light_shadow_map_program;

public:
	Renderer();
	~Renderer();
	bool										Init(int SCREEN_WIDTH, int SCREEN_HEIGHT);
	void										Update(float dt);
	bool										ResizeBuffers(int SCREEN_WIDTH, int SCREEN_HEIGHT);
	bool										ReloadShaders();
	void										Render();

	void PutTower();

	void RemoveTower();

	int isTower();

	// Passes
	void										RenderShadowMaps();
	void										RenderGeometry();
	void DrawTrees(GeometryNode* node, glm::mat4 model_matrix, glm::mat4 normal_matrix);

	void ShadowTrees(GeometryNode* node, glm::mat4 model_matrix, glm::mat4 normal_matrix);

	void IfEnemy(float time);

	

	
	
	int IfRoad();
	
	void										RenderToOutFB();
	
	// Set functions
	void										SetRenderingMode(RENDERING_MODE mode);

	// Camera Function
	void										CameraMoveForward(bool enable);
	void										CameraMoveBackWard(bool enable);
	void										CameraMoveLeft(bool enable);
	void										CameraMoveRight(bool enable);
	void										CameraLook(glm::vec2 lookDir);
	void TileMoveForward(bool enable);

	void TileMoveBackWard(bool enable);

	void TileMoveLeft(bool enable);

	void TileMoveRight(bool enable);

	void										SkeletonMoveForward(bool enable);
	void										SkeletonMoveBackWard(bool enable);
	void										SkeletonMoveLeft(bool enable);
	void										SkeletonMoveRight(bool enable);
};

#endif

