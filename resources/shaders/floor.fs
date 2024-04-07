#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform Material material;
uniform Light dirLight;

void main()
{
    // ambient
    vec3 ambient = dirLight.ambient * texture(material.diffuse, TexCoords).rgb;

    // diffuse
    vec3 norm = normalize(Normal);
    vec3 dirLightDir = normalize(dirLight.position - FragPos);
    float diff = max(dot(norm, dirLightDir), 0.0);
    vec3 diffuse = dirLight.diffuse * diff * texture(material.diffuse, TexCoords).rgb;

    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-dirLightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = dirLight.specular * spec * texture(material.specular, TexCoords).rgb;

    // attenuation
    float distance    = length(dirLight.position - FragPos);
    float attenuation = 1.0 / (dirLight.constant + dirLight.linear * distance + dirLight.quadratic * (distance * distance));

    ambient  *= attenuation;
    diffuse   *= attenuation;
    specular *= attenuation;

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}