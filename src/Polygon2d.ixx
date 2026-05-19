// Polygon2d.ixx

module;

#include <algorithm>
#include <optional>
#include <vector>

#include <glm/glm.hpp>

export module Polygon2d;

export struct LineSegment2d
{
	glm::vec2 start;
	glm::vec2 end;
};

export struct Intersection2d
{
	glm::vec2 point;      // world-space intersection point
	float     t;          // parametric distance along the *query* segment [0, 1]
	int       edge_index; // which polygon edge (index of the edge's first vertex)
};

export class Polygon2d
{
public:
	Polygon2d() = default;
	explicit Polygon2d(std::vector<glm::vec2> vertices)
		: m_vertices{ std::move(vertices) } {}

	void SetVertices(std::vector<glm::vec2> vertices) { m_vertices = std::move(vertices); }
	std::vector<glm::vec2> const & GetVertices() const { return m_vertices; }
	int EdgeCount() const { return static_cast<int>(m_vertices.size()); }
	bool IsValid() const { return m_vertices.size() >= 3; }

	// Returns every intersection of `segment` with the polygon's edges,
	// sorted by t (closest to segment.start first).
	std::vector<Intersection2d> FindIntersections(LineSegment2d const & segment) const;

	// Returns the single closest intersection to segment.start, if any.
	std::optional<Intersection2d> FindClosestIntersection(LineSegment2d const & segment) const;

	// Ray-casting point-in-polygon test.
	bool Contains(glm::vec2 point) const;

	// Constrains a movement from `from` (assumed inside) to `to`.
	//  • If `to` is still inside the polygon → returns `to` unchanged.
	//  • If the path exits the polygon → returns the boundary intersection
	//    point nudged inward by `skin_width` so the caller never sits exactly
	//    on an edge.
	glm::vec2 ClampToBoundary(
		glm::vec2 from,
		glm::vec2 to,
		float skin_width = 0.001f) const;

	// Slides `to` along the boundary edge instead of stopping dead.
	// Useful for wall-sliding movement that feels natural.
	glm::vec2 SlideAlongBoundary(
		glm::vec2 from,
		glm::vec2 to,
		float skin_width = 0.001f,
		int max_bounces = 4) const;

private:
    static constexpr int k_no_excluded_edge = -1;

	// Remove any component of `v` that points in the direction of `outward_normal`.
	// Equivalent to projecting v onto the plane defined by outward_normal.
	static glm::vec2 clip_against_normal(glm::vec2 v, glm::vec2 outward_normal);

	// Takes a movement *vector* rather than a destination so we can scale it by
	// (1 - t) at each step without recomputing from scratch.
	//
	// `hit_outward_normals` accumulates the outward-facing normals of every wall
	// we have already bounced off.  Each new step clips `move` against all of
	// them before proceeding, which is the Quake/id-software corner fix.
	glm::vec2 slide_impl(
		glm::vec2 from,
		glm::vec2 move,
		float skin_width,
		int excluded_edge,
		int bounces_left,
		std::vector<glm::vec2> hit_outward_normals) const;

	std::vector<Intersection2d> find_intersections_impl(
		LineSegment2d const & segment,
		int excluded_edge) const;

	// Intersect segment AB with segment CD.
	// Returns {t, s} — parameters along AB and CD — when segments properly
	// cross (t ∈ [0,1], s ∈ [0,1]).
	//
	//   A + t·r = C + s·sv       where r = B−A, sv = D−C
	//   t = (C−A) × sv / (r × sv)
	//   s = (C−A) × r  / (r × sv)
	//
	// 2-D cross product: a×b = a.x·b.y − a.y·b.x  (scalar)
	static std::optional<std::pair<float, float>> segment_intersect(
		glm::vec2 a, glm::vec2 b,  // query segment
		glm::vec2 c, glm::vec2 d); // polygon edge

private:
	std::vector<glm::vec2> m_vertices;
};

std::vector<Intersection2d> Polygon2d::FindIntersections(LineSegment2d const &segment) const
{
	return find_intersections_impl(segment, k_no_excluded_edge);
}

std::optional<Intersection2d> Polygon2d::FindClosestIntersection(LineSegment2d const &segment) const
{
	auto hits = FindIntersections(segment);
	if (hits.empty())
		return std::nullopt;
	return hits.front();
}

bool Polygon2d::Contains(glm::vec2 point) const
{
	if (!IsValid())
		return false;

	// Cast a ray rightward (+X) from `point` and count edge crossings.
	const int n = EdgeCount();
	int hits = 0;

	for (int i = 0; i < n; ++i)
	{
		glm::vec2 a = m_vertices[i];
		glm::vec2 b = m_vertices[(i + 1) % n];

		// Only test edges that straddle the point's Y value.
		if ((a.y <= point.y && b.y > point.y) ||
			(b.y <= point.y && a.y > point.y))
		{
			// X coordinate of the edge at point.y
			float x_intersect = a.x + (point.y - a.y) / (b.y - a.y) * (b.x - a.x);
			if (point.x < x_intersect)
				++hits;
		}
	}

	return (hits % 2) != 0; // odd == inside
}

