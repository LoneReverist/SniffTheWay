// Baby.ixx

module;

#include <utility>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

export module Baby;

import Dreamhearth;

import AssetManager;
import AssetPool;
import Dog;
import Input;
import SniffTheWayConstants;
import SpriteSheet;
import SpritePipeline;
import Vertex;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

export class Baby
{
private:
	enum class State
	{
		Idle,
		Walking
	};

public:
	Baby() = default;

	void Init(
		AssetManager & asset_manager,
		glm::vec3 const & camera_dir,
		glm::vec2 const & initial_pos);
	void Update(float dt, Dog const * dog, SceneState scene_state);
	void OnSceneStateChanged(SceneState new_state);
	void Reload(glm::vec3 const & camera_dir, glm::vec2 pos);

	void SetPosition(glm::vec2 pos);
	void SetTint(glm::vec3 tint);
	void SetOpacity(float opacity);

	MeshId<TextureVertex2d> GetMeshId() const { return m_mesh_id; }
	SpriteSheet const & GetSpriteSheet() const { return m_sprite_sheet; }
	SpritePipeline::ObjectData const & GetPipelineData() const { return m_pipeline_data; }
	glm::vec2 GetPosition() const { return glm::vec2(m_pipeline_data.model[3]); }
	
private:
	void set_state(State state);
	void update_frame_uvs();

	MeshId<TextureVertex2d> m_mesh_id;

	SpriteSheet m_sprite_sheet;
	SpritePipeline::ObjectData m_pipeline_data;
	State m_state = State::Idle;
	bool facing_right = true;

	float m_animation_timer = 0.0f;
	float m_frame_duration = 0.1f; // 100ms per frame = 10 FPS
	
	float m_move_speed = 3.0f; // units per second
};

namespace
{
	constexpr float FollowStartDistance = 3.0f;
	constexpr float FollowStopDistance = 1.5f;

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

void Baby::Init(
	AssetManager & asset_manager,
	glm::vec3 const & camera_dir,
	glm::vec2 const & initial_pos)
{
	AssetId tex_id = asset_manager.AddTexture(asset_manager.GetTexturesPath() / "baby_crawl.png",
		 dh::PixelFormat::RGBA_SRGB, false /*flip_vertically*/, false /*use_mip_map*/);

	m_sprite_sheet = SpriteSheet{
		tex_id,
		900,    // Texture width
		900,    // Texture height
		300,    // Frame width
		300,    // Frame height
		9       // Frame count: 9
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

void Baby::Update(float dt, Dog const * dog, SceneState scene_state)
{
	if (m_sprite_sheet.GetFrameCount() == 0)
		return;

	glm::vec2 velocity(0.0f);
	if (dog && scene_state == SceneState::Gameplay)
	{
		glm::vec2 const move_dir = dog->GetPosition() - GetPosition();
		float const distance = glm::length(move_dir);

		if (m_state == State::Idle && distance >= FollowStartDistance)
			set_state(State::Walking);
		else if (m_state == State::Walking && distance <= FollowStopDistance)
			set_state(State::Idle);

		if (m_state == State::Walking && distance > FollowStopDistance)
		{
			velocity = glm::normalize(move_dir) * m_move_speed;
			float movement_distance = m_move_speed * dt;
			bool const reached_stop_distance = movement_distance >= distance - FollowStopDistance;
			if (movement_distance > distance - FollowStopDistance)
				movement_distance = distance - FollowStopDistance;

			glm::vec2 const movement = glm::normalize(move_dir) * movement_distance;
			const glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(movement, 0.0f));
			m_pipeline_data.model = translation * m_pipeline_data.model;

			if (reached_stop_distance)
				set_state(State::Idle);
		}
	}
	else
	{
		set_state(State::Idle);
	}

	if (velocity.x > 0.0f && !facing_right	  // moving right, but facing left
		|| velocity.x < 0.0f && facing_right) // moving left, but facing right
	{
		facing_right = !facing_right;
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

void Baby::OnSceneStateChanged(SceneState new_state)
{
	if (new_state != SceneState::Gameplay)
		set_state(State::Idle);
}

void Baby::Reload(glm::vec3 const & camera_dir, glm::vec2 pos)
{
	m_state = State::Idle;
	facing_right = true;
	m_animation_timer = 0.0f;
	m_sprite_sheet.SetCurrentFrame(0);
	update_frame_uvs();

	m_pipeline_data.model = create_camera_facing_model(camera_dir);
	SetPosition(pos);
}

void Baby::SetPosition(glm::vec2 pos)
{
	m_pipeline_data.model[3] = glm::vec4(pos, 0.0f, 1.0f);
}

void Baby::SetOpacity(float opacity)
{
	m_pipeline_data.tint.a = opacity;
}

void Baby::SetTint(glm::vec3 tint)
{
	m_pipeline_data.tint = glm::vec4{ tint, m_pipeline_data.tint.a };
}

void Baby::set_state(State state)
{
	if (m_state == state)
		return;

	m_state = state;
	if (m_state == State::Idle)
	{
		m_animation_timer = 0.0f;
		m_sprite_sheet.SetCurrentFrame(0);
		update_frame_uvs();
	}
}

void Baby::update_frame_uvs()
{
	m_pipeline_data.frame_uvs = m_sprite_sheet.GetCurrentFrameUVs();
	if (!facing_right)
		std::swap(m_pipeline_data.frame_uvs.x, m_pipeline_data.frame_uvs.y);
}
