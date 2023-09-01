#pragma once

#include <thread>;

#include <SFML/Graphics.hpp>;
#include "Rotation.hpp";
#include "Scene.hpp";
#include "Ray.hpp";

namespace Manta {

	class CameraData {
	public:
		sf::Vector3f position;
		sf::Vector3f rotation;

		sf::Vector2u dimensions;

		float clampThreshold = .01f;
		float maxDistance = 100;

		float fov = degToRad(45);

		Scene* targetScene;
	};



	class RenderHandler abstract {
	public:
		sf::Uint8* getBitmap() { return this->bitmap; };
		virtual void onFinish() = 0;

	protected:
		sf::Uint8* bitmap;
		CameraData* cameraData;

		RenderHandler(CameraData* cameraData) {
			this->cameraData = cameraData;

			this->bitmap = new sf::Uint8[cameraData->dimensions.x * cameraData->dimensions.y * 4];
		}

		~RenderHandler() {
			delete[] this->bitmap;
		}
	};

	class DirectRenderHandler : public RenderHandler {
	public:

		void update() {
			this->tex.update(this->bitmap);
			this->targetWindow->clear();
			this->targetWindow->draw(sprite);
			this->targetWindow->display();
		}

		void onFinish() override {

		}

		DirectRenderHandler(CameraData* cameraData, sf::RenderWindow* targetWindow) : RenderHandler(cameraData) {
			this->targetWindow = targetWindow;

			this->tex = sf::Texture();

			sf::Vector2u wSize = targetWindow->getSize();
			this->tex.create(wSize.x, wSize.y);
			this->sprite = sf::Sprite();
			this->sprite.setTexture(this->tex);
		}

	private:
		sf::Texture tex;
		sf::Sprite sprite;
		sf::RenderWindow* targetWindow;
	};




	class Camera {
	public:
		
		CameraData* cameraData;

		static inline sf::Vector3f getVector(float xFactor, float yFactor, float fov, sf::Vector3f rotation) {
			sf::Vector3f straightForward(1, 0, 0);

			// Rotate by factors
			straightForward = rotateZ(&straightForward, fov * yFactor * .5f);
			straightForward = rotateY(&straightForward, fov * xFactor * .5f);

			// Rotate by camera rotation
			straightForward = rotateY(&straightForward, rotation.y);
			straightForward = rotateX(&straightForward, rotation.x);
			straightForward = rotateZ(&straightForward, rotation.z);

			return straightForward;
		}

		static inline sf::Vector2f fragToFactor(sf::Vector2i frag, sf::Vector2u dimensions) {
			sf::Vector2f fFrag(frag.x, frag.y);
			return sf::Vector2f(
				((float)((fFrag.x - dimensions.x / 2) / dimensions.x)) * 2,
				((float)((fFrag.y - dimensions.y / 2) / dimensions.x)) * 2
			);
		}

		void testTexture() {
			// Get bitmap from handler
			sf::Uint8* bitmap = this->renderHandler->getBitmap();

			// Render
			for (unsigned int x = 0; x < this->cameraData->dimensions.x; x++) {
				for (unsigned int y = 0; y < this->cameraData->dimensions.y; y++) {
					unsigned int offset = x + y * this->cameraData->dimensions.x;

					sf::Color frag = sf::Color(x%255, y%255, 0);

					bitmap[offset * 4] = frag.r;
					bitmap[offset * 4 + 1] = frag.g;
					bitmap[offset * 4 + 2] = frag.b;
					bitmap[offset * 4 + 3] = 255;
				}
			}

			// Finish
			this->renderHandler->onFinish();
		}
		
		void render() {
			std::thread worker(&Camera::renderFrame, this);
			worker.detach();
		}

		sf::Color cast(sf::Vector2i pixelCoordinate) {
			sf::Vector2f factor = fragToFactor(pixelCoordinate, this->cameraData->dimensions);

			Ray ray(this->cameraData->position, getVector(factor.x, factor.y, this->cameraData->fov, this->cameraData->rotation), this->cameraData->targetScene);

			while (ray.step() > this->cameraData->clampThreshold) {
				if (ray.distance >= this->cameraData->maxDistance) {
					return this->cameraData->targetScene->getSkyColor();
				}
			}
			return this->cameraData->targetScene->getColorAt(ray.getPosition());
		}

		void renderFrame() {
			// Get bitmap
			sf::Uint8* bitmap = this->renderHandler->getBitmap();

			float initialSceneIndex = this->cameraData->targetScene->sceneIndex(
				this->cameraData->position,
				this->cameraData->targetScene->getShapes()
			);

			// Render
			for (unsigned int x = 0; x < this->cameraData->dimensions.x; x++) {
				for (unsigned int y = 0; y < this->cameraData->dimensions.y; y++) {
					unsigned int offset = x + y * this->cameraData->dimensions.x;

					sf::Color frag;

					// Copied from cast()
					sf::Vector2f factor = fragToFactor(sf::Vector2i(x, y), this->cameraData->dimensions);

					Ray ray(this->cameraData->position, getVector(factor.x, factor.y, this->cameraData->fov, this->cameraData->rotation), this->cameraData->targetScene);

					ray.manualStep(initialSceneIndex);

					while (ray.step() > this->cameraData->clampThreshold) {
						if (ray.distance >= this->cameraData->maxDistance) {
							frag = this->cameraData->targetScene->getSkyColor();
							break;
						}
					}
					if (ray.distance < this->cameraData->maxDistance) frag = this->cameraData->targetScene->getColorAt(ray.getPosition());
					// ----


					bitmap[offset * 4] = frag.r;
					bitmap[offset * 4 + 1] = frag.g;
					bitmap[offset * 4 + 2] = frag.b;
					bitmap[offset * 4 + 3] = 255;
				}
				//std::cout << x << "(" << ((float)x) / this->cameraData->dimensions.x << "%)" << std::endl;
			}

			// Finish
			this->renderHandler->onFinish();
		}

		


