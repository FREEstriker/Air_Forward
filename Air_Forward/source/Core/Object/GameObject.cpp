#include "Core/Object/GameObject.h"
#include "Core/Component/Component.h"
#include <cassert>
#include <rttr/registration>
#include "Utils/Log.h"

RTTR_REGISTRATION
{
	using namespace rttr;
	registration::class_<Core::Object::GameObject>("Core::Object::GameObject")
		.method("GetComponent", &Core::Object::GameObject::GetComponent)
		.method("GetComponents", &Core::Object::GameObject::GetComponents)
		.method("AddComponent", &Core::Object::GameObject::AddComponent)
		.method("RemoveComponent", select_overload<void(Core::Component::Component*)>(&Core::Object::GameObject::RemoveComponent))
		.method("RemoveComponent", select_overload<Core::Component::Component*(std::string)>(&Core::Object::GameObject::RemoveComponent))
		.method("RemoveComponents", &Core::Object::GameObject::RemoveComponents)
		.method("Parent", &Core::Object::GameObject::Parent)
		.method("Child", &Core::Object::GameObject::Child)
		.method("Brother", &Core::Object::GameObject::Brother)
		.method("AddChild", &Core::Object::GameObject::AddChild)
		.method("RemoveChild", &Core::Object::GameObject::RemoveChild)
		.method("RemoveSelf", &Core::Object::GameObject::RemoveSelf)
		;
}

Core::Object::GameObject::GameObject(std::string name)
	: LifeCycle()
	, Object()
	, name(name)
	, _components()
	, transform()
	, _chain()
{
	_active = true;

	_chain.SetObject(this);

	OnAwake();
}

Core::Object::GameObject::GameObject()
	: GameObject("New GameObject")
{
}

Core::Object::GameObject::~GameObject()
{
	_components.clear();
}

void Core::Object::GameObject::AddComponent(Core::Component::Component* targetComponent)
{
	_components.push_back(targetComponent);
	targetComponent->_gameObject = this;

	if (_active && targetComponent->_active)
	{
		targetComponent->OnEnable();
		targetComponent->OnStart();
	}
}

void Core::Object::GameObject::RemoveComponent(Core::Component::Component* targetComponent)
{
	for (auto iter = _components.begin(), end = _components.end(); iter < end; iter++)
	{
		Core::Component::Component* iterComponent = *iter;
		if (iterComponent == targetComponent)
		{
			if (_active && targetComponent->_active)
			{
				targetComponent->OnDisable();
			}

			targetComponent->_gameObject = nullptr;
			_components.erase(iter);
			return;
		}
	}
	Utils::Log::Exception("GameObject " + name + " do not have the " + targetComponent->TypeName() + " component.");
}

Core::Component::Component* Core::Object::GameObject::RemoveComponent(std::string targetTypeName)
{
	using namespace rttr;

	type targetType = type::get_by_name(targetTypeName);
	type componentType = type::get<Core::Component::Component>();

	if (targetType)
	{
		if (!targetType.is_derived_from(componentType))
		{
			Utils::Log::Exception(targetTypeName + " is not a component.");
		}
		for (auto iter = _components.begin(), end = _components.end(); iter != end; iter++)
		{
			Core::Component::Component* iterComponent = *iter;
			type iterType = type::get(*iterComponent);
			if (iterType == targetType || targetType.is_base_of(iterType))
			{
				if (_active && iterComponent->_active)
				{
					iterComponent->OnDisable();
				}

				iterComponent->_gameObject = nullptr;
				_components.erase(iter);

				return iterComponent;
			}
		}
	}
	else
	{
		Utils::Log::Exception("Do not have " + targetTypeName + ".");
	}
}

