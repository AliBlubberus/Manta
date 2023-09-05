#pragma once

#include <thread>;

#include <SFML/Graphics.hpp>;
#include "Rotation.hpp";
#include "Scene.hpp";
#include "Ray.hpp";

namespace Manta {

	inline float distanceAB(sf::Vector3f a, sf::Vector3f b) {
		return sqrtf(
			powf(b.x - a.x, 2) +
			powf(b.y - a.y, 2) +
			powf(b.z - a.z, 2)
		);
	}



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

		virtual void onStart() = 0;
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

	class MultipassRenderHandler : public RenderHandler {
	public:
		sf::Uint8* getAlbedo() { return this->albedo; };
		sf::Uint16* getLightMap() { return this->light; };

		sf::Uint8* getMist() { return this->mist; };
		sf::Uint8* getAO() { return this->ao; };

		virtual void onStart() = 0;
		virtual void onFinish() = 0;

	protected:
		sf::Uint8* comp;

		sf::Uint8* albedo;
		sf::Uint16* light;

		sf::Uint8* mist;
		sf::Uint8* ao;


		MultipassRenderHandler(CameraData* cameraData):
		RenderHandler(cameraData) {
			this->cameraData = cameraData;

			this->comp = new sf::Uint8[cameraData->dimensions.x * cameraData->dimensions.y * 4];

			this->albedo = new sf::Uint8[cameraData->dimensions.x * cameraData->dimensions.y * 4];
			this->light = new sf::Uint16[cameraData->dimensions.x * cameraData->dimensions.y * 4];

			this->mist = new sf::Uint8[cameraData->dimensions.x * cameraData->dimensions.y];
			this->ao = new sf::Uint8[cameraData->dimensions.x * cameraData->dimensions.y];

		}

