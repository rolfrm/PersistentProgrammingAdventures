#version 300 es
precision highp float;

out vec4 out_color;
in vec3 position;
in vec3 local_position;
in vec3 local_ray;
in vec3 normal;
float distance_func(vec3 p){
  float a = length(p - vec3(0,-70,-100)) - 80.0;
  float b = length(p - vec3(6,0,-20)) - 4.0;
  float c = length(p - vec3(0,6,-21)) - 4.0;
  if( a < b && a < c){
    return a;
  }
  if(b < c)
    return b;
  return c;
}

vec3 color_func(vec3 p){
  float a = length(p - vec3(0,-70,-100)) - 80.0;
  float b = length(p - vec3(6,0,-20)) - 4.0;
  float c = length(p - vec3(0,6,-21)) - 4.0;
  if( a < b && a < c){
    return vec3(0.6,0.8,0.6);
  }
  if(b < c)
    return vec3(1,0.7,0.1);
  return vec3(1.0,1.0,0.2);
}

void main(){

     
  vec3 dir = local_ray;
  dir = dir / length(dir);
  vec3 ray = local_position;
  for(int i = 0; i < 0; i++){
    float d = distance_func(ray);
    if(d < 0.001) break;
    ray += dir * d;
  }
  float col = distance_func(ray);
  float distance2 = distance_func(ray);
  //  if(distance2 < 0.0) distance2 =0.0;
  vec3 color = color_func(ray);
  //  float ang = dot(normalize(normal), normalize(dir));
  //  if(ang > -0.2) discard;
  out_color = vec4(color, 1.0 - distance2 * 1.0);

}
