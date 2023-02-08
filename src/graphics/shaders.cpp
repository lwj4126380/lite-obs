#include "shaders.h"

std::string conversion_shaders = R"(
Planar_Y
---------------------------------------

const bool obs_glsl_compile = true;

struct FragPos {
    vec4 pos;
};

FragPos VSPos(uint id)
{
    float idHigh = float(id >> 1);
    float idLow = float(id & uint(1));

    float x = idHigh * 4.0 - 1.0;
    float y = idLow * 4.0 - 1.0;

    FragPos vert_out;
    vert_out.pos = vec4(x, y, 0.0, 1.0);
    return vert_out;
}

FragPos _main_wrap(uint id)
{
    return VSPos(id);
}

void main(void)
{
    uint id;
    FragPos outputval;

    id = uint(gl_VertexID);

    outputval = _main_wrap(id);

    gl_Position = outputval.pos;
}

---------------------------------------
---------------------------------------
---------------------------------------
=======================================
Planar_Y
---------------------------------------

const bool obs_glsl_compile = true;

uniform sampler2D image;
uniform vec4 color_vec0;


out float _pixel_shader_attrib0;

struct FragPos {
    vec4 pos;
};

float PS_Y(FragPos frag_in)
{
    vec3 rgb = texelFetch(image, (ivec3(frag_in.pos.xy, 0)).xy, 0).rgb;
    float y = dot(color_vec0.xyz, rgb) + color_vec0.w;
    return y;
}

float _main_wrap(FragPos frag_in)
{
    return PS_Y(frag_in);
}

void main(void)
{
    FragPos frag_in;
    frag_in.pos = gl_FragCoord;

    _pixel_shader_attrib0 = _main_wrap(frag_in);
}

---------------------------------------
texture2d image null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec0 null 3 0 18446744073709551615
---------------------------------------
---------------------------------------
=======================================
Planar_U
---------------------------------------

const bool obs_glsl_compile = true;

struct FragPos {
    vec4 pos;
};

FragPos VSPos(uint id)
{
    float idHigh = float(id >> 1);
    float idLow = float(id & uint(1));

    float x = idHigh * 4.0 - 1.0;
    float y = idLow * 4.0 - 1.0;

    FragPos vert_out;
    vert_out.pos = vec4(x, y, 0.0, 1.0);
    return vert_out;
}

FragPos _main_wrap(uint id)
{
    return VSPos(id);
}

void main(void)
{
    uint id;
    FragPos outputval;

    id = uint(gl_VertexID);

    outputval = _main_wrap(id);

    gl_Position = outputval.pos;
}

---------------------------------------
---------------------------------------
---------------------------------------
=======================================
Planar_U
---------------------------------------

const bool obs_glsl_compile = true;

uniform sampler2D image;
uniform vec4 color_vec1;


out float _pixel_shader_attrib0;

struct FragPos {
    vec4 pos;
};

float PS_U(FragPos frag_in)
{
    vec3 rgb = texelFetch(image, (ivec3(frag_in.pos.xy, 0)).xy, 0).rgb;
    float u = dot(color_vec1.xyz, rgb) + color_vec1.w;
    return u;
}

float _main_wrap(FragPos frag_in)
{
    return PS_U(frag_in);
}

void main(void)
{
    FragPos frag_in;
    frag_in.pos = gl_FragCoord;

    _pixel_shader_attrib0 = _main_wrap(frag_in);
}

---------------------------------------
texture2d image null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec1 null 3 0 18446744073709551615
---------------------------------------
---------------------------------------
=======================================
Planar_V
---------------------------------------

const bool obs_glsl_compile = true;

struct FragPos {
    vec4 pos;
};

FragPos VSPos(uint id)
{
    float idHigh = float(id >> 1);
    float idLow = float(id & uint(1));

    float x = idHigh * 4.0 - 1.0;
    float y = idLow * 4.0 - 1.0;

    FragPos vert_out;
    vert_out.pos = vec4(x, y, 0.0, 1.0);
    return vert_out;
}

FragPos _main_wrap(uint id)
{
    return VSPos(id);
}

void main(void)
{
    uint id;
    FragPos outputval;

    id = uint(gl_VertexID);

    outputval = _main_wrap(id);

    gl_Position = outputval.pos;
}

---------------------------------------
---------------------------------------
---------------------------------------
=======================================
Planar_V
---------------------------------------

const bool obs_glsl_compile = true;

uniform sampler2D image;
uniform vec4 color_vec2;


out float _pixel_shader_attrib0;

struct FragPos {
    vec4 pos;
};

float PS_V(FragPos frag_in)
{
    vec3 rgb = texelFetch(image, (ivec3(frag_in.pos.xy, 0)).xy, 0).rgb;
    float v = dot(color_vec2.xyz, rgb) + color_vec2.w;
    return v;
}

float _main_wrap(FragPos frag_in)
{
    return PS_V(frag_in);
}

void main(void)
{
    FragPos frag_in;
    frag_in.pos = gl_FragCoord;

    _pixel_shader_attrib0 = _main_wrap(frag_in);
}

---------------------------------------
texture2d image null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec2 null 3 0 18446744073709551615
---------------------------------------
---------------------------------------
=======================================
Planar_U_Left
---------------------------------------

const bool obs_glsl_compile = true;

uniform float width_i;


out vec3 _vertex_shader_attrib0;

struct VertTexPosWide {
    vec3 uuv;
    vec4 pos;
};

VertTexPosWide VSTexPos_Left(uint id)
{
    float idHigh = float(id >> 1);
    float idLow = float(id & uint(1));

    float x = idHigh * 4.0 - 1.0;
    float y = idLow * 4.0 - 1.0;

    float u_right = idHigh * 2.0;
    float u_left = u_right - width_i;
    float v = obs_glsl_compile ? (idLow * 2.0) : (1.0 - idLow * 2.0);

    VertTexPosWide vert_out;
    vert_out.uuv = vec3(u_left, u_right, v);
    vert_out.pos = vec4(x, y, 0.0, 1.0);
    return vert_out;
}

VertTexPosWide _main_wrap(uint id)
{
    return VSTexPos_Left(id);
}

void main(void)
{
    uint id;
    VertTexPosWide outputval;

    id = uint(gl_VertexID);

    outputval = _main_wrap(id);

    _vertex_shader_attrib0 = outputval.uuv;
    gl_Position = outputval.pos;
}

---------------------------------------
float width_i null 3 0 18446744073709551615
---------------------------------------
_vertex_shader_attrib0 TEXCOORD0 0
---------------------------------------
=======================================
Planar_U_Left
---------------------------------------

const bool obs_glsl_compile = true;

uniform sampler2D image;
uniform vec4 color_vec1;

in vec3 _vertex_shader_attrib0;

out float _pixel_shader_attrib0;

struct FragTexWide {
    vec3 uuv;
};

float PS_U_Wide(FragTexWide frag_in)
{
    vec3 rgb_left = texture(image, frag_in.uuv.xz).rgb;
    vec3 rgb_right = texture(image, frag_in.uuv.yz).rgb;
    vec3 rgb = (rgb_left + rgb_right) * 0.5;
    float u = dot(color_vec1.xyz, rgb) + color_vec1.w;
    return u;
}

float _main_wrap(FragTexWide frag_in)
{
    return PS_U_Wide(frag_in);
}

void main(void)
{
    FragTexWide frag_in;
    frag_in.uuv = _vertex_shader_attrib0;

    _pixel_shader_attrib0 = _main_wrap(frag_in);
}

---------------------------------------
texture2d image null 3 0 0
+++++++++++++++++++++++++++++++++++++++
float4 color_vec1 null 3 0 18446744073709551615
---------------------------------------
---------------------------------------
def_sampler
=======================================
Planar_V_Left
---------------------------------------

const bool obs_glsl_compile = true;

uniform float width_i;


out vec3 _vertex_shader_attrib0;

struct VertTexPosWide {
    vec3 uuv;
    vec4 pos;
};

