#pragma once

#include "RawBuffer.hpp"

namespace sr2
{
    /**
    * @brief A class for handling files
    */
    class Chunk : public RawBuffer
    {
    public:
        Chunk(uint32_t index);
        Chunk(uint32_t index, uint8_t* data, size_t size);
        ~Chunk();

        uint32_t GetChunkIndex() const;
    protected:
        uint32_t m_ChunkIndex;
    };
}
