#pragma once

#include <SFML/Graphics.hpp>

#include "Scene.hpp";

namespace Manta {
	class Ray {
	public:
		float distance = 0;

		float step() {
			float sceneIndex = Scene::sceneIndex(
				this->steps[0],
				this->scene->getShapes(),
				&this->closestShape,
				&this->closestIndex
			);

			sf::Vector3f newPos = this->steps[0] + this->direction * sceneIndex;

			this->steps.insert(this->steps.begin(), newPos);
			this->distance += sceneIndex;

			return sceneIndex;
		}

		void manualStep(float distance) {
			sf::Vector3f newPos = this->steps[0] + this->direction * distance;

			this->steps.insert(this->steps.begin(), newPos);
			this->distance += distance;
		}

		sf::Vector3f getPosition() {
			return this->steps[0];
		}


		Ray(sf::Vector3f position, sf::Vector3f direction, Scene* scene) {
			this->steps.insert(this->steps.begin(), position);
			this->direction = direction;
			this->scene = scene;
		}

		Shape* getClosest() {
			return this->closestShape;
		}

		unsigned int getClosestIndex() {
			return this->closestIndex;
		}

	private:
		std::vector<sf::Vector3f> steps;
		sf::Vector3f direction;
		Scene* scene;

		Shape* closestShape = nullptr;
		unsigned int closestIndex = 0;
	};

	class LightRay {
	public:
		float distance = 0;

		float step() {
			float sceneIndex = Scene::sceneIndex(
				this->steps[0],
				this->scene->getShapes(),
				&this->closestShape,
				&this->closestIndex
			);

			sf::Vector3f newPos = this->steps[0] + this->direction * sceneIndex;

			this->steps.insert(this->steps.begin(), newPos);
			this->distance += sceneIndex;

			return sceneIndex;
		}

		float step(unsigned int indexIgnored) {
			float sceneIndex = Scene::sceneIndex(
				this->steps[0],
				this->scene->getShapes(),
				&this->closestShape,
				&this->closestIndex,
				indexIgnored
			);

			sf::Vector3f newPos = this->steps[0] + this->direction * sceneIndex;

			this->steps.insert(this->steps.begin(), newPos);
			this->distance += sceneIndex;

			return sceneIndex;
		}

		void manualStep(float distance) {
			sf::Vector3f newPos = this->steps[0] + this->direction * distance;

			this->steps.insert(this->steps.begin(), newPos);
			this->distance += distance;
		}

		sf::Vector3f getPosition() {
			return this->steps[0];
		}


		LightRay(sf::Vector3f position, sf::Vector3f direction, Scene* scene) {
			this->steps.insert(this->steps.begin(), position);
			this->direction = direction;
			this->scene = scene;
		}

		Shape* getClosest() {
			return this->closestShape;
		}

		unsigned int getClosestIndex() {
			return this->closestIndex;
		}

	private:
		std::vector<sf::Vector3f> steps;
		sf::Vector3f direction;
		Scene* scene;

		Shape* closestShape = nullptr;
		unsigned int closestIndex = 0;
	};
}