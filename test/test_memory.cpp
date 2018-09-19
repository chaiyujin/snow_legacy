#define TEST_MEMORY  // open logging

#include <snow.h>
using namespace snow;

class Int {
public:
    int x;
    Int() : x(0) {}
    // ~Int() { printf("~Int()\n");}
};

int main() {
    snow::MemoryArena arena;

    Int * arr = arena.alloc<Int>(5);
    printf("ARR:  alloc  0x%X from 0x%X\n", arr, Block::queryFrom(arr));
    
    for (int i = 0; i < 5; ++i) { printf("%d ", arr[i].x); if (i % 10 == 9) printf("\n"); } printf("\n");
    for (int i = 0; i < 5; ++i) { arr[i].x = i; }

    Int * arr1 = arena.alloc<Int>(5);
    arena.free<Int>(arr1);
    arr1 = arena.alloc<Int>(1000000);

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