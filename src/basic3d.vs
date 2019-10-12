#version 300 es
uniform mat4 transform;
uniform mat4 model;
uniform mat4 model2;
uniform mat4 inv_model;
uniform mat4 camera_inv;

layout(location = 0) in vec3 point;
out vec3 position;
out vec3 local_position;
out vec3 local_ray;

float distance_func(vec3 p){

     return
	min(length(p - vec3(0,-70,-100)) - 80.0,
	min(length(p - vec3(6,0,-20)) - 4.0,
     	    length(p - vec3(0,6,-21)) - 4.0));
}

void main(){
   vec4 vertex = vec4(point.x, point.y, point.z, 1.0);
   vec4 vertex_world_pos = model * vertex;
   vec4 cam_pos4 = model2 * vec4(0,0,0,1);
   vec3 cam_pos = cam_pos4.xyz / cam_pos4.w;

   position = vertex_world_pos.xyz / vertex_world_pos.w;

   local_ray = position - cam_pos;
   vec3 r = local_ray / length(local_ray);
   for(int i = 0; i < 10; i++){
      float d = distance_func(vertex_world_pos.xyz);
      if(d > 0.0){
	vertex_world_pos.xyz = vertex_world_pos.xyz + d * r;
      }else{
         break;
      }
   }
   
   local_position = vertex_world_pos.xyz;
   local_ray = local_position - cam_pos;
   gl_Position = transform * camera_inv * vertex_world_pos;
}
