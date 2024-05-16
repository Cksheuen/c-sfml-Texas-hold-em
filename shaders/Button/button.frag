uniform vec2 global_pos;
uniform vec2 shape_size;
uniform float time;
uniform float disappear_time;
uniform int hover;
uniform float opacity;

uniform bool if_mid_texture;
uniform sampler2D mid_texture;
uniform sampler2D text_texture;

float random(in vec2 st) {
  return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

float noise(in vec2 st) {
  vec2 i = floor(st);
  vec2 f = fract(st);

  // Four corners in 2D of a tile
  float a = random(i);
  float b = random(i + vec2(1.0, 0.0));
  float c = random(i + vec2(0.0, 1.0));
  float d = random(i + vec2(1.0, 1.0));

  vec2 u = f * f * (3.0 - 2.0 * f);

  return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

#define OCTAVES 6
float fbm(in vec2 st) {
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

float pattern(in vec2 p, out vec2 q, out vec2 r) {
  q.x = fbm(p + vec2(0.0, 0.0));
  q.y = fbm(p + vec2(5.2, 1.3));

  r.x = fbm(p + 4.0 * q + vec2(1.7, 9.2));
  r.y = fbm(p + 4.0 * q + vec2(8.3, 2.8));

  return fbm(p + 4.0 * r);
}

vec3 rgb(int r, int g, int b) { return vec3(r / 255., g / 255., b / 255.); }

void main() {
  vec2 uv = (gl_FragCoord.xy - global_pos) / shape_size;

  float fx = abs(uv.x - .5);
  fx = 1. - smoothstep(0., .5, fx);
  float fy = abs(uv.y - .5);
  fy = 1. - smoothstep(0., .5, fy);

  vec3 col = texture2D(text_texture, uv + vec2(-0.2, 0.3)).xyz * 2.;

  if (hover == 1) {
    uv += (time) / 10.;
  }

  vec2 o, n;
  float pattern_ans = pattern(uv, o, n);

  col = mix(col, rgb(132, 126, 201), pattern_ans);
  col = mix(col, rgb(63, 78, 164), dot(n, n));
  // col = mix( col, rgb(158, 152, 222), 0.5*o.y*o.y );
  col = mix(col, rgb(92, 107, 189),
            0.5 * smoothstep(1.2, 1.3, abs(n.y) + abs(n.x)));
  col *= pattern_ans * 2.0;

  gl_FragColor = vec4(col, opacity) * (fx * fy);

  if (hover == 0)
    return;

  uv -= time / 10.;

  float f = fbm(uv * 5.) / 30.;
  float y_to_x = shape_size.x / shape_size.y;
  float edge_width = 0.04;
  float edge_to_border = 0.05;
  float cranny_width = edge_to_border - edge_width / 2.;
  float dx = min(abs(uv.x - edge_to_border - f),
                 abs(uv.x - (1. - edge_to_border - f)));
  float dy = min(abs(uv.y - (edge_to_border + f) * y_to_x),
                 abs(uv.y - (1. - (edge_to_border + f) * y_to_x)));
  float edge = smoothstep(0., edge_width / 2., min(dx, dy / y_to_x));

  if (edge <= .5)
    gl_FragColor = vec4(rgb(63, 78, 164) * .6, opacity) * (fx + fy);
}