std::vector<Core::Component::Component*> Core::Object::GameObject::RemoveComponents(std::string targetTypeName)
{
	using namespace rttr;
	type targetType = type::get_by_name(targetTypeName);
	type componentType = type::get<Core::Component::Component>();
	std::vector<Core::Component::Component*> targetComponents = std::vector<Core::Component::Component*>();

	if (targetType)
	{
		if (!targetType.is_derived_from(componentType))
		{
			Utils::Log::Exception(targetTypeName + " is not a component.");
		}
		for (auto iter = _components.begin(); iter != _components.end(); )
		{
			Core::Component::Component* iterComponent = *iter;
			type iterType = type::get(*iterComponent);
			if (iterType == targetType || targetType.is_base_of(iterType))
			{
				if (_active && iterComponent->_active)
				{
					iterComponent->OnDisable();
				}

				iterComponent->_gameObject = nullptr;
				iter = _components.erase(iter);

				targetComponents.push_back(iterComponent);
			}
			else
			{
				++iter;
			}
		}
		return targetComponents;
	}
	else
	{
		Utils::Log::Exception("Do not have " + targetTypeName + ".");
	}
}

Core::Component::Component* Core::Object::GameObject::GetComponent(std::string typeName)
{
	using namespace rttr;

	type class_type = type::get_by_name(typeName);
	type class_component = type::get<Core::Component::Component>();

	if (class_type)
	{
		if (!class_component.is_derived_from(class_component))
		{
			Utils::Log::Exception(typeName + " is not a component.");
		}
		for (auto component : _components)
		{
			type class_target = type::get(*component);
			if (class_target == class_type || class_type.is_base_of(class_target))
			{
				return component;
			}
		}
		Utils::Log::Exception("GameObject " + name + " do not have a " + typeName + " component.");
	}
	else
	{
		Utils::Log::Exception("Do not have " + typeName + ".");
	}
}

std::vector<Core::Component::Component*> Core::Object::GameObject::GetComponents(std::string targetTypeName)
{
	using namespace rttr;
	type targetType = type::get_by_name(targetTypeName);
	type componentType = type::get<Core::Component::Component>();
	std::vector<Core::Component::Component*> targetComponents = std::vector<Core::Component::Component*>();

	if (targetType)
	{
		if (!targetType.is_derived_from(componentType))
		{
			Utils::Log::Exception(targetTypeName + " is not a component.");
		}
		for (auto component : _components)
		{
			type iterType = type::get(*component);
			if (iterType == targetType || targetType.is_base_of(iterType))
			{
				targetComponents.push_back(component);
			}
		}
		return targetComponents;
	}
	else
	{
		Utils::Log::Exception("Do not have " + targetTypeName + ".");
	}
}


Core::Object::GameObject* Core::Object::GameObject::Parent()
{
	return &_chain.Parent().Object();
}
Core::Object::GameObject* Core::Object::GameObject::Child()
{
	return &_chain.Child().Object();
}
Core::Object::GameObject* Core::Object::GameObject::Brother()
{
	return &_chain.Brother().Object();
}
void Core::Object::GameObject::AddChild(Core::Object::GameObject* child)
{
	this->_chain.AddChild(child->_chain);
}

void Core::Object::GameObject::RemoveChild(Core::Object::GameObject* child)
{
	if (child->Parent() == this)
	{
		child->RemoveSelf();
	}
}

void Core::Object::GameObject::RemoveSelf()
{
	_chain.Remove();
}


bool Core::Object::GameObject::OnCheckValid()
{
	return Parent();
}

void Core::Object::GameObject::OnAwake()
{
}

void Core::Object::GameObject::OnEnable()
{
	for (const auto& component : _components)
	{
		if (component->_active)
		{
			component->OnEnable();
		}
	}
	for (Utils::ChildBrotherTree<Core::Object::GameObject>::Iterator iter = _chain.GetChildIterator(); iter.IsValid(); ++iter)
	{
		GameObject* go = &iter.Node().Object();
		if (go->_active)
		{
			go->OnEnable();
		}
	}
}

void Core::Object::GameObject::OnDisable()
{
	for (const auto& component : _components)
	{
		if (component->_active)
		{
			component->OnDisable();
		}
	}
	for (Utils::ChildBrotherTree<Core::Object::GameObject>::Iterator iter = _chain.GetChildIterator(); iter.IsValid(); ++iter)
	{
		GameObject* go = &iter.Node().Object();
		if (go->_active)
		{
			go->OnDisable();
		}
	}
}

void Core::Object::GameObject::OnDestory()
{
}
