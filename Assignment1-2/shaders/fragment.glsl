#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

struct Material {
    vec3  baseColor;
    float ambient;
    float diffuse;
    float specular;
    float shininess;
    float alpha;
};

struct Light {
    vec3  position;
    vec3  color;
    float intensity;
};

uniform Material material;
uniform Light    lights[3];
uniform vec3     viewPos;

uniform sampler2D texDiffuse;
uniform sampler2D texSpecular;
uniform sampler2D texRoughness;
uniform sampler2D texAlpha;

uniform bool      useTexture;
uniform int       renderPass; // 0 = Opaque, 1 = Transparent
uniform int       objectKind; // 0 = showcase model, 1 = floor, 2 = item, 3 = table
uniform vec2      screenSize;

vec3 acesToneMap(vec3 color)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

float softBoxShadow(vec2 point, vec2 center, vec2 halfSize, float feather)
{
    vec2 d = abs(point - center) - halfSize;
    float outside = length(max(d, 0.0));
    return 1.0 - smoothstep(0.0, feather, outside);
}

vec3 applyGrade(vec3 color)
{
    color = acesToneMap(color);

    vec2 uv = gl_FragCoord.xy / max(screenSize, vec2(1.0));
    float vignette = smoothstep(0.92, 0.22, length(uv - 0.5));
    color *= mix(0.58, 1.06, vignette);

    color *= vec3(0.92, 0.96, 1.10);
    return pow(color, vec3(1.0 / 2.2));
}

float luminance(vec3 color)
{
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
}

float saturation(vec3 color)
{
    return max(max(color.r, color.g), color.b) - min(min(color.r, color.g), color.b);
}