VertTexPosWide VSTexPos_Left(uint id)
{
    float idHigh = float(id >> 1);
    float idLow = float(id & uint(1));

    float x = idHigh * 4.0 - 1.0;
    float y = idLow * 4.0 - 1.0;

    float u_right = idHigh * 2.0;
    float u_left = u_right - width_i;
    float v = obs_glsl_compile ? (idLow * 2.0) : (1.0 - idLow * 2.0);

    VertTexPosWide vert_out;
    vert_out.uuv = vec3(u_left, u_right, v);
    vert_out.pos = vec4(x, y, 0.0, 1.0);
    return vert_out;
}

VertTexPosWide _main_wrap(uint id)
{
    return VSTexPos_Left(id);
}

void main(void)
{
    uint id;
    VertTexPosWide outputval;

    id = uint(gl_VertexID);

    outputval = _main_wrap(id);

    _vertex_shader_attrib0 = outputval.uuv;
    gl_Position = outputval.pos;
}

---------------------------------------
float width_i null 3 0 18446744073709551615
---------------------------------------
_vertex_shader_attrib0 TEXCOORD0 0
---------------------------------------
=======================================
Planar_V_Left
---------------------------------------

const bool obs_glsl_compile = true;

uniform sampler2D image;
uniform vec4 color_vec2;

in vec3 _vertex_shader_attrib0;

out float _pixel_shader_attrib0;

struct FragTexWide {
    vec3 uuv;
};

float PS_V_Wide(FragTexWide frag_in)
{
    vec3 rgb_left = texture(image, frag_in.uuv.xz).rgb;
    vec3 rgb_right = texture(image, frag_in.uuv.yz).rgb;
    vec3 rgb = (rgb_left + rgb_right) * 0.5;
    float v = dot(color_vec2.xyz, rgb) + color_vec2.w;
    return v;
}

float _main_wrap(FragTexWide frag_in)
{
    return PS_V_Wide(frag_in);
}

void main(void)
{
    FragTexWide frag_in;
    frag_in.uuv = _vertex_shader_attrib0;

    _pixel_shader_attrib0 = _main_wrap(frag_in);
}

---------------------------------------
texture2d image null 3 0 0
+++++++++++++++++++++++++++++++++++++++
float4 color_vec2 null 3 0 18446744073709551615
---------------------------------------
---------------------------------------
def_sampler
=======================================
NV12_Y
---------------------------------------

const bool obs_glsl_compile = true;

struct FragPos {
    vec4 pos;
};

FragPos VSPos(uint id)
{
    float idHigh = float(id >> 1);
    float idLow = float(id & uint(1));

    float x = idHigh * 4.0 - 1.0;
    float y = idLow * 4.0 - 1.0;

    FragPos vert_out;
    vert_out.pos = vec4(x, y, 0.0, 1.0);
    return vert_out;
}

FragPos _main_wrap(uint id)
{
    return VSPos(id);
}

void main(void)
{
    uint id;
    FragPos outputval;

    id = uint(gl_VertexID);

    outputval = _main_wrap(id);

    gl_Position = outputval.pos;
}

---------------------------------------
---------------------------------------
---------------------------------------
=======================================
NV12_Y
---------------------------------------

const bool obs_glsl_compile = true;

uniform sampler2D image;
uniform vec4 color_vec0;


out float _pixel_shader_attrib0;

struct FragPos {
    vec4 pos;
};

float PS_Y(FragPos frag_in)
{
    vec3 rgb = texelFetch(image, (ivec3(frag_in.pos.xy, 0)).xy, 0).rgb;
    float y = dot(color_vec0.xyz, rgb) + color_vec0.w;
    return y;
}

float _main_wrap(FragPos frag_in)
{
    return PS_Y(frag_in);
}

void main(void)
{
    FragPos frag_in;
    frag_in.pos = gl_FragCoord;

    _pixel_shader_attrib0 = _main_wrap(frag_in);
}

---------------------------------------
texture2d image null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec0 null 3 0 18446744073709551615
---------------------------------------
---------------------------------------
=======================================
NV12_UV
---------------------------------------

const bool obs_glsl_compile = true;

uniform float width_i;


out vec3 _vertex_shader_attrib0;

struct VertTexPosWide {
    vec3 uuv;
    vec4 pos;
};

VertTexPosWide VSTexPos_Left(uint id)
{
    float idHigh = float(id >> 1);
    float idLow = float(id & uint(1));

    float x = idHigh * 4.0 - 1.0;
    float y = idLow * 4.0 - 1.0;

    float u_right = idHigh * 2.0;
    float u_left = u_right - width_i;
    float v = obs_glsl_compile ? (idLow * 2.0) : (1.0 - idLow * 2.0);

    VertTexPosWide vert_out;
    vert_out.uuv = vec3(u_left, u_right, v);
    vert_out.pos = vec4(x, y, 0.0, 1.0);
    return vert_out;
}

VertTexPosWide _main_wrap(uint id)
{
    return VSTexPos_Left(id);
}

void main(void)
{
    uint id;
    VertTexPosWide outputval;

    id = uint(gl_VertexID);

    outputval = _main_wrap(id);

    _vertex_shader_attrib0 = outputval.uuv;
    gl_Position = outputval.pos;
}

---------------------------------------
float width_i null 3 0 18446744073709551615
---------------------------------------
_vertex_shader_attrib0 TEXCOORD0 0
---------------------------------------
=======================================
NV12_UV
---------------------------------------

const bool obs_glsl_compile = true;

uniform sampler2D image;
uniform vec4 color_vec1;
uniform vec4 color_vec2;

in vec3 _vertex_shader_attrib0;

out vec2 _pixel_shader_attrib0;

struct FragTexWide {
    vec3 uuv;
};

vec2 PS_UV_Wide(FragTexWide frag_in)
{
    vec3 rgb_left = texture(image, frag_in.uuv.xz).rgb;
    vec3 rgb_right = texture(image, frag_in.uuv.yz).rgb;
    vec3 rgb = (rgb_left + rgb_right) * 0.5;
    float u = dot(color_vec1.xyz, rgb) + color_vec1.w;
    float v = dot(color_vec2.xyz, rgb) + color_vec2.w;
    return vec2(u, v);
}

vec2 _main_wrap(FragTexWide frag_in)
{
    return PS_UV_Wide(frag_in);
}

void main(void)
{
    FragTexWide frag_in;
    frag_in.uuv = _vertex_shader_attrib0;

    _pixel_shader_attrib0 = _main_wrap(frag_in);
}

---------------------------------------
texture2d image null 3 0 0
+++++++++++++++++++++++++++++++++++++++
float4 color_vec1 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec2 null 3 0 18446744073709551615
---------------------------------------
---------------------------------------
def_sampler
)";

std::string conversion_shaders2 = R"(
UYVY_Reverse
---------------------------------------

const bool obs_glsl_compile = true;

uniform float width_d2;
uniform float height;


out vec2 _vertex_shader_attrib0;

struct VertTexPos {
    vec2 uv;
    vec4 pos;
};

VertTexPos VSTexPosHalf_Reverse(uint id)
{
    float idHigh = float(id >> 1);
    float idLow = float(id & uint(1));

    float x = idHigh * 4.0 - 1.0;
    float y = idLow * 4.0 - 1.0;

    float u = idHigh * 2.0;
    float v = obs_glsl_compile ? (idLow * 2.0) : (1.0 - idLow * 2.0);

    VertTexPos vert_out;
    vert_out.uv = vec2(width_d2 * u, height * v);
    vert_out.pos = vec4(x, y, 0.0, 1.0);
    return vert_out;
}

VertTexPos _main_wrap(uint id)
{
    return VSTexPosHalf_Reverse(id);
}

void main(void)
{
    uint id;
    VertTexPos outputval;

    id = uint(gl_VertexID);

    outputval = _main_wrap(id);

    _vertex_shader_attrib0 = outputval.uv;
    gl_Position = outputval.pos;
}

---------------------------------------
float width_d2 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float height null 3 0 18446744073709551615
---------------------------------------
_vertex_shader_attrib0 TEXCOORD0 0
---------------------------------------
=======================================
UYVY_Reverse
---------------------------------------

