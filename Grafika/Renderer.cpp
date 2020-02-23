#include "Renderer.h"
#include "GeometryNode.h"
#include "Tools.h"
#include <algorithm>
#include "ShaderProgram.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "OBJLoader.h"
#include "Tower.h"
#include "CannonBall.h"
#include "Pirate.h"
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>


// RENDERER
 int RemoveTowers = 0;
 int inTowers = 0;
 Tower* Towers[15];
 CannonBall* CannonBalls[15];
 int road_tiles[29][2] = {
		{0,0} ,
		{0,4},
		{0,8} ,
		{0,12},{4,12},{4,16},{4,20},{4,24},{4,28},{8,28},{8,32},{12,32},{16,32},{20,32},
		{24,32},{24,28},{24,24},{28,24},{28,20},{28,16},{28,12},{32,12},{36,12},{36,8},{36,4},{32,4},{28,4},{24,4},{24,0} };
 int Tower_put_time;
 int Tower_Remove_time;
 int pivot_remove=0;
 int pivot_put=0;
 std::vector<glm::vec3> Chests;
 int chests[3][3] = { {24,-2,100},{26,-4,100},{22,-4,100} };
 glm::vec3 Chest_Center = glm::vec3(-0.1760 * 0.09, 10.8174 * 0.09, -8.0619 * 0.09);
 glm::vec3 Tower_Center = glm::vec3((-0.0101 * 0.4, 6.3225 * 0.4, -0.0758 * 0.4));
 float chest_radius = 12.0284 * 0.09;
 std::vector<Pirate*> Pirates;
 float TimeSinceLastPirateSpawned = 0.0;
 //Pirates per wave
 int wave = 5;
 //spawned pirates in this wave
 int SpawnedPirates = 0;
 int Emptied_Chests = 0;
 Mix_Music *gMusic = NULL;
  
  
Renderer::Renderer()
{
	m_vbo_fbo_vertices = 0;
	m_vao_fbo = 0;
	
	m_geometric_object1 = nullptr;
	m_geometric_object2 = nullptr;
	m_geometric_redplane = nullptr;
	m_geometric_greenplane = nullptr;
	m_geometric_tower= nullptr;
	m_geometric_cannonball = nullptr;
	m_geometric_chest = nullptr;
	m_geometric_enemy = nullptr;

	road = 0;
	m_fbo = 0;
	m_fbo_texture = 0;


	Chests.push_back(glm::vec3(24, -2, 100));
	Chests.push_back(glm::vec3(26, -4, 100));
	Chests.push_back(glm::vec3(22, -4, 100));
	Chest_Center += glm::vec3(24, 0, -2);

	m_rendering_mode = RENDERING_MODE::TRIANGLES;	
	m_continous_time = 0.0;

	// initialize the camera parameters
	m_camera_position = glm::vec3(0.720552, 18.1377, -11.3135);
	m_camera_target_position = glm::vec3(4.005, 12.634, -5.66336);
	m_camera_up_vector = glm::vec3(0, 1, 0);

	m_tile_position = glm::vec3(16, 0, 8);
}

Renderer::~Renderer()
{
	// delete g_buffer
	glDeleteTextures(1, &m_fbo_texture);
	glDeleteFramebuffers(1, &m_fbo);

	// delete common data
	glDeleteVertexArrays(1, &m_vao_fbo);
	glDeleteBuffers(1, &m_vbo_fbo_vertices);

	delete m_geometric_object1;
	delete m_geometric_object2;
	delete m_geometric_redplane;
	delete m_geometric_greenplane;
	delete m_geometric_tower;
	delete m_geometric_chest;
	delete m_geometric_cannonball;
	delete m_geometric_enemy;
	for (auto& pirate : Pirates)
		delete pirate;
	Pirates.clear();
	
	Mix_FreeMusic(gMusic);
	gMusic = NULL;

	Mix_Quit();
	SDL_Quit();
	//delete Towers;
	//delete CannonBalls;
}

bool Renderer::Init(int SCREEN_WIDTH, int SCREEN_HEIGHT)
{
	this->m_screen_width = SCREEN_WIDTH;
	this->m_screen_height = SCREEN_HEIGHT;

	// Initialize OpenGL functions

	//Set clear color
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	//This enables depth and stencil testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearDepth(1.0f);
	// glClearDepth sets the value the depth buffer is set at, when we clear it

	// Enable back face culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// open the viewport
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT); //we set up our viewport

	bool techniques_initialization = InitRenderingTechniques();
	bool buffers_initialization = InitIntermediateShaderBuffers();
	bool items_initialization = InitCommonItems();
	bool lights_sources_initialization = InitLightSources();
	bool meshes_initialization = InitGeometricMeshes();

	//If there was any errors
	if (Tools::CheckGLError() != GL_NO_ERROR)
	{
		printf("Exiting with error at Renderer::Init\n");
		return false;
	}

	bool SDL_initialization = true;
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		SDL_initialization = false;
	}
	bool SDL_Mixer_initialization = true;
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
		SDL_Mixer_initialization = false;
	}
	
	//If everything initialized
	return techniques_initialization && items_initialization && buffers_initialization 
		&& meshes_initialization && lights_sources_initialization && SDL_initialization && SDL_Mixer_initialization;
}

