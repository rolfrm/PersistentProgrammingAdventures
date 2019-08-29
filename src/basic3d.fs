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
  float l = length(local_ray);
  l = LinearizeDepth(gl_FragCoord.z) * 0.3;

  vec3 ray = local_position;
  float distance_traveled = 0.0;
  for(int i = 0; i < 30; i++){
    float d = length(ray - vec3(0.3, 0.3, 0.5)) - 0.3;
    float d2 = length(ray - vec3(0.7, 0.5, 0.7)) - 0.3;
    float d3 = length(ray - vec3(0.3, 0.7, 0.5)) - 0.3;
    d = min(d3, min(d, d2));
    
    vec3 color = vec3(1.0,0.6,0.6);
    if(d == d2)
      color = vec3(0.6,1.0,0.6);
    if(d == d3)
      color = vec3(0.6,0.6,1.0);

    if(d < 0.01)
      {
        distance_traveled *= 2.0;
  		    if(distance_traveled > 1.0)	
	  distance_traveled = 1.0;
	  
	out_color = vec4(color * l * (1.0 - distance_traveled) + vec3(0.0) * distance_traveled, 1.0);	
	return;
      }
    if(d > 2.0)
      break;
    ray = ray + dir * d * 1.1;
    distance_traveled += d * 1.1;
    }
  //discard;
  l *= 0.1;
  distance_traveled *= 2.0;
  if(distance_traveled > 1.0)
     distance_traveled = 1.0;
	  
  out_color = vec4(vec3(l, l, l) *(1.0 - distance_traveled) + distance_traveled * vec3(0.0),1);	
	

}
