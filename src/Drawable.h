#pragma once
#include <iostream>
#include <AP2DGL/Entity.h>
#include "LineBuffer.h"
#include "../imgui/imgui.h"

enum class Type
{
	LINE, QUAD, TRIANGLE, CIRCLE, CUSTOM, IMAGE
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
struct CustomShape : public Drawable
{
	CustomShape()
	{
		vertexCount = 5;	// 5 verticies by default
		vertices = new ap::Vertex[vertexCount];   
		position = { 650.0f, 300.0f };
		color = ap::Vec4f(1.0f, 0.0f, 1.0f, 1.0f);
		reset();
		type = Type::CUSTOM;
	}
	~CustomShape()
	{
		delete[] vertices;
	}
	CustomShape(const CustomShape& cs)
	{
		type = Type::CUSTOM;
		vertexCount = cs.vertexCount;
		if (vertices != nullptr)
			delete[] vertices;
		vertices = new ap::Vertex[vertexCount];
		for (int i = 0; i < cs.vertexCount; i++)
			vertices[i] = cs.vertices[i];		// deep copy the vertex data
	}
	void move()
	{
		for (int i = 0; i < vertexCount; i++)
		{
			vertices[i].position.x += movementVector.x;
			vertices[i].position.y += movementVector.y;
		}
		position.x += movementVector.x;
		position.y += movementVector.y;																											
	}
	void reset()
	{
		float degStep = 360.0f / (float)vertexCount;
		float degrees = 0.0f;
		float radius  = 100.0f;
		for (int i = 0; i < vertexCount; i++)
		{
			vertices[i].position.x = position.x + (glm::cos(glm::radians(degrees)) * radius);
			vertices[i].position.y = position.y + (glm::sin(glm::radians(degrees)) * radius);
			vertices[i].color      = color;
			degrees += degStep;
		}
	}
	void update()
	{
		move();
		movementVector = { 0.0f, 0.0f };		
	}
	int activeIndex = 0;
	bool showPreview = false;
	ap::Vertex* vertices = nullptr;
	int vertexCount; // int because ImGui likes it that way
	ap::Vec2f position;
	ap::Vec2f movementVector;
	ap::Vec4f color;
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
	float previewScale = 1.0f;
	uint32_t previewVertexCount = 5.0f;
	int activeVertex = 0.0f;
	
	void reset()
	{
		previewColor = ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
		previewPosition[0] = 650.0f;
		previewPosition[1] = 300.0f;
		previewSize[0] = 100.0f;
		previewSize[1] = 100.0f;
		previewRadius = 25.0f;
		previewRotation = 0.0f;
		previewScale = 1.0f;
		filepath.clear();
		showPreview = false;
		delete previewDrawable;
		previewDrawable = nullptr;
		previewVertexCount = 5.0f;
		activeVertex = 0.0f;
	}
	~PreviewConfiguration() { delete previewDrawable; }
};
class SaveScreenShotData
{
public:
	SaveScreenShotData(ap::Window* win)
		: m_size(ap::Vec2f()), m_start(ap::Vec2f())
	{
		for (ap::Vertex& v : m_verticies)
			v.color = ap::Color::Black;
		definingArea = false;
		areaDefined = false;
		started = false;
		m_win = win;
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
	const ap::Vec2f& getStart() const { return { m_start.x, m_win->getWindowSize().y - m_start.y }; }
	const ap::Vec2f& getSize() const  { return m_size; }
public:
	bool definingArea, areaDefined, started;
private:
	std::array<ap::Vertex, 4> m_verticies;
	ap::Vec2f m_start, m_size;
	ap::Window* m_win;
};