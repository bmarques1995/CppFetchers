#include "Chunk.hpp"

sr2::Chunk::Chunk(uint32_t index)
{
	m_ChunkIndex = index;
}

sr2::Chunk::Chunk(uint32_t index, uint8_t* data, size_t size) : RawBuffer(data, size)
{
	m_ChunkIndex = index;
}

sr2::Chunk::~Chunk()
{
}

uint32_t sr2::Chunk::GetChunkIndex() const
{
	return m_ChunkIndex;
}
