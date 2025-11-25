#version 330 core

out vec4 frag_color;

struct AmbientLight {
    bool enabled;
    vec3 ambient;
};

struct PointLight {
    bool enabled;
    vec3 position;
    vec3 diffuse;
    vec3 specular;
    vec3 attenuation;
};

struct DirectionalLight {
    bool enabled;
    vec3 direction;
    vec3 diffuse;
    vec3 specular;
};
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

in vec3 world_position;
in vec3 world_normal;
in vec2 tex_coord;

uniform vec3 camera_position;

uniform Material material; // material properties of the object
uniform AmbientLight ambient_light;
uniform PointLight point_light; 
uniform DirectionalLight directional_light;
vec3 CalcAmbientLight();
vec3 CalcPointLight(vec3 normal, vec3 view_dir);
vec3 CalcDirectionalLight(vec3 normal, vec3 view_dir);

uniform bool ambient_texture_enabled; // whether to use texture or native material
uniform bool diffuse_texture_enabled;
uniform bool specular_texture_enabled;

uniform sampler2D ambient_text; // the textures themselves
uniform sampler2D diffuse_text;
uniform sampler2D specular_text;
// Shadow mapping uniforms
uniform int shadow_map_enabled;
uniform sampler2D shadow_map;
uniform mat4 world_to_light_ndc_matrix;

void main() {
    vec3 normal = normalize(world_normal);
    vec3 view_dir = normalize(camera_position - world_position);

    frag_color = vec4(0.0);

    if (ambient_light.enabled) {
        frag_color += vec4(CalcAmbientLight(), 1.0);
    }
    
    if (point_light.enabled) {
        frag_color += vec4(CalcPointLight(normal, view_dir), 1.0);
    }

    if (directional_light.enabled) {
        frag_color += vec4(CalcDirectionalLight(normal, view_dir), 1.0);
    }
}

vec3 GetAmbientColor() {
    if (ambient_texture_enabled){
        return texture(ambient_text,  tex_coord).rgb;
    } 
    return material.ambient;
}

vec3 GetDiffuseColor() {
    if (diffuse_texture_enabled){
        return texture(diffuse_text, tex_coord).rgb;
    }
    return material.diffuse;
}

vec3 GetSpecularColor() {
    if (specular_texture_enabled){
        return texture(specular_text, tex_coord).rgb;
    }
    return material.specular;
}

vec3 CalcAmbientLight() {
    return ambient_light.ambient * GetAmbientColor();
}

vec3 CalcPointLight(vec3 normal, vec3 view_dir) {
    PointLight light = point_light;
    vec3 light_dir = normalize(light.position - world_position);

    float diffuse_intensity = max(dot(normal, light_dir), 0.0);
    vec3 diffuse_color = diffuse_intensity * light.diffuse * GetDiffuseColor();

    vec3 reflect_dir = reflect(-light_dir, normal);
    float specular_intensity = pow(
        max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular_color = specular_intensity * 
        light.specular * GetSpecularColor();

    float distance = length(light.position - world_position);
    float attenuation = 1.0 / (light.attenuation.x + 
        light.attenuation.y * distance + 
        light.attenuation.z * (distance * distance));

    return attenuation * (diffuse_color + specular_color);
}

vec3 CalcDirectionalLight(vec3 normal, vec3 view_dir) {
    DirectionalLight light = directional_light;
    vec3 light_dir = normalize(-light.direction);
    float diffuse_intensity = max(dot(normal, light_dir), 0.0);
    vec3 diffuse_color = diffuse_intensity * light.diffuse * GetDiffuseColor();

    vec3 reflect_dir = reflect(-light_dir, normal);
    float specular_intensity = pow(
        max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular_color = specular_intensity * 
        light.specular * GetSpecularColor();

    vec3 final_color = diffuse_color + specular_color;

    return final_color;

    // Shadow test
     float shadow = 1.0;
    if (shadow_map_enabled == 1) {
        vec4 ls = world_to_light_ndc_matrix * vec4(world_position, 1.0);
        // Project to NDC and then to [0,1]
        vec3 proj = ls.xyz / ls.w;
        vec2 uv = proj.xy * 0.5 + 0.5;
        float depth = proj.z * 0.5 + 0.5;
        // If outside shadow map, consider lit
        if (uv.x >= 0.0 && uv.x <= 1.0 && uv.y >= 0.0 && uv.y <= 1.0) {
            float closest = texture(shadow_map, uv).r;
            float bias = 0.001;
            shadow = (depth - bias) > closest ? 0.0 : 1.0;
        }
    }

    return shadow * final_color;
}