const bool obs_glsl_compile = true;

uniform sampler2D image;
uniform vec3 color_range_min;
uniform vec3 color_range_max;
uniform vec4 color_vec0;
uniform vec4 color_vec1;
uniform vec4 color_vec2;

in vec2 _vertex_shader_attrib0;

out vec3 _pixel_shader_attrib0;

struct FragTex {
    vec2 uv;
};

vec3 YUV_to_RGB(vec3 yuv)
{
    yuv = clamp(yuv, color_range_min, color_range_max);
    float r = dot(color_vec0.xyz, yuv) + color_vec0.w;
    float g = dot(color_vec1.xyz, yuv) + color_vec1.w;
    float b = dot(color_vec2.xyz, yuv) + color_vec2.w;
    return vec3(r, g, b);
}

vec3 PSUYVY_Reverse(FragTex frag_in)
{
    vec4 y2uv = texelFetch(image, (ivec3(frag_in.uv.xy, 0)).xy, 0);
    vec2 y01 = y2uv.yw;
    vec2 cbcr = y2uv.zx;
    float leftover = fract(frag_in.uv.x);
    float y = (leftover < 0.5) ? y01.x : y01.y;
    vec3 yuv = vec3(y, cbcr);
    vec3 rgb = YUV_to_RGB(yuv);
    return rgb;
}

vec3 _main_wrap(FragTex frag_in)
{
    return PSUYVY_Reverse(frag_in);
}

void main(void)
{
    FragTex frag_in;
    frag_in.uv = _vertex_shader_attrib0;

    _pixel_shader_attrib0 = _main_wrap(frag_in);
}

---------------------------------------
texture2d image null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float3 color_range_min null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float3 color_range_max null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec0 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec1 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec2 null 3 0 18446744073709551615
---------------------------------------
---------------------------------------
=======================================
YUY2_Reverse
---------------------------------------

const bool obs_glsl_compile = true;

uniform float width_d2;
uniform float height;


out vec2 _vertex_shader_attrib0;

struct VertTexPos {
    vec2 uv;
    vec4 pos;
};

VertTexPos VSTexPosHalf_Reverse(uint id)
{
    float idHigh = float(id >> 1);
    float idLow = float(id & uint(1));

    float x = idHigh * 4.0 - 1.0;
    float y = idLow * 4.0 - 1.0;

    float u = idHigh * 2.0;
    float v = obs_glsl_compile ? (idLow * 2.0) : (1.0 - idLow * 2.0);

    VertTexPos vert_out;
    vert_out.uv = vec2(width_d2 * u, height * v);
    vert_out.pos = vec4(x, y, 0.0, 1.0);
    return vert_out;
}

VertTexPos _main_wrap(uint id)
{
    return VSTexPosHalf_Reverse(id);
}

void main(void)
{
    uint id;
    VertTexPos outputval;

    id = uint(gl_VertexID);

    outputval = _main_wrap(id);

    _vertex_shader_attrib0 = outputval.uv;
    gl_Position = outputval.pos;
}

---------------------------------------
float width_d2 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float height null 3 0 18446744073709551615
---------------------------------------
_vertex_shader_attrib0 TEXCOORD0 0
---------------------------------------
=======================================
YUY2_Reverse
---------------------------------------

const bool obs_glsl_compile = true;

uniform sampler2D image;
uniform vec3 color_range_min;
uniform vec3 color_range_max;
uniform vec4 color_vec0;
uniform vec4 color_vec1;
uniform vec4 color_vec2;

in vec2 _vertex_shader_attrib0;

out vec3 _pixel_shader_attrib0;

struct FragTex {
    vec2 uv;
};

vec3 YUV_to_RGB(vec3 yuv)
{
    yuv = clamp(yuv, color_range_min, color_range_max);
    float r = dot(color_vec0.xyz, yuv) + color_vec0.w;
    float g = dot(color_vec1.xyz, yuv) + color_vec1.w;
    float b = dot(color_vec2.xyz, yuv) + color_vec2.w;
    return vec3(r, g, b);
}

vec3 PSYUY2_Reverse(FragTex frag_in)
{
    vec4 y2uv = texelFetch(image, (ivec3(frag_in.uv.xy, 0)).xy, 0);
    vec2 y01 = y2uv.zx;
    vec2 cbcr = y2uv.yw;
    float leftover = fract(frag_in.uv.x);
    float y = (leftover < 0.5) ? y01.x : y01.y;
    vec3 yuv = vec3(y, cbcr);
    vec3 rgb = YUV_to_RGB(yuv);
    return rgb;
}

vec3 _main_wrap(FragTex frag_in)
{
    return PSYUY2_Reverse(frag_in);
}

void main(void)
{
    FragTex frag_in;
    frag_in.uv = _vertex_shader_attrib0;

    _pixel_shader_attrib0 = _main_wrap(frag_in);
}

---------------------------------------
texture2d image null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float3 color_range_min null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float3 color_range_max null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec0 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec1 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec2 null 3 0 18446744073709551615
---------------------------------------
---------------------------------------
=======================================
YVYU_Reverse
---------------------------------------

const bool obs_glsl_compile = true;

uniform float width_d2;
uniform float height;


out vec2 _vertex_shader_attrib0;

struct VertTexPos {
    vec2 uv;
    vec4 pos;
};

VertTexPos VSTexPosHalf_Reverse(uint id)
{
    float idHigh = float(id >> 1);
    float idLow = float(id & uint(1));

    float x = idHigh * 4.0 - 1.0;
    float y = idLow * 4.0 - 1.0;

    float u = idHigh * 2.0;
    float v = obs_glsl_compile ? (idLow * 2.0) : (1.0 - idLow * 2.0);

    VertTexPos vert_out;
    vert_out.uv = vec2(width_d2 * u, height * v);
    vert_out.pos = vec4(x, y, 0.0, 1.0);
    return vert_out;
}

VertTexPos _main_wrap(uint id)
{
    return VSTexPosHalf_Reverse(id);
}

void main(void)
{
    uint id;
    VertTexPos outputval;

    id = uint(gl_VertexID);

    outputval = _main_wrap(id);

    _vertex_shader_attrib0 = outputval.uv;
    gl_Position = outputval.pos;
}

---------------------------------------
float width_d2 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float height null 3 0 18446744073709551615
---------------------------------------
_vertex_shader_attrib0 TEXCOORD0 0
---------------------------------------
=======================================
YVYU_Reverse
---------------------------------------

const bool obs_glsl_compile = true;

uniform sampler2D image;
uniform vec3 color_range_min;
uniform vec3 color_range_max;
uniform vec4 color_vec0;
uniform vec4 color_vec1;
uniform vec4 color_vec2;

in vec2 _vertex_shader_attrib0;

out vec3 _pixel_shader_attrib0;

struct FragTex {
    vec2 uv;
};

vec3 YUV_to_RGB(vec3 yuv)
{
    yuv = clamp(yuv, color_range_min, color_range_max);
    float r = dot(color_vec0.xyz, yuv) + color_vec0.w;
    float g = dot(color_vec1.xyz, yuv) + color_vec1.w;
    float b = dot(color_vec2.xyz, yuv) + color_vec2.w;
    return vec3(r, g, b);
}

vec3 PSYVYU_Reverse(FragTex frag_in)
{
    vec4 y2uv = texelFetch(image, (ivec3(frag_in.uv.xy, 0)).xy, 0);
    vec2 y01 = y2uv.zx;
    vec2 cbcr = y2uv.wy;
    float leftover = fract(frag_in.uv.x);
    float y = (leftover < 0.5) ? y01.x : y01.y;
    vec3 yuv = vec3(y, cbcr);
    vec3 rgb = YUV_to_RGB(yuv);
    return rgb;
}

vec3 _main_wrap(FragTex frag_in)
{
    return PSYVYU_Reverse(frag_in);
}

