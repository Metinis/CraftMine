#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;
in float brightness;
in vec3 Normal;

uniform float minBrightness;
uniform float maxBrightnessFactor;

in vec3 FragPos;
in vec4 fragPosLightSpace;
uniform sampler2D ourTexture;
uniform sampler2D depthMap;
uniform vec3 cameraPos;
uniform float fogStart;
uniform float fogEnd;
uniform vec3 fogColor;
uniform vec3 lightPos;

float inShadow(vec4 fragPosLightSpace){
    
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(depthMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float bias = max(0.0001 * (1.0 - dot(normal, lightDir)), 0.00009 );
    //float bias = 0.0002;
    //float bias = 0.0001;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(depthMap, 0);

    int numSamples = 2;

    for(int x = -numSamples; x <= numSamples; ++x)
    {
        for(int y = -numSamples; y <= numSamples; ++y)
        {
            float pcfDepth = texture(depthMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= float((2 * numSamples + 1) * (2 * numSamples + 1));

    if(projCoords.z > 1.0)
        shadow = 1.0;

        
    shadow = clamp(shadow, 0, 1.0);

    return shadow;
}

void main()
{
    vec4 sampledColor = texture(ourTexture, TexCoord);

    // Adjust brightness
    vec3 adjustedColor;
    if(brightness < minBrightness)
    {
        adjustedColor = sampledColor.rgb * minBrightness;
    }
    else
    {
        adjustedColor = sampledColor.rgb * brightness * maxBrightnessFactor;
        float shadowBrightness = 1.0 - inShadow(fragPosLightSpace);
        if(shadowBrightness >= minBrightness){
            adjustedColor.rgb *= shadowBrightness;
        }
        else{
            adjustedColor.rgb *= minBrightness;
        }
    }

    

    float distance = length(FragPos.xyz - cameraPos);

    float fogFactor = smoothstep(fogStart, fogEnd, distance);

    vec3 finalColor = mix(adjustedColor, fogColor, fogFactor);

    // Set the output color
    FragColor = vec4(finalColor, sampledColor.a);
}

