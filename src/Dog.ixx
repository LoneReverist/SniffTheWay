// Dog.ixx

module;

#include <cstdint>
#include <utility>
#include <vector>

#include <glm/glm.hpp>

export module Dog;

import Dreamhearth;

import AssetManager;
import AssetPool;
import CharacterFacing;
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
		glm::vec2 const & initial_pos,
		AssetId shadow_tex_id);
	void Update(float dt, Input const & input, Polygon2d const & bounds, SceneState scene_state);
	void OnSceneStateChanged(SceneState new_state);
	void Reload(
		glm::vec3 const & camera_dir,
		glm::vec2 pos,
		CharacterCameraFacing camera_facing,
		CharacterHorizontalFacing horizontal_facing);

	void SetPosition(glm::vec2 pos);
	void SetFacing(CharacterCameraFacing camera_facing, CharacterHorizontalFacing horizontal_facing);
	void SetCameraDirection(glm::vec3 const & camera_dir);
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
	SpriteSheet & get_active_sprite_sheet();
	SpriteSheet const & get_active_sprite_sheet() const;
	void set_walk_direction(CharacterCameraFacing direction);
	void update_frame_uvs();

	MeshId<TextureVertex2d> m_mesh_id;
	MeshId<TextureVertex2d> m_shadow_mesh_id;

	SpriteSheet m_walk_sprite_sheet;
	SpriteSheet m_walk_away_sprite_sheet;
	SpritePipeline::ObjectData m_pipeline_data;
	SpritePipeline::ObjectData m_shadow_pipeline_data;
	State m_state = State::Idle;
	CharacterCameraFacing m_walk_direction = CharacterCameraFacing::TowardsCamera;
	CharacterHorizontalFacing m_horizontal_facing = CharacterHorizontalFacing::Right;

	float m_animation_timer = 0.0f;
	const float m_frame_duration = 1.0f / 20.0f;
	
	const float m_move_speed = 3.0f; // units per second
};

namespace
{
	constexpr float DirectionEpsilon = 1e-4f;
	glm::vec2 const DogShadowOffset{ 0.0f, -0.175f };

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
		std::vector<TextureVertex2d> const vertices{
			{ { -0.5f,  1.0f }, { 0.0f, 0.0f } },
			{ {  0.5f,  1.0f }, { 1.0f, 0.0f } },
			{ { -0.5f,  0.0f }, { 0.0f, 1.0f } },
			{ {  0.5f,  0.0f }, { 1.0f, 1.0f } }
		};
		std::vector<dh::Mesh::IndexT> const indices{
			1, 0, 2,
			1, 2, 3
		};
		return asset_manager.AddMesh(vertices, indices);
	}

	glm::mat4 create_shadow_model(glm::vec3 const & camera_dir, glm::vec2 pos)
	{
		glm::mat4 model = create_camera_facing_model(camera_dir);
		glm::vec3 const shadow_pos =
			glm::vec3{ pos, 0.0f }
			+ glm::vec3{ model[0] } * DogShadowOffset.x
			+ glm::vec3{ model[1] } * DogShadowOffset.y;
		model[3] = glm::vec4{ shadow_pos, 1.0f };
		return model;
	}
}

void Dog::Init(
	AssetManager & asset_manager,
	glm::vec3 const & camera_dir,
	glm::vec2 const & initial_pos,
	AssetId shadow_tex_id)
{
	AssetId const walk_tex_id = asset_manager.AddTexture(
		asset_manager.GetTexturesPath() / "dog_walk.png",
		dh::PixelFormat::RGBA_SRGB, false /*flip_vertically*/, false /*use_mip_map*/);
	AssetId const walk_away_tex_id = asset_manager.AddTexture(
		asset_manager.GetTexturesPath() / "dog_walk_away.png",
		dh::PixelFormat::RGBA_SRGB, false /*flip_vertically*/, false /*use_mip_map*/);

	m_walk_sprite_sheet = SpriteSheet{
		walk_tex_id,
		4096, // Texture width
		2560, // Texture height
		512,  // Frame width
		512,  // Frame height
		40    // Frame count
	};
	m_walk_away_sprite_sheet = SpriteSheet{
		walk_away_tex_id,
		4096, // Texture width
		2560, // Texture height
		512,  // Frame width
		512,  // Frame height
		40    // Frame count
	};
	m_walk_direction = CharacterCameraFacing::TowardsCamera;

	// this creates a quad on the xy axes, we have to rotate it up to face the camera with the model matrix
	m_mesh_id = m_walk_sprite_sheet.CreateQuadMesh(asset_manager);

	glm::mat4 model = create_camera_facing_model(camera_dir);
	model[3] = glm::vec4(initial_pos, 0.0f, 1.0f);

    m_pipeline_data = SpritePipeline::ObjectData{
		.model = model,
		.frame_uvs = m_walk_sprite_sheet.GetCurrentFrameUVs(),
		.tex_id = walk_tex_id,
	};

	if (shadow_tex_id.IsValid())
		m_shadow_mesh_id = create_shadow_mesh(asset_manager);
	m_shadow_pipeline_data = SpritePipeline::ObjectData{
		.model = create_shadow_model(camera_dir, initial_pos),
		.frame_uvs = glm::vec4{ 0.0f, 1.0f, 0.0f, 1.0f },
		.tex_id = shadow_tex_id,
	};
}

