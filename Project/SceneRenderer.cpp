#include "SceneRenderer.h"

#include "ObjectEntity.h"
#include "Scene.h"


namespace base_engine
{
SceneRenderer::SceneRenderer(Ref<Scene> scene, SceneRendererSpecification specification)
{
	scene_ = scene;
	specification_ = specification;

	Init();
}

void SceneRenderer::Init()
{
	command_buffer_ = RenderCommandBuffer::Create(0,"Scene Renderer");

}
}
