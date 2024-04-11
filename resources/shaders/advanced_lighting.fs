#version 330 core
out vec4 FragColor;

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};


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

#define SPOT_LIGHTS_AMOUNT 4

uniform DirLight dirLight;
uniform PointLight pointLight;
uniform SpotLight spotLights[SPOT_LIGHTS_AMOUNT];

vec4 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec4 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec4 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    vec3 normal = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);

    FragColor = CalcDirLight(dirLight, normal, viewDir);

    FragColor += CalcPointLight(pointLight, normal, fs_in.FragPos, viewDir);

    for(int i=0; i < SPOT_LIGHTS_AMOUNT; i++)
        FragColor += CalcSpotLight(spotLights[i], normal, fs_in.FragPos, viewDir);
}

vec4 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
        vec4 diffuseColor = texture(material.diffuseMap, fs_in.TexCoords).rgba;
        vec4 specularColor = vec4(texture(material.specularMap, fs_in.TexCoords).rrr, 0.5);
        vec3 lightDir = normalize(-light.direction);

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
        vec4 specular = vec4(light.specular, 1) * spec * specularColor; // assuming bright white light color

        return (ambient + diffuse + specular);
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
    vec4 specular = vec4(light.specular, 1) * spec * specularColor; // assuming bright white light color

    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    ambient *= attenuation;
            diffuse *= attenuation;
            specular *= attenuation;

            return ambient + diffuse + specular;
}

vec4 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
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
        vec4 specular = vec4(light.specular, 1) * spec * specularColor; // assuming bright white light color

        // attenuation
        float distance = length(light.position - fragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

        //spotlight intensity
        float theta = dot(lightDir, normalize(-light.direction));
        float epsilon = light.cutOff - light.outerCutOff;
        float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);


        ambient *= attenuation * intensity;
        diffuse *= attenuation * intensity;
        specular *= attenuation * intensity;

        return ambient + diffuse + specular;
}