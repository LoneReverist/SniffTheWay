#version 450

layout(std140, binding = 0) uniform ViewProjUniform {
	mat4 view;
	mat4 proj;
} transforms;

layout(std140, binding = 1) uniform ViewportUniform {
    vec2 size;
} viewport;

layout(location = 0) in vec2 in_pos;

layout(location = 1) in vec3 inP0;
layout(location = 2) in vec3 inP1;
layout(location = 3) in float inThicknessPx;
layout(location = 4) in vec4 inColor;

layout(location = 0) out vec4 fragColor;

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

    // Compute screen-space line direction in pixels so thickness stays uniform
    // regardless of viewport aspect ratio.
    vec2 screen0 = (ndc0 * 0.5 + 0.5) * viewport.size;
    vec2 screen1 = (ndc1 * 0.5 + 0.5) * viewport.size;
    vec2 lineDir = screen1 - screen0;

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

    vec2 halfOffset = normal * inThicknessPx * 0.5;

    bool useStart = in_pos.x < 0.0;
    bool useLeft = in_pos.y > 0.0;

    // Choose endpoint
    vec4 clipPos = useStart ? clip0 : clip1;
    vec2 ndcPos = useStart ? ndc0 : ndc1;

    // Apply pixel-space thickness offset, then convert back to NDC.
    vec2 screenPos = (ndcPos * 0.5 + 0.5) * viewport.size;
    screenPos += useLeft
        ? -halfOffset
        :  halfOffset;
    ndcPos = (screenPos / viewport.size - 0.5) * 2.0;

    // Convert back to clip space
    gl_Position = vec4(
        ndcPos * clipPos.w,
        clipPos.z,
        clipPos.w
    );
}