void Renderer::Update(float dt)
{
	float movement_speed = 6.0f;
	// compute the direction of the camera
	glm::vec3 direction = glm::normalize(m_camera_target_position - m_camera_position);

	// move the camera towards the direction of the camera
	m_camera_position += m_camera_movement.x *  movement_speed * direction * dt;
	m_camera_target_position += m_camera_movement.x * movement_speed * direction * dt;

	// move the camera sideways
	glm::vec3 right = glm::normalize(glm::cross(direction, m_camera_up_vector));
	m_camera_position += m_camera_movement.y *  movement_speed * right * dt;
	m_camera_target_position += m_camera_movement.y * movement_speed * right * dt;

	glm::mat4 rotation = glm::mat4(1.0f);
	float angular_speed = glm::pi<float>() * 0.0025f;

	// compute the rotation of the camera based on the mouse movement
	rotation *= glm::rotate(glm::mat4(1.0), m_camera_look_angle_destination.y * angular_speed, right);
	rotation *= glm::rotate(glm::mat4(1.0), -m_camera_look_angle_destination.x  * angular_speed, m_camera_up_vector);
	m_camera_look_angle_destination = glm::vec2(0);

	// rotate the camera direction
	direction = rotation * glm::vec4(direction, 0);
	float dist = glm::distance(m_camera_position, m_camera_target_position);
	m_camera_target_position = m_camera_position + direction * dist;

	// compute the view matrix
	m_view_matrix = glm::lookAt(m_camera_position, m_camera_target_position, m_camera_up_vector);

	m_continous_time += dt;
	Tower_put_time = m_continous_time;
	Tower_Remove_time = m_continous_time;
	// update meshes tranformations
	glm::mat4 object_translation = glm::translate(glm::mat4(1.0), glm::vec3(18, -0.0009, 18));
	glm::mat4 object_scale = glm::scale(glm::mat4(1.0), glm::vec3(20.f));
	m_geometric_object1_transformation_matrix = object_translation * object_scale;
	m_geometric_object1_transformation_normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(m_geometric_object1_transformation_matrix))));

	
	glm::mat4 object_translation2 = glm::translate(glm::mat4(1.0), m_tile_position);
	glm::mat4 object_scale2 = glm::scale(glm::mat4(1.0), glm::vec3(2.f));
	m_geometric_greenplane_transformation_matrix = object_translation2 * object_scale2;
	m_geometric_greenplane_transformation_normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(m_geometric_object2_transformation_matrix))));

	//m_skeleton_position.z += m_skeleton_movement.x * 0.001f;
	//m_skeleton_position.x += m_skeleton_movement.y * 0.001f;
	//glm::mat4 tran = glm::translate(glm::mat4(1.f), m_skeleton_position);
	glm::mat4 rotate180degrees = glm::rotate(glm::mat4(1.f), glm::radians(180.0f), glm::vec3(0, 1, 0));

	if (m_continous_time >= 5) {
		
		//If there is no music playing
		if (Mix_PlayingMusic() == 0)
		{
			//Play the music
			Mix_PlayMusic(gMusic, -1);
		}
		if (m_continous_time - TimeSinceLastPirateSpawned >= 0.5 && SpawnedPirates < wave) {
			Pirate* Pir = new Pirate(m_continous_time);
			Pir->SetArm(m_pirate_arm);
			Pir->SetBody(m_pirate_body);
			Pir->SetLeftFoot(m_pirate_left_foot);
			Pir->SetRightFoot(m_pirate_right_foot);

			Pirates.push_back(Pir);
			TimeSinceLastPirateSpawned = m_continous_time;
			SpawnedPirates++;
		}
		if (m_continous_time - TimeSinceLastPirateSpawned >= 20) {
			wave++;
			SpawnedPirates = 0;
		}
		//TimeSinceLastPirateSpawned = m_continous_time;

		std::vector<Pirate*>::iterator iter, end;
		int currentChest = Chests.size() - 1;
		for (iter = Pirates.begin(), end = Pirates.end(); iter != end; ++iter) {
			(*iter)->Update(dt);
			if ((*iter)->DetectCollisionWithChest(Chest_Center, chest_radius)) {
				std::cout << "we collided ";
				Chests.at(currentChest).z -= 10;
				//std::cout << Chests.at(currentChest).z;
				//(*iter)->m_skeleton_movement.x = 0.f;
				//(*iter)->m_skeleton_movement.y = 0.f;
				(*iter)->collidedWithChest = true;
				if (Chests.at(currentChest).z == 0) {

					if (Emptied_Chests < 2) {
						Emptied_Chests++;
						Chests.at(Chests.size() - 1).z = 100;
						Chests.pop_back();
						//Chests.erase(Chests.end());
					}
					else {
						std::cout << "GAME OVER! YOU LOSE!\nPress ESC to close the window";
						Chests.pop_back();
						system("pause");
					}
					//currentChest--;
				}
			}

		}

		Chests.erase(std::remove_if(Chests.begin(), Chests.end(), [](glm::vec3 chest) {return chest.z == 0; }), Chests.end());
		Pirates.erase(std::remove_if(Pirates.begin(), Pirates.end(), [](Pirate* p) {if (p->collidedWithChest) { delete p; return true; } return false; }), Pirates.end());
		//std::cout << Pirates.size();
		if (wave >= 15 && Pirates.size() == 0) {
			std::cout << "GGWP YOU ACTUALLY WON!!\nPress ESC to close the window";
			system("pause");
		}

	}

	IfEnemy(m_continous_time);

}

bool Renderer::InitCommonItems()
{
	glGenVertexArrays(1, &m_vao_fbo);
	glBindVertexArray(m_vao_fbo);

	GLfloat fbo_vertices[] = {
		-1, -1,
		1, -1,
		-1, 1,
		1, 1,
	};

	glGenBuffers(1, &m_vbo_fbo_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_fbo_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fbo_vertices), fbo_vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);

	return true;
}

