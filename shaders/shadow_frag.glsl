#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform mat4 lightSpaceMatrix;


uniform sampler2D colourTex;
uniform sampler2D depthTex;
uniform sampler2D shadowTex;
uniform sampler2D positionTex;

uniform vec3 lightPos;
uniform vec3 viewPos;

float ShadowCalculation()
{
    //view space position
    vec4 wpos = vec4(texture(positionTex, TexCoords).rgb,1.0);

    //NDC-space (not a problem with ortho projections)
    vec3 projCoords = wpos.xyz / wpos.w;

    // Transform to [0,1] range
    projCoords = projCoords * .5 + .5;


    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowTex, projCoords.xy).r;

    //       return vec3(texture(shadowTex, closestDepth).r);
    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;


    // Check whether current frag pos is in shadow
    //vec3 lightDir = normalize(lightPos - FragPos);


    //float bias = max(0.05 * (1.0 - dot(Normal, lightDir)), 0.005);
    float bias = .05;
    // check whether current frag pos is in shadow
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

    // PCF8=

/*
    shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowTex, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowTex, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;*/

   // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}

void main()
{
    //FragColor = vec4(texture(depthTex, TexCoords).rrr,1.0);
    //FragColor = vec4(texture(shadowTex, TexCoords).rrr,1.0);
    FragColor = vec4(texture(positionTex, TexCoords).rgb,1.0);
    //FragColor = vec4(texture(colourTex, TexCoords).rgb,1.0);


    float shadow = ShadowCalculation();
/*
    if(TexCoords.x > .5){
             FragColor = vec4(texture(positionTex, TexCoords).rgb,1.0);

    }else{
        FragColor = vec4(texture(colourTex, TexCoords).rgb * (1-(shadow/2)),1.0) ;
    }*/

    //FragColor = vec4(texture(colourTex, TexCoords).rgb * (1-(shadow/2)),1.0) ;
    //FragColor = lightSpaceMatrix*vec4(1);
}
