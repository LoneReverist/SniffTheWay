#version 450

layout(std140, binding = 0) uniform ViewProjUniform {
	mat4 view;
	mat4 proj;
} transforms;

layout(std140, binding = 1) uniform ViewportUniform {
    vec2 size;
} viewport;

layout(location = 0) in vec2 in_pos; // not used

layout(location = 1) in vec3 inP0;
layout(location = 2) in vec3 inP1;
layout(location = 3) in float inThicknessPx;
layout(location = 4) in vec4 inColor;

layout(location = 0) out vec4 fragColor;

#ifdef BUILD_VULKAN
#define VERTEX_ID gl_VertexIndex
#else //BUILD_OPENGL
#define VERTEX_ID gl_VertexID
#endif

void main()
{
    fragColor = inColor;

    // Transform endpoints into clip space
    mat4 view_proj = transforms.proj * transforms.view;
    vec4 clip0 = view_proj * vec4(inP0, 1.0);
    vec4 clip1 = view_proj * vec4(inP1, 1.0);

    // Avoid divide-by-zero issues
    float w0 = max(abs(clip0.w), 0.00001);
    float w1 = max(abs(clip1.w), 0.00001);

    // Convert to normalized device coordinates
    vec2 ndc0 = clip0.xy / w0;
    vec2 ndc1 = clip1.xy / w1;

    // Compute screen-space line direction
    vec2 lineDir = ndc1 - ndc0;

    float len = length(lineDir);

    // Degenerate line safety
    if (len < 0.00001)
    {
        lineDir = vec2(1.0, 0.0);
    }
    else
    {
        lineDir /= len;
    }

    // Perpendicular vector
    vec2 normal = vec2(-lineDir.y, lineDir.x);

    // Convert thickness in pixels to NDC space
    float pixelToNdcY = 2.0 / viewport.size.y;

    // Use height so thickness remains uniform
    float thicknessNdc =
        inThicknessPx * pixelToNdcY;

    vec2 halfOffset =
        normal * thicknessNdc * 0.5;

    // ---------------------------------------------------------
    // Determine which quad corner this vertex is
    // 6 vertices:
    //
    // 0 = start left
    // 1 = start right
    // 2 = end left
    // 3 = end left
    // 4 = start right
    // 5 = end right
    // ---------------------------------------------------------
    int v = VERTEX_ID % 6;

    bool useStart = false;
    bool useLeft = false;

    switch (v)
    {
        case 0:
            useStart = true;
            useLeft = true;
            break;

        case 1:
            useStart = true;
            useLeft = false;
            break;

        case 2:
            useStart = false;
            useLeft = true;
            break;

        case 3:
            useStart = false;
            useLeft = true;
            break;

        case 4:
            useStart = true;
            useLeft = false;
            break;

        case 5:
            useStart = false;
            useLeft = false;
            break;
    }

    // Choose endpoint
    vec4 clipPos = useStart ? clip0 : clip1;
    vec2 ndcPos = useStart ? ndc0 : ndc1;

    // Apply thickness offset
    ndcPos += useLeft
        ? -halfOffset
        :  halfOffset;

    // Convert back to clip space
    gl_Position = vec4(
        ndcPos * clipPos.w,
        clipPos.z,
        clipPos.w
    );
}
