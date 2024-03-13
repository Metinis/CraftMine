#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;
in float brightness;
in vec3 Normal;

float minBrightness = 0.5f;

in vec3 FragPos;
in vec4 fragPosLightSpace;
uniform sampler2D ourTexture;
uniform sampler2D depthMap;
uniform vec3 cameraPos;
uniform float fogStart;
uniform float fogEnd;
uniform vec3 fogColor;
uniform vec3 lightPos;

bool inShadow(vec4 fragPosLightSpace){
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(depthMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    //vec3 normal = normalize(Normal);
    //vec3 lightDir = normalize(lightPos - FragPos);
    //float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    return currentDepth - 0.0015 > closestDepth;
}

void main()
{
    vec4 sampledColor = texture(ourTexture, TexCoord);

    // Adjust brightness
    vec3 adjustedColor;
    if(brightness < minBrightness || inShadow(fragPosLightSpace))
    {
        adjustedColor = sampledColor.rgb * minBrightness;
    }
    else
    {
        adjustedColor = sampledColor.rgb * brightness;
    }

    float distance = length(FragPos.xyz - cameraPos);

    float fogFactor = smoothstep(fogStart, fogEnd, distance);

    vec3 finalColor = mix(adjustedColor, fogColor, fogFactor);

    // Set the output color
    FragColor = vec4(finalColor, sampledColor.a);
}

