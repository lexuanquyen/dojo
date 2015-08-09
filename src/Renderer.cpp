#include "Renderer.h"

#include "Renderable.h"
#include "TextArea.h"
#include "Platform.h"
#include "Viewport.h"
#include "Mesh.h"
#include "AnimatedQuad.h"
#include "Shader.h"

#include "Game.h"
#include "Texture.h"

using namespace Dojo;

Renderer::Renderer(int w, int h, Orientation deviceOr) :
	frameStarted(false),
	valid(true),
	width(w),
	height(h),
	renderOrientation(DO_LANDSCAPE_RIGHT),
	deviceOrientation(deviceOr),
	frameVertexCount(0),
	frameTriCount(0),
	frameBatchCount(0) {
	DEBUG_MESSAGE( "Creating OpenGL context...");
	DEBUG_MESSAGE ("querying GL info... ");
	DEBUG_MESSAGE ("vendor: " + std::string( (const char*)glGetString (GL_VENDOR)));
	DEBUG_MESSAGE ("renderer: " + std::string( (const char*)glGetString (GL_RENDERER)));
	DEBUG_MESSAGE ("version: OpenGL " + std::string( (const char*)glGetString (GL_VERSION)));

	//clean errors (some drivers leave errors on the stack)
	CHECK_GL_ERROR;
	CHECK_GL_ERROR;

	glEnable(GL_RESCALE_NORMAL);
	glEnable(GL_NORMALIZE);
	glEnable(GL_CULL_FACE);

	glCullFace(GL_BACK);

	//default status for blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#ifdef DOJO_GAMMA_CORRECTION_ENABLED
	glEnable( GL_FRAMEBUFFER_SRGB );
#endif

	setInterfaceOrientation(Platform::singleton().getGame().getNativeOrientation());

	CHECK_GL_ERROR;
}

Renderer::~Renderer() {
	clearLayers();
}

RenderLayer& Renderer::getLayer(RenderLayer::ID layerID) {
	//layers "always" exist
	if (layerID >= layers.size()) {
		layers.resize(layerID + 1);
	}

	return layers[layerID];
}

bool Renderer::hasLayer(RenderLayer::ID layerID) {
	return layerID < layers.size();
}

void Renderer::addRenderable(Renderable& s) {
	//get the needed layer
	RenderLayer& layer = getLayer(s.getLayer());

	//append at the end
	layer.elements.emplace(&s);
}

void Renderer::removeRenderable(Renderable& s) {
	if (hasLayer(s.getLayer())) {
		getLayer(s.getLayer()).elements.erase(&s);
	}
}

void Renderer::removeAllRenderables() {
	for (auto&& l : layers) {
		l.elements.clear();
	}
}

void Renderer::removeViewport(const Viewport& v) {

}

void Renderer::removeAllViewports() {
	viewportList.clear();
}

void Renderer::clearLayers() {
	layers.clear();
}

void Renderer::addViewport(Viewport& v) {
	viewportList.insert(&v);
}

void Renderer::setInterfaceOrientation(Orientation o) {
	renderOrientation = o;

	static const Degrees orientations[] = {
		0.0_deg,
		180.0_deg,
		90.0_deg,
		-90.0_deg
	};

	renderRotation = orientations[(int)renderOrientation] + orientations[(int)deviceOrientation];

	//compute matrix
	mRenderRotation = glm::mat4_cast(Quaternion(Vector(0, 0, renderRotation)));
}

