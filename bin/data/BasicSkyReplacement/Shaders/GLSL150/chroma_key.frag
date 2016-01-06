#version 150

uniform sampler2DRect tex0;

uniform vec4 keyColor;
uniform float tolerance;

in vec2 texCoordVarying;

out vec4 fragColor;

void main (void){
    vec2 pos = texCoordVarying;
    
    vec3 src = texture(tex0, pos).rgb;
    float alpha = gl_Color.a;
    
    if (abs(keyColor.r - src.r) <= tolerance &&
        abs(keyColor.g - src.g) <= tolerance &&
        abs(keyColor.b - src.b) <= tolerance)
    {
        alpha = 0.0;
    }
    
    fragColor = vec4( src , alpha);
}