bool Renderer::InitRenderingTechniques()
{
	bool initialized = true;

	// Geometry Rendering Program
	std::string vertex_shader_path = "../Data/Shaders/basic_rendering.vert";
	std::string fragment_shader_path = "../Data/Shaders/basic_rendering.frag";
	m_geometry_rendering_program.LoadVertexShaderFromFile(vertex_shader_path.c_str());
	m_geometry_rendering_program.LoadFragmentShaderFromFile(fragment_shader_path.c_str());
	initialized = m_geometry_rendering_program.CreateProgram();
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
	m_geometry_rendering_program.LoadUniform("uniform_light_projection_matrix");
	m_geometry_rendering_program.LoadUniform("uniform_light_view_matrix");
	m_geometry_rendering_program.LoadUniform("uniform_light_position");
	m_geometry_rendering_program.LoadUniform("uniform_light_direction");
	m_geometry_rendering_program.LoadUniform("uniform_light_color");
	m_geometry_rendering_program.LoadUniform("uniform_light_umbra");
	m_geometry_rendering_program.LoadUniform("uniform_light_penumbra");
	m_geometry_rendering_program.LoadUniform("uniform_cast_shadows");
	m_geometry_rendering_program.LoadUniform("shadowmap_texture");

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

bool Renderer::ReloadShaders()
{
	bool reloaded = true;

	reloaded = reloaded && m_geometry_rendering_program.ReloadProgram();
	reloaded = reloaded && m_postprocess_program.ReloadProgram();
	reloaded = reloaded && m_spot_light_shadow_map_program.ReloadProgram();

	return reloaded;
}

bool Renderer::InitIntermediateShaderBuffers()
{
	// generate texture handles 
	glGenTextures(1, &m_fbo_texture);
	glGenTextures(1, &m_fbo_depth_texture);

	// framebuffer to link everything together
	glGenFramebuffers(1, &m_fbo);

	return ResizeBuffers(m_screen_width, m_screen_height);
}

// resize post processing textures and save the screen size
bool Renderer::ResizeBuffers(int width, int height)
{
	m_screen_width = width;
	m_screen_height = height;

	// texture
	glBindTexture(GL_TEXTURE_2D, m_fbo_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_screen_width, m_screen_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	// depth texture
	glBindTexture(GL_TEXTURE_2D, m_fbo_depth_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_screen_width, m_screen_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	// framebuffer to link to everything together
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fbo_texture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_fbo_depth_texture, 0);

	GLenum status = Tools::CheckFramebufferStatus(m_fbo);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		return false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_projection_matrix = glm::perspective(glm::radians(60.f), width / (float)height, 0.1f, 1500.0f);
	m_view_matrix = glm::lookAt(m_camera_position, m_camera_target_position, m_camera_up_vector);

	return true;
}

// Initialize the light sources
bool Renderer::InitLightSources()
{
	// Initialize the spot light
	m_spotlight_node.SetPosition(glm::vec3(16, 30, 16));
	m_spotlight_node.SetTarget(glm::vec3(16.4, 0, 16));
	m_spotlight_node.SetColor(40.0f * glm::vec3(500,500,500) / 255.f);
	m_spotlight_node.SetConeSize(73, 80);
	m_spotlight_node.CastShadow(true);

	return true;
}

// Load Geometric Meshes
bool Renderer::InitGeometricMeshes()
{
	bool initialized = true;
	OBJLoader loader;
	// load geometric object 1
	auto mesh = loader.load("../Data/Terrain/terrain.obj");
	if (mesh != nullptr)
	{
		m_geometric_object1 = new GeometryNode();
		m_geometric_object1->Init(mesh);
	}
	else
		initialized = false;
	delete mesh;

	mesh = loader.load("../Data/Terrain/road.obj");
	if (mesh != nullptr)
	{
		m_geometric_object2 = new GeometryNode();
		m_geometric_object2->Init(mesh);
	}
	else
		initialized = false;
	delete mesh;

	mesh = loader.load("../Data/Various/plane_green.obj");
	if (mesh != nullptr)
	{
		m_geometric_greenplane = new GeometryNode();
		m_geometric_greenplane->Init(mesh);
	}
	else
		initialized = false;
	delete mesh;

	mesh = loader.load("../Data/Various/plane_red.obj");
	if (mesh != nullptr)
	{
		m_geometric_redplane = new GeometryNode();
		m_geometric_redplane->Init(mesh);
	}
	else
		initialized = false;
	delete mesh;

	mesh = loader.load("../Data/MedievalTower/tower.obj");
	if (mesh != nullptr)
	{
		m_geometric_tower = new GeometryNode();
		m_geometric_tower->Init(mesh);
	}
	else
		initialized = false;
	delete mesh;

	mesh = loader.load("../Data/Various/cannonball.obj");
	if (mesh != nullptr)
	{
		m_geometric_cannonball = new GeometryNode();
		m_geometric_cannonball->Init(mesh);
	}
	else
		initialized = false;
	delete mesh;

	mesh = loader.load("../Data/Treasure/treasure_chest.obj");
	if (mesh != nullptr)
	{
		m_geometric_chest = new GeometryNode();
		m_geometric_chest->Init(mesh);
	}
	else
		initialized = false;
	delete mesh;


	mesh = loader.load("../Data/Pirate/pirate_body.obj");
	if (mesh != nullptr)
	{
		m_pirate_body = new GeometryNode();
		m_pirate_body->Init(mesh);
	}
	else
		initialized = false;
	delete mesh;

	// load pirate arm
	mesh = loader.load("../Data/Pirate/pirate_arm.obj");
	if (mesh != nullptr)
	{
		m_pirate_arm = new GeometryNode();
		m_pirate_arm->Init(mesh);
	}
	else
		initialized = false;
	delete mesh;

	//load left foot
	mesh = loader.load("../Data/Pirate/pirate_left_foot.obj");
	if (mesh != nullptr)
	{
		m_pirate_left_foot = new GeometryNode();
		m_pirate_left_foot->Init(mesh);
	}
	else
		initialized = false;
	delete mesh;

	//load right foot
	mesh = loader.load("../Data/Pirate/pirate_right_foot.obj");
	if (mesh != nullptr)
	{
		m_pirate_right_foot = new GeometryNode();
		m_pirate_right_foot->Init(mesh);
	}
	else
		initialized = false;
	delete mesh;

	//Load music
	gMusic = Mix_LoadMUS("../Data/Various/crash_3_cortex_theme.ogg");
	if (gMusic == NULL)
	{
		printf("Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
		initialized = false;
	}
	return initialized;
}

void Renderer::SetRenderingMode(RENDERING_MODE mode)
{
	m_rendering_mode = mode;
}

// Render the Scene
void Renderer::Render()
{
	// Draw the geometry to the shadow maps
	RenderShadowMaps();

	// Draw the geometry
	RenderGeometry();

	// Render to screen
	RenderToOutFB();

	GLenum error = Tools::CheckGLError();
	if (error != GL_NO_ERROR)
	{
		printf("Reanderer:Draw GL Error\n");
		system("pause");
	}
}

void Renderer::RenderShadowMaps()
{
	// if the light source casts shadows
	if (m_spotlight_node.GetCastShadowsStatus())
	{
		int m_depth_texture_resolution = m_spotlight_node.GetShadowMapResolution();

		// bind the framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, m_spotlight_node.GetShadowMapFBO());
		glViewport(0, 0, m_depth_texture_resolution, m_depth_texture_resolution);
		GLenum drawbuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, drawbuffers);
		
		// clear depth buffer
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		
		// Bind the shadow mapping program
		m_spot_light_shadow_map_program.Bind();

		// pass the projection and view matrix to the uniforms
		glUniformMatrix4fv(m_spot_light_shadow_map_program["uniform_projection_matrix"], 1, GL_FALSE, glm::value_ptr(m_spotlight_node.GetProjectionMatrix()));
		glUniformMatrix4fv(m_spot_light_shadow_map_program["uniform_view_matrix"], 1, GL_FALSE, glm::value_ptr(m_spotlight_node.GetViewMatrix()));
		// draw the first object
		// draw the first object
		DrawGeometryNodeToShadowMap(m_geometric_object1, m_geometric_object1_transformation_matrix, m_geometric_object1_transformation_normal_matrix);

		// draw the second object
		
		for (int i = 0; i <= 28; i++) {
			glm::mat4 object_translation2 = glm::translate(glm::mat4(1.0), glm::vec3(road_tiles[i][0], 0, road_tiles[i][1]));
			glm::mat4 object_scale2 = glm::scale(glm::mat4(1.0), glm::vec3(2.f));
			m_geometric_object2_transformation_matrix = object_translation2 * object_scale2;
			m_geometric_object2_transformation_normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(m_geometric_object1_transformation_matrix))));

			DrawGeometryNodeToShadowMap(m_geometric_object2, m_geometric_object2_transformation_matrix, m_geometric_object2_transformation_normal_matrix);
		}

		if (inTowers > 0) {
			for (int tgwr = 0; tgwr < inTowers; tgwr++) {
				if (Towers[tgwr]->GetPositionX() != 0 || Towers[tgwr]->GetPositionZ() != 0) {

					DrawGeometryNodeToShadowMap(m_geometric_tower, Towers[tgwr]->GetTransformationMatrix(), Towers[tgwr]->GetNormalMatrix());
					DrawGeometryNodeToShadowMap(m_geometric_cannonball, CannonBalls[tgwr]->GetTransformationMatrix(), CannonBalls[tgwr]->GetNormalMatrix());
				}
			}

		}

		//chests
		/*for (int che = 0; che < 3; che++) {
			glm::mat4 object_translation4 = glm::translate(glm::mat4(1.0), glm::vec3(chests[che][0], 0, chests[che][1]));
			glm::mat4 object_scale4 = glm::scale(glm::mat4(1.0), glm::vec3(0.09f));
			m_geometric_chest_transformation_matrix = object_translation4 * object_scale4;
			m_geometric_chest_transformation_normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(m_geometric_chest_transformation_matrix))));
			DrawGeometryNodeToShadowMap(m_geometric_chest, m_geometric_chest_transformation_matrix, m_geometric_chest_transformation_normal_matrix);
		}*/

		if (Chests.size() > 0) {
			for (int che = 0; che < Chests.size(); che++) {
				glm::mat4 object_translation4 = glm::translate(glm::mat4(1.0), glm::vec3(Chests.at(che).x, 0, Chests.at(che).y));
				glm::mat4 object_scale4 = glm::scale(glm::mat4(1.0), glm::vec3(0.09f));
				m_geometric_chest_transformation_matrix = object_translation4 * object_scale4;
				m_geometric_chest_transformation_normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(m_geometric_chest_transformation_matrix))));
				//DrawGeometryNode(m_geometric_chest, m_geometric_chest_transformation_matrix, m_geometric_chest_transformation_normal_matrix);
				DrawGeometryNodeToShadowMap(m_geometric_chest, m_geometric_chest_transformation_matrix, m_geometric_chest_transformation_normal_matrix);
			}
		}
		
		//ShadowTrees(m_geometric_tree, m_geometric_tree_transformation_matrix, m_geometric_tree_transformation_normal_matrix);

		if (Pirates.size() > 0) {
			std::vector<Pirate*>::iterator iter, end;
			for (iter = Pirates.begin(), end = Pirates.end(); iter != end; ++iter)
				(*iter)->DrawShadowMapPirate();
		}

		glBindVertexArray(0);

		// Unbind shadow mapping program
		m_spot_light_shadow_map_program.Unbind();
		
		glDisable(GL_DEPTH_TEST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}


