#pragma once
#include <cstdint>
#include <cstdlib>

namespace sr2
{
	class RawBuffer
	{
	public:
		RawBuffer();
		RawBuffer(uint8_t* data, size_t size);
		~RawBuffer();

		void RecreateBuffer(uint8_t* data, size_t size);

		const uint8_t* GetData() const;
		const size_t GetSize() const;

	protected:
		uint8_t* m_Data;
		size_t m_Size;
	};
}