void main(void)
{
    FragTex frag_in;
    frag_in.uv = _vertex_shader_attrib0;

    _pixel_shader_attrib0 = _main_wrap(frag_in);
}

---------------------------------------
texture2d image null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float3 color_range_min null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float3 color_range_max null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec0 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec1 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec2 null 3 0 18446744073709551615
---------------------------------------
---------------------------------------
=======================================
I420_Reverse
---------------------------------------

const bool obs_glsl_compile = true;

uniform float width_d2;
uniform float height_d2;


out vec2 _vertex_shader_attrib0;

struct VertTexPos {
    vec2 uv;
    vec4 pos;
};

VertTexPos VSTexPosHalfHalf_Reverse(uint id)
{
    float idHigh = float(id >> 1);
    float idLow = float(id & uint(1));

    float x = idHigh * 4.0 - 1.0;
    float y = idLow * 4.0 - 1.0;

    float u = idHigh * 2.0;
    float v = obs_glsl_compile ? (idLow * 2.0) : (1.0 - idLow * 2.0);

    VertTexPos vert_out;
    vert_out.uv = vec2(width_d2 * u, height_d2 * v);
    vert_out.pos = vec4(x, y, 0.0, 1.0);
    return vert_out;
}

VertTexPos _main_wrap(uint id)
{
    return VSTexPosHalfHalf_Reverse(id);
}

void main(void)
{
    uint id;
    VertTexPos outputval;

    id = uint(gl_VertexID);

    outputval = _main_wrap(id);

    _vertex_shader_attrib0 = outputval.uv;
    gl_Position = outputval.pos;
}

---------------------------------------
float width_d2 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float height_d2 null 3 0 18446744073709551615
---------------------------------------
_vertex_shader_attrib0 TEXCOORD0 0
---------------------------------------
=======================================
I420_Reverse
---------------------------------------

const bool obs_glsl_compile = true;

uniform sampler2D image;
uniform sampler2D image1;
uniform sampler2D image2;
uniform vec3 color_range_min;
uniform vec3 color_range_max;
uniform vec4 color_vec0;
uniform vec4 color_vec1;
uniform vec4 color_vec2;

in vec2 _vertex_shader_attrib0;

out vec3 _pixel_shader_attrib0;

struct VertTexPos {
    vec2 uv;
    vec4 pos;
};

vec3 YUV_to_RGB(vec3 yuv)
{
    yuv = clamp(yuv, color_range_min, color_range_max);
    float r = dot(color_vec0.xyz, yuv) + color_vec0.w;
    float g = dot(color_vec1.xyz, yuv) + color_vec1.w;
    float b = dot(color_vec2.xyz, yuv) + color_vec2.w;
    return vec3(r, g, b);
}

vec3 PSPlanar420_Reverse(VertTexPos frag_in)
{
    float y = texelFetch(image, (ivec3(frag_in.pos.xy, 0)).xy, 0).x;
    ivec3 xy0_chroma = ivec3(frag_in.uv, 0);
    float cb = texelFetch(image1, (xy0_chroma).xy, 0).x;
    float cr = texelFetch(image2, (xy0_chroma).xy, 0).x;
    vec3 yuv = vec3(y, cb, cr);
    vec3 rgb = YUV_to_RGB(yuv);
    return rgb;
}

vec3 _main_wrap(VertTexPos frag_in)
{
    return PSPlanar420_Reverse(frag_in);
}

void main(void)
{
    VertTexPos frag_in;
    frag_in.uv = _vertex_shader_attrib0;
    frag_in.pos = gl_FragCoord;

    _pixel_shader_attrib0 = _main_wrap(frag_in);
}

---------------------------------------
texture2d image null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
texture2d image1 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
texture2d image2 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float3 color_range_min null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float3 color_range_max null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec0 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec1 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec2 null 3 0 18446744073709551615
---------------------------------------
---------------------------------------
)";

std::string conversion_shaders3 = R"(
I40A_Reverse
---------------------------------------

const bool obs_glsl_compile = true;

uniform float width_d2;
uniform float height_d2;


out vec2 _vertex_shader_attrib0;

struct VertTexPos {
    vec2 uv;
    vec4 pos;
};

VertTexPos VSTexPosHalfHalf_Reverse(uint id)
{
    float idHigh = float(id >> 1);
    float idLow = float(id & uint(1));

    float x = idHigh * 4.0 - 1.0;
    float y = idLow * 4.0 - 1.0;

    float u = idHigh * 2.0;
    float v = obs_glsl_compile ? (idLow * 2.0) : (1.0 - idLow * 2.0);

    VertTexPos vert_out;
    vert_out.uv = vec2(width_d2 * u, height_d2 * v);
    vert_out.pos = vec4(x, y, 0.0, 1.0);
    return vert_out;
}

VertTexPos _main_wrap(uint id)
{
    return VSTexPosHalfHalf_Reverse(id);
}

void main(void)
{
    uint id;
    VertTexPos outputval;

    id = uint(gl_VertexID);

    outputval = _main_wrap(id);

    _vertex_shader_attrib0 = outputval.uv;
    gl_Position = outputval.pos;
}

---------------------------------------
float width_d2 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float height_d2 null 3 0 18446744073709551615
---------------------------------------
_vertex_shader_attrib0 TEXCOORD0 0
---------------------------------------
=======================================
I40A_Reverse
---------------------------------------

const bool obs_glsl_compile = true;

uniform sampler2D image;
uniform sampler2D image1;
uniform sampler2D image2;
uniform sampler2D image3;
uniform vec3 color_range_min;
uniform vec3 color_range_max;
uniform vec4 color_vec0;
uniform vec4 color_vec1;
uniform vec4 color_vec2;

in vec2 _vertex_shader_attrib0;

out vec4 _pixel_shader_attrib0;

struct VertTexPos {
    vec2 uv;
    vec4 pos;
};

vec3 YUV_to_RGB(vec3 yuv)
{
    yuv = clamp(yuv, color_range_min, color_range_max);
    float r = dot(color_vec0.xyz, yuv) + color_vec0.w;
    float g = dot(color_vec1.xyz, yuv) + color_vec1.w;
    float b = dot(color_vec2.xyz, yuv) + color_vec2.w;
    return vec3(r, g, b);
}

vec4 PSPlanar420A_Reverse(VertTexPos frag_in)
{
    ivec3 xy0_luma = ivec3(frag_in.pos.xy, 0);
    float y = texelFetch(image, (xy0_luma).xy, 0).x;
    ivec3 xy0_chroma = ivec3(frag_in.uv, 0);
    float cb = texelFetch(image1, (xy0_chroma).xy, 0).x;
    float cr = texelFetch(image2, (xy0_chroma).xy, 0).x;
    float alpha = texelFetch(image3, (xy0_luma).xy, 0).x;
    vec3 yuv = vec3(y, cb, cr);
    vec4 rgba = vec4(YUV_to_RGB(yuv), alpha);
    return rgba;
}

vec4 _main_wrap(VertTexPos frag_in)
{
    return PSPlanar420A_Reverse(frag_in);
}

void main(void)
{
    VertTexPos frag_in;
    frag_in.uv = _vertex_shader_attrib0;
    frag_in.pos = gl_FragCoord;

    _pixel_shader_attrib0 = _main_wrap(frag_in);
}

---------------------------------------
texture2d image null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
texture2d image1 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
texture2d image2 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
texture2d image3 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float3 color_range_min null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float3 color_range_max null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec0 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec1 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec2 null 3 0 18446744073709551615
---------------------------------------
---------------------------------------
=======================================
I422_Reverse
---------------------------------------

const bool obs_glsl_compile = true;

uniform float width;
uniform float width_d2;
uniform float height;


out vec3 _vertex_shader_attrib0;

struct VertPosWide {
    vec3 pos_wide;
    vec4 pos;
};

