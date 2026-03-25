#ifndef STRING_STACK_H
#define STRING_STACK_H

#include <stdbool.h>
#include <stddef.h>

typedef struct StringStack StringStack;

typedef enum {
    STRING_STACK_OK = 0,
    STRING_STACK_INVALID_ARGUMENT,
    STRING_STACK_ALLOCATION_FAILED,
    STRING_STACK_EMPTY,
    STRING_STACK_CAPACITY_OVERFLOW
} StringStackStatus;

typedef struct {
    StringStackStatus status;
    StringStack *stack;
} StringStackCreateResponse;

typedef struct {
    StringStackStatus status;
    char *value;
} StringStackValueResponse;

typedef struct {
    StringStackStatus status;
} StringStackOpResponse;

StringStackCreateResponse string_stack_create(size_t initial_capacity);
StringStackOpResponse string_stack_destroy(StringStack **stack_ref);

StringStackOpResponse string_stack_push(StringStack *stack, const char *value);
StringStackValueResponse string_stack_pop(StringStack *stack);
StringStackValueResponse string_stack_peek(const StringStack *stack);

StringStackOpResponse string_stack_size(const StringStack *stack, size_t *out_size);
StringStackOpResponse string_stack_is_empty(const StringStack *stack, bool *out_is_empty);

#endif
