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
    //FragColor = texture(material.diffuse, TexCoords);
    //return;


    // ambient
    vec4 ambient = vec4(dirLight.ambient, 1.0) * texture(material.diffuse, TexCoords).rgba;

    // diffuse
    vec3 norm = normalize(Normal);
    vec3 dirLightDir = normalize(dirLight.position - FragPos);
    float diff = max(dot(norm, dirLightDir), 0.0);
    vec4 diffuse = vec4(dirLight.diffuse, 1.0) * diff * texture(material.diffuse, TexCoords).rgba;

    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-dirLightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec4 specular = vec4(dirLight.specular, 1.0) * spec * texture(material.specular, TexCoords).rgba;

    // attenuation
    float distance    = length(dirLight.position - FragPos);
    float attenuation = 1.0 / (dirLight.constant + dirLight.linear * distance + dirLight.quadratic * (distance * distance));

    ambient  *= attenuation;
    diffuse   *= attenuation;
    specular *= attenuation;

    vec4 result = ambient + specular + diffuse;

    FragColor = vec4(result);
}