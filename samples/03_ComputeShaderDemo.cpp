#include "VulkanExample.h"
#include "resource/Mesh.h"
#include "resource/Texture.h"

namespace mygfx::samples {

class ComputeShaderDemo : public Demo {
public:
    Ref<Texture> mStorageTexture;
    Ref<HwBuffer> mStorageBuffer;
    Ref<Shader> mComputeShader;

    struct UniformDataCompute { // Compute shader uniform block object
        glm::vec3 lightPos;
        float aspectRatio { 1.0f };
        glm::vec4 fogColor = glm::vec4(0.0f);
        struct {
            glm::vec3 pos = glm::vec3(0.0f, 0.0f, 4.0f);
            glm::vec3 lookat = glm::vec3(0.0f, 0.5f, 0.0f);
            float fov = 10.0f;
        } camera;
        glm::mat4 _pad;
    } uniformData;

#if defined(__ANDROID__)
    // Use a smaller image on Android for performance reasons
    const uint32_t textureSize = 1024;
#else
    const uint32_t textureSize = 2048;
#endif
    // Definitions for scene objects
    // The sample uses spheres and planes that are passed to the compute shader via a shader storage buffer
    // The computer shader uses the object type to select different calculations
    enum class SceneObjectType { Sphere = 0,
        Plane = 1 };
    // Spheres and planes are described by different properties, we use a union for this
    union SceneObjectProperty {
        glm::vec4 positionAndRadius;
        glm::vec4 normalAndDistance;
    };
    struct SceneObject {
        SceneObjectProperty objectProperties;
        glm::vec3 diffuse;
        float specular { 1.0f };
        uint32_t id { 0 };
        uint32_t objectType { 0 };
        // Due to alignment rules we need to pad to make the element align at 16-bytes
        glm::ivec2 _pad;
    };

    void start() override
    {
		auto& camera = mApp->camera;
		camera.type = Camera::CameraType::lookat;
		camera.setPerspective(60.0f, (float)mApp->width / (float)mApp->height, 0.1f, 512.0f);
		camera.setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
		camera.setTranslation(glm::vec3(0.0f, 0.0f, -4.0f));
		camera.rotationSpeed = 0.0f;
		camera.movementSpeed = 2.5f;


		prepareStorageBuffers();
		
		auto textureData = TextureData::texture2D(textureSize, textureSize, Format::R8G8B8A8_UNORM);
		textureData.usage = TextureUsage::SAMPLED | TextureUsage::STORAGE;
        mStorageTexture = Texture::createFromData(textureData);

        mComputeShader = new Shader(csCode);
        mComputeShader->updateDescriptorSet(0, 0, mStorageTexture);
        mComputeShader->updateDescriptorSet(0, 2, mStorageBuffer);

    }

    // Setup and fill the compute shader storage buffes containing object definitions for the raytraced scene
    void prepareStorageBuffers()
    {
        // Id used to identify objects by the ray tracing shader
        uint32_t currentId = 0;

        std::vector<SceneObject> sceneObjects {};

        // Add some spheres to the scene
        // std::vector<Sphere> spheres;
        // Lambda to simplify object creation
        auto addSphere = [&sceneObjects, &currentId](glm::vec3 pos, float radius, glm::vec3 diffuse, float specular) {
            SceneObject sphere {};
            sphere.id = currentId++;
            sphere.objectProperties.positionAndRadius = glm::vec4(pos, radius);
            sphere.diffuse = diffuse;
            sphere.specular = specular;
            sphere.objectType = (uint32_t)SceneObjectType::Sphere;
            sceneObjects.push_back(sphere);
        };

        auto addPlane = [&sceneObjects, &currentId](glm::vec3 normal, float distance, glm::vec3 diffuse, float specular) {
            SceneObject plane {};
            plane.id = currentId++;
            plane.objectProperties.normalAndDistance = glm::vec4(normal, distance);
            plane.diffuse = diffuse;
            plane.specular = specular;
            plane.objectType = (uint32_t)SceneObjectType::Plane;
            sceneObjects.push_back(plane);
        };

        addSphere(glm::vec3(1.75f, -0.5f, 0.0f), 1.0f, glm::vec3(0.0f, 1.0f, 0.0f), 32.0f);
        addSphere(glm::vec3(0.0f, 1.0f, -0.5f), 1.0f, glm::vec3(0.65f, 0.77f, 0.97f), 32.0f);
        addSphere(glm::vec3(-1.75f, -0.75f, -0.5f), 1.25f, glm::vec3(0.9f, 0.76f, 0.46f), 32.0f);

        const float roomDim = 4.0f;
        addPlane(glm::vec3(0.0f, 1.0f, 0.0f), roomDim, glm::vec3(1.0f), 32.0f);
        addPlane(glm::vec3(0.0f, -1.0f, 0.0f), roomDim, glm::vec3(1.0f), 32.0f);
        addPlane(glm::vec3(0.0f, 0.0f, 1.0f), roomDim, glm::vec3(1.0f), 32.0f);
        addPlane(glm::vec3(0.0f, 0.0f, -1.0f), roomDim, glm::vec3(0.0f), 32.0f);
        addPlane(glm::vec3(-1.0f, 0.0f, 0.0f), roomDim, glm::vec3(1.0f, 0.0f, 0.0f), 32.0f);
        addPlane(glm::vec3(1.0f, 0.0f, 0.0f), roomDim, glm::vec3(0.0f, 1.0f, 0.0f), 32.0f);

        mStorageBuffer = gfxApi().createBuffer1(BufferUsage::STORAGE, MemoryUsage::GPU_ONLY, std::span { sceneObjects });
    }