void main()
{
    if (objectKind == 1) {
        vec3 floorColor = vec3(0.018, 0.022, 0.040);

        float shadow = softBoxShadow(FragPos.xz, vec2(0.20, -0.65), vec2(2.25, 0.55), 2.25);
        shadow = max(shadow, softBoxShadow(FragPos.xz, vec2(-0.75, -0.20), vec2(1.35, 0.40), 1.75));
        floorColor *= mix(1.0, 0.22, shadow);

        float centerFade = smoothstep(10.0, 1.5, length(FragPos.xz));
        vec3 result = floorColor * mix(0.45, 1.0, centerFade);

        FragColor = vec4(applyGrade(result), 1.0);
        return;
    }

    bool isItem = (objectKind == 2);
    bool isTable = (objectKind == 3);
    float alphaSample = useTexture ? texture(texAlpha, TexCoords).r : material.alpha;
    float glassMask = isItem ? 0.0 : smoothstep(0.74, 0.92, alphaSample);
    
    // Opaque pass: draw frame, shelf, and items. Transparent pass: glass only.
    if (renderPass == 0 && glassMask > 0.50) {
        discard;
    }
    else if (renderPass == 1 && glassMask <= 0.50) {
        discard;
    }

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 texSrgb = useTexture ? texture(texDiffuse, TexCoords).rgb : material.baseColor;
    vec3 texColor = useTexture ? pow(max(texSrgb, vec3(0.0)), vec3(0.95)) : material.baseColor;
    float specMask = useTexture ? texture(texSpecular, TexCoords).r : 1.0;
    float roughness = useTexture ? texture(texRoughness, TexCoords).r : 0.35;
    float texLum = luminance(texSrgb);
    float texSat = saturation(texSrgb);
    float woodMask = isItem ? 0.0 : max(isTable ? (1.0 - glassMask) : 0.0,
        smoothstep(0.035, 0.16, texLum) * smoothstep(0.01, 0.10, texSrgb.r - texSrgb.b) * (1.0 - glassMask));
    
    vec3 materialTint = useTexture ? mix(vec3(1.0), material.baseColor, isItem ? 0.38 : 0.24) : material.baseColor;
    vec3 finalBaseColor = texColor * materialTint;
    float finalAlpha = 1.0;
    float shininess = mix(220.0, 38.0, clamp(roughness, 0.0, 1.0));
    float finalSpecular = material.specular * mix(0.55, 1.25, specMask);
    float fresnel = pow(1.0 - max(dot(norm, viewDir), 0.0), 3.5);
    
    if (isItem) {
        float metalMask = smoothstep(0.035, 0.22, texLum) * (1.0 - smoothstep(0.045, 0.16, texSat));
        float leatherMask = smoothstep(0.025, 0.16, texSrgb.r - texSrgb.b) * smoothstep(0.035, 0.24, texLum);
        float goldMask = smoothstep(0.035, 0.18, texSrgb.r - texSrgb.b) * smoothstep(0.00, 0.12, texSrgb.g - texSrgb.b) * smoothstep(0.06, 0.30, texLum);

        vec3 liftedTexture = pow(max(texSrgb, vec3(0.0)), vec3(0.58));
        vec3 bladeColor = mix(vec3(0.48, 0.52, 0.56), vec3(0.86, 0.90, 0.92), smoothstep(0.05, 0.28, texLum));
        vec3 leatherColor = liftedTexture * vec3(1.20, 0.86, 0.76);
        vec3 goldColor = vec3(0.96, 0.70, 0.28) * mix(0.55, 1.15, smoothstep(0.05, 0.26, texLum));

        finalBaseColor = liftedTexture * vec3(1.06, 1.03, 1.00);
        finalBaseColor = mix(finalBaseColor, bladeColor, metalMask * 0.82);
        finalBaseColor = mix(finalBaseColor, leatherColor, leatherMask * (1.0 - metalMask) * 0.72);
        finalBaseColor = mix(finalBaseColor, goldColor, goldMask * 0.72);

        shininess = mix(90.0, 260.0, metalMask);
        finalSpecular = mix(0.34, 1.90, max(metalMask, specMask * 0.65));
    } else if (glassMask > 0.50) {
        // Purple smoked glass like the reference image.
        finalBaseColor = mix(vec3(0.13, 0.08, 0.18), vec3(0.50, 0.46, 0.72), fresnel);
        finalAlpha = mix(0.18, 0.40, fresnel);
        shininess = 260.0;
        finalSpecular = 1.55;
    } else if (woodMask > 0.02) {
        vec3 atlasWood = pow(max(texSrgb, vec3(0.01)), vec3(0.62)) * vec3(1.65, 1.12, 0.78);
        vec3 warmWood = mix(vec3(0.17, 0.075, 0.038), atlasWood, smoothstep(0.02, 0.20, texLum));
        float subtleGrain = 0.92 + 0.08 * sin(TexCoords.x * 150.0 + TexCoords.y * 18.0);
        finalBaseColor = warmWood * subtleGrain;
        shininess = mix(24.0, 48.0, 1.0 - roughness);
        finalSpecular = material.specular * mix(0.10, 0.26, specMask);
    } else {
        finalBaseColor = mix(finalBaseColor * vec3(0.62, 0.50, 0.58), vec3(0.11, 0.075, 0.11), 0.35);
    }

    vec3 ambientColor = mix(vec3(0.030, 0.038, 0.080), vec3(0.115, 0.065, 0.045), woodMask);
    vec3 result = ambientColor * finalBaseColor * max(material.ambient, woodMask > 0.02 ? 0.62 : (isItem ? 0.58 : 0.18));

    for(int i = 0; i < 3; i++) {
        vec3 lightDir = normalize(lights[i].position - FragPos);
        
        // Diffuse
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lights[i].color * material.diffuse * finalBaseColor;
        
        // Specular (Blinn-Phong)
        vec3 halfDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(norm, halfDir), 0.0), max(shininess, material.shininess));
        vec3 specular = spec * lights[i].color * finalSpecular; 
        
        // Distance attenuation
        float dist = length(lights[i].position - FragPos);
        float attenuation = 1.0 / (1.0 + 0.065 * dist + 0.026 * (dist * dist));
        
        result += (diffuse + specular) * attenuation * lights[i].intensity;
    }

    float rim = pow(1.0 - max(dot(norm, viewDir), 0.0), 2.2);
    result += rim * vec3(0.18, 0.24, 0.46) * (glassMask > 0.50 ? 1.10 : (isItem ? 0.42 : 0.18));

    if (isItem) {
        float bladeLine = smoothstep(0.10, 0.85, 1.0 - abs(TexCoords.y - 0.53) * 3.2);
        result += bladeLine * specMask * vec3(0.20, 0.28, 0.38);
    }

    FragColor = vec4(applyGrade(result), finalAlpha);
}
