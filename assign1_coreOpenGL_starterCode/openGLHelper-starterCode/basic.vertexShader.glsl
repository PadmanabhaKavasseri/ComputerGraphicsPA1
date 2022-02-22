#version 150

in vec3 position;
in vec4 color;
in vec3 left, right, up, down;
out vec4 col;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
//uniform int mode;

float eps = 0.00001f;

void main(){
    int mode = 0;
    if (mode == 0){
        gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0f);
        col = color;
    }
    else{
        float smoothHeight = (left.y + right.y + down.y + up.y)/4.0f;
        col = smoothHeight * max(color,vec4(eps))/max(position.y,eps);
        gl_Position = projectionMatrix * modelViewMatrix * vec4(position.x, smoothHeight, position.z, 1.0f);
    }
  // compute the transformed and projected vertex position (into gl_Position) 
  // compute the vertex color (into col)
  
  
    
//    col = vec4(1,1,1,1);
}

