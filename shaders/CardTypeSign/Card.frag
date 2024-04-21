uniform vec2 global_pos;
uniform vec2 shape_size;
uniform float time;
uniform float disappear_time;

uniform vec2 texture_pos;
uniform sampler2D texture;

vec3 rgb(int r, int g, int b) {
	return vec3(r / 255., g / 255., b / 255.);
}

void main() {
    vec2 uv = (gl_FragCoord.xy - global_pos) / shape_size / 2.;
    uv += texture_pos / 2.;
    uv.x -= .06;
    uv.y = 1. - uv.y - .06;
    vec2 texCoords = uv;
    vec4 texColor = texture2D(texture, texCoords);
    if (texColor.z > 0.)
    gl_FragColor = vec4(rgb(63, 78, 164) * .4, 1.);
    else 
    gl_FragColor = texColor;
}