#include "shaders.h"

const char *default_vertex_shader_str = R"(
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
)";

const char *default_pixel_shader_str = R"(
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

)";

gs_shader_info default_vertex_shader = {
    gs_shader_type::GS_SHADER_VERTEX,
    default_vertex_shader_str,
    {
        {
            "float4x4",
            "ViewProj",
            "",
            shader_var_type::SHADER_VAR_UNIFORM,
            0,
            0,
            {}
        }
    },
    {
        {
            "_input_attrib0",
            "POSITION",
            true
        },
        {
            "_input_attrib1",
            "TEXCOORD0",
            true
        },
        {
            "_vertex_shader_attrib0",
            "TEXCOORD0",
            false
        }
    },
    {

    }
};

gs_shader_info default_pixel_shader = {
    gs_shader_type::GS_SHADER_PIXEL,
    default_pixel_shader_str,
    {
        {
            "texture2d",
            "image",
            "",
            shader_var_type::SHADER_VAR_UNIFORM,
            0,
            0,
            {}
        }
    },
    {},
    {
        {
            "def_sampler",
            {"Filter", "AddressU", "AddressV"},
            {"Linear", "Clamp", "Clamp"}
        }
    }
};

gs_shader_item default_shader = {
    "default_effect",
    {
        default_vertex_shader,
        default_pixel_shader
    }

};

std::vector<gs_shader_item> all_shaders = {
    {
        default_shader,
    }
};


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