		Camera(CameraData* cameraData, RenderHandler* renderHandler) {
			this->cameraData = cameraData;
			this->renderHandler = renderHandler;
		}

	protected:
		RenderHandler* renderHandler;
	};

	class ThreadedCamera {
	public:

		CameraData* cameraData;

		static inline sf::Vector3f getVector(float xFactor, float yFactor, float fov, sf::Vector3f rotation) {
			sf::Vector3f straightForward(1, 0, 0);

			// Rotate by factors
			straightForward = rotateZ(&straightForward, fov * yFactor * .5f);
			straightForward = rotateY(&straightForward, fov * xFactor * .5f);

			// Rotate by camera rotation
			straightForward = rotateY(&straightForward, rotation.y);
			straightForward = rotateX(&straightForward, rotation.x);
			straightForward = rotateZ(&straightForward, rotation.z);

			return straightForward;
		}

		static inline sf::Vector2f fragToFactor(sf::Vector2i frag, sf::Vector2u dimensions) {
			sf::Vector2f fFrag(frag.x, frag.y);
			return sf::Vector2f(
				((float)((fFrag.x - dimensions.x / 2) / dimensions.x)) * 2,
				((float)((fFrag.y - dimensions.y / 2) / dimensions.x)) * 2
			);
		}

		void testTexture() {
			// Get bitmap from handler
			sf::Uint8* bitmap = this->renderHandler->getBitmap();

			// Render
			for (unsigned int x = 0; x < this->cameraData->dimensions.x; x++) {
				for (unsigned int y = 0; y < this->cameraData->dimensions.y; y++) {
					unsigned int offset = x + y * this->cameraData->dimensions.x;

					sf::Color frag = sf::Color(x % 255, y % 255, 0);

					bitmap[offset * 4] = frag.r;
					bitmap[offset * 4 + 1] = frag.g;
					bitmap[offset * 4 + 2] = frag.b;
					bitmap[offset * 4 + 3] = 255;
				}
			}

			// Finish
			this->renderHandler->onFinish();
		}

		void render() {
			std::thread manager(&ThreadedCamera::initWorkers, this);
			manager.detach();
		}

		void initWorkers() {
			unsigned int subframeWidth = this->cameraData->dimensions.x / this->nThreads;

			std::vector<std::thread> workers;

			float initialSceneIndex = this->cameraData->targetScene->sceneIndex(
				this->cameraData->position,
				this->cameraData->targetScene->getShapes()
			);

			for (unsigned short i = 0; i < this->nThreads; i++) {
				unsigned int start = subframeWidth * i + (i > 0 ? 1 : 0);
				unsigned int end = i == this->nThreads - 1 ?
					this->cameraData->dimensions.x - 1 :
					subframeWidth * (i + 1);

				workers.push_back(std::thread(&ThreadedCamera::renderSubframe, this, start, end, initialSceneIndex));
				
			}

			while (!workers.empty()) {
				workers.back().join();
				workers.pop_back();
			}

			this->renderHandler->onFinish();
		}

		sf::Color cast(sf::Vector2i pixelCoordinate) {
			sf::Vector2f factor = fragToFactor(pixelCoordinate, this->cameraData->dimensions);

			Ray ray(this->cameraData->position, getVector(factor.x, factor.y, this->cameraData->fov, this->cameraData->rotation), this->cameraData->targetScene);

			while (ray.step() > this->cameraData->clampThreshold) {
				if (ray.distance >= this->cameraData->maxDistance) {
					return this->cameraData->targetScene->getSkyColor();
				}
			}
			return this->cameraData->targetScene->getColorAt(ray.getPosition());
		}

		void renderSubframe(unsigned int startCol, unsigned int endCol, float initialSceneIndex) {
			// Get bitmap
			sf::Uint8* bitmap = this->renderHandler->getBitmap();

			// Render
			for (unsigned int x = startCol; x <= endCol; x++) {
				for (unsigned int y = 0; y < this->cameraData->dimensions.y; y++) {
					unsigned int offset = x + y * this->cameraData->dimensions.x;

					sf::Color frag;

					// Copied from cast()
					sf::Vector2f factor = fragToFactor(sf::Vector2i(x, y), this->cameraData->dimensions);

					Ray ray(this->cameraData->position, getVector(factor.x, factor.y, this->cameraData->fov, this->cameraData->rotation), this->cameraData->targetScene);

					ray.manualStep(initialSceneIndex);

					while (ray.step() > this->cameraData->clampThreshold) {
						if (ray.distance >= this->cameraData->maxDistance) {
							frag = this->cameraData->targetScene->getSkyColor();
							break;
						}
					}
					if (ray.distance < this->cameraData->maxDistance) frag = this->cameraData->targetScene->getColorAt(ray.getPosition());
					// ----


					bitmap[offset * 4] = frag.r;
					bitmap[offset * 4 + 1] = frag.g;
					bitmap[offset * 4 + 2] = frag.b;
					bitmap[offset * 4 + 3] = 255;
				}
				//std::cout << x << "(" << ((float)x) / this->cameraData->dimensions.x << "%)" << std::endl;
			}
		}




		ThreadedCamera(CameraData* cameraData, RenderHandler* renderHandler, unsigned short nThreads) {
			this->cameraData = cameraData;
			this->renderHandler = renderHandler;

			this->nThreads = nThreads;
		}

	protected:
		RenderHandler* renderHandler;

		unsigned short nThreads;
	};
}