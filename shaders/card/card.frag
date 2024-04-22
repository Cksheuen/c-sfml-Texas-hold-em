uniform vec2 global_pos;
uniform vec2 shape_size;
uniform float time;
uniform float disappear_time;

uniform bool if_mid_texture;
uniform sampler2D mid_texture;
uniform sampler2D text_texture;

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

mat2 rotate2d(float _angle){
    return mat2(cos(_angle),-sin(_angle),
                sin(_angle),cos(_angle));
}


void main()
{
    vec2 uv = (gl_FragCoord.xy - global_pos) / shape_size;

    uv -= 0.5;
    uv = rotate2d(3.14 / 180. * 0.) * uv;
    uv += 0.5;

    /*if (uv.x < 0. || uv.x > 1. || uv.y < 0. || uv.y > 1.) {
        gl_FragColor = vec4(0., 0., 0., 0.);
		return;
    }*/

    vec2 o, n;
    float pattern_ans = pattern(uv, o, n);
        
    vec3 col = vec3(0.2,0.1,0.4);
    col = mix( col, rgb(132, 126, 201), pattern_ans );
    col = mix( col, rgb(63, 78, 164), dot(n,n) );
    //col = mix( col, rgb(158, 152, 222), 0.5*o.y*o.y );
    col = mix( col, rgb(92, 107, 189), 0.5*smoothstep(1.2,1.3,abs(n.y)+abs(n.x)) );
    col *= pattern_ans*2.0;
    gl_FragColor = vec4(col,1.0);


    float f = fbm(uv * 5.) / 30.;
	float y_to_x = shape_size.x / shape_size.y;
	float edge_width = 0.04;
    float edge_to_border = 0.05;
    float cranny_width = edge_to_border - edge_width / 2.;
	float dx = min(abs(uv.x - edge_to_border - f), abs(uv.x - (1. - edge_to_border - f)));
	float dy = min(abs(uv.y - (edge_to_border + f) * y_to_x), abs(uv.y - (1. - (edge_to_border + f) * y_to_x)));
    float edge = smoothstep(0., edge_width / 2., min(dx, dy / y_to_x));

    vec2 d =abs(uv - vec2(.5));
    float edge_color = smoothstep(.5, 1., slow(length(d), f * 60.));
    
    float per = 0.;
    if (uv.x < cranny_width || uv.x > 1.0 - cranny_width ||
    uv.y < cranny_width * y_to_x || uv.y > 1.0 - cranny_width * y_to_x) per = 1;
    edge = mix(edge, edge_color, per);
    
	if (edge <= .5) gl_FragColor = vec4(rgb(63, 78, 164) * .6, 1.);


    vec2 edge_vec = edge_to_border * shape_size * 2.;
    edge_vec.y *= y_to_x;

    float signWidth = shape_size.x / 5.;
    dx = min(gl_FragCoord.x - global_pos.x - shape_size.x / 2. - edge_vec.x, global_pos.x + shape_size.x * 1.5 - edge_vec.x - gl_FragCoord.x);
    dy = signWidth - min(gl_FragCoord.y - global_pos.y - shape_size.x / 2. - edge_vec.y, global_pos.y + shape_size.y * 1.5 - edge_vec.y - gl_FragCoord.y);

    vec2 sign_uv = vec2(dx, dy) / signWidth;
    sign_uv -= 0.5;
    sign_uv = rotate2d(3.14 / 180. * time * 10.) * sign_uv;
    sign_uv += 0.5;

    if ( dx <= signWidth && dx >= 0. && dy <=signWidth && dy >= 0. && abs(uv.x - uv.y) > .5) {
        vec4 text_color = texture(text_texture, sign_uv);
        //if (text_color != vec4(0., 0., 0., 0.)) 
        if (text_color.z > 0.)
        gl_FragColor = vec4(rgb(63, 78, 164) * .4, 1.);
    }

    if (if_mid_texture) {
        float ratio = 2.;
        dx = gl_FragCoord.x - global_pos.x - (shape_size.x - shape_size.x / ratio) / 1.7;
        dy = gl_FragCoord.y - global_pos.y - (shape_size.y - shape_size.x / ratio) / 2.;

        sign_uv = vec2(dx, dy) / shape_size.x;
        sign_uv -= 0.5;
        sign_uv = rotate2d(3.14 / 180. *0.) * sign_uv;
        sign_uv += 0.5;

        if (sign_uv.x > 0. && sign_uv.x <= 1. && sign_uv.y > 0. && sign_uv.y <= 1.) {
            vec4 mid_color = texture(mid_texture, sign_uv);
			if (mid_color.a > 0.) 
            gl_FragColor = vec4(rgb(63, 78, 164) * .4, 1.);
            //gl_FragColor = mid_color;
        }
        
	}

    if (disappear_time > 0.) {
        vec3 base = vec3(edge);
        vec2 vUv = uv;
        uv.y /= y_to_x;
        float t = 1. -  pow(3. * disappear_time, .9);

        float edges_mask = 1. - max(.4, pow(length(vUv - vec2(.5)), 1.5));
        float noise_mask = fbm(vec2(.01 * shape_size * uv)) / edges_mask;
        noise_mask -= .06 * length(base.rgb);

        vec3 color = mix(base.rgb, vec3(0.), smoothstep(noise_mask - .15, noise_mask - .1, t));
        vec3 fire_color = fbm(6. * vUv + .1 * t) * vec3(6., 1.4, .0);
        color = mix(color, fire_color, smoothstep(noise_mask - .1, noise_mask - .05, t));
        color -= .3 * fbm(3. * vUv) * pow(t, 4.);

        float opacity = 1. - smoothstep(noise_mask - .01, noise_mask, t);

        gl_FragColor = vec4(color, opacity);
    }

}