VertPosWide VSPosWide_Reverse(uint id)
{
    float idHigh = float(id >> 1);
    float idLow = float(id & uint(1));

    float x = idHigh * 4.0 - 1.0;
    float y = idLow * 4.0 - 1.0;

    float u = idHigh * 2.0;
    float v = obs_glsl_compile ? (idLow * 2.0) : (1.0 - idLow * 2.0);

    VertPosWide vert_out;
    vert_out.pos_wide = vec3(vec2(width, width_d2) * u, height * v);
    vert_out.pos = vec4(x, y, 0.0, 1.0);
    return vert_out;
}

VertPosWide _main_wrap(uint id)
{
    return VSPosWide_Reverse(id);
}

void main(void)
{
    uint id;
    VertPosWide outputval;

    id = uint(gl_VertexID);

    outputval = _main_wrap(id);

    _vertex_shader_attrib0 = outputval.pos_wide;
    gl_Position = outputval.pos;
}

---------------------------------------
float width null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float width_d2 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float height null 3 0 18446744073709551615
---------------------------------------
_vertex_shader_attrib0 TEXCOORD0 0
---------------------------------------
=======================================
I422_Reverse
---------------------------------------

const bool obs_glsl_compile = true;

uniform sampler2D image;
uniform sampler2D image1;
uniform sampler2D image2;
uniform vec3 color_range_min;
uniform vec3 color_range_max;
uniform vec4 color_vec0;
uniform vec4 color_vec1;
uniform vec4 color_vec2;

in vec3 _vertex_shader_attrib0;

out vec3 _pixel_shader_attrib0;

struct FragPosWide {
    vec3 pos_wide;
};

vec3 YUV_to_RGB(vec3 yuv)
{
    yuv = clamp(yuv, color_range_min, color_range_max);
    float r = dot(color_vec0.xyz, yuv) + color_vec0.w;
    float g = dot(color_vec1.xyz, yuv) + color_vec1.w;
    float b = dot(color_vec2.xyz, yuv) + color_vec2.w;
    return vec3(r, g, b);
}

vec3 PSPlanar422_Reverse(FragPosWide frag_in)
{
    float y = texelFetch(image, (ivec3(frag_in.pos_wide.xz, 0)).xy, 0).x;
    ivec3 xy0_chroma = ivec3(frag_in.pos_wide.yz, 0);
    float cb = texelFetch(image1, (xy0_chroma).xy, 0).x;
    float cr = texelFetch(image2, (xy0_chroma).xy, 0).x;
    vec3 yuv = vec3(y, cb, cr);
    vec3 rgb = YUV_to_RGB(yuv);
    return rgb;
}

vec3 _main_wrap(FragPosWide frag_in)
{
    return PSPlanar422_Reverse(frag_in);
}

void main(void)
{
    FragPosWide frag_in;
    frag_in.pos_wide = _vertex_shader_attrib0;

    _pixel_shader_attrib0 = _main_wrap(frag_in);
}

---------------------------------------
texture2d image null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
texture2d image1 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
texture2d image2 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float3 color_range_min null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float3 color_range_max null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec0 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec1 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec2 null 3 0 18446744073709551615
---------------------------------------
---------------------------------------
=======================================
I42A_Reverse
---------------------------------------

const bool obs_glsl_compile = true;

uniform float width;
uniform float width_d2;
uniform float height;


out vec3 _vertex_shader_attrib0;

struct VertPosWide {
    vec3 pos_wide;
    vec4 pos;
};

VertPosWide VSPosWide_Reverse(uint id)
{
    float idHigh = float(id >> 1);
    float idLow = float(id & uint(1));

    float x = idHigh * 4.0 - 1.0;
    float y = idLow * 4.0 - 1.0;

    float u = idHigh * 2.0;
    float v = obs_glsl_compile ? (idLow * 2.0) : (1.0 - idLow * 2.0);

    VertPosWide vert_out;
    vert_out.pos_wide = vec3(vec2(width, width_d2) * u, height * v);
    vert_out.pos = vec4(x, y, 0.0, 1.0);
    return vert_out;
}

VertPosWide _main_wrap(uint id)
{
    return VSPosWide_Reverse(id);
}

void main(void)
{
    uint id;
    VertPosWide outputval;

    id = uint(gl_VertexID);

    outputval = _main_wrap(id);

    _vertex_shader_attrib0 = outputval.pos_wide;
    gl_Position = outputval.pos;
}

---------------------------------------
float width null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float width_d2 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float height null 3 0 18446744073709551615
---------------------------------------
_vertex_shader_attrib0 TEXCOORD0 0
---------------------------------------
=======================================
I42A_Reverse
---------------------------------------

const bool obs_glsl_compile = true;

uniform sampler2D image;
uniform sampler2D image1;
uniform sampler2D image2;
uniform sampler2D image3;
uniform vec3 color_range_min;
uniform vec3 color_range_max;
uniform vec4 color_vec0;
uniform vec4 color_vec1;
uniform vec4 color_vec2;

in vec3 _vertex_shader_attrib0;

out vec4 _pixel_shader_attrib0;

struct FragPosWide {
    vec3 pos_wide;
};

vec3 YUV_to_RGB(vec3 yuv)
{
    yuv = clamp(yuv, color_range_min, color_range_max);
    float r = dot(color_vec0.xyz, yuv) + color_vec0.w;
    float g = dot(color_vec1.xyz, yuv) + color_vec1.w;
    float b = dot(color_vec2.xyz, yuv) + color_vec2.w;
    return vec3(r, g, b);
}

vec4 PSPlanar422A_Reverse(FragPosWide frag_in)
{
    ivec3 xy0_luma = ivec3(frag_in.pos_wide.xz, 0);
    float y = texelFetch(image, (xy0_luma).xy, 0).x;
    ivec3 xy0_chroma = ivec3(frag_in.pos_wide.yz, 0);
    float cb = texelFetch(image1, (xy0_chroma).xy, 0).x;
    float cr = texelFetch(image2, (xy0_chroma).xy, 0).x;
    float alpha = texelFetch(image3, (xy0_luma).xy, 0).x;
    vec3 yuv = vec3(y, cb, cr);
    vec4 rgba = vec4(YUV_to_RGB(yuv), alpha);
    return rgba;
}

vec4 _main_wrap(FragPosWide frag_in)
{
    return PSPlanar422A_Reverse(frag_in);
}

void main(void)
{
    FragPosWide frag_in;
    frag_in.pos_wide = _vertex_shader_attrib0;

    _pixel_shader_attrib0 = _main_wrap(frag_in);
}

---------------------------------------
texture2d image null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
texture2d image1 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
texture2d image2 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
texture2d image3 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float3 color_range_min null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float3 color_range_max null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec0 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec1 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec2 null 3 0 18446744073709551615
---------------------------------------
---------------------------------------
=======================================
I444_Reverse
---------------------------------------

const bool obs_glsl_compile = true;

struct FragPos {
    vec4 pos;
};

FragPos VSPos(uint id)
{
    float idHigh = float(id >> 1);
    float idLow = float(id & uint(1));

    float x = idHigh * 4.0 - 1.0;
    float y = idLow * 4.0 - 1.0;

    FragPos vert_out;
    vert_out.pos = vec4(x, y, 0.0, 1.0);
    return vert_out;
}

FragPos _main_wrap(uint id)
{
    return VSPos(id);
}

void main(void)
{
    uint id;
    FragPos outputval;

    id = uint(gl_VertexID);

    outputval = _main_wrap(id);

    gl_Position = outputval.pos;
}

---------------------------------------
---------------------------------------
---------------------------------------
=======================================
I444_Reverse
---------------------------------------

const bool obs_glsl_compile = true;

uniform sampler2D image;
uniform sampler2D image1;
uniform sampler2D image2;
uniform vec3 color_range_min;
uniform vec3 color_range_max;
uniform vec4 color_vec0;
uniform vec4 color_vec1;
uniform vec4 color_vec2;


out vec3 _pixel_shader_attrib0;

struct FragPos {
    vec4 pos;
};

vec3 YUV_to_RGB(vec3 yuv)
{
    yuv = clamp(yuv, color_range_min, color_range_max);
    float r = dot(color_vec0.xyz, yuv) + color_vec0.w;
    float g = dot(color_vec1.xyz, yuv) + color_vec1.w;
    float b = dot(color_vec2.xyz, yuv) + color_vec2.w;
    return vec3(r, g, b);
}

