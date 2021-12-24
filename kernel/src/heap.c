#include "heap.h"

#include "globals.h"
#include "pagetable_manager.h"
#include "pageframe_allocator.h"
#include "paging.h"

static void *_heap_start;
static void *_heap_end;
static heap_hdr_t *_last_segment;

static bool __expand(size_t length);
static bool __combine_next(heap_hdr_t*);
static bool __combine_prev(heap_hdr_t*);
static bool __split(heap_hdr_t*, size_t);

void heap_init(void *address, size_t pages)
{
    void *ptr = address;
    for (int i = 0; i < pages; i++) {
        pagetable_map(g_pml4, ptr, pageframe_request());
        ptr = (void *)((size_t)ptr + PAGE_SIZE);
    }

    size_t len = pages * PAGE_SIZE;
    _heap_start = address;
    _heap_end = (void *)((size_t)_heap_start + len);
    heap_hdr_t *first_segment = (heap_hdr_t *)_heap_start;
    first_segment->length = len - sizeof(heap_hdr_t);
    first_segment->next = NULL;
    first_segment->prev = NULL;
    first_segment->free = true;
    _last_segment = first_segment;

}

void* heap_alloc(size_t size)
{
    if (size % 0x10) {
        size -= (size % 0x10);
        size += 0x10;
    }

    if (size == 0) return NULL;

    heap_hdr_t *current_segment = (heap_hdr_t *)_heap_start;
    while (true) {
        if (current_segment->free) {
            if (current_segment->length >= size) {
                if (current_segment->length > size)
                    __split(current_segment, size);

                current_segment->free = false;
                return (void *)((uint64_t)current_segment + sizeof(heap_hdr_t));
            }
        }

        if (current_segment->next == NULL) break;
        current_segment = current_segment->next;
    }

    __expand(size);
    return heap_alloc(size);
}

void* heap_calloc(size_t size)
{
    void *alloc = heap_alloc(size);
    memzero(alloc, size);
    return alloc;
}

void* heap_realloc(void *ptr, size_t size)
{

}

void heap_free(void *address)
{
    heap_hdr_t *segment = (heap_hdr_t *)((uint64_t)address - sizeof(heap_hdr_t));
    segment->free = true;
    __combine_next(segment);
    __combine_prev(segment);
}

static bool __expand(size_t length)
{
    if (length % PAGE_SIZE) {
        length -= (length % PAGE_SIZE);
        length += PAGE_SIZE;
    }

    size_t pages = length / PAGE_SIZE;
    heap_hdr_t *segment = (heap_hdr_t *)_heap_end;

    for (size_t i = 0; i < pages; i++) {
        pagetable_map(g_pml4, _heap_end, pageframe_request());
        _heap_end = (void *)((size_t)_heap_end + PAGE_SIZE);
    }

    segment->free = true;
    segment->prev = _last_segment;
    _last_segment->next = segment;
    _last_segment = segment;
    segment->next = NULL;
    segment->length = length - sizeof(heap_hdr_t);
    __combine_prev(segment);

    return true;
}

static bool __combine_next(heap_hdr_t *segment)
{
    if (segment->next == NULL || !segment->next->free) return false;
    if (segment->next == _last_segment) _last_segment = segment;
    if (segment->next->next != NULL) segment->next->next->prev = segment;
    segment->length += (segment->next->length + sizeof(heap_hdr_t));
    segment->next = segment->next->next;

    return true;
}

static bool __combine_prev(heap_hdr_t *segment)
{
    return (segment->prev != NULL && segment->prev->free)
        ? __combine_next(segment->prev)
        : false;
}

static bool __split(heap_hdr_t *segment, size_t length)
{
    if (length < 0x10) return false;
    int64_t remaining = segment->length - sizeof(heap_hdr_t) - length;
    if (remaining < 0x10) return false;

    heap_hdr_t *split = (heap_hdr_t *)((size_t)segment + sizeof(heap_hdr_t) + length);
    segment->next->prev = split;
    split->next = segment->next;
    segment->next = split;
    split->prev = segment;
    split->length = remaining;
    split->free = segment->free;
    segment->length = length;

    if (_last_segment == segment) _last_segment = split;
    return true;
}