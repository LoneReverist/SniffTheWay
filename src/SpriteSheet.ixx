// SpriteSheet.ixx

module;

#include <cstdint>
#include <iostream>
#include <vector>

#include <glm/vec4.hpp>

export module SpriteSheet;

import Dreamhearth;
namespace dh = Dreamhearth;

import AssetPool;
import AssetManager;
import Vertex;

export class SpriteSheet
{
public:
    SpriteSheet() = default;
	explicit SpriteSheet(
		AssetId tex_id,
		std::uint32_t texture_width,
		std::uint32_t texture_height,
		std::uint32_t frame_width,
		std::uint32_t frame_height,
		std::uint32_t frame_count);

	glm::vec4 GetFrameUVs(std::uint32_t frame_index) const;
	glm::vec4 GetCurrentFrameUVs() const;

	void SetCurrentFrame(std::uint32_t frame_index);
	void AdvanceFrame();

	AssetId GetTextureId() const { return m_tex_id; }
	std::uint32_t GetCurrentFrame() const { return m_current_frame; }
	std::uint32_t GetFrameCount() const { return m_frame_count; }
	std::uint32_t GetCols() const { return m_cols; }
	std::uint32_t GetRows() const { return m_rows; }
	std::uint32_t GetFrameWidth() const { return m_frame_width; }
	std::uint32_t GetFrameHeight() const { return m_frame_height; }
	std::uint32_t GetTextureWidth() const { return m_texture_width; }
	std::uint32_t GetTextureHeight() const { return m_texture_height; }

	MeshId<TextureVertex2d> CreateQuadMesh(AssetManager & asset_manager) const;

private:
	AssetId m_tex_id;
	std::uint32_t m_texture_width = 0;
	std::uint32_t m_texture_height = 0;
	std::uint32_t m_frame_width = 0;
	std::uint32_t m_frame_height = 0;
	std::uint32_t m_cols = 0;
	std::uint32_t m_rows = 0;
	std::uint32_t m_frame_count = 0;
	std::uint32_t m_current_frame = 0;
};

SpriteSheet::SpriteSheet(
	AssetId tex_id,
    std::uint32_t texture_width,
    std::uint32_t texture_height,
    std::uint32_t frame_width,
    std::uint32_t frame_height,
    std::uint32_t frame_count)
    : m_tex_id(tex_id)
	, m_texture_width(texture_width)
    , m_texture_height(texture_height)
    , m_frame_width(frame_width)
    , m_frame_height(frame_height)
    , m_cols(texture_width / frame_width)
    , m_rows(texture_height / frame_height)
    , m_frame_count(frame_count)
    , m_current_frame(0)
{
}

// Get UV coordinates for a specific frame
glm::vec4 SpriteSheet::GetFrameUVs(std::uint32_t frame_index) const
{
    if (frame_index >= m_frame_count)
        frame_index = m_frame_count - 1;

    std::uint32_t col = frame_index % m_cols;
    std::uint32_t row = frame_index / m_cols;

    float min_u = (static_cast<float>(col) * m_frame_width) / m_texture_width;
    float max_u = min_u + (static_cast<float>(m_frame_width) / m_texture_width);
    float min_v = (static_cast<float>(row) * m_frame_height) / m_texture_height;
    float max_v = min_v + (static_cast<float>(m_frame_height) / m_texture_height);

    return glm::vec4{min_u, max_u, min_v, max_v};
}

// Get UV coordinates for current frame
glm::vec4 SpriteSheet::GetCurrentFrameUVs() const
{
    return GetFrameUVs(m_current_frame);
}

void SpriteSheet::SetCurrentFrame(std::uint32_t frame_index)
{
    if (frame_index < m_frame_count)
        m_current_frame = frame_index;
}

void SpriteSheet::AdvanceFrame()
{
    m_current_frame = (m_current_frame + 1) % m_frame_count;
}

MeshId<TextureVertex2d> SpriteSheet::CreateQuadMesh(AssetManager & asset_manager) const
{
	float aspect_ratio = static_cast<float>(m_frame_width) / m_frame_height;
	float y_size = 1.0f;
	float y_pos = 0.0f;
	float x_size = 1.0f * aspect_ratio;
	float x_pos = -x_size / 2.0f;

	std::vector<TextureVertex2d> verts{
		{ { x_pos,          y_pos + y_size }, { 0.0, 0.0 } },
		{ { x_pos + x_size, y_pos + y_size }, { 1.0, 0.0 } },
		{ { x_pos,          y_pos          }, { 0.0, 1.0 } },
		{ { x_pos + x_size, y_pos          }, { 1.0, 1.0 } } };

	std::vector<dh::Mesh::IndexT> indices{
		1, 0, 2,
		1, 2, 3 };

	return asset_manager.AddMesh(verts, indices);
}
