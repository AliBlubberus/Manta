#pragma once

#include <SFML/Graphics.hpp>;

#include "Shape.hpp";

namespace Manta {
	class Scene {
	public:
		static inline float sceneIndex(const sf::Vector3f input, std::vector<std::shared_ptr<Shape>>* shapes) {
			if (shapes->size() == 0) return UINT16_MAX;

			float smallest = (*shapes)[0]->distanceEstimate(input);

			if (shapes->size() == 1) return smallest;

			for (uint16_t i = 1; i < shapes->size(); i++) {
				float current = (*shapes)[i]->distanceEstimate(input);
				if (current < smallest) smallest = current;
			}

			return smallest;
		}

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

	private:
		std::vector<std::shared_ptr<Shape>> shapes;
		
		sf::Color skyColor;
	};
}