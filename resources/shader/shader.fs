#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform float minBrightness;
uniform float maxBrightnessFactor;

vec3 Normal;
vec3 FragPos;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2DArray depthMap;
uniform vec3 cameraPos;
uniform float fogStart;
uniform float fogEnd;
uniform vec3 fogColor;
uniform vec3 lightDir;
uniform vec3 lightColor;

layout(std140) uniform LightSpaceMatrices
{
    mat4 lightSpaceMatrices[16]; // UBO for 16 light space matrices
};

uniform float cascadePlaneDistances[16];
uniform int cascadeCount;   // number of frusta - 1
uniform float farPlane;
uniform mat4 view;


float inShadow(vec3 fragPosWorldSpace)
{
    // select cascade layer
    vec4 fragPosViewSpace = view * vec4(fragPosWorldSpace, 1.0);
    float depthValue = abs(fragPosViewSpace.z);

    int layer = -1;
    for (int i = 0; i < cascadeCount; ++i)
    {
        if (depthValue < cascadePlaneDistances[i])
        {
            layer = i;
            break;
        }
    }
    if (layer == -1)
    {
        layer = cascadeCount;
    }
    vec4 fragPosLightSpace = lightSpaceMatrices[layer] * vec4(fragPosWorldSpace, 1.0);

    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    if(currentDepth > 1.0){
        return 1.0;
    }

    // check whether current frag pos is in shadow
    vec3 normal = normalize(Normal);

    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    const float biasModifier = 0.5f;
    if (layer == cascadeCount)
        {
            bias *= 1 / (farPlane * biasModifier);
        }
        else
        {
            bias *= 1 / (cascadePlaneDistances[layer] * biasModifier);
        }
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(depthMap, 0));

        for(int x = -1; x <= 1; ++x)
        {
            for(int y = -1; y <= 1; ++y)
            {
                float pcfDepth = texture(depthMap, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r;
                shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;
            }
        }
        shadow /= 9.0;

        return shadow;
}

void main()
{
    Normal = texture(gNormal, TexCoord).rgb;
    FragPos = texture(gPosition, TexCoord).rgb;

    //ambient
    float ambientStrength = minBrightness;
    vec3 ambient = ambientStrength * lightColor;

    // diffuse
    vec3 norm = normalize(Normal);
    //vec3 lightDir = normalize(lightPos - FragPos);
    float diff = clamp(max(dot(norm, lightDir), 0.0), 0.0, minBrightness);
    vec3 diffuse = diff * lightColor;

    vec4 sampledColor = texture(gAlbedoSpec, TexCoord);

    float shadow = inShadow(FragPos);

    //fog

    float distance = length(FragPos.xyz - cameraPos);

    float fogFactor = smoothstep(fogStart, fogEnd, distance);



    //result

    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse)) * sampledColor.rgb * maxBrightnessFactor;

    vec3 finalColor = mix(lighting, fogColor, fogFactor);

    FragColor = vec4(finalColor, sampledColor.a);
}

