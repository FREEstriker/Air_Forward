#include "Test/MeshRendererBehaviour.h"
#include "Logic/Component/Renderer/MeshRenderer.h"
#include "Logic/Object/GameObject.h"
#include "Logic/Core/Instance.h"
#include <algorithm>
#include <Graphic/Asset/TextureCube.h>
#include <rttr/registration>
RTTR_REGISTRATION
{
	rttr::registration::class_<Test::MeshRendererBehaviour>("Test::MeshRendererBehaviour");
}

Test::MeshRendererBehaviour::MeshRendererBehaviour()
	: meshTask()
	, shaderTask()
	, texture2DTask()
	, normalTexture2DTask()
	, loaded(false)
	, mesh(nullptr)
	, shader(nullptr)
	, texture2d(nullptr)
	, normalTexture2d(nullptr)
	, material(nullptr)
	, rotationSpeed(0.5235987755982988f)
{
}

Test::MeshRendererBehaviour::~MeshRendererBehaviour()
{
}

void Test::MeshRendererBehaviour::OnAwake()
{
}

void Test::MeshRendererBehaviour::OnStart()
{
	meshTask = Graphic::Asset::Mesh::LoadAsync("..\\Asset\\Mesh\\DefaultMesh.ply");
	shaderTask = Graphic::Asset::Shader::LoadAsync("..\\Asset\\Shader\\DefaultShader.shader");
	texture2DTask = Graphic::Asset::Texture2D::LoadAsync("..\\Asset\\Texture\\DefaultTexture2D.json");
	normalTexture2DTask = Graphic::Asset::Texture2D::LoadAsync("..\\Asset\\Texture\\DefaultNormalTexture2D.json");
}

void Test::MeshRendererBehaviour::OnUpdate()
{
	if (!loaded && meshTask._Is_ready() && shaderTask._Is_ready() && texture2DTask._Is_ready())
	{
		mesh = meshTask.get();
		shader = shaderTask.get();
		texture2d = texture2DTask.get();
		normalTexture2d = normalTexture2DTask.get();
		material = new Graphic::Material(shader);
		material->SetTexture2D("albedo", texture2d);
		material->SetTexture2D("normalTexture", normalTexture2d);

		loaded = true;

		auto meshRenderer = GameObject()->GetComponent<Logic::Component::Renderer::Renderer>();
		meshRenderer->material = material;
		meshRenderer->mesh = mesh;
	}

	auto rotation = GameObject()->transform.Rotation();
	rotation.x = std::fmod(rotation.x + rotationSpeed * 0.3f * static_cast<float>(Logic::Core::Instance::time.DeltaDuration()), 6.283185307179586f);
	rotation.y = std::fmod(rotation.y + rotationSpeed * 0.6f * static_cast<float>(Logic::Core::Instance::time.DeltaDuration()), 6.283185307179586f);
	rotation.z = std::fmod(rotation.z + rotationSpeed * static_cast<float>(Logic::Core::Instance::time.DeltaDuration()), 6.283185307179586f);
	GameObject()->transform.SetRotation(rotation);
}

void Test::MeshRendererBehaviour::OnDestroy()
{
}
