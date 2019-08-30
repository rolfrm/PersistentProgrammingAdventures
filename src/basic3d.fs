#version 300 es
precision highp float;
out vec4 out_color;
in vec3 position;
in vec3 local_position;
in vec3 local_ray;
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

vec3[2] advance_ray(vec3 dir, vec3 p){
  vec3 isneg = sign(dir) * 0.5 - 0.5;
  vec3 p2 = p + dir * 0.0001 + isneg;
  p2 = ceil(p2);
  vec3 d = (p2 - p) / dir;
  vec3 out1 = p + dir * vmin(d);
  vec3 out2 = floor(out1);
  if(d.x < d.z)
    out2.x += isneg.x;
  else
    out2.z += isneg.z;
  vec3 outp[2];
  outp[0] = out1;
  outp[1] = out2;
  return outp;
}

void main(){
  vec3 dir = local_ray / length(local_ray);

  int map[16] = int[](1,0,0,0, 1,0,2,0, 1,3,1,0, 1,0,0,0);
  int map2[16] = int[](0,0,0,0, 0,1,1,0, 0,1,1,0, 1,0,0,0);
  int colors[16] = int[](1,2,3,4, 1,1,2,0, 1,3,4,0, 1,0,0,0);
  
  vec3 ray = local_position * 4.0 - dir * 0.001;
  bool hit = false;
  int color = 0;
  for(int i = 0; i < 10; i++){
    vec3 p = ray;
    
    int x2 = int(p.x);
    int z2 = int(p.z);

    int low2 = map[x2 + z2 * 4];
    int high2 = low2 + map2[x2 + z2 * 4];
    
    vec3 isneg = sign(dir) * 0.5 - 0.5;
    isneg.y = 0.0;
    vec3 p2 = p + dir * 0.001 + isneg;
    if(float(low2) < p.y)
      p2.y = float(high2);
    else
      p2.y = -10.0;
    p2 = ceil(p2);
    vec3 d = (p2 - p) / dir;
    vec3 out1 = p + dir * vmin(d) * 1.001;
    vec3 out2 = floor(out1);
    
    ray = out1;
    int x = int(out2.x);
    int z = int(out2.z);
    if(x < 0 || x >= 4 || z < 0 || z >= 4)
      continue;
    int low = map[x + z * 4];
    int high = low + map2[x + z * 4];
    int y = int(ray.y);
    if(y >= low && y < high ){
      hit = true;
      color = colors[x + z * 4];
      break;
    }
  }

  float l = length(local_position - ray);
  if(hit == false)
    discard;
  if(l > 1.0)
    l = 1.0;
  l *= 0.2;

  vec3 c = vec3(1,1,1);
  if(color == 1)
    c = vec3(1,0,0);
  if(color == 2)
    c = vec3(0,1,0);
  if(color == 3)
    c = vec3(0,0,1);
  if(color == 4)    
    c = vec3(0,1,1);  
  //l = 0.0;
  out_color = vec4(c *(1.0 - l) + l * vec3(0.0),1);	
	

}
