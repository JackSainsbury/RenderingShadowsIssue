#version 420                                            // Keeping you on the bleeding edge!
#extension GL_EXT_gpu_shader4 : enable

// Attributes passed on from the vertex shader
smooth in vec3 WSVertexPosition;
smooth in vec3 WSVertexNormal;
smooth in vec2 WSTexCoord;

// Structure for holding light parameters
struct LightInfo {
    vec4 Position; // Light position in eye coords.
    vec3 La; // Ambient light intensity
    vec3 Ld; // Diffuse light intensity
    vec3 Ls; // Specular light intensity
};

// We'll have a single light in the scene with some default values
uniform LightInfo Light = LightInfo(
            vec4(2.0, 2.0, 10.0, 1.0),   // position
            vec3(0.2, 0.2, 0.2),        // La
            vec3(1.0, 1.0, 1.0),        // Ld
            vec3(1.0, 1.0, 1.0)         // Ls
            );

// The material properties of our object
struct MaterialInfo {
    vec3 Ka; // Ambient reflectivity
    vec3 Kd; // Diffuse reflectivity
    vec3 Ks; // Specular reflectivity
    float Shininess; // Specular shininess factor
};

// The object has a material
uniform MaterialInfo Material = MaterialInfo(
            vec3(0.1, 0.1, 0.1),    // Ka
            vec3(0.5, 0.2, 0.2),    // Kd
            vec3(0.0,0.0,0.0),    // Ks
            10.0                    // Shininess
            );

// This is no longer a built-in variable
out vec4 FragColor;
float random (vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
                 43758.5453123);
}
// 2D Noise based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise (in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    // Smooth Interpolation

    // Cubic Hermine Curve.  Same as SmoothStep()
    vec2 u = f*f*(3.0-2.0*f);
    // u = smoothstep(0.,1.,f);

    // Mix 4 coorners porcentages
    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

mat2 rotate2d(float angle){
    return mat2(cos(angle),-sin(angle),
                sin(angle),cos(angle));
}

float lines(in vec2 pos, float b){
    float scale = 2.680;
    pos *= scale;
    return smoothstep(0.006,
                      0.5+b*-0.5,
                      abs((sin(pos.x*6.736)+b*1.016))*0.476);
}
#define OCTAVES 5

//https://www.shadertoy.com/view/lsSyDw
float fbm (in vec2 uv) {
    // Initial values
    float value = 0.840;
    float amplitud = 0.172;
    float frequency = 3.704;
    //
    // Loop of octaves
    for (int i = 0; i < OCTAVES; i++) {
        value += amplitud * noise(uv);
        uv *= 0.024;
        amplitud *= 1.132;
    }
    return value;
}

void main() {

    vec2 st = WSTexCoord.xy;
    vec2 pos = st.yx*vec2(10.,10.);

    float pattern = pos.x;
    pos = rotate2d( noise(pos) ) * pos;

    // Draw lines
    pattern = lines(pos,-4.940);
    vec2 uv = WSTexCoord.xy ;



    vec3 value = vec3(0.335,0.048,0.149);

    float planks;
    planks = abs(sin(uv.y*8.648));
    value *= planks;

    vec3 colorA = vec3(0.000,0.000,0.000);
    value = mix(value, colorA, vec3(fbm(uv.yy * 4.136)));

    value = mix(value, vec3(0.955,0.260,0.074), vec3(fbm(uv.yy*9.824)));
    value = mix(value, vec3(0.232,0.930,0.198), random(uv.yy)*-0.212);
    value -= (noise(uv*vec2(0.370,0.280) - noise(uv*vec2(-0.300,-0.210))) * 0.732);




    // Calculate the normal (this is the expensive bit in Phong)
    vec3 n = normalize( WSVertexNormal );

    // Calculate the light vector
    vec3 s = normalize( vec3(Light.Position) - WSVertexPosition );

    // Calculate the view vector
    vec3 v = normalize(vec3(-WSVertexPosition));

    // Reflect the light about the surface normal
    vec3 r = reflect( -s, n );

    // Compute the light from the ambient, diffuse and specular components
    vec3 lightColor = (
                Light.La * Material.Ka +
                Light.Ld * Material.Kd * max( dot(s, n), 0.0 ) +
                Light.Ls * Material.Ks * pow( max( dot(r,v), 0.0 ), Material.Shininess ));

    // Set the output color of our current pixel
    FragColor = vec4(vec3(value*pattern)*lightColor, 1.0);

}
