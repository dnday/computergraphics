#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 lightPos;
uniform vec3 viewPos;

// --- Material struct ---
struct Material {
    vec3  ambient;
    vec3  diffuse;
    vec3  specular;
    float shininess;
};

uniform Material material;

// --- Texture samplers ---
uniform sampler2D texDiffuse;   // texture unit 0
uniform sampler2D texSpecular;  // texture unit 1
uniform bool      useTexture;   // toggle: true = sample textures, false = use material colors

void main()
{
    vec3 lightColor =
        vec3(1.0);

    // Choose base colors from texture or material
    vec3 diffuseColor;
    vec3 specularColor;
    vec3 ambientColor;

    if (useTexture) {
        diffuseColor  = texture(texDiffuse,  TexCoords).rgb;
        specularColor = texture(texSpecular, TexCoords).rgb;
        ambientColor  = diffuseColor * 0.2;  // derive ambient from diffuse texture
    } else {
        diffuseColor  = material.diffuse;
        specularColor = material.specular;
        ambientColor  = material.ambient;
    }

    // ambient
    vec3 ambient =
        ambientColor * lightColor;

    // diffuse
    vec3 norm = normalize(Normal);

    vec3 lightDir =
        normalize(lightPos - FragPos);

    float diff =
        max(dot(norm, lightDir), 0.0);

    vec3 diffuse =
        diff * diffuseColor * lightColor;

    // specular
    vec3 viewDir =
        normalize(viewPos - FragPos);

    vec3 reflectDir =
        reflect(-lightDir, norm);

    float spec =
        pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    vec3 specular =
        specularColor
        * spec
        * lightColor;

    vec3 result =
        ambient + diffuse + specular;

    FragColor =
        vec4(result, 1.0);
}