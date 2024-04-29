uniform vec2 global_pos;
uniform vec2 shape_size;
uniform float time;
uniform float disappear_time;
uniform int hover;
uniform float radius;
uniform vec3 color_set_in;

uniform sampler2D mid_texture;
uniform sampler2D text_texture;

#define PI 3.14159265359

float random (in vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}

float noise (in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

#define OCTAVES 6
float fbm (in vec2 st) {
    // Initial values
    float value = 0.0;
    float amplitude = .5;
    float frequency = 0.;
    //
    // Loop of octaves
    for (int i = 0; i < OCTAVES; i++) {
        value += amplitude * noise(st);
        st *= 2.;
        amplitude *= .5;
    }
    return value;
}

float slow(float x, float a) {
  if (x < .5) {
    return pow(2. * x, a) / 2.;
  } else {
    return 1. - pow(2. * (1. - x), a) / 2.;
  }
}

float pattern( in vec2 p, out vec2 q, out vec2 r )
{
    q.x = fbm( p + vec2(0.0,0.0) );
    q.y = fbm( p + vec2(5.2,1.3) );

    r.x = fbm( p + 4.0*q + vec2(1.7,9.2) );
    r.y = fbm( p + 4.0*q + vec2(8.3,2.8) );

    return fbm( p + 4.0*r );
}

vec3 rgb(int r, int g, int b) {
	return vec3(r / 255., g / 255., b / 255.);
}

vec3 adjustBrightness(vec3 color, float adjustment) {
    return clamp(color + adjustment, 0.0, 1.0);
}

void main()
{
    vec2 uv = gl_FragCoord.xy  - global_pos;
    float len = length(abs(uv));
    vec3 colorIn = color_set_in / 255.;

    float light = 3. - colorIn.x +- colorIn.y - colorIn.z;

    float ratio = .6;
    vec2 origin = global_pos - vec2(radius * ratio, radius * ratio);
    vec2 vUv = (gl_FragCoord.xy - origin) / (radius * ratio * 2.);

    float a = atan(uv.y, uv.x);
    a = mod(a + 2 * PI, 2 * PI);
    a = abs(a - PI) / PI;
    float shader = slow(smoothstep(-1.5, 1.5, a), 1.);

    if (len < radius * ratio) {
        vec3 col_text = texture2D(text_texture, vUv).xyz;
        if (col_text.x > 0.) gl_FragColor = vec4(colorIn * light * smoothstep(0., 1., texture2D(text_texture, vUv).r), 1.); 
        else {
            vec2 o, n;
            float dir = pow(mod(time, 2.) - 1., 2.); // + vec2(dir * cos(time), -dir * sin(time))
            float pattern_ans = pattern(vUv + time / 2., o, n);
            pattern_ans = pattern(vUv + pattern_ans, o, n);
            pattern_ans = pattern(vUv + pattern_ans * time / 10., o, n);
            vec3 col = vec3(0.2,0.1,0.4);
            col = mix( col, adjustBrightness(colorIn, 0.1), pattern_ans );
            col = mix( col, adjustBrightness(colorIn, -0.1), dot(n,n) );
            col = mix( col, colorIn, 0.5*smoothstep(1.2,1.3,abs(n.y)+abs(n.x)) );
            col *= pattern_ans*2.0;
            if (texture2D(text_texture, vUv + .02 * sin(time) * cos(time)).x > 0.) gl_FragColor = vec4(col * pow(shader, 2.), 1.);
            else {
                gl_FragColor = vec4(col * shader * light, 1.);
            }
        }
	} else if (len < radius * (ratio + (1. - ratio) / 3. * 2.)) {
        float color = fbm(vUv + time / 10.) * 1.2;
        float theta = atan(uv.y, uv.x);

        for (int i=0; i< 3; i++) {
            theta = mod(theta + (time / float(i + 1) ) + 2. * PI + i, PI);
            vec2 theta_uv = vec2(len, theta / PI / 2.);
            float f = fbm(theta_uv) / 2.;

            float line_width = .005;

            float stripe = (1. - ratio) / 4. * (float(i + 1) + f);
            if (len > radius * (ratio + stripe - line_width) && len < radius * (ratio + stripe + line_width)) {
                color = 1.;
				break;
			}
        }

        //color *= pow(shader, .01);
        color *= light;

        gl_FragColor = vec4(color * colorIn, 1.);
        
    }else if (len < radius) {
        float rotate_time = sin(time / 10.) * cos(time);
        vUv *= mat2(cos(rotate_time), -sin(rotate_time), sin(rotate_time), cos(rotate_time));
        vec3 color = rgb(255, 255, 255);
        color = mix( color, adjustBrightness(colorIn, -0.1), fbm(vUv) );
        color = mix( color, colorIn, fbm(vUv + fbm(vUv)) );
        color = mix( color, adjustBrightness(colorIn, -0.1), fbm(vUv + fbm(vUv +  fbm(vUv + time))) );
        color *= fbm(vUv + fbm(vUv +  fbm(vUv + sin(time / 10.))));

        color *= light;
        gl_FragColor = vec4(color * colorIn, 1.);
    }
}