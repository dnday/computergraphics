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
uniform sampler2D texAlpha;

uniform bool      useTexture;
uniform int       renderPass; // 0 = Opaque, 1 = Transparent
uniform int       objectKind; // 0 = showcase model, 1 = floor, 2 = item, 3 = table

void main()
{
    // Floor (Latar Belakang / Bg)
    if (objectKind == 1) {
        // Membuat efek floor studio dengan gradasi terang di tengah dan gelap di pinggir
        float centerFade = smoothstep(15.0, 0.5, length(FragPos.xz));
        vec3 floorColor = vec3(0.15, 0.16, 0.18) * centerFade;
        FragColor = vec4(floorColor, 1.0f);
        return;
    }

    bool isItem = (objectKind == 2);
    float alphaSample = useTexture ? texture(texAlpha, TexCoords).r : material.alpha;
    
    // Asumsikan: nilai alpha yang tinggi (mendekati 1.0) di mask transparansi atau alpha material < 1.0 adalah kaca
    // Kita gunakan logic sederhana: jika material.alpha < 1.0 atau texture alpha menunjukkan ini kaca
    float alphaVal = useTexture ? texture(texAlpha, TexCoords).r : 1.0;
    bool isGlass = useTexture ? (alphaVal > 0.5 && !isItem) : (material.alpha < 0.99);

    if (renderPass == 0 && isGlass) {
        discard; // Jangan gambar kaca di Opaque pass
    }
    else if (renderPass == 1 && !isGlass) {
        discard; // Jangan gambar benda solid di Transparent pass
    }

    vec3 ambient = vec3(0.1);
    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);

    vec3 texColor = useTexture ? texture(texDiffuse, TexCoords).rgb : material.baseColor;
    float specMask = useTexture ? texture(texSpecular, TexCoords).r : material.specular;
    
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    ambient = lights[0].color * material.ambient * texColor;

    for(int i = 0; i < 3; i++) {
        vec3 lightDir = normalize(lights[i].position - FragPos);
        
        // Diffuse
        float diff = max(dot(norm, lightDir), 0.0);
        diffuse += lights[i].intensity * diff * lights[i].color * material.diffuse * texColor;
        
        // Specular (Phong)
        vec3 reflectDir = reflect(-lightDir, norm);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        specular += lights[i].intensity * spec * lights[i].color * specMask; 
    }

    vec3 result = ambient + diffuse + specular;
    
    // Set transparency yang benar
    float finalAlpha = isGlass ? (useTexture ? 0.35 : material.alpha) : 1.0;
    FragColor = vec4(result, finalAlpha);
}

