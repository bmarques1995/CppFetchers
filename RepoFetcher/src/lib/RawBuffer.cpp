#include "RawBuffer.hpp"
#include <cstring>
#include <cstdlib>

RawBuffer::RawBuffer()
{
	m_Data = nullptr;
	m_Size = 0;
}

RawBuffer::RawBuffer(uint8_t* data, size_t size)
{
	m_Size = size;
	m_Data = new uint8_t[m_Size];
	memcpy(m_Data, data, m_Size);
}

RawBuffer::~RawBuffer()
{
	if (m_Data != nullptr)
		delete[] m_Data;
}

void RawBuffer::RecreateBuffer(uint8_t* data, size_t size)
{
	if (m_Data != nullptr)
		delete[] m_Data;

	m_Size = size;
	m_Data = new uint8_t[m_Size];
	memcpy(m_Data, data, m_Size);
}

const uint8_t* RawBuffer::GetData() const 
{
	return m_Data;
}

const size_t RawBuffer::GetSize() const
{
	return m_Size;
}