vec3 PSPlanar444_Reverse(FragPos frag_in)
{
    ivec3 xy0 = ivec3(frag_in.pos.xy, 0);
    float y = texelFetch(image, (xy0).xy, 0).x;
    float cb = texelFetch(image1, (xy0).xy, 0).x;
    float cr = texelFetch(image2, (xy0).xy, 0).x;
    vec3 yuv = vec3(y, cb, cr);
    vec3 rgb = YUV_to_RGB(yuv);
    return rgb;
}

vec3 _main_wrap(FragPos frag_in)
{
    return PSPlanar444_Reverse(frag_in);
}

void main(void)
{
    FragPos frag_in;
    frag_in.pos = gl_FragCoord;

    _pixel_shader_attrib0 = _main_wrap(frag_in);
}

---------------------------------------
texture2d image null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
texture2d image1 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
texture2d image2 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float3 color_range_min null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float3 color_range_max null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec0 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec1 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec2 null 3 0 18446744073709551615
---------------------------------------
---------------------------------------
)";

std::string conversion_shaders4 = R"(
YUVA_Reverse
---------------------------------------

const bool obs_glsl_compile = true;

struct FragPos {
    vec4 pos;
};

FragPos VSPos(uint id)
{
    float idHigh = float(id >> 1);
    float idLow = float(id & uint(1));

    float x = idHigh * 4.0 - 1.0;
    float y = idLow * 4.0 - 1.0;

    FragPos vert_out;
    vert_out.pos = vec4(x, y, 0.0, 1.0);
    return vert_out;
}

FragPos _main_wrap(uint id)
{
    return VSPos(id);
}

void main(void)
{
    uint id;
    FragPos outputval;

    id = uint(gl_VertexID);

    outputval = _main_wrap(id);

    gl_Position = outputval.pos;
}

---------------------------------------
---------------------------------------
---------------------------------------
=======================================
YUVA_Reverse
---------------------------------------

const bool obs_glsl_compile = true;

uniform sampler2D image;
uniform sampler2D image1;
uniform sampler2D image2;
uniform sampler2D image3;
uniform vec3 color_range_min;
uniform vec3 color_range_max;
uniform vec4 color_vec0;
uniform vec4 color_vec1;
uniform vec4 color_vec2;


out vec4 _pixel_shader_attrib0;

struct FragPos {
    vec4 pos;
};

vec3 YUV_to_RGB(vec3 yuv)
{
    yuv = clamp(yuv, color_range_min, color_range_max);
    float r = dot(color_vec0.xyz, yuv) + color_vec0.w;
    float g = dot(color_vec1.xyz, yuv) + color_vec1.w;
    float b = dot(color_vec2.xyz, yuv) + color_vec2.w;
    return vec3(r, g, b);
}

vec4 PSPlanar444A_Reverse(FragPos frag_in)
{
    ivec3 xy0 = ivec3(frag_in.pos.xy, 0);
    float y = texelFetch(image, (xy0).xy, 0).x;
    float cb = texelFetch(image1, (xy0).xy, 0).x;
    float cr = texelFetch(image2, (xy0).xy, 0).x;
    float alpha = texelFetch(image3, (xy0).xy, 0).x;
    vec3 yuv = vec3(y, cb, cr);
    vec4 rgba = vec4(YUV_to_RGB(yuv), alpha);
    return rgba;
}

vec4 _main_wrap(FragPos frag_in)
{
    return PSPlanar444A_Reverse(frag_in);
}

void main(void)
{
    FragPos frag_in;
    frag_in.pos = gl_FragCoord;

    _pixel_shader_attrib0 = _main_wrap(frag_in);
}

---------------------------------------
texture2d image null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
texture2d image1 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
texture2d image2 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
texture2d image3 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float3 color_range_min null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float3 color_range_max null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec0 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec1 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec2 null 3 0 18446744073709551615
---------------------------------------
---------------------------------------
=======================================
AYUV_Reverse
---------------------------------------

const bool obs_glsl_compile = true;

struct FragPos {
    vec4 pos;
};

FragPos VSPos(uint id)
{
    float idHigh = float(id >> 1);
    float idLow = float(id & uint(1));

    float x = idHigh * 4.0 - 1.0;
    float y = idLow * 4.0 - 1.0;

    FragPos vert_out;
    vert_out.pos = vec4(x, y, 0.0, 1.0);
    return vert_out;
}

FragPos _main_wrap(uint id)
{
    return VSPos(id);
}

void main(void)
{
    uint id;
    FragPos outputval;

    id = uint(gl_VertexID);

    outputval = _main_wrap(id);

    gl_Position = outputval.pos;
}

---------------------------------------
---------------------------------------
---------------------------------------
=======================================
AYUV_Reverse
---------------------------------------

const bool obs_glsl_compile = true;

uniform sampler2D image;
uniform vec3 color_range_min;
uniform vec3 color_range_max;
uniform vec4 color_vec0;
uniform vec4 color_vec1;
uniform vec4 color_vec2;


out vec4 _pixel_shader_attrib0;

struct FragPos {
    vec4 pos;
};

vec3 YUV_to_RGB(vec3 yuv)
{
    yuv = clamp(yuv, color_range_min, color_range_max);
    float r = dot(color_vec0.xyz, yuv) + color_vec0.w;
    float g = dot(color_vec1.xyz, yuv) + color_vec1.w;
    float b = dot(color_vec2.xyz, yuv) + color_vec2.w;
    return vec3(r, g, b);
}

vec4 PSAYUV_Reverse(FragPos frag_in)
{
    vec4 yuva = texelFetch(image, (ivec3(frag_in.pos.xy, 0)).xy, 0);
    vec4 rgba = vec4(YUV_to_RGB(yuva.xyz), yuva.a);
    return rgba;
}

vec4 _main_wrap(FragPos frag_in)
{
    return PSAYUV_Reverse(frag_in);
}

void main(void)
{
    FragPos frag_in;
    frag_in.pos = gl_FragCoord;

    _pixel_shader_attrib0 = _main_wrap(frag_in);
}

---------------------------------------
texture2d image null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float3 color_range_min null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float3 color_range_max null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec0 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec1 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec2 null 3 0 18446744073709551615
---------------------------------------
---------------------------------------
=======================================
NV12_Reverse
---------------------------------------

const bool obs_glsl_compile = true;

uniform float width_d2;
uniform float height_d2;


out vec2 _vertex_shader_attrib0;

struct VertTexPos {
    vec2 uv;
    vec4 pos;
};

VertTexPos VSTexPosHalfHalf_Reverse(uint id)
{
    float idHigh = float(id >> 1);
    float idLow = float(id & uint(1));

    float x = idHigh * 4.0 - 1.0;
    float y = idLow * 4.0 - 1.0;

    float u = idHigh * 2.0;
    float v = obs_glsl_compile ? (idLow * 2.0) : (1.0 - idLow * 2.0);

    VertTexPos vert_out;
    vert_out.uv = vec2(width_d2 * u, height_d2 * v);
    vert_out.pos = vec4(x, y, 0.0, 1.0);
    return vert_out;
}

VertTexPos _main_wrap(uint id)
{
    return VSTexPosHalfHalf_Reverse(id);
}

void main(void)
{
    uint id;
    VertTexPos outputval;

    id = uint(gl_VertexID);

    outputval = _main_wrap(id);

    _vertex_shader_attrib0 = outputval.uv;
    gl_Position = outputval.pos;
}

---------------------------------------
float width_d2 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float height_d2 null 3 0 18446744073709551615
---------------------------------------
_vertex_shader_attrib0 TEXCOORD0 0
---------------------------------------
=======================================
NV12_Reverse
---------------------------------------

const bool obs_glsl_compile = true;

