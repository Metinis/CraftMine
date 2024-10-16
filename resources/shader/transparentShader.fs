#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;
in float brightness;
in vec3 Normal;

uniform float minBrightness;
uniform float maxBrightnessFactor;

uniform mat4 lightSpaceMatrix;

in vec3 FragPos;
uniform sampler2D ourTexture;
uniform sampler2D depthMap;
uniform vec3 cameraPos;
uniform float fogStart;
uniform float fogEnd;
uniform vec3 fogColor;
uniform vec3 lightPos;
uniform vec3 lightColor;

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
    float bias = max(0.00075 * (1.0 - dot(normal, lightDir)), 0.000075 );
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(depthMap, 0);

    int numSamples = 1;

    for(int x = -numSamples; x <= numSamples; ++x)
    {
        for(int y = -numSamples; y <= numSamples; ++y)
        {
            float pcfDepth = texture(depthMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= float((2 * numSamples + 1) * (2 * numSamples + 1));

    if(projCoords.z >= 1.0)
        shadow = 1.0;



    shadow = clamp(shadow, 0, 1.0);

    return shadow;
}

void main()
{
    //ambient
    float ambientStrength = minBrightness;
    vec3 ambient = ambientStrength * lightColor;

    // diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = clamp(max(dot(norm, lightDir), 0.0), 0.0, minBrightness);
    vec3 diffuse = diff * lightColor;

    vec4 fragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);

    vec4 sampledColor = texture(ourTexture, TexCoord);

    float shadow = inShadow(fragPosLightSpace);

    //fog

    float distance = length(FragPos.xyz - cameraPos);

    float fogFactor = smoothstep(fogStart, fogEnd, distance);



    //result

    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse)) * sampledColor.rgb * maxBrightnessFactor * brightness;

    vec3 finalColor = mix(lighting, fogColor, fogFactor);

    FragColor = vec4(finalColor, sampledColor.a);
}
