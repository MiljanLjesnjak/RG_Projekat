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
};

uniform Material material;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform bool blinn;

//-----------------

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform PointLight pointLight;

vec4 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    vec3 normal = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);

    FragColor = CalcPointLight(pointLight, normal, fs_in.FragPos, viewDir);
}

vec4 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec4 diffuseColor = texture(material.diffuseMap, fs_in.TexCoords).rgba;
    vec4 specularColor = vec4(texture(material.specularMap, fs_in.TexCoords).rrr, 0.5);

    vec3 lightDir = normalize(light.position - fragPos);

    // ambient
    vec4 ambient = vec4(light.ambient, 1.0) * diffuseColor;

    // diffuse
    float diff = max(dot(lightDir, normal), 0.0);
    vec4 diffuse = vec4(light.diffuse, 1.0) * diff * diffuseColor;

    // specular
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
    vec4 specular = vec4(light.specular, 0.5f) * spec * specularColor; // assuming bright white light color

    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec4 result = ambient + diffuse + specular;

    result *= vec4(vec3(attenuation), 1.0);

    return result;
}