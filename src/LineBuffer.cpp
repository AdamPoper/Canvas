#include "LineBuffer.h"

LineBuffer::LineBuffer()
{
	m_vertexData = nullptr;
	m_size = 0;
	reAlloc(1000);
}
LineBuffer::LineBuffer(DrawType t)
{
	m_vertexData = nullptr;
	m_size = 0;
	reAlloc(1000);
	m_type = t;
}
LineBuffer::~LineBuffer()
{
	std::cout << "Deleting Memory" << std::endl;
	delete[] m_vertexData;
}
LineBuffer::LineBuffer(const LineBuffer& lb)
{
	m_width = lb.m_width;
	m_color = lb.m_color;
	m_size = lb.m_size;
	m_capacity = lb.m_capacity;
	m_type = lb.m_type;
	m_vertexData = new ap::Vertex[m_capacity];
	for (int i = 0; i < m_size; i++)
		m_vertexData[i] = lb[i];
}
void LineBuffer::Push(const ap::Vertex& vertex)
{
	if (m_size < m_capacity)
	{
		m_vertexData[m_size] = vertex;
		m_size++;
	}
	else
	{
		reAlloc(m_capacity * 3);
		m_vertexData[m_size] = vertex;
		m_size++;
	}
}
void LineBuffer::setType(DrawType dt) { m_type = dt; }
void LineBuffer::reAlloc(size_t size)
{
	m_capacity = size;
	if (m_vertexData == nullptr)
		m_vertexData = new ap::Vertex[m_capacity];
	else
	{
		ap::Vertex* temp = new ap::Vertex[m_size];
		for (int i = 0; i < m_size; i++)
			temp[i] = m_vertexData[i];
		delete[] m_vertexData;
		m_vertexData = new ap::Vertex[m_capacity];
		for (int i = 0; i < m_size; i++)
			m_vertexData[i] = temp[i];
	}
}
void LineBuffer::Clear()
{
	delete[] m_vertexData;
	m_vertexData = nullptr;
	m_size = 0;
	reAlloc(1000);
}

void LineBuffer::Draw(ap::Renderer& renderer)
{
	glLineWidth(m_width);
	renderer.Draw(m_vertexData, m_size, GL_LINE_STRIP);
}
void LineBuffer::SetWidth(float w)
{
	m_width = w;
}
void LineBuffer::SetColor(const ap::Vec4f& col)
{
	m_color = col;
	for (int i = 0; i < m_size; i++)
		m_vertexData[i].color = m_color;
}
size_t LineBuffer::Count() const { return m_size; }
ap::Vertex& LineBuffer::operator[](int index)
{
	return m_vertexData[index];
}
const ap::Vertex& LineBuffer::operator[](int index) const
{
	return m_vertexData[index];
}