glm::vec2 Polygon2d::ClampToBoundary(
	glm::vec2 from,
	glm::vec2 to,
	float skin_width /*= 0.001f*/) const
{
	if (Contains(to))
		return to;

	auto hit = FindClosestIntersection(LineSegment2d{from, to});
	if (!hit.has_value())
		return from;

	// Pull back along the movement direction so we stay just inside.
	glm::vec2 dir = to - from;
	float len = glm::length(dir);
	if (len > 1e-6f)
		return hit->point - (dir / len) * skin_width;

	return hit->point;
}

glm::vec2 Polygon2d::SlideAlongBoundary(
	glm::vec2 from,
	glm::vec2 to,
	float skin_width /*= 0.001f*/,
	int max_bounces /*= 4*/) const
{
	glm::vec2 result = slide_impl(from, to - from, skin_width, k_no_excluded_edge, max_bounces, {});

	// Safety net: if result somehow ended up outside (e.g. degenerate corner
	// geometry), the caller's `from` is always a known-good interior point.
	if (!Contains(result))
		return from;

	return result;
}

// static
glm::vec2 Polygon2d::clip_against_normal(glm::vec2 v, glm::vec2 outward_normal)
{
	float d = glm::dot(v, outward_normal);
	if (d > 0.0f)
		v -= d * outward_normal;
	return v;
}

glm::vec2 Polygon2d::slide_impl(
	glm::vec2 from,
	glm::vec2 move,
	float skin_width,
	int excluded_edge,
	int bounces_left,
	std::vector<glm::vec2> hit_outward_normals) const
{
	if (bounces_left == 0 || !IsValid())
		return from;

	if (glm::length(move) < 1e-6f)
		return from;

	if (Contains(from + move))
		return from + move;

	auto hits = find_intersections_impl(LineSegment2d{from, from + move}, excluded_edge);
	if (hits.empty())
		return from;

	Intersection2d const &hit = hits.front();

	// ── Edge geometry ─────────────────────────────────────────────────────

	const int n = EdgeCount();
	glm::vec2 edge_vec = m_vertices[(hit.edge_index + 1) % n] - m_vertices[hit.edge_index];
	glm::vec2 tangent = glm::normalize(edge_vec);

	// Outward normal — whichever perpendicular points away from `from`.
	glm::vec2 outward_normal = glm::vec2(-tangent.y, tangent.x);
	if (glm::dot(outward_normal, from - hit.point) > 0.0f)
		outward_normal = -outward_normal;

	// Nudge back along the *incoming* direction instead of along the wall
	// normal. This is the fix for acute corners: the incoming direction always
	// points toward the interior, whereas the wall normal can cross an adjacent
	// wall when the corner angle is small.
	glm::vec2 incoming_dir = glm::normalize(move);
	glm::vec2 slide_from = hit.point - incoming_dir * skin_width;

	// ── Velocity clipping ─────────────────────────────────────────────────

	float t_remaining = 1.0f - hit.t;
	glm::vec2 remaining = move * t_remaining;

	// Clip against the current wall.
	remaining = clip_against_normal(remaining, outward_normal);

	// Clip against all previously hit walls. When two opposing wall normals
	// have both been applied, the result reaches zero and the mover stops.
	for (glm::vec2 const &prev : hit_outward_normals)
		remaining = clip_against_normal(remaining, prev);

	if (glm::length(remaining) < 1e-6f)
		return slide_from;

	hit_outward_normals.push_back(outward_normal);
	return slide_impl(slide_from, remaining, skin_width, hit.edge_index, bounces_left - 1, std::move(hit_outward_normals));
}

std::vector<Intersection2d> Polygon2d::find_intersections_impl(
	LineSegment2d const & segment,
	int excluded_edge) const
{
	std::vector<Intersection2d> results;
	if (!IsValid())
		return results;

	const int n = EdgeCount();
	for (int i = 0; i < n; ++i)
	{
		if (i == excluded_edge)
			continue;

		glm::vec2 const &edge_a = m_vertices[i];
		glm::vec2 const &edge_b = m_vertices[(i + 1) % n];

		if (auto hit = segment_intersect(segment.start, segment.end, edge_a, edge_b))
		{
			float t = hit->first;
			results.push_back(Intersection2d{
				.point = segment.start + t * (segment.end - segment.start),
				.t = t,
				.edge_index = i,
			});
		}
	}

	std::ranges::sort(results, {}, &Intersection2d::t);
	return results;
}

// static
std::optional<std::pair<float, float>> Polygon2d::segment_intersect(
	glm::vec2 a, glm::vec2 b, // query segment
	glm::vec2 c, glm::vec2 d) // polygon edge
{
	auto cross2d = [](glm::vec2 u, glm::vec2 v) -> float
	{
		return u.x * v.y - u.y * v.x;
	};

	glm::vec2 r = b - a;
	glm::vec2 sv = d - c;
	float rxs = cross2d(r, sv);

	if (std::abs(rxs) < 1e-8f)
		return std::nullopt; // parallel or collinear

	glm::vec2 ca = c - a;
	float t = cross2d(ca, sv) / rxs;
	float s = cross2d(ca, r) / rxs;

	constexpr float k_eps = 1e-6f;
	if (t >= -k_eps && t <= 1.0f + k_eps &&
		s >= -k_eps && s <= 1.0f + k_eps)
	{
		return std::make_pair(
			std::clamp(t, 0.0f, 1.0f),
			std::clamp(s, 0.0f, 1.0f));
	}

	return std::nullopt;
}
