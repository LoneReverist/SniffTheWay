// Dog.ixx

module;

export module Dog;

import AssetPool;
import Input;
import MeshManager;
import SpriteSheet;
import SpritesheetPipeline;
import Vertex;

export class Dog
{
public:
    Dog() = default;

    void Init(AssetId tex_id, MeshId<Texture2dVertex> mesh_id, AssetId render_object_id);
    void Update(float dt, Input const & input);

    AssetId GetTextureId() const { return m_tex_id; }
    MeshId<Texture2dVertex> GetMeshId() const { return m_mesh_id; }
    AssetId GetRenderObjectId() const { return m_render_object_id; }
    SpriteSheet const & GetSpriteSheet() const { return m_sprite_sheet; }
    SpritesheetPipeline::ObjectData const & GetSpriteData() const { return m_sprite_data; }

private:
	AssetId m_tex_id;
	MeshId<Texture2dVertex> m_mesh_id;
	AssetId m_render_object_id;

	SpriteSheet m_sprite_sheet;
	SpritesheetPipeline::ObjectData m_sprite_data;

	float m_animation_timer = 0.0f;
	float m_frame_duration = 0.1f; // 100ms per frame = 10 FPS
};

void Dog::Init(AssetId tex_id, MeshId<Texture2dVertex> mesh_id, AssetId render_object_id)
{
    m_tex_id = tex_id;
    m_mesh_id = mesh_id;
    m_render_object_id = render_object_id;

    m_sprite_sheet = SpriteSheet{
		1200,   // Texture width: 4 frames * 300px
		700,    // Texture height: 2 rows * 350px
		300,    // Frame width
		350,    // Frame height
		7       // Frame count: 7 (4 in first row, 3 in second row)
    };

    m_sprite_data = SpritesheetPipeline::ObjectData{
		.frame_uvs = m_sprite_sheet.GetCurrentFrameUVs()
	};
}

void Dog::Update(float dt, Input const & /*input*/)
{
	if (m_sprite_sheet.GetFrameCount() == 0)
		return;

	m_animation_timer += dt;
	if (m_animation_timer >= m_frame_duration)
	{
		m_animation_timer -= m_frame_duration;
		m_sprite_sheet.AdvanceFrame();
		m_sprite_data.frame_uvs = m_sprite_sheet.GetCurrentFrameUVs();
	}
}