		~MultipassRenderHandler() {
			delete[] this->comp;
			delete[] this->albedo;
			delete[] this->light;
			delete[] this->mist;
			delete[] this->ao;
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

		void onStart() override {

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

	class DirectMultipassRenderHandler : public MultipassRenderHandler {
	public:

		void update() {
			for (unsigned int i = 0; i < this->cameraData->dimensions.x * this->cameraData->dimensions.y; i++) {
				this->bitmap[i * 4] =
					std::min(this->light[i * 4], (sf::Uint16)255)
					* ((float)this->albedo[i * 4] / 255)
					;
				this->bitmap[i * 4 + 1] =
					std::min(this->light[i * 4 + 1], (sf::Uint16)255)
					* ((float)this->albedo[i * 4 + 1] / 255)
					;
				this->bitmap[i * 4 + 2] =
					std::min(this->light[i * 4 + 2], (sf::Uint16)255)
					* ((float)this->albedo[i * 4 + 2] / 255)
					;
				this->bitmap[i * 4 + 3] = 255;
			}
			
			this->tex.update(this->bitmap);
			this->targetWindow->clear();
			this->targetWindow->draw(sprite);
			this->targetWindow->display();
		}

		void onFinish() override {

		}

		void onStart() override {

		}

		DirectMultipassRenderHandler(CameraData* cameraData, sf::RenderWindow* targetWindow):
			MultipassRenderHandler(cameraData) {
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
		
		virtual void render() = 0;

	protected:

		Camera(CameraData* cameraData, RenderHandler* renderHandler) {
			this->cameraData = cameraData;
			this->renderHandler = renderHandler;
		}

		RenderHandler* renderHandler;
		CameraData* cameraData;
	};

	class ThreadedCamera : public Camera {
	public:

		void render() override {
			std::thread manager(&ThreadedCamera::initWorkers, this);
			manager.detach();
		}

		void initWorkers() {
			unsigned int subframeWidth = this->cameraData->dimensions.x / this->nThreads;

			this->renderHandler->onStart();

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




		ThreadedCamera(CameraData* cameraData, RenderHandler* renderHandler, unsigned short nThreads):
		Camera(cameraData, renderHandler) {
			this->cameraData = cameraData;
			this->renderHandler = renderHandler;

			this->nThreads = nThreads;
		}

	protected:
		unsigned short nThreads;
	};

	class PBRCamera : public Camera {
	public:

		void render() override {
			std::thread manager(&PBRCamera::initWorkers, this);
			manager.detach();
		}

		void initWorkers() {
			unsigned int subframeWidth = this->cameraData->dimensions.x / this->nThreads;

			this->renderHandler->onStart();

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

				workers.push_back(std::thread(&PBRCamera::renderSubframe, this, start, end, initialSceneIndex));

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
			// Pass RenderHandler
			auto renderHandler = ((MultipassRenderHandler*)this->renderHandler);

			// Get bitmap
			sf::Uint8* albedo = renderHandler->getAlbedo();

			// Render
			for (unsigned int x = startCol; x <= endCol; x++) {
				for (unsigned int y = 0; y < this->cameraData->dimensions.y; y++) {
					unsigned int offset = x + y * this->cameraData->dimensions.x;

					sf::Color frag;

					// Copied from cast()
					sf::Vector2f factor = fragToFactor(sf::Vector2i(x, y), this->cameraData->dimensions);

					Ray ray(this->cameraData->position, getVector(factor.x, factor.y, this->cameraData->fov, this->cameraData->rotation), this->cameraData->targetScene);

					ray.manualStep(initialSceneIndex);
					bool albedoHit = true;

					while (ray.step() > this->cameraData->clampThreshold) {
						if (ray.distance >= this->cameraData->maxDistance) {
							frag = this->cameraData->targetScene->getSkyColor();
							albedoHit = false;
							break;
						}
					}
					if (albedoHit) frag = this->cameraData->targetScene->getColorAt(ray.getPosition());
					// ----

					// Set albedo fragment
					albedo[offset * 4] = frag.r;
					albedo[offset * 4 + 1] = frag.g;
					albedo[offset * 4 + 2] = frag.b;
					albedo[offset * 4 + 3] = 255;

					// Set mist fragment
					renderHandler->getMist()[offset] = (sf::Uint8)fmin((ray.distance / this->cameraData->maxDistance) * 255, 255);
				
					// Set light fragment
					sf::Uint16* lightMap = renderHandler->getLightMap();

					sf::Uint16 light[3] = {20, 20, 20};

					bool globalLightOccluded = true;
					if (albedoHit) {
						// Check if globalLight is occluded (direct shadow)
						sf::Vector3f direction = this->cameraData->targetScene->globalLight.direction;

						LightRay globalLightRay(ray.getPosition(), -direction, this->cameraData->targetScene);
						
						if (ray.getPosition().y < 0.0) {
							//Catcher
							int lool = 10;
						}

						//globalLightRay.manualStep(this->cameraData->clampThreshold);
						
						while (globalLightRay.step(ray.getClosestIndex()) >= this->cameraData->clampThreshold) {
							if (globalLightRay.distance >= this->cameraData->maxDistance) {
								globalLightOccluded = false;
								break;
							}
						}

						if (!globalLightOccluded) {
							GlobalLight* globalLight = &this->cameraData->targetScene->globalLight;
							light[0] += globalLight->getColor().r * globalLight->getIntensity();
							light[1] += globalLight->getColor().g * globalLight->getIntensity();
							light[2] += globalLight->getColor().b * globalLight->getIntensity();
						}
					}

					lightMap[offset * 4] = light[0];
					lightMap[offset * 4 + 1] = light[1];
					lightMap[offset * 4 + 2] = light[2];
					lightMap[offset * 4 + 3] = 255;
				}
				//std::cout << x << "(" << ((float)x) / this->cameraData->dimensions.x << "%)" << std::endl;
			}
		}




		PBRCamera(CameraData* cameraData, MultipassRenderHandler* renderHandler, unsigned short nThreads) :
		Camera(cameraData, renderHandler) {
			this->cameraData = cameraData;
			this->renderHandler = renderHandler;

			this->nThreads = nThreads;
		}

	protected:
		unsigned short nThreads;
	};
}