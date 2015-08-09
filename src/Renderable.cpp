#include "Renderable.h"

#include "Game.h"
#include "Viewport.h"
#include "Mesh.h"
#include "GameState.h"
#include "Object.h"
#include "Platform.h"
#include "Renderer.h"

using namespace Dojo;

Renderable::Renderable(Object& parent, RenderLayer::ID layer) :
	Component(parent),
	layer(layer) {
	color = Color::White;
}

Renderable::Renderable(Object& parent, RenderLayer::ID layer, Mesh& m, Shader& shader) :
	Renderable(parent, layer) {
	mesh = &m;
	pShader = &shader;
}

Renderable::Renderable(Object& parent, RenderLayer::ID layer, const std::string& meshName, const std::string& shaderName) :
	Renderable(parent, layer) {
	DEBUG_ASSERT(meshName.size(), "Use another constructor if you don't want to supply a mesh");

	mesh = parent.getGameState().getMesh(meshName);
	DEBUG_ASSERT_INFO( mesh, "Tried to create a Renderable but the mesh wasn't found", "name = " + meshName );

	pShader = parent.getGameState().getShader(shaderName);
	DEBUG_ASSERT_INFO(pShader, "Tred to create a Renderable but the shader wasn't found", "name = " + shaderName);
}

Renderable::~Renderable() {

}

void Renderable::startFade(const Color& start, const Color& end, float duration) {
	DEBUG_ASSERT(duration > 0, "The duration of a fade must be greater than 0");

	fadeStartColor = start;
	fadeEndColor = end;

	color = start;

	currentFadeTime = 0;

	fadeEndTime = duration;

	fading = true;

	setVisible(true);
}

void Renderable::startFade(float startAlpha, float endAlpha, float duration) {
	color.a = startAlpha;

	Color end = color;
	end.a = endAlpha;

	startFade(color, end, duration);
}

void Renderable::update(float dt) {
	if (mesh) {
		AABB bounds = mesh->getBounds();
		bounds.max = Vector::mul(bounds.max, scale);
		bounds.min = Vector::mul(bounds.min, scale);
		worldBB = self.transformAABB(bounds);

		advanceFade(dt);
	}
}

bool Renderable::canBeRendered() const {
	return isVisible() && mesh && mesh->isLoaded() && mesh->getVertexCount() > 2;
}

void Renderable::stopFade() {
	fading = false;
}

void Renderable::advanceFade(float dt) {
	if (fading) { //fade is scheduled
		float fade = currentFadeTime / fadeEndTime;
		float invf = 1.f - fade;

		color.r = fadeEndColor.r * fade + invf * fadeStartColor.r;
		color.g = fadeEndColor.g * fade + invf * fadeStartColor.g;
		color.b = fadeEndColor.b * fade + invf * fadeStartColor.b;
		color.a = fadeEndColor.a * fade + invf * fadeStartColor.a;

		if (currentFadeTime > fadeEndTime) {
			fading = false;

			if (fadeEndColor.a == 0) {
				setVisible(false);
			}
		}

		currentFadeTime += dt;
	}
}

GameState& Dojo::Renderable::getGameState() const {
	return getObject().getGameState();
}

void Renderable::onAttach() {
	Platform::singleton().getRenderer().addRenderable(*this);
}

void Renderable::onDetach() {
	Platform::singleton().getRenderer().removeRenderable(*this);
}