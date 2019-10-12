#version 300 es
precision highp float;

out vec4 out_color;
in vec3 position;
in vec3 local_position;
in vec3 local_ray;

uniform sampler2D tex;
uniform float size;

#define NEAR = 3
float near = 0.1; 
float far  = 100.0; 
  
float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}
float vmin(vec3 v){
  if(v.x < v.y){
    if(v.z < v.x)
      return v.z;
    return v.x;
  }
  if(v.z < v.y)
    return v.z;
  return v.y;
}

//
// x : base level
// y : top level
// z : color
vec3 get_height(vec3 ray){

  vec4 t = texture(tex, ray.xz / size);
  return t.xyz * size;
}


float distance_func(vec3 p){
  float a = length(p - vec3(0,-70,-100)) - 80.0;
  float b = length(p - vec3(3,0,-10)) - 4.0;
  float c = length(p - vec3(0,3,-11)) - 4.0;
  if( a < b && a < c){
    return a;
  }
  if(b < c)
    return b;
  return c;
}

vec3 color_func(vec3 p){
  float a = length(p - vec3(0,-70,-100)) - 80.0;
  float b = length(p - vec3(3,0,-10)) - 4.0;
  float c = length(p - vec3(0,3,-11)) - 4.0;
  if( a < b && a < c){
    return vec3(1,0,0);
  }
  if(b < c)
    return vec3(0,1,0);
  return vec3(0,0,1);
}

void main(){

  vec3 dir = local_ray / length(local_ray);

  vec3 ray = local_position;
  for(int i = 0; i < 0; i++){
    float d = distance_func(ray);
    if(d < 0.0) break;
    ray += dir * d;
  }
  float col = distance_func(ray);

  float distance2 = length(ray);
  vec3 color = color_func(ray);
  out_color = vec4(color - vec3(distance2 * 0.01),1.0);

}