void Renderer::RenderGeometry()
{
	// Bind the Intermediate framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glViewport(0, 0, m_screen_width, m_screen_height);
	GLenum drawbuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawbuffers);

	// clear color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	switch (m_rendering_mode)
	{
	case RENDERING_MODE::TRIANGLES:
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case RENDERING_MODE::LINES:
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case RENDERING_MODE::POINTS:
		glPointSize(2);
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		break;
	};

	// Bind the geometry rendering program
	m_geometry_rendering_program.Bind();

	// pass camera parameters to uniforms
	glUniformMatrix4fv(m_geometry_rendering_program["uniform_projection_matrix"], 1, GL_FALSE, glm::value_ptr(m_projection_matrix));
	glUniformMatrix4fv(m_geometry_rendering_program["uniform_view_matrix"], 1, GL_FALSE, glm::value_ptr(m_view_matrix));
	glUniform3f(m_geometry_rendering_program["uniform_camera_position"], m_camera_position.x, m_camera_position.y, m_camera_position.z);

	// pass the light source parameters to uniforms
	glm::vec3 light_position = m_spotlight_node.GetPosition();
	glm::vec3 light_direction = m_spotlight_node.GetDirection();
	glm::vec3 light_color = m_spotlight_node.GetColor();
	glUniformMatrix4fv(m_geometry_rendering_program["uniform_light_projection_matrix"], 1, GL_FALSE, glm::value_ptr(m_spotlight_node.GetProjectionMatrix()));
	glUniformMatrix4fv(m_geometry_rendering_program["uniform_light_view_matrix"], 1, GL_FALSE, glm::value_ptr(m_spotlight_node.GetViewMatrix()));
	glUniform3f(m_geometry_rendering_program["uniform_light_position"], light_position.x, light_position.y, light_position.z);
	glUniform3f(m_geometry_rendering_program["uniform_light_direction"], light_direction.x, light_direction.y, light_direction.z);
	glUniform3f(m_geometry_rendering_program["uniform_light_color"], light_color.x, light_color.y, light_color.z);
	glUniform1f(m_geometry_rendering_program["uniform_light_umbra"], m_spotlight_node.GetUmbra());
	glUniform1f(m_geometry_rendering_program["uniform_light_penumbra"], m_spotlight_node.GetPenumbra());
	glUniform1i(m_geometry_rendering_program["uniform_cast_shadows"], (m_spotlight_node.GetCastShadowsStatus()) ? 1 : 0);
	// Set the sampler2D uniform to use texture unit 1
	glUniform1i(m_geometry_rendering_program["shadowmap_texture"], 1);
	// Bind the shadow map texture to texture unit 1
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, (m_spotlight_node.GetCastShadowsStatus()) ? m_spotlight_node.GetShadowMapDepthTexture() : 0);

	// Enable Texture Unit 0
	glUniform1i(m_geometry_rendering_program["uniform_diffuse_texture"], 0);
	glActiveTexture(GL_TEXTURE0);

	// draw the first object
	DrawGeometryNode(m_geometric_object1, m_geometric_object1_transformation_matrix, m_geometric_object1_transformation_normal_matrix);

	// draw the second object
	
	for (int i = 0; i <= 28; i++) {
		glm::mat4 object_translation2 = glm::translate(glm::mat4(1.0), glm::vec3(road_tiles[i][0], 0, road_tiles[i][1]));

		glm::mat4 object_scale2 = glm::scale(glm::mat4(1.0), glm::vec3(2.f));
		m_geometric_object2_transformation_matrix = object_translation2 * object_scale2;
		m_geometric_object2_transformation_normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(m_geometric_object2_transformation_matrix))));

		DrawGeometryNode(m_geometric_object2, m_geometric_object2_transformation_matrix, m_geometric_object2_transformation_normal_matrix);


	}

	road = IfRoad();
	
	if (road == 1) {
		glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE);
		DrawGeometryNode(m_geometric_redplane, m_geometric_greenplane_transformation_matrix, m_geometric_greenplane_transformation_normal_matrix);

	}
	else {
		glColorMask(GL_FALSE, GL_TRUE, GL_FALSE, GL_FALSE);
		DrawGeometryNode(m_geometric_greenplane, m_geometric_greenplane_transformation_matrix, m_geometric_greenplane_transformation_normal_matrix);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		if (inTowers < 3 || Tower_put_time - pivot_put >= 120 || RemoveTowers > 0) {
			m_geometric_tower_transformation_matrix = glm::translate(glm::mat4(1.f), glm::vec3(m_tile_position)) * glm::scale(glm::mat4(1.f), glm::vec3(0.4f));
			m_geometric_tower_transformation_normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(m_geometric_tower_transformation_matrix))));
			glColorMask(GL_FALSE, GL_TRUE, GL_TRUE, GL_FALSE);
			DrawGeometryNode(m_geometric_tower, m_geometric_tower_transformation_matrix, m_geometric_tower_transformation_normal_matrix);
		}
	}
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);


	if (inTowers > 0) {
		for (int twr = 0; twr < inTowers; twr++) {
			if (Towers[twr]->GetPositionX() !=0 || Towers[twr]->GetPositionZ() != 0) {

			DrawGeometryNode(m_geometric_tower, Towers[twr]->GetTransformationMatrix(), Towers[twr]->GetNormalMatrix());
			DrawGeometryNode(m_geometric_cannonball, CannonBalls[twr]->GetTransformationMatrix(), CannonBalls[twr]->GetNormalMatrix());

		}
		}

	}

	//chests
	/*for (int che = 0; che < 3; che++) {
		glm::mat4 object_translation4 = glm::translate(glm::mat4(1.0), glm::vec3(chests[che][0], 0, chests[che][1]));
		glm::mat4 object_scale4 = glm::scale(glm::mat4(1.0), glm::vec3(0.09f));
		m_geometric_chest_transformation_matrix = object_translation4 * object_scale4;
		m_geometric_chest_transformation_normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(m_geometric_chest_transformation_matrix))));
		DrawGeometryNode(m_geometric_chest, m_geometric_chest_transformation_matrix, m_geometric_chest_transformation_normal_matrix);
	}*/
	if (Chests.size() > 0) {
		for (int che = 0; che < Chests.size(); che++) {
			glm::mat4 object_translation4 = glm::translate(glm::mat4(1.0), glm::vec3(Chests.at(che).x, 0, Chests.at(che).y));
			glm::mat4 object_scale4 = glm::scale(glm::mat4(1.0), glm::vec3(0.09f));
			m_geometric_chest_transformation_matrix = object_translation4 * object_scale4;
			m_geometric_chest_transformation_normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(m_geometric_chest_transformation_matrix))));
			DrawGeometryNode(m_geometric_chest, m_geometric_chest_transformation_matrix, m_geometric_chest_transformation_normal_matrix);
			//DrawGeometryNodeToShadowMap(m_geometric_chest, m_geometric_chest_transformation_matrix, m_geometric_chest_transformation_normal_matrix);
		}
	}

	//DrawGeometryNode(m_geometric_pirate_body, m_geometric_pirate_body_transformation_matrix, m_geometric_pirate_body_transformation_normal_matrix);
	
	//DrawTrees(m_geometric_tree, m_geometric_tree_transformation_matrix, m_geometric_tree_transformation_normal_matrix);

	if (Pirates.size() > 0) {
		std::vector<Pirate*>::iterator iter, end;
		for (iter = Pirates.begin(), end = Pirates.end(); iter != end; ++iter)
			(*iter)->DrawGeometryPirate(m_continous_time);
	}

	glBindVertexArray(0);
	m_geometry_rendering_program.Unbind();

	glDisable(GL_DEPTH_TEST);
	if(m_rendering_mode != RENDERING_MODE::TRIANGLES)
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glPointSize(1.0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawTrees(GeometryNode* node, glm::mat4 model_matrix, glm::mat4 normal_matrix) {
	for (int h = 0; h <= 28; h++) {
		if (h == 3 || h ==8 || h == 10 | h == 27) {
			model_matrix = glm::translate(glm::mat4(1.0), glm::vec3(road_tiles[h][0] , 0, road_tiles[h][1]+2)) * glm::rotate(glm::mat4(1.f), glm::radians(-90.0f), glm::vec3(1, 0, 0)) * glm::scale(glm::mat4(1.0), glm::vec3(0.004f));
			normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(model_matrix))));
			DrawGeometryNode(node, model_matrix, normal_matrix);
		}
		else {
			if (road_tiles[h][0] == road_tiles[h + 1][0]) {
				model_matrix = glm::translate(glm::mat4(1.0), glm::vec3(road_tiles[h][0] + 2, 0, road_tiles[h][1])) * glm::rotate(glm::mat4(1.f), glm::radians(-90.0f), glm::vec3(1, 0, 0)) * glm::scale(glm::mat4(1.0), glm::vec3(0.004f));
				normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(model_matrix))));
				DrawGeometryNode(node, model_matrix, normal_matrix);
			}
			else {
				model_matrix = glm::translate(glm::mat4(1.0), glm::vec3(road_tiles[h][0], 0, road_tiles[h][1] - 2)) * glm::rotate(glm::mat4(1.f), glm::radians(-90.0f), glm::vec3(1, 0, 0)) * glm::scale(glm::mat4(1.0), glm::vec3(0.004f));
				normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(model_matrix))));
				DrawGeometryNode(node, model_matrix, normal_matrix);
			}
		}
	}
}

