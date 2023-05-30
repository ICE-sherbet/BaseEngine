#include "IFieldStorage.h"

#include "MonoGCManager.h"
#include "MonoScriptUtilities.h"
namespace base_engine {
FieldStorage::FieldStorage(MonoFieldInfo* field_info): IFieldStorage(field_info)
{
	data_ = Variant{field_info->field_info.type};
}

GCHandle FieldStorage::GetInstance() const
{ return instance_; }

void FieldStorage::SetRuntimeInstance(const GCHandle instance)
{
	instance_ = instance;
	if (instance_ == nullptr) return;
	SetValueRuntime(&data_);
}

Variant FieldStorage::GetValueVariant() const
{
	if (instance_ == nullptr) {
		return data_;
	}
	auto value = Variant{field_info_->field_info.type};
	GetValueRuntime(&value);
	return value;
}

void FieldStorage::SetValueVariant(const Variant& buffer)
{
	if (instance_ != nullptr) {
		SetValueRuntime(&buffer);
	} else {
		data_ = buffer;
	}
}

void FieldStorage::SetValueRuntime(const Variant* in_buffer) const {
  if (instance_ == nullptr) return;

  MonoObject* runtime_object = MonoGCManager::GetReferencedObject(instance_);
  MonoScriptUtilities::SetFieldVariant(runtime_object, field_info_, *in_buffer);
}

bool FieldStorage::GetValueRuntime(Variant* out_buffer) const {
  if (instance_ == nullptr) return false;

  MonoObject* runtime_object = MonoGCManager::GetReferencedObject(instance_);
  if (runtime_object == nullptr) return false;

  (*out_buffer) =
      MonoScriptUtilities::GetFieldVariant(runtime_object, field_info_, false);
  return true;
}
}  // namespace base_engine
