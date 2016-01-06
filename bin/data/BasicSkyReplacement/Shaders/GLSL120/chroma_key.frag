#version 120
#extension GL_ARB_texture_rectangle : enable
#extension GL_EXT_gpu_shader4 : enable

uniform sampler2DRect tex0;

uniform vec4 keyColor;
uniform float tolerance;

void main (void){
    vec2 pos = gl_TexCoord[0].st;
    
    vec3 src = texture2DRect(tex0, pos).rgb;
    float alpha = gl_Color.a;
        
    if (abs(keyColor.r - src.r) <= tolerance &&
        abs(keyColor.g - src.g) <= tolerance &&
        abs(keyColor.b - src.b) <= tolerance)
    {
        alpha = 0.0;
    }
    
    gl_FragColor = vec4( src , alpha );
}