void Renderer::ShadowTrees(GeometryNode* node, glm::mat4 model_matrix, glm::mat4 normal_matrix) {
	for (int h = 0; h <= 28; h++) {
		if (h == 3 || h == 8 || h == 10 | h == 27) {
			model_matrix = glm::translate(glm::mat4(1.0), glm::vec3(road_tiles[h][0], 0, road_tiles[h][1] + 2)) * glm::rotate(glm::mat4(1.f), glm::radians(-90.0f), glm::vec3(1, 0, 0)) * glm::scale(glm::mat4(1.0), glm::vec3(0.004f));
			normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(model_matrix))));
			DrawGeometryNodeToShadowMap(node, model_matrix, normal_matrix);
		}
		else {
			if (road_tiles[h][0] == road_tiles[h + 1][0]) {
				model_matrix = glm::translate(glm::mat4(1.0), glm::vec3(road_tiles[h][0] + 2, 0, road_tiles[h][1])) * glm::rotate(glm::mat4(1.f), glm::radians(-90.0f), glm::vec3(1, 0, 0)) * glm::scale(glm::mat4(1.0), glm::vec3(0.004f));
				normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(model_matrix))));
				DrawGeometryNodeToShadowMap(node, model_matrix, normal_matrix);
			}
			else {
				model_matrix = glm::translate(glm::mat4(1.0), glm::vec3(road_tiles[h][0], 0, road_tiles[h][1] - 2)) * glm::rotate(glm::mat4(1.f), glm::radians(-90.0f), glm::vec3(1, 0, 0)) * glm::scale(glm::mat4(1.0), glm::vec3(0.004f));
				normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(model_matrix))));
				DrawGeometryNodeToShadowMap(node, model_matrix, normal_matrix);
			}
		}
	}
}



