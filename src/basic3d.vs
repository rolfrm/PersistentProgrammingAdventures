#version 300 es
uniform mat4 transform;
uniform mat4 model;
uniform mat4 inv_model;
uniform vec3 camera_center;

layout(location = 0) in vec3 point;
out vec3 position;
out vec3 local_position;
out vec3 local_ray;

void main(){
   local_position = point;
   vec4 vertex = vec4(point.x, point.y, point.z, 1.0);
   vec4 vertex_world_pos = model * vertex;
   vec4 cam_pos4 = inv_model * vec4(camera_center, 1);
   vec3 cam_pos = cam_pos4.xyz / cam_pos4.w;
   vec4 center = model * vec4(0.5, 0.5, 0.5, 1.0);

   position = vertex_world_pos.xyz / vertex_world_pos.w;
   local_ray = local_position - cam_pos;
   gl_Position = transform * vertex;
}
