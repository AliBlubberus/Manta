#include <iostream>;

#include <SFML/Graphics.hpp>

#include "Shape.hpp";
#include "Transform.hpp";
#include "Scene.hpp";
#include "Camera.hpp";

int main() {
	sf::RenderWindow _window;
	_window.create(sf::VideoMode(1280, 720), "Manta");

	_window.setFramerateLimit(120);

	sf::Event _windowEvent;

	auto scene = Manta::Scene();
	scene.setSkyColor(sf::Color(70, 90, 240));
	
	auto cameraData = Manta::CameraData();
	cameraData.targetScene = &scene;
	cameraData.dimensions = sf::Vector2u(1280, 720);
	cameraData.position = sf::Vector3f(-50, 0, 0);

	auto renderHandler = Manta::DirectMultipassRenderHandler(&cameraData, &_window);

	auto camera = Manta::PBRCamera(&cameraData, &renderHandler, 16);

	// GENERATE TEST SCENE
	const unsigned int NUM_ENTITIES = 20;
	
	for (unsigned int i = 0; i < NUM_ENTITIES; i++) {
		
		auto sphere = (std::rand() % 2) == 0 || true ? Manta::Sphere() : Manta::Box();

		sphere->color = sf::Color(std::rand() % 255, std::rand() % 255, std::rand() % 255);

		auto transform = new Manta::Translate(sf::Vector3f(
			std::rand() % 10 - 5,
			std::rand() % 20 - 10,
			std::rand() % 20 - 10
		));

		sphere->pushTransform(transform);

		scene.mountShape(sphere);
	}	


	camera.render();

	while (_window.isOpen()) {

		renderHandler.update();

		while (_window.pollEvent(_windowEvent)) {
			if (_windowEvent.type == sf::Event::Closed) {
				_window.close();
			}
		}
	}

	return 0;
}