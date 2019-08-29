#version 300 es
precision mediump float;
out vec4 out_color;
in vec3 position;
in vec3 local_position;
in vec3 local_ray;
void main(){
  vec3 dir = local_ray / length(local_ray);
  float l = length(local_ray);
  
  vec3 ray = local_position;
  for(int i = 0; i < 10; i++){
    float d = length(ray - vec3(0.3, 0.3, 0.5)) - 0.3;
    float d2 = length(ray - vec3(0.7, 0.5, 0.7)) - 0.3;
    float d3 = length(ray - vec3(0.3, 0.7, 0.5)) - 0.3;
    d = min(d3, min(d, d2));
    if(d < 0.005)
      {
	out_color = vec4(l, l, l,1);	
	return;
      }
    if(d > 2.0)
      break;
    ray = ray + dir * d * 1.1;
    }
  //discard;
  l *= 0.1;
  out_color = vec4(l, l, l,1);	
	

}
