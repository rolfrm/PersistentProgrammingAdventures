#version 300 es
precision mediump float;
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

void main(){
  vec3 dir = local_ray / length(local_ray);

  int map[16] = int[](1,0,0,0, 1,0,2,0, 1,3,1,0, 1,0,0,0);
  int map2[16] = int[](0,0,0,0, 0,1,1,0, 0,1,1,0, 1,0,0,0);
  int colors[16] = int[](1,2,3,4, 1,1,2,0, 1,3,4,0, 1,0,0,0);
  
  vec3 ray = local_position;
  bool hit = false;
  int color = 0;
  for(int i = 0; i < 100; i++){

    int x = int(ray.x * 4.0);
    int z = int(ray.z * 4.0);
    ray = ray + dir * 0.01;
    if(x >= 4 || x < 0 || z >= 4 || z < 0 || ray.y < 0.0)
      continue;
    int low = map[x + z * 4];
    int high = low + map2[x + z * 4];
    int y = int(ray.y * 4.0);
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