uniform sampler2D image;
uniform sampler2D image1;
uniform vec3 color_range_min;
uniform vec3 color_range_max;
uniform vec4 color_vec0;
uniform vec4 color_vec1;
uniform vec4 color_vec2;

in vec2 _vertex_shader_attrib0;

out vec3 _pixel_shader_attrib0;

struct VertTexPos {
    vec2 uv;
    vec4 pos;
};

vec3 YUV_to_RGB(vec3 yuv)
{
    yuv = clamp(yuv, color_range_min, color_range_max);
    float r = dot(color_vec0.xyz, yuv) + color_vec0.w;
    float g = dot(color_vec1.xyz, yuv) + color_vec1.w;
    float b = dot(color_vec2.xyz, yuv) + color_vec2.w;
    return vec3(r, g, b);
}

vec3 PSNV12_Reverse(VertTexPos frag_in)
{
    float y = texelFetch(image, (ivec3(frag_in.pos.xy, 0)).xy, 0).x;
    vec2 cbcr = texelFetch(image1, (ivec3(frag_in.uv, 0)).xy, 0).xy;
    vec3 yuv = vec3(y, cbcr);
    vec3 rgb = YUV_to_RGB(yuv);
    return rgb;
}

vec3 _main_wrap(VertTexPos frag_in)
{
    return PSNV12_Reverse(frag_in);
}

void main(void)
{
    VertTexPos frag_in;
    frag_in.uv = _vertex_shader_attrib0;
    frag_in.pos = gl_FragCoord;

    _pixel_shader_attrib0 = _main_wrap(frag_in);
}

---------------------------------------
texture2d image null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
texture2d image1 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float3 color_range_min null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float3 color_range_max null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec0 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec1 null 3 0 18446744073709551615
+++++++++++++++++++++++++++++++++++++++
float4 color_vec2 null 3 0 18446744073709551615
---------------------------------------
---------------------------------------
=======================================
Y800_Limited
---------------------------------------

const bool obs_glsl_compile = true;

struct FragPos {
    vec4 pos;
};

FragPos VSPos(uint id)
{
    float idHigh = float(id >> 1);
    float idLow = float(id & uint(1));

    float x = idHigh * 4.0 - 1.0;
    float y = idLow * 4.0 - 1.0;

    FragPos vert_out;
    vert_out.pos = vec4(x, y, 0.0, 1.0);
    return vert_out;
}

FragPos _main_wrap(uint id)
{
    return VSPos(id);
}

void main(void)
{
    uint id;
    FragPos outputval;

    id = uint(gl_VertexID);

    outputval = _main_wrap(id);

    gl_Position = outputval.pos;
}

---------------------------------------
---------------------------------------
---------------------------------------
=======================================
Y800_Limited
---------------------------------------

const bool obs_glsl_compile = true;

uniform sampler2D image;


out vec3 _pixel_shader_attrib0;

struct FragPos {
    vec4 pos;
};

vec3 PSY800_Limited(FragPos frag_in)
{
    float limited = texelFetch(image, (ivec3(frag_in.pos.xy, 0)).xy, 0).x;
    float full = (255.0 / 219.0) * limited - (16.0 / 219.0);
    return vec3(full, full, full);
}

vec3 _main_wrap(FragPos frag_in)
{
    return PSY800_Limited(frag_in);
}

void main(void)
{
    FragPos frag_in;
    frag_in.pos = gl_FragCoord;

    _pixel_shader_attrib0 = _main_wrap(frag_in);
}

---------------------------------------
texture2d image null 3 0 18446744073709551615
---------------------------------------
---------------------------------------
=======================================
Y800_Full
---------------------------------------

const bool obs_glsl_compile = true;

struct FragPos {
    vec4 pos;
};

FragPos VSPos(uint id)
{
    float idHigh = float(id >> 1);
    float idLow = float(id & uint(1));

    float x = idHigh * 4.0 - 1.0;
    float y = idLow * 4.0 - 1.0;

    FragPos vert_out;
    vert_out.pos = vec4(x, y, 0.0, 1.0);
    return vert_out;
}

FragPos _main_wrap(uint id)
{
    return VSPos(id);
}

void main(void)
{
    uint id;
    FragPos outputval;

    id = uint(gl_VertexID);

    outputval = _main_wrap(id);

    gl_Position = outputval.pos;
}

---------------------------------------
---------------------------------------
---------------------------------------
=======================================
Y800_Full
---------------------------------------

const bool obs_glsl_compile = true;

uniform sampler2D image;


out vec3 _pixel_shader_attrib0;

struct FragPos {
    vec4 pos;
};

vec3 PSY800_Full(FragPos frag_in)
{
    vec3 full = texelFetch(image, (ivec3(frag_in.pos.xy, 0)).xy, 0).xxx;
    return full;
}

vec3 _main_wrap(FragPos frag_in)
{
    return PSY800_Full(frag_in);
}

void main(void)
{
    FragPos frag_in;
    frag_in.pos = gl_FragCoord;

    _pixel_shader_attrib0 = _main_wrap(frag_in);
}

---------------------------------------
texture2d image null 3 0 18446744073709551615
---------------------------------------
---------------------------------------
)";

std::string conversion_shaders5 = R"(
RGB_Limited
---------------------------------------

const bool obs_glsl_compile = true;

struct FragPos {
    vec4 pos;
};

FragPos VSPos(uint id)
{
    float idHigh = float(id >> 1);
    float idLow = float(id & uint(1));

    float x = idHigh * 4.0 - 1.0;
    float y = idLow * 4.0 - 1.0;

    FragPos vert_out;
    vert_out.pos = vec4(x, y, 0.0, 1.0);
    return vert_out;
}

FragPos _main_wrap(uint id)
{
    return VSPos(id);
}

void main(void)
{
    uint id;
    FragPos outputval;

    id = uint(gl_VertexID);

    outputval = _main_wrap(id);

    gl_Position = outputval.pos;
}

---------------------------------------
---------------------------------------
---------------------------------------
=======================================
RGB_Limited
---------------------------------------

const bool obs_glsl_compile = true;

uniform sampler2D image;


out vec4 _pixel_shader_attrib0;

struct FragPos {
    vec4 pos;
};

vec4 PSRGB_Limited(FragPos frag_in)
{
    vec4 rgba = texelFetch(image, (ivec3(frag_in.pos.xy, 0)).xy, 0);
    rgba.rgb = (255.0 / 219.0) * rgba.rgb - (16.0 / 219.0);
    return rgba;
}

vec4 _main_wrap(FragPos frag_in)
{
    return PSRGB_Limited(frag_in);
}

void main(void)
{
    FragPos frag_in;
    frag_in.pos = gl_FragCoord;

    _pixel_shader_attrib0 = _main_wrap(frag_in);
}

---------------------------------------
texture2d image null 3 0 18446744073709551615
---------------------------------------
---------------------------------------
=======================================
BGR3_Limited
---------------------------------------

const bool obs_glsl_compile = true;

struct FragPos {
    vec4 pos;
};

FragPos VSPos(uint id)
{
    float idHigh = float(id >> 1);
    float idLow = float(id & uint(1));

    float x = idHigh * 4.0 - 1.0;
    float y = idLow * 4.0 - 1.0;

    FragPos vert_out;
    vert_out.pos = vec4(x, y, 0.0, 1.0);
    return vert_out;
}

FragPos _main_wrap(uint id)
{
    return VSPos(id);
}

void main(void)
{
    uint id;
    FragPos outputval;

    id = uint(gl_VertexID);

    outputval = _main_wrap(id);

    gl_Position = outputval.pos;
}

---------------------------------------
---------------------------------------
---------------------------------------
=======================================
BGR3_Limited
---------------------------------------

const bool obs_glsl_compile = true;

uniform sampler2D image;


out vec3 _pixel_shader_attrib0;

struct FragPos {
    vec4 pos;
};

