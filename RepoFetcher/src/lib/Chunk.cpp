#include "Chunk.hpp"

Chunk::Chunk(uint32_t index)
{
	m_ChunkIndex = index;
}

Chunk::Chunk(uint32_t index, uint8_t* data, size_t size) : RawBuffer(data, size)
{
	m_ChunkIndex = index;
}

Chunk::~Chunk()
{
}

uint32_t Chunk::GetChunkIndex() const
{
	return m_ChunkIndex;
}
