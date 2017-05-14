#version 430

// The texture to be mapped
uniform sampler2D colourTex;
uniform sampler2D depthTex;

// The depth at which we want to focus
uniform float focalDepth = 0.5;

// A scale factor for the radius of blur
uniform float blurRadius = 0.008;

// The output colour. At location 0 it will be sent to the screen.()
layout (location=0) out vec4 fragColor;

// The fragment UV coordinates
in vec2 FragmentUV;

/***************************************************************************************************
 * Gaussian Blur functions and constants
 ***************************************************************************************************/
// Gaussian coefficients
const float G5x5[25] = {0.0035,    0.0123,    0.0210,    0.0123,    0.0035,
                        0.0123,    0.0543,    0.0911,    0.0543,    0.0123,
                        0.0210,    0.0911,    0.2224,    0.0911,    0.0210,
                        0.0123,    0.0543,    0.0911,    0.0543,    0.0123,
                        0.0035,    0.0123,    0.0210,    0.0123,    0.0035};

const float G9x9[81] = {0,         0,    0.0039,    0.0039,    0.0039,    0.0039,    0.0039,         0,         0,
                        0,    0.0039,    0.0078,    0.0117,    0.0117,    0.0117,    0.0078,    0.0039,         0,
                        0.0039,    0.0078,    0.0117,    0.0234,    0.0273,    0.0234,    0.0117,    0.0078,    0.0039,
                        0.0039,    0.0117,    0.0234,    0.0352,    0.0430,    0.0352,    0.0234,    0.0117,    0.0039,
                        0.0039,    0.0117,    0.0273,    0.0430,    0.0469,    0.0430,    0.0273,    0.0117,    0.0039,
                        0.0039,    0.0117,    0.0234,    0.0352,    0.0430,    0.0352,    0.0234,    0.0117,    0.0039,
                        0.0039,    0.0078,    0.0117,    0.0234,    0.0273,    0.0234,    0.0117,    0.0078,    0.0039,
                        0,    0.0039,    0.0078,    0.0117,    0.0117,    0.0117,    0.0078,    0.0039,         0,
                        0,         0,    0.0039,    0.0039,    0.0039,    0.0039,    0.0039,         0,         0};

// These define which Gaussian kernel and the size to use (G5x5 and 5 also possible)
#define SZ 9
#define G  G9x9
vec4 gaussianBlur( vec2 pos, float radius){
    float halfWidth = float(SZ)*0.5;
    vec4 retCol = vec4(0.0);

    for (int i=0;i<SZ;++i)
    {
        for (int j=0;j<SZ;++j)
        {
            vec2 offset = radius * vec2(float(i)-halfWidth,
                                        float(j)-halfWidth);
            retCol+=G[i*SZ +j]* texture(colourTex, pos + offset);
        }
    }
    return retCol;

}

/***************************************************************************************************
 * Poisson Blur functions and constants
 ***************************************************************************************************/
// Tap locations for 13 sample Poisson unit disc
const vec2 PoissonDisc[12] = {vec2(-0.326212,-0.40581),vec2(-0.840144,-0.07358),
                              vec2(-0.695914,0.457137),vec2(-0.203345,0.620716),
                              vec2(0.96234,-0.194983),vec2(0.473434,-0.480026),
                              vec2(0.519456,0.767022),vec2(0.185461,-0.893124),
                              vec2(0.507431,0.064425),vec2(0.89642,0.412458),
                              vec2(-0.32194,-0.932615),vec2(-0.791559,-0.59771)};

// A pseudorandom generator for the Poisson disk rotation
float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}


vec4 poissonBlur(vec2 pos, float radius){
    vec4 retCol = vec4(0);
    float coeff = 1.0/12.0;
    for (int i =0; i<12; ++i){
        retCol += coeff * texture(colourTex, pos+radius*PoissonDisc[i]);
    }
    return retCol;
}



void main() {
    // Determine the texture coordinate from the window size
    vec2 texpos = FragmentUV;

    // Determine sigma, the blur radius of this pixel here!
    float focalBlur = abs(texture(depthTex, texpos).r - focalDepth);
    // Call your blurFunction on the input texture based on the blur radius here to return a texture
    // Output a bit of the depth texture in this image for debugging
   // fragColor = poissonBlur(texpos,blurRadius*focalBlur);
    //debugger
    //fragColor = texture(colourTex, texpos);
    fragColor = vec4(texture(depthTex, texpos).rrr,1.0);

   //fragColor = texture(colourTex, texpos);
    //inversion
    // fragColor = vec4(vec3(1.0-texture(colourTex, texpos)),1.0);
    //grayscale
    //fragColor = texture(colourTex, texpos);
    //float average = 0.2126 * fragColor.r + 0.7152 * fragColor.g + 0.0722 * fragColor.b;
    //fragColor = vec4(average, average, average, 1.0);



}

