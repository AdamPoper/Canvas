#pragma once
#include <AP2DGL/Renderer.h>
#include <vector>
enum class DrawType
{
	DRAW, ERASE
};
class LineBuffer
{
public:
	LineBuffer();
	LineBuffer(DrawType t);
	~LineBuffer();
	LineBuffer(const LineBuffer& lb);
	void Clear();
	size_t Count() const;
	const ap::Vertex& operator[](int index) const;
	ap::Vertex& operator[](int index);	
	void Draw(ap::Renderer& rend);
	void Push(const ap::Vertex& vert);
	void SetColor(const ap::Vec4f& col);
	void SetWidth(float w);
	DrawType Type() const { return m_type; }
	void setType(DrawType dt);
private:
	void reAlloc(size_t);
private:
	DrawType m_type;
	ap::Vertex* m_vertexData;
	size_t m_size;
	size_t m_capacity;
	float m_width = 1.0f;
	ap::Vec4f m_color;
};