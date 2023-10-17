// @WindowFactory.cpp
// @brief
// @author ICE
// @date 2023/10/17
//
// @details

#include "../../../IWindow.h"
#include "Window.h"

namespace base_engine {

IWindow* IWindow::Create() { return new Window(); }
}  // namespace base_engine