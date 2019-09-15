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

void main(){
  vec3 dir = local_ray / length(local_ray);

  vec3 ray = local_position * size;
  vec3 test1 = ray + dir * 0.01;
  
  float color = 0.0;
  bool hit = false;
  bool blam = false;
  vec3 h1 = get_height(ray);
  if(test1.y < h1.y && test1.y > h1.x){
      color = h1.z;
      hit = true;
    }
  
  
  if(!hit){
    for(int i = 0; i < int(size* 1.5) ; i++){
      vec3 p = ray;
      vec3 h = get_height(ray);
      
      vec3 isneg = sign(dir) * 0.5 - 0.5;
      isneg.y = 0.0;
      vec3 p2 = p + dir * 0.001 + isneg;
      if(h.y < p2.y)
	p2.y = h.y;
      else{
	p2.y = -10.0;
	blam = true;
      }
      p2 = ceil(p2);
      vec3 d = (p2 - p) / dir;
      vec3 out1 = p + dir * vmin(d) * 1.001;
      vec3 out2 = out1;
      
      ray = out1;
      float x = out2.x;
      float z = out2.z;
      if(x < 0.0 || x >= size || z < 0.0 || z >= size)
	continue;
      h = get_height(out2);
      float y = ray.y;
      if(y > h.x && y < h.y + 1.0 ){
      hit = true;
      color = h.z;
      break;
      }
    }
  }

  float l = length(local_position - ray);
  if(hit == false/* && blam*/)
    discard;//color = 2.0;
  if(l > 1.0)
    l = 1.0;
  l *= 0.2;

  vec3 c = vec3(1,1,1);
  if(abs(color - 0.0) < 0.4)
    c = vec3(1,0,0);
  if(abs(color - 1.0) < 0.4)
    c = vec3(0,1,0);
  if(abs(color - 2.0) < 0.4)
    c = vec3(0,0,1);
  if(abs(color - 3.0) < 0.4)    
    c = vec3(0,1,1);  
  //l = 0.0;
  //vec4 texl = texture( tex, ray.xz);
  //c = c * texl.xyz;
  out_color = vec4(c *(1.0 - l) + l * vec3(0.0),1);	
	

}
