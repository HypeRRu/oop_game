#ifndef RENDERER_H
#define RENDERER_H

#include <SFML/Graphics.hpp>
#include <string>
#include <memory>
#include <functional>
#include <map>

#include "texture_manager.h"
#include "base_window.h"
#include "../actions/base_observed.h"
#include "../interfaces/drawable.h"

class Renderer: public BaseObserved
{
public:
	Renderer(size_t w_width = 0, size_t w_height = 0, std::string title = "");
	~Renderer();

	sf::RenderWindow& getWindow();

	bool renderFrame();
	bool isWindowOpen();

	void addObject(
		const Drawable& object,
		size_t object_w,
		size_t object_h,
		size_t layer = 0
	);
	void setWindowRendering(
		std::shared_ptr<BaseWindow> _window_rendering
	);
	void removeWindowRendering();

	bool removeObject(const Drawable& object);
	void flushObjects();
private:
	using slayer = std::map<size_t, sf::Sprite>;

	sf::RenderWindow window;
	size_t w_width, w_height;
	std::string title;

	std::vector<slayer> render_objects;
	std::weak_ptr<BaseWindow> window_rendering;
	bool is_rendering_window;
};

#endif