void Renderer::PutTower() {
	int isTower = 0;
	
		for (int twrs = 0; twrs < inTowers; twrs++) {
			if (Towers[twrs]->GetPositionX() == m_tile_position.x && Towers[twrs]->GetPositionZ() == m_tile_position.z) {

				isTower = 1;
			}
		}
		if (isTower == 0) {
			int l = IfRoad();
			if (l == 0) {
				//std::cout << inTowers;
				if (RemoveTowers > 0) {
					Towers[RemoveTowers-1]->SetCoordinates(m_tile_position.x, m_tile_position.z);
					Towers[RemoveTowers - 1]->SetTransformationMatrix(m_tile_position);
					Towers[RemoveTowers - 1]->SetNormalMatrix(Towers[RemoveTowers - 1]->GetTransformationMatrix());
					CannonBalls[RemoveTowers - 1]->SetCoordinates(m_tile_position.x, m_tile_position.z);
					CannonBalls[RemoveTowers - 1]->SetTransformationMatrix(m_tile_position);
					CannonBalls[RemoveTowers - 1]->SetNormalMatrix(Towers[RemoveTowers - 1]->GetTransformationMatrix());
					RemoveTowers = 0;
				}
				else {
					if (inTowers < 3 || Tower_put_time - pivot_put >= 120) {
						if (inTowers <= 15) {
							Towers[inTowers] = new Tower();
							Towers[inTowers]->SetCoordinates(m_tile_position.x, m_tile_position.z);
							Towers[inTowers]->SetTransformationMatrix(m_tile_position);
							Towers[inTowers]->SetNormalMatrix(Towers[inTowers]->GetTransformationMatrix());
							CannonBalls[inTowers] = new CannonBall();
							CannonBalls[inTowers]->SetCoordinates(m_tile_position.x, m_tile_position.z);
							CannonBalls[inTowers]->SetTransformationMatrix(m_tile_position);
							CannonBalls[inTowers]->SetNormalMatrix(Towers[inTowers]->GetTransformationMatrix());
							
							inTowers++;
							pivot_put = Tower_put_time;
						}
					}
				}
			}
		}
		
}

