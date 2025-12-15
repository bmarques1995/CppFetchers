#include "RawBuffer.hpp"
#include <cstring>
#include <cstdlib>

sr2::RawBuffer::RawBuffer()
{
	m_Data = nullptr;
	m_Size = 0;
}

sr2::RawBuffer::RawBuffer(uint8_t* data, size_t size)
{
	m_Size = size;
	m_Data = new uint8_t[m_Size];
	memcpy(m_Data, data, m_Size);
}

sr2::RawBuffer::~RawBuffer()
{
	if (m_Data != nullptr)
		delete[] m_Data;
}

void sr2::RawBuffer::RecreateBuffer(uint8_t* data, size_t size)
{
	if (m_Data != nullptr)
		delete[] m_Data;

	m_Size = size;
	m_Data = new uint8_t[m_Size];
	memcpy(m_Data, data, m_Size);
}

const uint8_t* sr2::RawBuffer::GetData() const 
{
	return m_Data;
}

const size_t sr2::RawBuffer::GetSize() const
{
	return m_Size;
}
