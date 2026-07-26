// Dog.ixx

module;

#include <utility>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

export module Dog;

import Dreamhearth;

import AssetManager;
import AssetPool;
import Input;
import Polygon2d;
import SniffTheWayConstants;
import SpriteSheet;
import SpritePipeline;
import Vertex;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

export class Dog
{
private:
	enum class State
	{
		Idle,
		Walking
	};

public:
    Dog() = default;

    void Init(
		AssetManager & asset_manager,
		glm::vec3 const & camera_dir,
		glm::vec2 const & initial_pos);
	void Update(float dt, Input const & input, Polygon2d const & bounds, SceneState scene_state);
	void OnSceneStateChanged(SceneState new_state);
	void Reload(glm::vec3 const & camera_dir, glm::vec2 pos);

	void SetPosition(glm::vec2 pos);
	void SetOpacity(float opacity);

    MeshId<TextureVertex2d> GetMeshId() const { return m_mesh_id; }
    SpriteSheet const & GetSpriteSheet() const { return m_sprite_sheet; }
    SpritePipeline::ObjectData const & GetPipelineData() const { return m_pipeline_data; }
	glm::vec2 GetPosition() const { return glm::vec2(m_pipeline_data.model[3]); }
    
private:
	void update_frame_uvs();

	MeshId<TextureVertex2d> m_mesh_id;

	SpriteSheet m_sprite_sheet;
	SpritePipeline::ObjectData m_pipeline_data;
	State m_state = State::Idle;
	bool m_facing_right = true;

	float m_animation_timer = 0.0f;
	const float m_frame_duration = 1.0f / 20.0f;
	
	const float m_move_speed = 3.0f; // units per second
};

namespace
{
	glm::mat4 create_camera_facing_model(glm::vec3 const & camera_dir)
	{
		if (glm::length(camera_dir) < 1e-6f)
			return glm::mat4{ 1.0f };

		glm::vec3 const normal = glm::normalize(-camera_dir);

		glm::vec3 const world_up{ 0.0f, 0.0f, 1.0f };
		glm::vec3 up = world_up - normal * glm::dot(world_up, normal);
		if (glm::length(up) < 1e-6f)
			return glm::mat4{ 1.0f };
		up = glm::normalize(up);

		glm::vec3 const right = glm::normalize(glm::cross(up, normal));

		glm::mat4 model{ 1.0f };
		model[0] = glm::vec4{ right, 0.0f };
		model[1] = glm::vec4{ up, 0.0f };
		model[2] = glm::vec4{ normal, 0.0f };
		return model;
	}
}

void Dog::Init(
	AssetManager & asset_manager,
	glm::vec3 const & camera_dir,
	glm::vec2 const & initial_pos)
{
	AssetId tex_id = asset_manager.AddTexture(asset_manager.GetTexturesPath() / "dog_walk.png",
		dh::PixelFormat::RGBA_SRGB, false /*flip_vertically*/, false /*use_mip_map*/);

    m_sprite_sheet = SpriteSheet{
		tex_id,
		5120,   // Texture width
		3840,   // Texture height
		640,    // Frame width
		640,    // Frame height
		44      // Active frame count; atlas cells 45-48 are intentionally transparent
    };

	// this creates a quad on the xy axes, we have to rotate it up to face the camera with the model matrix
	m_mesh_id = m_sprite_sheet.CreateQuadMesh(asset_manager);

	glm::mat4 model = create_camera_facing_model(camera_dir);
	model[3] = glm::vec4(initial_pos, 0.0f, 1.0f);

    m_pipeline_data = SpritePipeline::ObjectData{
		.model = model,
		.frame_uvs = m_sprite_sheet.GetCurrentFrameUVs(),
		.tex_id = tex_id,
	};
}

void Dog::Update(float dt, Input const & input, Polygon2d const & bounds, SceneState scene_state)
{
	if (m_sprite_sheet.GetFrameCount() == 0)
		return;

	// Handle WASD input for 3D movement
	glm::vec2 move_dir(0.0f);
	if (scene_state == SceneState::Gameplay)
	{
		if (input.KeyIsDown('W') || input.KeyIsDown(Input::Key::Up))
			move_dir.y += 1.0f;
		if (input.KeyIsDown('S') || input.KeyIsDown(Input::Key::Down))
			move_dir.y -= 1.0f;
		if (input.KeyIsDown('A') || input.KeyIsDown(Input::Key::Left))
			move_dir.x -= 1.0f;
		if (input.KeyIsDown('D') || input.KeyIsDown(Input::Key::Right))
			move_dir.x += 1.0f;
	}

	glm::vec2 velocity(0.0f);
	if (move_dir != glm::vec2(0.0f))
	{
		glm::vec2 cur_pos = GetPosition();
		velocity = glm::normalize(move_dir) * m_move_speed;
		glm::vec2 desired_pos = cur_pos + velocity * dt;
		
		glm::vec2 new_pos = desired_pos;
		if (bounds.IsValid())
			new_pos = bounds.SlideAlongBoundary(cur_pos, desired_pos);

		SetPosition(new_pos);

		m_state = State::Walking;
	}
	else
	{
		m_state = State::Idle;
	}

	if (velocity.x > 0.0f && !m_facing_right // moving right, but facing left
		|| velocity.x < 0.0f && m_facing_right) // moving left, but facing right
	{
		m_facing_right = !m_facing_right;
		update_frame_uvs();
	}

	if (m_state == State::Walking)
	{
		m_animation_timer += dt;
		if (m_animation_timer >= m_frame_duration)
		{
			m_animation_timer -= m_frame_duration;
			m_sprite_sheet.AdvanceFrame();
			update_frame_uvs();
		}
	}
}

void Dog::OnSceneStateChanged(SceneState new_state)
{
	if (new_state != SceneState::Gameplay)
		m_state = State::Idle;
}

void Dog::Reload(glm::vec3 const & camera_dir, glm::vec2 pos)
{
	m_state = State::Idle;
	m_facing_right = true;
	m_animation_timer = 0.0f;
	m_sprite_sheet.SetCurrentFrame(0);
	update_frame_uvs();

	m_pipeline_data.model = create_camera_facing_model(camera_dir);
	SetPosition(pos);
}

void Dog::SetPosition(glm::vec2 pos)
{
	m_pipeline_data.model[3] = glm::vec4(pos, 0.0f, 1.0f);
}

void Dog::SetOpacity(float opacity)
{
	m_pipeline_data.tint.a = opacity;
}

void Dog::update_frame_uvs()
{
	m_pipeline_data.frame_uvs = m_sprite_sheet.GetCurrentFrameUVs();
	if (!m_facing_right)
		std::swap(m_pipeline_data.frame_uvs.x, m_pipeline_data.frame_uvs.y);
}