void Renderer::_renderElement(Renderable& elem) {
	Mesh* m = elem.getMesh();

	DEBUG_ASSERT( frameStarted, "Tried to render an element but the frame wasn't started" );
	DEBUG_ASSERT(m, "Cannot render without a mesh!");
	DEBUG_ASSERT(m->isLoaded(), "Rendering with a mesh with no GPU data!");
	DEBUG_ASSERT(m->getVertexCount() > 0, "Rendering a mesh with no vertices");

#ifndef PUBLISH
	frameVertexCount += m->getVertexCount();
	frameTriCount += m->getPrimitiveCount();

	//each renderable is a single batch
	++frameBatchCount;
#endif // !PUBLISH

	currentState.world = glm::scale(elem.getObject().getWorldTransform(), elem.scale);
	currentState.worldView = currentState.view * currentState.world;
	currentState.worldViewProjection = currentState.projection * currentState.worldView;

	elem.getShader().use(elem);

	elem.commitChanges();

	static const GLenum glModeMap[] = {
		GL_TRIANGLE_STRIP, //TriangleStrip,
		GL_TRIANGLES, //TriangleList,
		GL_LINE_STRIP, //LineStrip,
		GL_LINES, //LineList
		GL_POINTS
	};

	GLenum mode = glModeMap[(byte)m->getTriangleMode()];

	if (!m->isIndexed()) {
		glDrawArrays(mode, 0, m->getVertexCount());
	}
	else {
		DEBUG_ASSERT(m->getIndexCount() > 0, "Rendering an indexed mesh with no indices");
		glDrawElements(mode, m->getIndexCount(), m->getIndexGLType(), nullptr); //on OpenGLES, we have max 65536 indices!!!
	}

#ifndef DOJO_DISABLE_VAOS
	glBindVertexArray( 0 );
#endif

#ifdef DOJO_SHADERS_AVAILABLE

	//HACK //TODO remove fixed function pipeline (it breaks if generic arrays are set)
	if (elem.getShader()) {
		for (auto&& attr : elem.getShader()->getAttributes()) {
			glDisableVertexAttribArray(attr.second.location);
		}
	}

#endif
}

bool _cull(const RenderLayer& layer, const Viewport& viewport, const Renderable& r) {
	return layer.orthographic ? viewport.isInViewRect(r) : viewport.isContainedInFrustum(r);
}

void Renderer::_renderLayer(Viewport& viewport, const RenderLayer& layer) {
	if (!layer.elements.size() || !layer.visible) {
		return;
	}

#ifdef DOJO_WIREFRAME_AVAILABLE
	glPolygonMode(GL_FRONT_AND_BACK, layer.wireframe ? GL_LINE : GL_FILL);
#endif

	//make state changes
	if (layer.depthCheck) {
		glEnable(GL_DEPTH_TEST);
	}
	else {
		glDisable(GL_DEPTH_TEST);
	}

	//set projection state
	currentState.projection = mRenderRotation * (layer.orthographic ? viewport.getOrthoProjectionTransform() : viewport.getPerspectiveProjectionTransform());

	//we don't want different layers to be depth-checked together?
// 	if (layer.depthClear) {
// 		glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
// 	}

	for (auto&& r : layer.elements) {
		if (r->canBeRendered() && _cull(layer, viewport, *r)) {
			_renderElement(*r);
		}
	}
}

void Renderer::_renderViewport(Viewport& viewport) {
	Texture* rt = viewport.getRenderTarget();

	if (rt) {
		rt->bindAsRenderTarget(true);    //TODO guess if this viewport doesn't render 3D layers to save memory?
	}
	else {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	glFrontFace(rt ? GL_CW : GL_CCW); //invert vertex winding when inverting the view

	currentState.targetDimension.x = (float)(rt ? rt->getWidth() : width);
	currentState.targetDimension.y = (float)(rt ? rt->getHeight() : height);

	glViewport(0, 0, (GLsizei) currentState.targetDimension.x, (GLsizei)currentState.targetDimension.y);

	//clear the viewport
	if (viewport.getClearEnabled()) {
		glClearColor(
			viewport.getClearColor().r,
			viewport.getClearColor().g,
			viewport.getClearColor().b,
			viewport.getClearColor().a);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	currentState.view = viewport.getViewTransform();
	currentState.viewDirection = viewport.getWorldDirection();

	if (viewport.getVisibleLayers().empty()) { //using the default layer ordering/visibility
		for (auto&& l : layers) {
			_renderLayer(viewport, l);
		}
	}
	else { //use the custom layer ordering/visibility
		for (auto&& layer : viewport.getVisibleLayers()) {
			_renderLayer(viewport, getLayer(layer));
		}
	}
}

void Renderer::_updateRenderables(const LayerList& layers, float dt) {
	for (auto&& layer : layers) {
		for (auto&& r : layer.elements) {
			if ((r->getObject().isActive() && r->isVisible()) || r->getGraphicsAABB().isEmpty()) {
				r->update(dt);
			}
		}
	}
}

void Renderer::renderFrame(float dt) {
	DEBUG_ASSERT( !frameStarted, "Tried to start rendering but the frame was already started" );

	frameVertexCount = frameTriCount = frameBatchCount = 0;
	frameStarted = true;

	//update all the renderables
	_updateRenderables(layers, dt);

	//render all the viewports
	for (auto&& viewport : viewportList) {
		_renderViewport(*viewport);
	}

	frameStarted = false;
}
