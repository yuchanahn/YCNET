#pragma once

namespace YC
{
	template<typename T,
		bool[std::is_base_of<IMemoryPoolObject, T>::value] = nullptr>
	class mp_Data
	{
	private:
		// 공유 카운트.
		int sh_cnt = 0;
	public:
		PROP(T*, Value);
		PROP(bool, IsUsed) = false;
		T* Shared()
		{

		}
	public:
		static stack<T*> mBuffer;
		static T* _new()
		{
			if (mBuffer.empty())
			{
				for (int i = 0; i < 100; i++)
				{
					mBuffer.push(new T());
				}
			}
			
			auto data = mBuffer.top();
			mBuffer.pop();
			
			return data;
		}

	};


	class MemoryPool
	{
		int mSize;

		byte* mBuffer;
		stack<byte*> mMemStack;

		void Init();
	public:
		MemoryPool(int size) : mSize(size), mBuffer(new byte[size]) { Init(); }
		~MemoryPool() { delete[] mBuffer; }
	public:

		byte* GetDataOfSize(int size);

		template <class T>
		static mp_Data<T> _new()
		{
			
			mMemStack = GetDataOfSize
			return ;
		}
	};

}