void Dog::Update(float dt, Input const & input, Polygon2d const & bounds, SceneState scene_state)
{
	if (get_active_sprite_sheet().GetFrameCount() == 0)
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

	if (m_state == State::Walking)
	{
		if (velocity.y > DirectionEpsilon)
			set_walk_direction(CharacterCameraFacing::AwayFromCamera);
		else if (velocity.y < -DirectionEpsilon)
			set_walk_direction(CharacterCameraFacing::TowardsCamera);
	}

	if (velocity.x > DirectionEpsilon && m_horizontal_facing != CharacterHorizontalFacing::Right
		|| velocity.x < -DirectionEpsilon && m_horizontal_facing != CharacterHorizontalFacing::Left)
	{
		m_horizontal_facing = velocity.x > DirectionEpsilon
			? CharacterHorizontalFacing::Right
			: CharacterHorizontalFacing::Left;
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

void Dog::OnSceneStateChanged(SceneState new_state)
{
	if (new_state != SceneState::Gameplay)
		m_state = State::Idle;
}

void Dog::Reload(
	glm::vec3 const & camera_dir,
	glm::vec2 pos,
	CharacterCameraFacing camera_facing,
	CharacterHorizontalFacing horizontal_facing)
{
	m_state = State::Idle;
	m_animation_timer = 0.0f;
	m_walk_sprite_sheet.SetCurrentFrame(0);
	m_walk_away_sprite_sheet.SetCurrentFrame(0);
	SetFacing(camera_facing, horizontal_facing);
	SetCameraDirection(camera_dir);
	SetPosition(pos);
}

void Dog::SetFacing(CharacterCameraFacing camera_facing, CharacterHorizontalFacing horizontal_facing)
{
	set_walk_direction(camera_facing);
	m_horizontal_facing = horizontal_facing;
	update_frame_uvs();
}

void Dog::SetCameraDirection(glm::vec3 const & camera_dir)
{
	glm::vec2 const pos = GetPosition();
	m_pipeline_data.model = create_camera_facing_model(camera_dir);
	m_shadow_pipeline_data.model = create_shadow_model(camera_dir, pos);
	SetPosition(pos);
}

void Dog::SetPosition(glm::vec2 pos)
{
	m_pipeline_data.model[3] = glm::vec4(pos, 0.0f, 1.0f);
	glm::vec3 const shadow_pos =
		glm::vec3{ pos, 0.0f }
		+ glm::vec3{ m_shadow_pipeline_data.model[0] } * DogShadowOffset.x
		+ glm::vec3{ m_shadow_pipeline_data.model[1] } * DogShadowOffset.y;
	m_shadow_pipeline_data.model[3] = glm::vec4{ shadow_pos, 1.0f };
}

void Dog::SetOpacity(float opacity)
{
	m_pipeline_data.tint.a = opacity;
	m_shadow_pipeline_data.tint.a = opacity;
}

void Dog::SetTint(glm::vec3 tint)
{
	m_pipeline_data.tint = glm::vec4{ tint, m_pipeline_data.tint.a };
}

SpriteSheet & Dog::get_active_sprite_sheet()
{
	return m_walk_direction == CharacterCameraFacing::AwayFromCamera
		? m_walk_away_sprite_sheet
		: m_walk_sprite_sheet;
}

SpriteSheet const & Dog::get_active_sprite_sheet() const
{
	return m_walk_direction == CharacterCameraFacing::AwayFromCamera
		? m_walk_away_sprite_sheet
		: m_walk_sprite_sheet;
}

void Dog::set_walk_direction(CharacterCameraFacing direction)
{
	if (m_walk_direction == direction)
		return;

	SpriteSheet const & source_sheet = get_active_sprite_sheet();
	std::uint32_t const source_count = source_sheet.GetFrameCount();
	std::uint32_t const source_frame = source_sheet.GetCurrentFrame();

	m_walk_direction = direction;
	SpriteSheet & destination_sheet = get_active_sprite_sheet();
	std::uint32_t const destination_count = destination_sheet.GetFrameCount();
	if (source_count > 0 && destination_count > 0)
	{
		std::uint32_t const destination_frame = static_cast<std::uint32_t>(
			(static_cast<std::uint64_t>(source_frame) * destination_count) / source_count);
		destination_sheet.SetCurrentFrame(destination_frame);
	}

	m_pipeline_data.tex_id = destination_sheet.GetTextureId();
	update_frame_uvs();
}

void Dog::update_frame_uvs()
{
	m_pipeline_data.frame_uvs = get_active_sprite_sheet().GetCurrentFrameUVs();
	m_shadow_pipeline_data.frame_uvs = glm::vec4{ 0.0f, 1.0f, 0.0f, 1.0f };
	if (m_horizontal_facing == CharacterHorizontalFacing::Left)
		std::swap(m_pipeline_data.frame_uvs.x, m_pipeline_data.frame_uvs.y);

	bool flip_shadow = m_horizontal_facing == CharacterHorizontalFacing::Left;
	if (m_walk_direction == CharacterCameraFacing::AwayFromCamera)
		flip_shadow = !flip_shadow;
	if (flip_shadow)
		std::swap(m_shadow_pipeline_data.frame_uvs.x, m_shadow_pipeline_data.frame_uvs.y);
}