    void preDraw(GraphicsApi& cmd) override
    {
		uniformData.aspectRatio = (float)mApp->width / (float)mApp->height;
		uniformData.lightPos.x = 0.0f + sin(glm::radians(mApp->timer * 360.0f)) * cos(glm::radians(mApp->timer * 360.0f)) * 2.0f;
		uniformData.lightPos.y = 0.0f + sin(glm::radians(mApp->timer * 360.0f)) * 2.0f;
		uniformData.lightPos.z = 0.0f + cos(glm::radians(mApp->timer * 360.0f)) * 2.0f;
		uniformData.camera.pos = mApp->camera.position * -1.0f;

        cmd.bindShaderProgram(mComputeShader->getProgram());
        auto ubo = cmd.allocConstant(uniformData);
        cmd.bindUniforms({ ubo });
        cmd.dispatch(textureSize / 16, textureSize / 16, 1);
    }

    void draw(GraphicsApi& cmd) override
    {
        auto shader = ShaderLibs::getFullscreenShader();
        cmd.bindPipelineState(shader->pipelineState);
		cmd.bindUniforms({});
        cmd.draw(3, 1, 0, mStorageTexture->index());
    }

    const char* csCode = R"(
#version 450

layout (local_size_x = 16, local_size_y = 16) in;
layout (binding = 0, rgba8) uniform writeonly image2D resultImage;

#define EPSILON 0.0001
#define MAXLEN 1000.0
#define SHADOW 0.5
#define RAYBOUNCES 2
#define REFLECTIONS true
#define REFLECTIONSTRENGTH 0.4
#define REFLECTIONFALLOFF 0.5

#define SceneObjectTypeSphere 0
#define SceneObjectTypePlane 1

struct Camera 
{
	vec3 pos;   
	vec3 lookat;
	float fov; 
};

layout (binding = 1) uniform UBO 
{
	vec3 lightPos;
	float aspectRatio;
	vec4 fogColor;
	Camera camera;
	mat4 rotMat;
} ubo;

struct SceneObject
{
	vec4 objectProperties;
	vec3 diffuse;
	float specular;
	int id;
	int objectType;
};

layout (std140, binding = 2) buffer SceneObjects
{
	SceneObject sceneObjects[ ];
};

void reflectRay(inout vec3 rayD, in vec3 mormal)
{
	rayD = rayD + 2.0 * -dot(mormal, rayD) * mormal;
}

// Lighting =========================================================

float lightDiffuse(vec3 normal, vec3 lightDir) 
{
	return clamp(dot(normal, lightDir), 0.1, 1.0);
}

float lightSpecular(vec3 normal, vec3 lightDir, float specularFactor)
{
	vec3 viewVec = normalize(ubo.camera.pos);
	vec3 halfVec = normalize(lightDir + viewVec);
	return pow(clamp(dot(normal, halfVec), 0.0, 1.0), specularFactor);
}

// Sphere ===========================================================

float sphereIntersect(in vec3 rayO, in vec3 rayD, in SceneObject sphere)
{
	vec3 oc = rayO - sphere.objectProperties.xyz;
	float b = 2.0 * dot(oc, rayD);
	float c = dot(oc, oc) - sphere.objectProperties.w * sphere.objectProperties.w;
	float h = b*b - 4.0*c;
	if (h < 0.0) 
	{
		return -1.0;
	}
	float t = (-b - sqrt(h)) / 2.0;

	return t;
}

vec3 sphereNormal(in vec3 pos, in SceneObject sphere)
{
	return (pos - sphere.objectProperties.xyz) / sphere.objectProperties.w;
}

// Plane ===========================================================

float planeIntersect(vec3 rayO, vec3 rayD, SceneObject plane)
{
	float d = dot(rayD, plane.objectProperties.xyz);

	if (d == 0.0)
		return 0.0;

	float t = -(plane.objectProperties.w + dot(rayO, plane.objectProperties.xyz)) / d;

	if (t < 0.0)
		return 0.0;

	return t;
}

	
int intersect(in vec3 rayO, in vec3 rayD, inout float resT)
{
	int id = -1;
	float t = -1000.0f;

	for (int i = 0; i < sceneObjects.length(); i++)
	{
		// Sphere
		if (sceneObjects[i].objectType == SceneObjectTypeSphere) {
			t = sphereIntersect(rayO, rayD, sceneObjects[i]);
		}
		// Plane
		if (sceneObjects[i].objectType == SceneObjectTypePlane) {
			t = planeIntersect(rayO, rayD, sceneObjects[i]);
		}
		if ((t > EPSILON) && (t < resT))
		{
			id = sceneObjects[i].id;
			resT = t;
		}
	}	

	return id;
}

float calcShadow(in vec3 rayO, in vec3 rayD, in int objectId, inout float t)
{
	for (int i = 0; i < sceneObjects.length(); i++)
	{
		if (sceneObjects[i].id == objectId)
			continue;
		
		float tLoc = MAXLEN;

		// Sphere
		if (sceneObjects[i].objectType == SceneObjectTypeSphere) {
			tLoc = sphereIntersect(rayO, rayD, sceneObjects[i]);	
		}
		// Plane
		if (sceneObjects[i].objectType == SceneObjectTypePlane) {
			tLoc = planeIntersect(rayO, rayD, sceneObjects[i]);
		}
		if ((tLoc > EPSILON) && (tLoc < t))
		{
			t = tLoc;
			return SHADOW;
		}
	}		
	return 1.0;
}

vec3 fog(in float t, in vec3 color)
{
	return mix(color, ubo.fogColor.rgb, clamp(sqrt(t*t)/20.0, 0.0, 1.0));
}

vec3 renderScene(inout vec3 rayO, inout vec3 rayD, inout int id)
{
	vec3 color = vec3(0.0);
	float t = MAXLEN;

	// Get intersected object ID
	int objectID = intersect(rayO, rayD, t);
	
	if (objectID == -1)
	{
		return color;
	}
	
	vec3 pos = rayO + t * rayD;
	vec3 lightVec = normalize(ubo.lightPos - pos);				
	vec3 normal;
	
	for (int i = 0; i < sceneObjects.length(); i++)
	{
		if (objectID == sceneObjects[i].id) {
			// Sphere
			if (sceneObjects[i].objectType == SceneObjectTypeSphere) {
				normal = sphereNormal(pos, sceneObjects[i]);	
			}
			// Plane
			if (sceneObjects[i].objectType == SceneObjectTypePlane) {
				normal = sceneObjects[i].objectProperties.xyz;
			}
			// Lighting
			float diffuse = lightDiffuse(normal, lightVec);
			float specular = lightSpecular(normal, lightVec, sceneObjects[i].specular);
			color = diffuse * sceneObjects[i].diffuse + specular;	
		}
	}

	if (id == -1)
		return color;

	id = objectID;

	// Shadows
	t = length(ubo.lightPos - pos);
	color *= calcShadow(pos, lightVec, id, t);
	
	// Fog
	color = fog(t, color);	
	
	// Reflect ray for next render pass
	reflectRay(rayD, normal);
	rayO = pos;	
	
	return color;
}

void main()
{
	ivec2 dim = imageSize(resultImage);
	vec2 uv = vec2(gl_GlobalInvocationID.xy) / dim;

	vec3 rayO = ubo.camera.pos;
	vec3 rayD = normalize(vec3((-1.0 + 2.0 * uv) * vec2(ubo.aspectRatio, 1.0), -1.0));
		
	// Basic color path
	int id = 0;
	vec3 finalColor = renderScene(rayO, rayD, id);
	
	// Reflection
	if (REFLECTIONS)
	{
		float reflectionStrength = REFLECTIONSTRENGTH;
		for (int i = 0; i < RAYBOUNCES; i++)
		{
			vec3 reflectionColor = renderScene(rayO, rayD, id);
			finalColor = (1.0 - reflectionStrength) * finalColor + reflectionStrength * mix(reflectionColor, finalColor, 1.0 - reflectionStrength);			
			reflectionStrength *= REFLECTIONFALLOFF;
		}
	}
			
	imageStore(resultImage, ivec2(gl_GlobalInvocationID.xy), vec4(finalColor, 0.0));
}
		)";
};

DEF_DEMO(ComputeShaderDemo, "Compute Shader Demo");
}