#include "Framebuffer.h"

#include "dojo_gl_header.h"

#include "Texture.h"
#include "Platform.h"
#include "Renderer.h"

using namespace Dojo;

Dojo::Framebuffer::~Framebuffer() {
	if (isCreated()) { //fbos are destroyed on unload, the user must care to rebuild their contents after a purge
		glDeleteFramebuffers(1, &mFBO);
	}
}

void Framebuffer::addColorAttachment(Texture& texture, uint8_t miplevel /*= 0*/) {
	DEBUG_ASSERT(!isCreated(), "Already configured. Too late");
	mColorAttachments.emplace_back(Attachment{ &texture, miplevel });
}

uint32_t Framebuffer::getWidth() const {
	if (isBackbuffer()) {
		return Platform::singleton().getRenderer().getBackbuffer().getWidth();
	}
	else {
		auto& attach = mColorAttachments[0];
		return attach.texture->getWidth() >> attach.miplevel;
	}
}

uint32_t Framebuffer::getHeight() const {
	if (isBackbuffer()) {
		return Platform::singleton().getRenderer().getBackbuffer().getHeight();
	}
	else {
		auto& attach = mColorAttachments[0];
		return attach.texture->getHeight() >> attach.miplevel;
	}
}

void Framebuffer::bind() {

	if (isBackbuffer()) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glFrontFace(GL_CCW);
		glReadBuffer(GL_BACK);
		GLenum buffer[] = { GL_BACK };
		glDrawBuffers(1, buffer);
	}
	else {
		if (not isCreated()) {
			//create the framebuffer and attach all the stuff

			glGenFramebuffers(1, &mFBO);
			glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

			auto width = getWidth();
			auto height = getHeight();
			uint32_t i = 0;
			for (auto&& color : mColorAttachments) {
				color.texture->_addAsAttachment(i, width, height, color.miplevel);
				mAttachmentList.push_back(GL_COLOR_ATTACHMENT0 + i);
				++i;
			}

			if (mHasDepth) {
				glGenRenderbuffers(1, &mDepthBuffer);
				glBindRenderbuffer(GL_RENDERBUFFER, mDepthBuffer);
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthBuffer);
				mAttachmentList.push_back(GL_DEPTH_ATTACHMENT);
			}

			auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			DEBUG_ASSERT(status == GL_FRAMEBUFFER_COMPLETE, "The framebuffer is incomplete");
		}
		else {
			glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
		}

		glFrontFace(GL_CW); //invert vertex winding when inverting the view
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glDrawBuffers(mAttachmentList.size() - mHasDepth, mAttachmentList.data());
	}
}

void Framebuffer::invalidate() {
	if (not isBackbuffer()) {
		bind();

		glInvalidateFramebuffer(GL_FRAMEBUFFER, mAttachmentList.size(), mAttachmentList.data());
	}
}
