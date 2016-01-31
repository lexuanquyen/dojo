#include "RenderState.h"

#include "Texture.h"
#include "Mesh.h"
#include "FrameSet.h"
#include "Timer.h"
#include "Shader.h"

using namespace Dojo;

RenderState::TextureUnit::TextureUnit() :
	scale(1, 1),
	rotation(0),
	offset(0, 0) {

}

RenderState::TextureUnit::TextureUnit(Texture* t) :
	TextureUnit() {
	texture = t;
}

RenderState::TextureUnit::~TextureUnit() {

}


Matrix RenderState::TextureUnit::getTransform() const {
	if (hasTextureTransform) {
		//build the transform
		Matrix m = glm::scale(Matrix(1), scale);
		m = glm::translate(m, offset);
		m = glm::rotate(m, (float)(Degrees)rotation, Vector::UnitZ);
		return m;
	}
	else {
		return Matrix(1);
	}
}

RenderState::RenderState() :
	cullMode(CullMode::Back),
	blendingEnabled(true),
	srcBlend(GL_SRC_ALPHA),
	destBlend(GL_ONE_MINUS_SRC_ALPHA),
	blendFunction(GL_FUNC_ADD) {
	memset(textures, 0, sizeof(textures)); //zero all the textures
}

RenderState::~RenderState() {

}

void RenderState::setTexture(Texture* tex, int ID /*= 0 */) {
	DEBUG_ASSERT(ID >= 0, "Passed a negative texture ID to setTexture()");
	DEBUG_ASSERT(ID < DOJO_MAX_TEXTURES, "An ID passed to setTexture must be smaller than DOJO_MAX_TEXTURE_UNITS");

	textures[ID] = TextureUnit(tex);
}

void RenderState::setBlending(BlendingMode mode) {
	struct GLBlend {
		GLenum src, dest, func;
	};

	static const GLBlend modeToGLTable[] = {
		{GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD}, //alphablend
		{GL_DST_COLOR, GL_ZERO, GL_FUNC_ADD}, //multiply
		{GL_ONE, GL_ONE, GL_FUNC_ADD}, //add
		{GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_FUNC_ADD}, //invert
		{GL_ONE, GL_ONE, GL_FUNC_SUBTRACT} //subtract
	};

	auto& blend = modeToGLTable[(int)mode];

	setBlending(blend.src, blend.dest);
	blendFunction = blend.func;
}

void Dojo::RenderState::setShader(Shader& shader) {
	pShader = shader;
}

Texture* RenderState::getTexture(int ID /*= 0 */) const {
	DEBUG_ASSERT(ID >= 0, "Can't retrieve a negative texture ID");
	DEBUG_ASSERT(ID < DOJO_MAX_TEXTURES, "An ID passed to getTexture must be smaller than DOJO_MAX_TEXTURE_UNITS");

	return textures[ID].texture;
}

const RenderState::TextureUnit& RenderState::getTextureUnit(int ID) const {
	DEBUG_ASSERT(ID >= 0, "Can't retrieve a negative textureUnit");
	DEBUG_ASSERT(ID < DOJO_MAX_TEXTURES, "An ID passed to getTextureUnit must be smaller than DOJO_MAX_TEXTURE_UNITS");

	return textures[ID];
}

int RenderState::getDistance(RenderState* s) {
	DEBUG_ASSERT(s, "getDistance: The input RenderState is null");

	int dist = 0;

	DEBUG_TODO; //dunno
	//
	// 	if (s->mesh != mesh)
	// 		dist += 3;
	//
	// 	for (int i = 0; i < DOJO_MAX_TEXTURES; ++i) {
	// 		if (textures[i] != s->textures[i])
	// 			dist += 2;
	// 	}
	//
	// 	if (s->isAlphaRequired() != isAlphaRequired())
	// 		dist += 1;

	return dist;
}

void RenderState::applyState() {
	for (int i = 0; i < DOJO_MAX_TEXTURES; ++i) {
		//select current slot
		glActiveTexture(GL_TEXTURE0 + i);

		if (textures[i].texture) {
			textures[i].texture->bind(i);

			if (textures[i].isTransformRequired()) {
				FAIL("To remove?");
			}
		}
	}

	if (blendingEnabled) {
		glEnable(GL_BLEND);
	}
	else {
		glDisable(GL_BLEND);
	}

	glBlendFunc(srcBlend, destBlend);
	glBlendEquation(blendFunction);

	switch (cullMode) {
	case CullMode::None:
		glDisable(GL_CULL_FACE);
		break;

	case CullMode::Back:
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		break;

	case CullMode::Front:
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		break;
	}

	mesh.unwrap().bind(pShader.unwrap());
}

void RenderState::commitChanges() {
	//TODO incremental state switches please!
	applyState();
}