void Renderer::IfEnemy(float time) {
	for (int twr = 0; twr < inTowers; twr++) {
		
		if (Towers[twr]->GetPositionX() != 0 || Towers[twr]->GetPositionZ() != 0) {
			if (Pirates.size() > 0) {
				std::vector<Pirate*>::iterator iter, end;
				for (iter = Pirates.begin(), end = Pirates.end(); iter != end; ++iter) {
					glm::vec3 a = { Towers[twr]->GetPositionX() + Tower_Center.x, 0 + Tower_Center.y, Towers[twr]->GetPositionZ() + Tower_Center.z };
					float distance = glm::distance((*iter)->center + (*iter)->m_skeleton_position,a);
					//std::cout << distance;
					//std::cout << " ";
					

					if (distance <= 8) {
						if (time - Towers[twr]->GetTimeHit() > 5) {
							if ((*iter)->target == false){
								CannonBalls[twr]->SetLockedEnemy((*iter), time);
								Towers[twr]->SetTimeHit(time);
								(*iter)->target = true;
							}
						
						}
						
					}
					
				}
					
			}
		}
		 if (CannonBalls[twr]->Getshooting() == true) {
			CannonBalls[twr]->Shoot(time);
			if (CannonBalls[twr]->Getcollision() == true){

				CannonBalls[twr]->Setcollision(false);
			}
		}
	}
}

void Renderer::RemoveTower() {

	if (Tower_Remove_time - pivot_remove >=30) {
		for (int twrs = 0; twrs < inTowers; twrs++) {
			if (Towers[twrs]->GetPositionX() == m_tile_position.x && Towers[twrs]->GetPositionZ() == m_tile_position.z) {

				RemoveTowers = twrs+1;
				Towers[twrs]->SetCoordinates(0,0);
				CannonBalls[twrs]->SetCoordinates(0, 0);
				pivot_remove = Tower_Remove_time;
			}
		}
	}
}

int Renderer::IfRoad() {
	
	if (0== m_tile_position.x) {
		
		if (0 == m_tile_position.z) {

			return  1;
		}
		if (4 == m_tile_position.z) {

			return  1;
		}
		if (8 == m_tile_position.z) {

			return  1;
		}
		if (12 == m_tile_position.z) {

			return  1;
		}
		return 0;
		
	}
	if (4 == m_tile_position.x ) {
		if (12 == m_tile_position.z) {

			return  1;
		}
		if ( 16 == m_tile_position.z) {

			return  1;
		}
		if (20 == m_tile_position.z) {

			return  1;
		}
		if (24 == m_tile_position.z) {

			return  1;
		}
		if (28 == m_tile_position.z) {

			return  1;
		}

		return  0;
	}
	if (8 == m_tile_position.x) {
		if (28 == m_tile_position.z) {

			return  1;
		}
		if (32 == m_tile_position.z) {

			return  1;
		}
		return 0;
	}
	if (32 == m_tile_position.z) {
		if (12 == m_tile_position.x) {

			return  1;
		}
		if (16 == m_tile_position.x) {

			return  1;
		}
		if (20 == m_tile_position.x) {

			return  1;
		}
		if (24 == m_tile_position.x) {

			return  1;
		}

		return  0;
	}
	if (24 == m_tile_position.x) {
		if (28 == m_tile_position.z) {

			return  1;
		}
		if (24 == m_tile_position.z) {

			return  1;
		}
		if (4 == m_tile_position.z) {

			return  1;
		}
		if (0 == m_tile_position.z) {

			return  1;
		}
		return 0;
	}
	if (28 == m_tile_position.x) {

		if (24 == m_tile_position.z) {

			return  1;
		}
		if (20 == m_tile_position.z) {

			return  1;
		}
		if (16 == m_tile_position.z) {

			return  1;
		}
		if (12 == m_tile_position.z) {

			return  1;
		}
		if (4 == m_tile_position.z) {

			return  1;
		}
		return 0;

	}
	if (32 == m_tile_position.x) {
		if (12 == m_tile_position.z) {

			return  1;
		}
		if (4 == m_tile_position.z) {

			return  1;
		}
		return 0;
	}
	if (36 == m_tile_position.x) {
		if (12 == m_tile_position.z) {

			return  1;
		}
		if (8 == m_tile_position.z) {

			return  1;
		}
		if (4 == m_tile_position.z) {

			return  1;
		}
		return 0;
	}
	return 0;
}

