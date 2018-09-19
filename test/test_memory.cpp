#define TEST_MEMORY  // open logging

#include <snow.h>
using namespace snow;

class Int {
public:
    int x;
    Int() : x(0) {}
    virtual ~Int() { }
};

int main() {
    snow::MemoryArena arena;

    Int * arr = arena.alloc<Int>(5);
    printf("ARR:  alloc  0x%X from 0x%X\n", arr, Block::queryFrom(arr));
    
    for (int i = 0; i < 5; ++i) { printf("%d ", arr[i].x); if (i % 10 == 9) printf("\n"); } printf("\n");
    for (int i = 0; i < 5; ++i) { arr[i].x = i; }

    Int * arr1 = arena.alloc<Int>(5);
    arena.free<Int>(arr1);
    arr1 = arena.alloc<Int>(100);
    printf("arena meta info: from %X, size %d\n", *(size_t *)((void *)arr1 - 16), *(size_t *)((void *)arr1 - 8));
    arena.free(arr1);
    arr1 = new Int[100];
    printf("new   meta info: size0 %d, size1 %d \n", *(size_t *)((void *)arr1 - 16), *(size_t *)((void *)arr1 - 8));
    {
        snow::StopWatch watch("arena");
        for (int i = 0; i < 10; ++i) {
            if (i % 2) arr1 = arena.alloc<Int>(10000000); else arr1 = arena.alloc<Int>(1000000);
            if ((uintptr_t)arr1 % 32 != 0) printf("[arena]: error not aligned with 32!\n");
            arena.free<Int>(arr1);
        }
    }
    {
        snow::StopWatch watch("new");
        for (int i = 0; i < 10; ++i) {
            if (i % 2) arr1 = new Int[10000000]; else arr1 = new Int[1000000];
            if ((uintptr_t)arr1 % 32 != 0) printf("[new]: error not aligned with 32!\n");
            delete[] arr1;
        }
    }

    arr1 = arena.alloc<Int>(10000000);
    for (int i = 0; i < 5; ++i) { printf("%d ", arr1[i].x); if (i % 10 == 9) printf("\n"); } printf("\n");
    for (int i = 0; i < 5; ++i) { printf("%d ", arr[i].x); if (i % 10 == 9) printf("\n"); } printf("\n");

    printf("count: %d 0x%X 0x%X\n", MemoryArena::queryCount(arr), arr + MemoryArena::queryCount(arr), arr1);
    printf("0x%X 0x%X\n", Block::queryFrom(arr), Block::queryFrom(arr1));
    printf("%d\t%d\n", Block::querySize(arr), Block::querySize(arr1));

    arena.free<Int>(arr);

    arr = arena.alloc<Int>(5);
    printf("ARR: realloc 0x%X from 0x%X\n", arr, Block::queryFrom(arr));

    return 0;
}