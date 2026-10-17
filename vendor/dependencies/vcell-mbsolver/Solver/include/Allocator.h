#ifndef Allocator_h
#define Allocator_h
namespace vcell_util {
	/**
	* abstract allocation of multiple objects 
	* @param T class to allocate
	*/
	template <class T>
	struct Allocator {
		/**
		* return pointer to already constructed object 
		*/
		virtual T *provide( ) = 0;
	};

	/**
	* allocate in large blocks (arena allocator)
	* not in use 5/2016
	*/
	template <class T, int CHUNK_SIZE>
	struct ChunkAllocator : public Allocator<T> {
		ChunkAllocator( )
			:initial( ),
			current(&initial),
			index(0) {}

		virtual T *provide( ) {
			if (index < CHUNK_SIZE) {
				return &(current->data[index++]);
			}
			index = 0;
			current->next = new Chunk( );
			current = current->next;
			return provide( );
		}

		private:
			struct Chunk {
				Chunk( )
					:data( ),
					next(0){}
				~Chunk( ) {
					delete next;
				}
				T data[CHUNK_SIZE];
				Chunk *next;
			};
			Chunk initial;
			Chunk *current;
			int index;
	};
 }


#endif
