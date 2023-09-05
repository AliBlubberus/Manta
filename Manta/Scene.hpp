#pragma once

#include <SFML/Graphics.hpp>;

#include "Shape.hpp";
#include "Light.hpp";

namespace Manta {
	class Scene {
	public:

		// STATICS

		static inline float sceneIndex(const sf::Vector3f input, std::vector<std::shared_ptr<Shape>>* shapes) {
			if (shapes->size() == 0) return UINT8_MAX;

			float smallest = (*shapes)[0]->distanceEstimate(input);

			if (shapes->size() == 1) return smallest;

			for (uint16_t i = 1; i < shapes->size(); i++) {
				float current = (*shapes)[i]->distanceEstimate(input);
				if (current < smallest) smallest = current;
			}

			return smallest;
		}

		static inline float sceneIndex(
			const sf::Vector3f input,
			std::vector<std::shared_ptr<Shape>>* shapes,
			Shape** outClosest,
			unsigned int* outClosestIndex
		) {
			if (shapes->size() == 0) return UINT8_MAX;

			float smallest = (*shapes)[0]->distanceEstimate(input);

			if (shapes->size() == 1) return smallest;

			unsigned int targetIndex = 0;

			for (uint16_t i = 1; i < shapes->size(); i++) {
				float current = (*shapes)[i]->distanceEstimate(input);
				if (current < smallest) {
					smallest = current;
					targetIndex = i;
				}
			}

			*outClosestIndex = targetIndex;
			*outClosest = (*shapes)[targetIndex].get();

			return smallest;
		}

		static inline float sceneIndex(
			const sf::Vector3f input,
			std::vector<std::shared_ptr<Shape>>* shapes,
			Shape** outClosest,
			unsigned int* outClosestIndex,
			unsigned int ignoredIndex
		) {
			if (shapes->size() <= 1) return UINT8_MAX;

			float smallest = (*shapes)[0]->distanceEstimate(input);

			if (shapes->size() == 1) return smallest;

			unsigned int targetIndex = 0;

			for (uint16_t i = 1; i < shapes->size(); i++) {
				if (i == ignoredIndex) continue;

				float current = (*shapes)[i]->distanceEstimate(input);
				if (current < smallest) {
					smallest = current;
					targetIndex = i;
				}
			}

			*outClosestIndex = targetIndex;
			*outClosest = (*shapes)[targetIndex].get();

			return smallest;
		}

		// ---

		sf::Color getColorAt(sf::Vector3f point) {
			int closestIndex = 0;
			float distance = this->shapes[0]->distanceEstimate(point);

			for (unsigned int i = 1; i < this->shapes.size(); i++) {
				float dE = this->shapes[i]->distanceEstimate(point);
				if (dE < distance) {
					closestIndex = i;
					distance = dE;
				}
			}
			return this->shapes[closestIndex]->color;
		}


		sf::Color getSkyColor() {
			return this->skyColor;
		}

		void setSkyColor(sf::Color color) {
			this->skyColor = color;
		}


		void mountShape(Shape* shape) {
			this->shapes.push_back(std::shared_ptr<Shape>(shape));
		}

		std::vector<std::shared_ptr<Shape>>* getShapes() {
			return &this->shapes;
		}

		GlobalLight globalLight;

	private:
		std::vector<std::shared_ptr<Shape>> shapes;

		std::vector<std::shared_ptr<Light>> lights;

		sf::Color skyColor;
	};
}