vec3 PSBGR3_Limited(FragPos frag_in)
{
    float x = frag_in.pos.x * 3.0;
    float y = frag_in.pos.y;
    float b = texelFetch(image, (ivec3(x - 1.0, y, 0)).xy, 0).x;
    float g = texelFetch(image, (ivec3(x, y, 0)).xy, 0).x;
    float r = texelFetch(image, (ivec3(x + 1.0, y, 0)).xy, 0).x;
    vec3 rgb = vec3(r, g, b);
    rgb = (255.0 / 219.0) * rgb - (16.0 / 219.0);
    return rgb;
}

vec3 _main_wrap(FragPos frag_in)
{
    return PSBGR3_Limited(frag_in);
}

void main(void)
{
    FragPos frag_in;
    frag_in.pos = gl_FragCoord;

    _pixel_shader_attrib0 = _main_wrap(frag_in);
}

---------------------------------------
texture2d image null 3 0 18446744073709551615
---------------------------------------
---------------------------------------
=======================================
BGR3_Full
---------------------------------------

const bool obs_glsl_compile = true;

struct FragPos {
    vec4 pos;
};

FragPos VSPos(uint id)
{
    float idHigh = float(id >> 1);
    float idLow = float(id & uint(1));

    float x = idHigh * 4.0 - 1.0;
    float y = idLow * 4.0 - 1.0;

    FragPos vert_out;
    vert_out.pos = vec4(x, y, 0.0, 1.0);
    return vert_out;
}

FragPos _main_wrap(uint id)
{
    return VSPos(id);
}

void main(void)
{
    uint id;
    FragPos outputval;

    id = uint(gl_VertexID);

    outputval = _main_wrap(id);

    gl_Position = outputval.pos;
}

---------------------------------------
---------------------------------------
---------------------------------------
=======================================
BGR3_Full
---------------------------------------

const bool obs_glsl_compile = true;

uniform sampler2D image;


out vec3 _pixel_shader_attrib0;

struct FragPos {
    vec4 pos;
};

vec3 PSBGR3_Full(FragPos frag_in)
{
    float x = frag_in.pos.x * 3.0;
    float y = frag_in.pos.y;
    float b = texelFetch(image, (ivec3(x - 1.0, y, 0)).xy, 0).x;
    float g = texelFetch(image, (ivec3(x, y, 0)).xy, 0).x;
    float r = texelFetch(image, (ivec3(x + 1.0, y, 0)).xy, 0).x;
    vec3 rgb = vec3(r, g, b);
    return rgb;
}

vec3 _main_wrap(FragPos frag_in)
{
    return PSBGR3_Full(frag_in);
}

void main(void)
{
    FragPos frag_in;
    frag_in.pos = gl_FragCoord;

    _pixel_shader_attrib0 = _main_wrap(frag_in);
}

---------------------------------------
texture2d image null 3 0 18446744073709551615
---------------------------------------
---------------------------------------
)";

std::string draw_shader = R"(
Draw
---------------------------------------
const bool obs_glsl_compile = true;

uniform mat4x4 ViewProj;

in vec4 _input_attrib0;
in vec2 _input_attrib1;

out vec2 _vertex_shader_attrib0;

struct VertInOut {
    vec4 pos;
    vec2 uv;
};

VertInOut VSDefault(VertInOut vert_in)
{
    VertInOut vert_out;
    vert_out.pos = ((vec4(vert_in.pos.xyz, 1.0)) * (ViewProj));
    vert_out.uv  = vert_in.uv;
    return vert_out;
}

VertInOut _main_wrap(VertInOut vert_in)
{
    return VSDefault(vert_in);
}

void main(void)
{
    VertInOut vert_in;
    VertInOut outputval;

    vert_in.pos = _input_attrib0;
    vert_in.uv = _input_attrib1;

    outputval = _main_wrap(vert_in);

    gl_Position = outputval.pos;
    _vertex_shader_attrib0 = outputval.uv;
}

---------------------------------------
float4x4 ViewProj null 3 0 18446744073709551615
---------------------------------------
_input_attrib0 POSITION 1
+++++++++++++++++++++++++++++++++++++++
_input_attrib1 TEXCOORD0 1
+++++++++++++++++++++++++++++++++++++++
_vertex_shader_attrib0 TEXCOORD0 0
---------------------------------------
=======================================
Draw
---------------------------------------
const bool obs_glsl_compile = true;

uniform sampler2D image;

in vec2 _vertex_shader_attrib0;

out vec4 _pixel_shader_attrib0;

struct VertInOut {
    vec4 pos;
    vec2 uv;
};

vec4 PSDrawBare(VertInOut vert_in)
{
    return texture(image, vert_in.uv);
}

vec4 _main_wrap(VertInOut vert_in)
{
    return PSDrawBare(vert_in);
}

void main(void)
{
    VertInOut vert_in;
    vert_in.pos = gl_FragCoord;
    vert_in.uv = _vertex_shader_attrib0;

    _pixel_shader_attrib0 = _main_wrap(vert_in);
}

---------------------------------------
texture2d image null 3 0 0
---------------------------------------
---------------------------------------
def_sampler
=======================================
DrawAlphaDivide
---------------------------------------
const bool obs_glsl_compile = true;

uniform mat4x4 ViewProj;

in vec4 _input_attrib0;
in vec2 _input_attrib1;

out vec2 _vertex_shader_attrib0;

struct VertInOut {
    vec4 pos;
    vec2 uv;
};

VertInOut VSDefault(VertInOut vert_in)
{
    VertInOut vert_out;
    vert_out.pos = ((vec4(vert_in.pos.xyz, 1.0)) * (ViewProj));
    vert_out.uv  = vert_in.uv;
    return vert_out;
}

VertInOut _main_wrap(VertInOut vert_in)
{
    return VSDefault(vert_in);
}

void main(void)
{
    VertInOut vert_in;
    VertInOut outputval;

    vert_in.pos = _input_attrib0;
    vert_in.uv = _input_attrib1;

    outputval = _main_wrap(vert_in);

    gl_Position = outputval.pos;
    _vertex_shader_attrib0 = outputval.uv;
}

---------------------------------------
float4x4 ViewProj null 3 0 18446744073709551615
---------------------------------------
_input_attrib0 POSITION 1
+++++++++++++++++++++++++++++++++++++++
_input_attrib1 TEXCOORD0 1
+++++++++++++++++++++++++++++++++++++++
_vertex_shader_attrib0 TEXCOORD0 0
---------------------------------------
=======================================
DrawAlphaDivide
---------------------------------------
const bool obs_glsl_compile = true;

uniform sampler2D image;

in vec2 _vertex_shader_attrib0;

out vec4 _pixel_shader_attrib0;

struct VertInOut {
    vec4 pos;
    vec2 uv;
};

vec4 PSDrawAlphaDivide(VertInOut vert_in)
{
    vec4 rgba = texture(image, vert_in.uv);
    float alpha = rgba.a;
    float multiplier = (alpha > 0.0) ? (1.0 / alpha) : 0.0;
    return vec4(rgba.rgb * multiplier, alpha);
}

vec4 _main_wrap(VertInOut vert_in)
{
    return PSDrawAlphaDivide(vert_in);
}

void main(void)
{
    VertInOut vert_in;
    vert_in.pos = gl_FragCoord;
    vert_in.uv = _vertex_shader_attrib0;

    _pixel_shader_attrib0 = _main_wrap(vert_in);
}

---------------------------------------
texture2d image null 3 0 0
---------------------------------------
---------------------------------------
def_sampler
)";

std::string conversion_shaders_total =
        conversion_shaders + "=======================================" +
        conversion_shaders2 + "=======================================" +
        conversion_shaders3 + "=======================================" +
        conversion_shaders4 + "=======================================" +
        conversion_shaders5 + "=======================================" +
        draw_shader;


static const char *astrblank = "";

int astrcmpi(const char *str1, const char *str2)
{
    if (!str1)
        str1 = astrblank;
    if (!str2)
        str2 = astrblank;

    do {
        char ch1 = (char)toupper(*str1);
        char ch2 = (char)toupper(*str2);

        if (ch1 < ch2)
            return -1;
        else if (ch1 > ch2)
            return 1;
    } while (*str1++ && *str2++);

    return 0;
}

