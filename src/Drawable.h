#pragma once
#include <iostream>
#include <AP2DGL/Entity.h>
#include "LineBuffer.h"

enum class Type
{
	LINE, QUAD, TRIANGLE, CIRCLE, POLYGON, IMAGE
};
struct Drawable
{
	Type type;
	virtual ~Drawable() {}
};
struct Quad : public Drawable
{
	ap::Quad quad;
	~Quad() { std::cout << "Quad Deleted" << std::endl; }
};
struct Line : public Drawable
{
	LineBuffer buffer;
	~Line() {}
};
struct Circle : public Drawable
{
	ap::Circle circle;
	~Circle() { std::cout << "Circle Deleted" << std::endl; }
};
struct Triangle : public Drawable
{
	ap::Triangle triangle;
	~Triangle() { std::cout << "Triangle Deleted" << std::endl; }
};
struct Image : public Drawable
{
	Image(const char* file)
	{
		texture.LoadFromFile(file);
		filepath = file;
		//image.setTexture(&texture);
	}
	ap::Sprite image;
	ap::Texture texture;
	std::string filepath;
	~Image() { std::cout << "Image deleted" << std::endl; }
};
struct PreviewConfiguration
{
	ImVec4 previewColor = ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
	float previewPosition[2] = { 650.0f, 300.0f };
	float previewSize[2] = { 100.0f, 100.0f };
	float previewRadius = 25.0f;
	bool showPreview = false;
	Drawable* previewDrawable = nullptr;
	float previewRotation = 0.0f;
	std::string filepath;
	void reset()
	{
		previewColor = ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
		previewPosition[0] = 650.0f;
		previewPosition[1] = 300.0f;
		previewSize[0] = 100.0f;
		previewSize[1] = 100.0f;
		previewRadius = 25.0f;
		previewRotation = 0.0f;
		filepath.clear();
		showPreview = false;
		delete previewDrawable;
		previewDrawable = nullptr;
	}
	~PreviewConfiguration() { delete previewDrawable; }
};
class SaveScreenShotData
{
public:
	SaveScreenShotData()
		: m_size(ap::Vec2f()), m_start(ap::Vec2f())
	{
		for (ap::Vertex& v : m_verticies)
			v.color = ap::Color::Black;
		definingArea = false;
		areaDefined = false;
		started = false;
	}
	void setStartPosition(const ap::Vec2f& start)
	{
		m_start = start;
	}
	void update(const ap::Vec2f& end)
	{
		m_size.x = end.x - m_start.x;
		m_size.y = end.y - m_start.y;
		
		m_verticies[0].position = m_start;
		m_verticies[1].position.x = end.x;
		m_verticies[1].position.y = m_start.y;
		m_verticies[2].position = end;
		m_verticies[3].position.x = m_start.x;
		m_verticies[3].position.y = end.y;
	}
	void reset()
	{
		for (ap::Vertex& v : m_verticies)
			v.position = ap::Vec2f();
		m_start = ap::Vec2f();
		m_size  = ap::Vec2f();
		definingArea =  false;
		areaDefined  =  false;
	}
	
	void draw(ap::Renderer& renderer)
	{
		renderer.Draw(m_verticies.data(), m_verticies.size(), ap::LINE_LOOP);
	}
	const ap::Vec2f& getStart() const { return m_start; }
	const ap::Vec2f& getSize() const  { return m_size;  }
public:
	bool definingArea, areaDefined, started;
private:
	std::array<ap::Vertex, 4> m_verticies;
	ap::Vec2f m_start, m_size;

};
