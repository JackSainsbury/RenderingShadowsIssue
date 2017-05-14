#version 430
uniform sampler2D NormalTexture;
uniform sampler2D ColourTexture;

float bumpStrength = 2.0;
layout (location=0) out vec4 fragColour;


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

uniform LightInfo Light2 = LightInfo(
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
            vec3(1.0, 1.0, 1.0),    // Kd
            vec3(1.0, 1.0, 1.0),    // Ks
            10.0                    // Shininess
            );

// Attributes passed on from the vertex shader
smooth in vec3 WSVertexPosition;
smooth in vec3 WSVertexNormal;
smooth in vec2 WSTexCoord;

//change this
vec3 hash3( vec2 p )
{
    vec3 q = vec3( dot(p,vec2(127.1,311.7)),
                   dot(p,vec2(269.5,183.3)),
                   dot(p,vec2(419.2,371.9)) );
    return fract(sin(q)*43758.5453);
}

float iqnoise( in vec2 x, float u, float v )
{
    vec2 p = floor(x);
    vec2 f = fract(x);

    float k = 1.0+63.0*pow(1.0-v,4.0);

    float va = 0.0;
    float wt = 0.0;
    for( int j=-2; j<=2; j++ )
        for( int i=-2; i<=2; i++ )
        {
            vec2 g = vec2( float(i),float(j) );
            vec3 o = hash3( p + g )*vec3(u,u,1.0);
            vec2 r = g - f + o.xy;
            float d = dot(r,r);
            float ww = pow( 1.0-smoothstep(0.0,1.414,sqrt(d)), k );
            va += o.z*ww;
            wt += ww;
        }

    return va/wt;
}

mat4 rotationMatrix(vec3 axis, float angle)
{
    //axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

/**
  * Rotate a vector vec by using the rotation that transforms from src to tgt.
  */
vec3 rotateVector(vec3 src, vec3 tgt, vec3 vec) {
    float angle = acos(dot(src,tgt));

    // Check for the case when src and tgt are the same vector, in which case
    // the cross product will be ill defined.
    if (angle == 0.0) {
        return vec;
    }
    vec3 axis = normalize(cross(src,tgt));
    mat4 R = rotationMatrix(axis,angle);

    // Rotate the vec by this rotation matrix
    vec4 _norm = R*vec4(vec,1.0);
    return _norm.xyz / _norm.w;
}

vec2 rotate(vec2 v, float a) {
    float s = sin(a);
    float c = cos(a);
    mat2 m = mat2(c, -s, s, c);
    return m * v;
}
vec2 grid(vec2 uv)
{
    vec2 pixel = uv * vec2(200,200);
    vec2 dif = pixel - floor(pixel);
    return  floor(dif + vec2(0.1,0.1));
}

//https://www.shadertoy.com/view/ls33DN
float spiral(vec2 p) {

    p.x *= sign(fract(sin(dot(floor(p), vec2(100,20)))*3)-.5);
    p=fract(p);
    return pow(cos(min(length(p), length(p - 1))*6.283*2),5);

}
void main() {
    float h = spiral(50.0*vec2(WSTexCoord.x+0.002,WSTexCoord.y));
    float h1 = spiral( 50.0*vec2(WSTexCoord.x, WSTexCoord.y+0.002));
    float h2 = spiral(50.0*WSTexCoord.xy);

    vec3 n = normalize( WSVertexNormal );

    // Calculate the view vector
    vec3 v = normalize(vec3(-WSVertexPosition));

    vec3 src = vec3(0.0, 0.0, 1.0);

    // Calculate the light vector
    vec3 s = normalize( vec3(Light.Position) - WSVertexPosition );


    float f = iqnoise( 24.0*vec2(WSTexCoord.x+0.003,WSTexCoord.y), s.x,s.y );
    float f1 = iqnoise( 24.0*vec2(WSTexCoord.x, WSTexCoord.y+0.003), s.x,s.y );
    float f2 = iqnoise( 24.0*WSTexCoord.xy, s.x,s.y );


    vec2 uv = rotate(WSTexCoord,45);

    vec2 gridcoor = grid(vec2(uv.x+0.0015,uv.y));
    vec2 gridcoor1 = grid(vec2(uv.x,uv.y+0.0015));
    vec2 gridcoor2 = grid(vec2(uv.x,uv.y));

    vec3 texNormal = texture(NormalTexture, WSTexCoord*3).rgb;
    vec3 texColor =max( vec3(floor(gridcoor2.x + gridcoor2.y)-0.5,floor(gridcoor2.x + gridcoor2.y)-0.5,floor(gridcoor2.x + gridcoor2.y)-0.5),vec3(h2+0.5,h2,0.3));

    float val = max((floor(gridcoor.x + gridcoor.y)-0.5),h);
    float val1 = max((floor(gridcoor1.x + gridcoor1.y)-0.5),h1);
    float val2 = max((floor(gridcoor2.x + gridcoor2.y)-0.5),h2);

    //vec4 bump = vec4(clamp(vec3(val+texNormal.r,val1+texNormal.g,val2+texNormal.b), vec3(0,0,0), vec3(1,1,1)),1.0)*2;
    vec3 bump = vec3(val,val1,val2)*3;

    // vec3 bump2 = vec3(h,h1,h2)*3;
    vec3 tgt=normalize((bump.rgb)*2.0-1.0);
     //vec3 tgt2=normalize((bump2.rgb)*2.0-1.0);

    //vec3 tgt2= normalize(texture(NormalTexture, WSTexCoord*3).rgb * 2.0 - 1.0);


    vec3 np = rotateVector(src, tgt, n);
     // vec3 np2 = rotateVector(src, tgt2, n);


    // Reflect the light about the surface normal
    vec3 r = reflect( -s, np);
    //vec3 r2 = reflect( -s, np2);

    // Compute the light from the ambient, diffuse and specular components
    //  vec3 lightColor = (
    //           Light.La * Material.Ka +
    //         Light.Ld * (Material.Kd * max( dot(s, np), 0.0 ) + Material.Kd * max( dot(s, np2), 0.0 ))+
    //         Light.Ls * (Material.Ks * pow( max( dot(r,v), 0.0 ), Material.Shininess ) + Material.Ks * pow( max( dot(r2,v), 0.0 ), Material.Shininess )));

    vec3 lightColor2 = (
                Light2.La * Material.Ka +
                Light2.Ld * Material.Kd * max( dot(s, np), 0.0 ) +
                Light2.Ls * Material.Ks * pow( max( dot(r,v), 0.0 ), Material.Shininess ));

    // Set the output color of our current pixel
    //fragColour =  vec4((clamp(vec3((lightColor)+lightColor2), vec3(0,0,0), vec3(1.0,1.0,1.0))),1.0);
    // fragColour =  vec4(clamp(vec3(val2,val2,val2)+texColor, vec3(0,0,0), vec3(1,1,1))*lightColor2,1.0);

    fragColour =  vec4(texColor*0.5*lightColor2,1.0);



}