void Renderer::DrawGeometryNode(GeometryNode* node, glm::mat4 model_matrix, glm::mat4 normal_matrix)
{
	glBindVertexArray(node->m_vao);
	glUniformMatrix4fv(m_geometry_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
	glUniformMatrix4fv(m_geometry_rendering_program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(normal_matrix));
	for (int j = 0; j < node->parts.size(); j++)
	{
		glm::vec3 diffuseColor = node->parts[j].diffuseColor;
		glm::vec3 specularColor = node->parts[j].specularColor;
		float shininess = node->parts[j].shininess;
		glUniform3f(m_geometry_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
		glUniform3f(m_geometry_rendering_program["uniform_specular"], specularColor.r, specularColor.g, specularColor.b);
		glUniform1f(m_geometry_rendering_program["uniform_shininess"], shininess);
		glUniform1f(m_geometry_rendering_program["uniform_has_texture"], (node->parts[j].textureID > 0) ? 1.0f : 0.0f);
		glBindTexture(GL_TEXTURE_2D, node->parts[j].textureID);

		glDrawArrays(GL_TRIANGLES, node->parts[j].start_offset, node->parts[j].count);
	}
}

void Renderer::DrawGeometryNodeToShadowMap(GeometryNode* node, glm::mat4 model_matrix, glm::mat4 normal_matrix)
{
	glBindVertexArray(node->m_vao);
	glUniformMatrix4fv(m_spot_light_shadow_map_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
	for (int j = 0; j < node->parts.size(); j++)
	{
		glDrawArrays(GL_TRIANGLES, node->parts[j].start_offset, node->parts[j].count);
	}
}

void Renderer::RenderToOutFB()
{
	// Bind the default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, m_screen_width, m_screen_height);
	
	// clear the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// disable depth testing and blending
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	// bind the post processing program
	m_postprocess_program.Bind();
	
	glBindVertexArray(m_vao_fbo);
	// Bind the intermediate color image to texture unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_fbo_texture);
	glUniform1i(m_postprocess_program["uniform_texture"], 0);
	// Bind the intermediate depth buffer to texture unit 1
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_fbo_depth_texture);	
	glUniform1i(m_postprocess_program["uniform_depth"], 1);

	glUniform1f(m_postprocess_program["uniform_time"], m_continous_time);
	glm::mat4 projection_inverse_matrix = glm::inverse(m_projection_matrix);
	glUniformMatrix4fv(m_postprocess_program["uniform_projection_inverse_matrix"], 1, GL_FALSE, glm::value_ptr(projection_inverse_matrix));

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindVertexArray(0);

	// Unbind the post processing program
	m_postprocess_program.Unbind();
}

void Renderer::CameraMoveForward(bool enable)
{
	m_camera_movement.x = (enable)? 1 : 0;
}
void Renderer::CameraMoveBackWard(bool enable)
{
	m_camera_movement.x = (enable) ? -1 : 0;
}

void Renderer::CameraMoveLeft(bool enable)
{
	m_camera_movement.y = (enable) ? -1 : 0;
}
void Renderer::CameraMoveRight(bool enable)
{
	m_camera_movement.y = (enable) ? 1 : 0;
}

void Renderer::CameraLook(glm::vec2 lookDir)
{
	m_camera_look_angle_destination = glm::vec2(1, -1) * lookDir;

}

// Set the tile move direction
void Renderer::TileMoveForward(bool enable)
{
	m_tile_movement.x = (enable) ? 1.f : 0.f;
	if (m_tile_position.z < 36) {
		m_tile_position.z += m_tile_movement.x*4.f;
	}
	
}
void Renderer::TileMoveBackWard(bool enable)
{
	m_tile_movement.x = (enable) ? -1.f : 0.f;
	if (m_tile_position.z > 0) {
		m_tile_position.z += m_tile_movement.x*4.f;
	}
	road = IfRoad();
}

void Renderer::TileMoveLeft(bool enable)
{
	m_tile_movement.y = (enable) ? 1.f : 0.f;
	if (m_tile_position.x < 36) {
		m_tile_position.x += m_tile_movement.y*4.f;
	}
	road = IfRoad();
}
void Renderer::TileMoveRight(bool enable)
{
	m_tile_movement.y = (enable) ? -1.f : 0.f;
	if (m_tile_position.x > 0) {
	
		m_tile_position.x += m_tile_movement.y*4.f; 

	}
	road = IfRoad();
}

void Renderer::SkeletonMoveForward(bool enable)
{
	m_skeleton_movement.x = (enable) ? 1.f : 0.f;
}
void Renderer::SkeletonMoveBackWard(bool enable)
{
	m_skeleton_movement.x = (enable) ? -1.f : 0.f;
}

void Renderer::SkeletonMoveLeft(bool enable)
{
	m_skeleton_movement.y = (enable) ? -1.f : 0.f;
}
void Renderer::SkeletonMoveRight(bool enable)
{
	m_skeleton_movement.y = (enable) ? 1.f : 0.f;
}

