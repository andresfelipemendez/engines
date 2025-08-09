#version 410 core

layout(location = 0) in vec2 a_position;

uniform float u_time;
uniform float u_rotation;

out vec3 v_color;

void main() {
    // Create rotation matrix
    float c = cos(u_rotation);
    float s = sin(u_rotation);
    mat2 rotation = mat2(c, -s, s, c);
    
    // Apply rotation and scale based on time
    vec2 pos = rotation * a_position;
    pos *= (0.8 + 0.2 * sin(u_time * 2.0));
    
    gl_Position = vec4(pos, 0.0, 1.0);
    
    // Generate color based on position and time
    v_color = vec3(
        0.5 + 0.5 * sin(u_time + a_position.x * 3.14159),
        0.5 + 0.5 * cos(u_time + a_position.y * 3.14159),
        0.5 + 0.5 * sin(u_time * 2.0)
    );
}
