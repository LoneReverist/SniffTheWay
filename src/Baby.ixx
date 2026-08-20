// Baby.ixx

module;

#include <cstdint>
#include <utility>
#include <vector>

#include <glm/glm.hpp>

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
		glm::vec2 const & initial_pos,
		AssetId shadow_tex_id);
	void Update(float dt, Dog const * dog, SceneState scene_state);
	void OnSceneStateChanged(SceneState new_state);
	void Reload(glm::vec3 const & camera_dir, glm::vec2 pos);

	void SetPosition(glm::vec2 pos);
	void SetTint(glm::vec3 tint);
	void SetOpacity(float opacity);

	MeshId<TextureVertex2d> GetMeshId() const { return m_mesh_id; }
	SpriteSheet const & GetSpriteSheet() const { return get_active_sprite_sheet(); }
	SpritePipeline::ObjectData const & GetPipelineData() const { return m_pipeline_data; }
	bool HasShadow() const { return m_shadow_mesh_id.IsValid() && m_shadow_pipeline_data.tex_id.IsValid(); }
	MeshId<TextureVertex2d> GetShadowMeshId() const { return m_shadow_mesh_id; }
	SpritePipeline::ObjectData const & GetShadowPipelineData() const { return m_shadow_pipeline_data; }
	glm::vec2 GetPosition() const { return glm::vec2(m_pipeline_data.model[3]); }
	
private:
	enum class CrawlDirection
	{
		TowardsCamera,
		AwayFromCamera
	};

	SpriteSheet & get_active_sprite_sheet();
	SpriteSheet const & get_active_sprite_sheet() const;
	glm::vec2 get_shadow_offset() const;
	void set_crawl_direction(CrawlDirection direction);
	void set_state(State state);
	void update_frame_uvs();

	MeshId<TextureVertex2d> m_mesh_id;
	MeshId<TextureVertex2d> m_shadow_mesh_id;

	SpriteSheet m_crawl_sprite_sheet;
	SpriteSheet m_crawl_away_sprite_sheet;
	SpritePipeline::ObjectData m_pipeline_data;
	SpritePipeline::ObjectData m_shadow_pipeline_data;
	State m_state = State::Idle;
	CrawlDirection m_crawl_direction = CrawlDirection::TowardsCamera;
	bool facing_right = true;

	float m_animation_timer = 0.0f;
	float m_frame_duration = 0.05f; // 50ms per frame = 20 FPS
	
	float m_move_speed = 2.0f; // units per second
};

namespace
{
	constexpr float FollowStartDistance = 3.0f;
	constexpr float FollowStopDistance = 1.5f;
	constexpr float DirectionEpsilon = 1e-4f;
	glm::vec2 const BabyShadowTowardsCameraOffset{ 0.0f, -0.075f };
	glm::vec2 const BabyShadowAwayFromCameraOffset{ 0.0f, -0.05f };
	glm::vec2 const BabyShadowSize{ 0.8f, 0.65f };

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

	MeshId<TextureVertex2d> create_shadow_mesh(AssetManager & asset_manager)
	{
		float const half_width = BabyShadowSize.x * 0.5f;
		std::vector<TextureVertex2d> const vertices{
			{ { -half_width, BabyShadowSize.y }, { 0.0f, 0.0f } },
			{ {  half_width, BabyShadowSize.y }, { 1.0f, 0.0f } },
			{ { -half_width, 0.0f },             { 0.0f, 1.0f } },
			{ {  half_width, 0.0f },             { 1.0f, 1.0f } }
		};
		std::vector<dh::Mesh::IndexT> const indices{
			1, 0, 2,
			1, 2, 3
		};
		return asset_manager.AddMesh(vertices, indices);
	}

	glm::mat4 create_shadow_model(
		glm::vec3 const & camera_dir,
		glm::vec2 pos,
		glm::vec2 const & shadow_offset)
	{
		glm::mat4 model = create_camera_facing_model(camera_dir);
		glm::vec3 const shadow_pos =
			glm::vec3{ pos, 0.0f }
			+ glm::vec3{ model[0] } * shadow_offset.x
			+ glm::vec3{ model[1] } * shadow_offset.y;
		model[3] = glm::vec4{ shadow_pos, 1.0f };
		return model;
	}
}

void Baby::Init(
	AssetManager & asset_manager,
	glm::vec3 const & camera_dir,
	glm::vec2 const & initial_pos,
	AssetId shadow_tex_id)
{
	AssetId const crawl_tex_id = asset_manager.AddTexture(
		asset_manager.GetTexturesPath() / "baby_crawl_sprite_sheet.png",
		 dh::PixelFormat::RGBA_SRGB, false /*flip_vertically*/, false /*use_mip_map*/);
	AssetId const crawl_away_tex_id = asset_manager.AddTexture(
		asset_manager.GetTexturesPath() / "baby_crawl_away_sprite_sheet.png",
		dh::PixelFormat::RGBA_SRGB, false /*flip_vertically*/, false /*use_mip_map*/);

	m_crawl_sprite_sheet = SpriteSheet{
		crawl_tex_id,
		4096, // Texture width
		2560, // Texture height
		512,  // Frame width
		512,  // Frame height
		40    // Frame count
	};
	m_crawl_away_sprite_sheet = SpriteSheet{
		crawl_away_tex_id,
		4096, // Texture width
		2560, // Texture height
		512,  // Frame width
		512,  // Frame height
		40    // Frame count
	};
	m_crawl_direction = CrawlDirection::TowardsCamera;

	// this creates a quad on the xy axes, we have to rotate it up to face the camera with the model matrix
	m_mesh_id = m_crawl_sprite_sheet.CreateQuadMesh(asset_manager);

	glm::mat4 model = create_camera_facing_model(camera_dir);
	model[3] = glm::vec4(initial_pos, 0.0f, 1.0f);

	m_pipeline_data = SpritePipeline::ObjectData{
		.model = model,
		.frame_uvs = m_crawl_sprite_sheet.GetCurrentFrameUVs(),
		.tex_id = crawl_tex_id,
	};

	if (shadow_tex_id.IsValid())
		m_shadow_mesh_id = create_shadow_mesh(asset_manager);
	m_shadow_pipeline_data = SpritePipeline::ObjectData{
		.model = create_shadow_model(camera_dir, initial_pos, BabyShadowTowardsCameraOffset),
		.frame_uvs = glm::vec4{ 0.0f, 1.0f, 0.0f, 1.0f },
		.tint = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f },
		.tex_id = shadow_tex_id,
	};
}

