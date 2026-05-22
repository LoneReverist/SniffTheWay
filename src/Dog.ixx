// Dog.ixx

module;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

export module Dog;

import Dreamhearth;
namespace dh = Dreamhearth;

import AssetManager;
import AssetPool;
import Input;
import Polygon2d;
import SniffTheWayConstants;
import SpriteSheet;
import SpritePipeline;
import Vertex;

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
		glm::vec3 const & camera_dir);
    void Update(float dt, Input const & input, Polygon2d const & bounds, SceneState scene_state);
	void OnSceneStateChanged(SceneState new_state);

    AssetId GetTextureId() const { return m_tex_id; }
    MeshId<TextureVertex2d> GetMeshId() const { return m_mesh_id; }
    SpriteSheet const & GetSpriteSheet() const { return m_sprite_sheet; }
    SpritePipeline::ObjectData const & GetSpriteData() const { return m_sprite_data; }
    
private:
	AssetId m_tex_id;
	MeshId<TextureVertex2d> m_mesh_id;

	SpriteSheet m_sprite_sheet;
	SpritePipeline::ObjectData m_sprite_data;
	State m_state = State::Idle;
	bool facing_right = true;

	float m_animation_timer = 0.0f;
	float m_frame_duration = 0.1f; // 100ms per frame = 10 FPS
	
	float m_move_speed = 3.0f; // units per second
};

void Dog::Init(
	AssetManager & asset_manager,
	glm::vec3 const & camera_dir)
{
	m_tex_id = asset_manager.AddTexture(asset_manager.GetTexturesPath() / "dog_walk.png",
		dh::PixelFormat::RGBA_SRGB, false /*flip_vertically*/, false /*use_mip_map*/);

    m_sprite_sheet = SpriteSheet{
		m_tex_id,
		1200,   // Texture width: 4 frames * 300px
		700,    // Texture height: 2 rows * 350px
		300,    // Frame width
		350,    // Frame height
		7       // Frame count: 7 (4 in first row, 3 in second row)
    };

	// this creates a quad on the xy axes, we have to rotate it up to face the camera with the model matrix
	m_mesh_id = m_sprite_sheet.CreateQuadMesh(asset_manager);

	glm::vec3 up_dir = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 target_dir = -camera_dir;
	float angle = glm::acos(glm::dot(up_dir, target_dir));
    glm::mat4 model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(1.0f, 0.0f, 0.0f));

    m_sprite_data = SpritePipeline::ObjectData{
		.model = model,
		.frame_uvs = m_sprite_sheet.GetCurrentFrameUVs()
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
		glm::vec2 pos = glm::vec2(m_sprite_data.model[3]);
		velocity = glm::normalize(move_dir) * m_move_speed;
		glm::vec2 desired_pos = pos + velocity * dt;
		
		glm::vec2 new_pos = desired_pos;
		if (bounds.IsValid())
			new_pos = bounds.SlideAlongBoundary(pos, desired_pos);

		m_sprite_data.model[3] = glm::vec4(new_pos, 0.0f, 1.0f);

		m_state = State::Walking;
	}
	else
	{
		m_state = State::Idle;
	}

	if (velocity.x > 0.0f && !facing_right // moving right, but facing left
		|| velocity.x < 0.0f && facing_right) // moving left, but facing right
	{
		facing_right = !facing_right;
		// rotate 180 degrees around Z axis to flip the sprite
		glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::pi<float>(), glm::vec3(0.0f, 0.0f, 1.0f));
		glm::vec3 current_pos = glm::vec3(m_sprite_data.model[3]); // preserve translation
		m_sprite_data.model[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		m_sprite_data.model = rotation * m_sprite_data.model;
		m_sprite_data.model[3] = glm::vec4(current_pos, 1.0f); // restore translation after rotation
	}

	if (m_state == State::Walking)
	{
		m_animation_timer += dt;
		if (m_animation_timer >= m_frame_duration)
		{
			m_animation_timer -= m_frame_duration;
			m_sprite_sheet.AdvanceFrame();
			m_sprite_data.frame_uvs = m_sprite_sheet.GetCurrentFrameUVs();
		}
	}
}

void Dog::OnSceneStateChanged(SceneState new_state)
{
	if (new_state != SceneState::Gameplay)
		m_state = State::Idle;
}
