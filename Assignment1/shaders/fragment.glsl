#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

struct Material {
    vec3  baseColor;
    float ambientStr;
    float diffuseStr;
    float specularStr;
    float shininess;
    float alpha;
};

struct Light {
    vec3  position;
    vec3  color;
    float intensity;
};

uniform Material  material;
uniform Light     lights[3];
uniform vec3      viewPos;
uniform bool      useTexture;
uniform sampler2D texture_diffuse1;

void main() {
    vec3 baseColor = material.baseColor;
    if (useTexture) {
        vec4 tex = texture(texture_diffuse1, TexCoords);
        if (tex.a > 0.05) baseColor = tex.rgb;
    }

    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);

    // Ambient — warm dark purple tint, not pure black
    vec3 ambientTint = vec3(0.08, 0.07, 0.13);
    vec3 result = material.ambientStr * ambientTint * baseColor;

    for (int i = 0; i < 3; i++) {
        vec3  L    = normalize(lights[i].position - FragPos);
        vec3  H    = normalize(L + V);
        float dist = length(lights[i].position - FragPos);

        // Generous attenuation for scene scale ~3-6 units
        float att   = 1.0 / (0.5 + 0.07 * dist + 0.008 * dist * dist);
        float power = lights[i].intensity * att;

        // Diffuse
        float NdotL  = max(dot(N, L), 0.0);
        vec3  diffuse = NdotL * lights[i].color * material.diffuseStr * baseColor;

        // Specular Blinn-Phong
        float NdotH  = max(dot(N, H), 0.0);
        float spec   = pow(NdotH, material.shininess);
        vec3  specular = spec * lights[i].color * material.specularStr;

        result += (diffuse + specular) * power;
    }

    // Reinhard tone mapping → prevents harsh clipping
    result = result / (result + vec3(1.0));

    // Gamma correction
    result = pow(result, vec3(1.0 / 2.2));

    FragColor = vec4(result, material.alpha);
}
