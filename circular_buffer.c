#ifndef CIRCULAR_BUFFER
#define CIRCULAR_BUFFER

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

typedef struct
{
    bool is_empty;
    unsigned char * data_begin;
    unsigned char * data_end;
    unsigned char * head;
    unsigned char * tail;
} circular_buffer_t;

void
circular_buffer_construct(circular_buffer_t * const buffer, size_t const max_capacity)
{
    buffer->data_begin = malloc(max_capacity);
    buffer->data_end = buffer->data_begin + max_capacity;
    buffer->head = buffer->data_begin;
    buffer->tail = buffer->data_begin;
    buffer->is_empty = true;
}

bool
circular_buffer_is_empty(circular_buffer_t const * const buffer)
{
    return buffer->is_empty;
}

bool
circular_buffer_is_full(circular_buffer_t const * const buffer)
{
    return !circular_buffer_is_empty(buffer) && buffer->head == buffer->tail;
}

unsigned char const *
circular_buffer_insert_range(circular_buffer_t * const buffer,
                             unsigned char const * const data_in_begin,
                             unsigned char const * const data_in_end)
{
    unsigned char const * data_in_iter = data_in_begin;
    if(data_in_iter >= data_in_end || circular_buffer_is_full(buffer))
    {
        return data_in_iter;
    }
    size_t copy_byte_count;
    buffer->is_empty = false;
    if(buffer->tail >= buffer->head)
    {
        copy_byte_count = MIN(buffer->data_end - buffer->tail, data_in_end - data_in_iter);
        memcpy(buffer->tail, data_in_iter, copy_byte_count);
        buffer->tail += copy_byte_count;
        if(buffer->tail == buffer->data_end)
        {
            buffer->tail = buffer->data_begin;
        }
        data_in_iter += copy_byte_count;
        if(data_in_iter == data_in_end)
        {
            return data_in_iter;
        }
    }
    copy_byte_count = MIN(buffer->head - buffer->tail, data_in_end - data_in_iter);
    memcpy(buffer->tail, data_in_iter, copy_byte_count);
    buffer->tail += copy_byte_count;
    data_in_iter += copy_byte_count;
    return data_in_iter;
}

bool
circular_buffer_insert_value(circular_buffer_t * const buffer, unsigned char const value)
{
    return circular_buffer_insert_range(buffer, &value, &value + 1) == (&value + 1);
}

unsigned char *
circular_buffer_extract_range(circular_buffer_t * const buffer,
                              unsigned char * const data_out_begin,
                              unsigned char * const data_out_end)
{
    unsigned char * data_out_iter = data_out_begin;
    if(data_out_iter >= data_out_end || circular_buffer_is_empty(buffer))
    {
        return data_out_iter;
    }
    size_t copy_byte_count;
    if(buffer->head >= buffer->tail)
    {
        copy_byte_count = MIN(buffer->data_end - buffer->head, data_out_end - data_out_iter);
        memcpy(data_out_iter, buffer->head, copy_byte_count);
        buffer->head += copy_byte_count;
        if(buffer->head == buffer->data_end)
        {
            buffer->head = buffer->data_begin;
        }
        data_out_iter += copy_byte_count;
        if(buffer->head == buffer->tail)
        {
            buffer->is_empty = true;
        }
        if(data_out_iter == data_out_end)
        {
            return data_out_iter;
        }
    }
    copy_byte_count = MIN(buffer->tail - buffer->head, data_out_end - data_out_iter);
    memcpy(data_out_iter, buffer->head, copy_byte_count);
    buffer->head += copy_byte_count;
    if(buffer->head == buffer->tail)
    {
        buffer->is_empty = true;
    }
    data_out_iter += copy_byte_count;
    return data_out_iter;
}

unsigned char
circular_buffer_extract_value(circular_buffer_t * const buffer)
{
    unsigned char value;
    circular_buffer_extract_range(buffer, &value, &value + 1);
    return value;
}

void
circular_buffer_destruct(circular_buffer_t * const buffer)
{
    free(buffer->data_begin);
}

#endif // CIRCULAR_BUFFER

int
main(int const argc, char const * const * const argv)
{
    size_t const buffer_capacity = 5U;
    circular_buffer_t buffer;
    circular_buffer_construct(&buffer, buffer_capacity);
    assert(circular_buffer_is_empty(&buffer));
    assert(!circular_buffer_is_full(&buffer));

    size_t const data_size = 2 * buffer_capacity;
    unsigned char * const data = (unsigned char *)(malloc(data_size));
    for(size_t i = 0U; i < data_size; ++i)
    {
        data[i] = (unsigned char)('0' + i);
    }

    unsigned char const * data_initial_insert_ptr
        = circular_buffer_insert_range(&buffer, data, data + buffer_capacity);
    assert(data_initial_insert_ptr == data + buffer_capacity);
    assert(!circular_buffer_is_empty(&buffer));
    assert(circular_buffer_is_full(&buffer));
    for(size_t i = 0U; i < data_size; ++i)
    {
        data[i] = (unsigned char)('a');
    }
    unsigned char const * data_initial_extract_ptr
        = circular_buffer_extract_range(&buffer, data, data + buffer_capacity);
    assert(data_initial_extract_ptr == data + buffer_capacity);
    assert(circular_buffer_is_empty(&buffer));
    assert(!circular_buffer_is_full(&buffer));
    for(size_t i = 0U; i < buffer_capacity; ++i)
    {
        assert(data[i] == (unsigned char)('0' + i));
    }
    for(size_t i = buffer_capacity; i < data_size; ++i)
    {
        assert(data[i] == 'a');
    }
    for(size_t i = buffer_capacity; i < data_size; ++i)
    {
        data[i] = (unsigned char)('a' + i);
    }
    circular_buffer_destruct(&buffer);
}