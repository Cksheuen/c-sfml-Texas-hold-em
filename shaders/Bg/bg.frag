uniform vec2 shape_size;
uniform float time;

#define PI 3.14159265359
#define RADIUS 0.3

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

#define OCTAVES 10
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

vec3 cal_turntable(vec2 uv, float theta_noise, float rotate_speed, float len, float a, vec3 colorIn) {
    vec3 color = colorIn;
	float new_speed = mod(time, 2. * PI) * (1.5 + sin(mod(time, 2. * PI)) * .5 );
    int check = 0;

    float angle[4] = float[4](PI / 2. + new_speed , PI + new_speed, PI / 2. * 3. + new_speed, PI * 2. + new_speed);
    vec2 balls[5];
    float ball_radius = RADIUS * .45;
    vec2 coners[4] = vec2[4](vec2(.5 - ball_radius), vec2(.5 + ball_radius), 
    vec2(.5 - ball_radius, .5 + ball_radius), vec2(.5 + ball_radius, .5 - ball_radius));
    balls[0] = vec2(.5, .5);
    for (int i=0; i< 4; i++) {
        int times = 0;
        for (int j = 0; j < 4; j++) {
            vec2 coner = vec2(.5 + cos(angle[j] + PI / 4.) * ball_radius, .5 + sin(angle[j] + PI / 4.) * ball_radius);
			float d = distance(uv, coner);
			if (d > ball_radius * .7 && len < RADIUS * .25 * theta_noise) {
                times += 1;
            }
        }
        if (times == 4) {
            color = mix(color, rgb(253, 212, 149), .2);
            check = 1;
        }
    }
    float turntable = smoothstep(.8, 2.3, cos((a - new_speed - rotate_speed )*4.));
    turntable = turntable * 5.+ 0.03 ;
    turntable = 1.-smoothstep(turntable,turntable+0.02,len * 2.);
    if (turntable > 0.) {
	    color = vec3(turntable * rgb(253, 212, 149) );
        check = 1;
	}
    for (int i = 0; i < 4; i++) {
	    balls[i + 1] = vec2(.5 + cos(angle[i]) * ball_radius, .5 + sin(angle[i]) * ball_radius);
        float d = distance(uv, balls[i + 1]);
        if (d < RADIUS * .04) {
		    color = rgb(253, 212, 149);
            check = 1;
		}
	}
    if (check == 1) {
        vec2 vuv = (uv - coners[0]) / vec2(ball_radius * 2.);
        color *= smoothstep(-1., 1., vuv.x * vuv.y );
        return color;
	}
    return vec3(-1.);
}

void main()
{
    /*if (gl_FragCoord.x > 0. && gl_FragCoord.x < shape_size.x / 2. || gl_FragCoord.y > 0.&& gl_FragCoord.y < shape_size.y / 2.) {
        gl_FragColor = vec4(0., 0., 0., 1.);
		return;
	}*/
    vec2 uv = gl_FragCoord.xy  / shape_size;
    uv.x *= shape_size.x / shape_size.y;
    uv.x += (1. - shape_size.x / shape_size.y) / 2.;

    vec2 center = vec2(.5, .5);
    vec2 p = uv - center;
    float a = atan(p.y, p.x);
    float len = length(p);
    float noise = fbm(uv + fbm(uv + fbm(uv * 5. + time)));
    a = mod(a, PI * 2.);
    float theta_noise = 1. - fbm(vec2(a / PI, len / RADIUS) * 5. + 1.) * .02 + sin(time) * (1. - smoothstep(0., 1., time)) * .25;
    float rotate_speed = time / 2.;

    vec3 color = vec3(1.);
    if (len < RADIUS * .5 * theta_noise) {
        a += rotate_speed;
        float f;
        f = cos(a * 8.);
        f = smoothstep(.5, .9, f) * .5;
        float in_len = len * ( 1. - smoothstep(.0, RADIUS * .5, len) * .5);
        color = (1. - smoothstep(f, f+0.02, in_len)) * rgb(146, 63, 24) * .8 + rgb(146, 63, 24) * smoothstep(f, f+0.02, len);

        vec3 turntable = cal_turntable(uv, theta_noise, rotate_speed, len, a, color);
        if (turntable.x > 0.) {
			color = vec3(turntable);
		}

        vec3 shader_check = cal_turntable(uv + vec2(.001, .001), theta_noise, rotate_speed, len, a, color);
        if (shader_check.x  > 0.) {
            color *= .5;
        }

    } else
    if (len < RADIUS * .63 * theta_noise) {
        a = mod(a + 2 * PI - rotate_speed, 2 * PI);
        vec3 inner_color = rgb(67, 162, 151);
        if (mod(a * 20. / PI, 2.) < 1.) {
            inner_color = vec3(1.);
        }
        float f;
        f = cos(a * 20.);
        f = smoothstep(.8, .9 + noise , f);
        color = (1. - smoothstep(f, f+0.02, len)) * rgb(186, 172, 147) * .8 + inner_color * smoothstep(f, f+0.02, len);
        if (len < RADIUS * .53 * theta_noise) {
			color *= smoothstep(0., 1., (len - RADIUS * .5 * theta_noise) / (RADIUS * .03 * theta_noise))
            * smoothstep(0., 1., mod(abs(a - PI), PI) / PI);
		}
    } else 
    if (len < RADIUS * .65 * theta_noise) {
		color = rgb(161, 103, 72);
	} else
    if (len < RADIUS * .8 * theta_noise) {
        if (len > RADIUS * .69 * (1. - theta_noise * 10.) && len < RADIUS * .69 * (1. + theta_noise * 10.)) {
            
            color = rgb(161, 103, 72);
		}
        a += rotate_speed;
        a = cos(a * 20.);
        color = (1. - smoothstep(a, a+0.02, len)) * rgb(255, 125, 86) * .8 + rgb(47, 41, 36) * smoothstep(a, a+0.02, len);
    } else 
    if (len < RADIUS * .86 * theta_noise) {
        color = rgb(142, 94, 53) * (.7 * theta_noise + .5 * smoothstep(0., 1., abs(len - RADIUS * .815 * theta_noise) / (RADIUS * .015 * theta_noise)));
    } else 
    if (len < RADIUS) {
        a = mod(a + 2 * PI - rotate_speed, 2 * PI);
        float f;
        f = cos(a * 8.);
        float small_circle = RADIUS * .93 * theta_noise;
        f = smoothstep(.9825, 1., f);
        color = (1. - smoothstep(f, f+0.02, len)) * rgb(108, 78, 54) * .8 + rgb(116, 133, 102) * smoothstep(f, f+0.02, len);
        if (len < RADIUS * .89 * theta_noise) {
		    color *= smoothstep(0., 1., (len - RADIUS * .5 * theta_noise) / (RADIUS * .03 * theta_noise))
            * smoothstep(0., 1., mod(abs(a - PI), PI) / PI);
		}
        if (small_circle - .005 < len && len < small_circle + .005) {
			float circle_f = cos(a * 56.);
            circle_f = smoothstep(0., 1., circle_f);
            circle_f = smoothstep(circle_f, circle_f+0.02, len);
            if (circle_f == 0.) {
                color = vec3(1.);
            }
        }
    }
    
    
    gl_FragColor = vec4(color * noise, 1.0);
}