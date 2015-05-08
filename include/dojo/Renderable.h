/*
 *  Renderable.h
 *  Dojo Training
 *
 *  Created by Tommaso Checchi on 7/14/10.
 *  Copyright 2010 none. All rights reserved.
 *
 */

#pragma once

#include "dojo_common_header.h"

#include "RenderState.h"
#include "Color.h"
#include "AABB.h"

namespace Dojo {
	class Object;
	class Renderer;
	class GameState;

	class Renderable : public RenderState {
	public:

		Vector uvOffset;
		Vector scale;

		Renderable(Object& parent, Mesh* m = nullptr);

		Renderable(Object& parent, const String& meshName);

		virtual ~Renderable();

		Object& getObject() {
			return parent;
		}

		const Object& getObject() const {
			return parent;
		}

		GameState& getGameState();

		void setVisible(bool v) {
			visible = v;
		}

		///starts a linear fade on the color of this Renderable, from start to end and "duration" seconds long
		void startFade(const Color& start, const Color& end, float duration);

		///starts a linear fade on the alpha of this Renderable, from start to end and "duration" seconds long
		void startFade(float startAlpha, float endAlpha, float duration);

		///stops a current fade leaving the Renderable in the current state
		void stopFade();

		///returns the ID of the Render::Layer this object is assigned to
		int getLayer() const {
			return layer;
		}

		int getRenderingOrder() {
			return renderingOrder;
		}

		const AABB& getGraphicsAABB() const {
			return worldBB;
		}

		///true if this object has been assigned to a Render::Layer
		bool hasLayer() {
			return layer != INT_MIN ;
		}

		///tells if the object is either visible or has a mesh
		bool isVisible() const {
			return visible;
		}

		bool canBeRendered() const;

		bool isFading() const {
			return fading;
		}

		void advanceFade(float dt);

		virtual void update(float dt);

		void _notifyRenderInfo(int layerID, int renderIdx);
	protected:

		Object& parent;
		bool visible;

		int layer;
		int renderingOrder;

		bool fading;
		float currentFadeTime;
		float fadeEndTime;
		Color fadeStartColor;
		Color fadeEndColor;

		AABB worldBB;
	};
}