void Baby::Update(float dt, Dog const * dog, SceneState scene_state)
{
	if (get_active_sprite_sheet().GetFrameCount() == 0)
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
			SetPosition(GetPosition() + movement);

			if (reached_stop_distance)
				set_state(State::Idle);
		}
	}
	else
	{
		set_state(State::Idle);
	}

	if (m_state == State::Walking)
	{
		if (velocity.y > DirectionEpsilon)
			set_crawl_direction(CrawlDirection::AwayFromCamera);
		else if (velocity.y < -DirectionEpsilon)
			set_crawl_direction(CrawlDirection::TowardsCamera);
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
			get_active_sprite_sheet().AdvanceFrame();
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
	m_crawl_sprite_sheet.SetCurrentFrame(0);
	m_crawl_away_sprite_sheet.SetCurrentFrame(0);
	m_crawl_direction = CrawlDirection::TowardsCamera;
	m_pipeline_data.tex_id = m_crawl_sprite_sheet.GetTextureId();
	update_frame_uvs();

	m_pipeline_data.model = create_camera_facing_model(camera_dir);
	m_shadow_pipeline_data.model = create_shadow_model(
		camera_dir,
		pos,
		BabyShadowTowardsCameraOffset);
	SetPosition(pos);
}

void Baby::SetPosition(glm::vec2 pos)
{
	m_pipeline_data.model[3] = glm::vec4(pos, 0.0f, 1.0f);
	glm::vec2 const shadow_offset = get_shadow_offset();
	glm::vec3 const shadow_pos =
		glm::vec3{ pos, 0.0f }
		+ glm::vec3{ m_shadow_pipeline_data.model[0] } * shadow_offset.x
		+ glm::vec3{ m_shadow_pipeline_data.model[1] } * shadow_offset.y;
	m_shadow_pipeline_data.model[3] = glm::vec4{ shadow_pos, 1.0f };
}

void Baby::SetOpacity(float opacity)
{
	m_pipeline_data.tint.a = opacity;
	m_shadow_pipeline_data.tint.a = opacity;
}

void Baby::SetTint(glm::vec3 tint)
{
	m_pipeline_data.tint = glm::vec4{ tint, m_pipeline_data.tint.a };
}

SpriteSheet & Baby::get_active_sprite_sheet()
{
	return m_crawl_direction == CrawlDirection::AwayFromCamera
		? m_crawl_away_sprite_sheet
		: m_crawl_sprite_sheet;
}

SpriteSheet const & Baby::get_active_sprite_sheet() const
{
	return m_crawl_direction == CrawlDirection::AwayFromCamera
		? m_crawl_away_sprite_sheet
		: m_crawl_sprite_sheet;
}

glm::vec2 Baby::get_shadow_offset() const
{
	return m_crawl_direction == CrawlDirection::AwayFromCamera
		? BabyShadowAwayFromCameraOffset
		: BabyShadowTowardsCameraOffset;
}

void Baby::set_crawl_direction(CrawlDirection direction)
{
	if (m_crawl_direction == direction)
		return;

	SpriteSheet const & source_sheet = get_active_sprite_sheet();
	std::uint32_t const source_count = source_sheet.GetFrameCount();
	std::uint32_t const source_frame = source_sheet.GetCurrentFrame();

	m_crawl_direction = direction;
	SpriteSheet & destination_sheet = get_active_sprite_sheet();
	std::uint32_t const destination_count = destination_sheet.GetFrameCount();
	if (source_count > 0 && destination_count > 0)
	{
		std::uint32_t const destination_frame = static_cast<std::uint32_t>(
			(static_cast<std::uint64_t>(source_frame) * destination_count) / source_count);
		destination_sheet.SetCurrentFrame(destination_frame);
	}

	m_pipeline_data.tex_id = destination_sheet.GetTextureId();
	SetPosition(GetPosition()); // update shadow offset based on new crawl direction
	update_frame_uvs();
}

void Baby::set_state(State state)
{
	if (m_state == state)
		return;

	m_state = state;
	if (m_state == State::Idle)
	{
		m_animation_timer = 0.0f;
		m_crawl_sprite_sheet.SetCurrentFrame(0);
		m_crawl_away_sprite_sheet.SetCurrentFrame(0);
		update_frame_uvs();
	}
}

void Baby::update_frame_uvs()
{
	m_pipeline_data.frame_uvs = get_active_sprite_sheet().GetCurrentFrameUVs();
	m_shadow_pipeline_data.frame_uvs = glm::vec4{ 0.0f, 1.0f, 0.0f, 1.0f };
	if (!facing_right)
		std::swap(m_pipeline_data.frame_uvs.x, m_pipeline_data.frame_uvs.y);

	bool flip_shadow = !facing_right;
	if (m_crawl_direction == CrawlDirection::AwayFromCamera)
		flip_shadow = !flip_shadow;
	if (flip_shadow)
		std::swap(m_shadow_pipeline_data.frame_uvs.x, m_shadow_pipeline_data.frame_uvs.y);
}
