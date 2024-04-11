#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

struct Material {
    sampler2D diffuseMap;
    sampler2D specularMap;
    float shininess;
};

uniform Material material;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform bool blinn;

void main()
{
    vec4 diffuseColor = texture(material.diffuseMap, fs_in.TexCoords).rgba;
    vec4 specularColor = texture(material.specularMap, fs_in.TexCoords).rgba;

    // ambient
    vec4 ambient = vec4(0.05, 0.15, 0.05, 1.0) * diffuseColor;


    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    vec3 normal = normalize(fs_in.Normal);
    float diff = max(dot(lightDir, normal), 0.0);
    vec4 diffuse = diff * diffuseColor;


    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    if(blinn)
    {
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    }
    else
    {
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);
    }
    vec4 specular = vec4(material.shininess) * spec * specularColor; // assuming bright white light color


    FragColor = ambient + diffuse + specular;
}