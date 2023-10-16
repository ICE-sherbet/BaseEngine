#include "Callable.h"

#include "ConnectableObject.h"


namespace base_engine
{

void Callable::Call(const Variant** arguments, int arg_count,
                    Variant& return_value) const {
  if (!object_)
  {
    return;
  }
  object_->Call(arguments, arg_count, return_value);
}
}
