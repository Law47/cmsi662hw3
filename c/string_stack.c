#include "string_stack.h"

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct StringStack {
    char **items;
    size_t size;
    size_t capacity;
};

static void secure_zero(void *ptr, size_t len) {
    volatile unsigned char *bytes = (volatile unsigned char *)ptr;
    while (len > 0U) {
        *bytes = 0U;
        bytes++;
        len--;
    }
}

static void secure_wipe_string_and_free(char *value) {
    if (value == NULL) {
        return;
    }
    secure_zero(value, strlen(value));
    free(value);
}

static StringStackOpResponse status_response(StringStackStatus status) {
    StringStackOpResponse response;
    response.status = status;
    return response;
}

static StringStackValueResponse value_response(StringStackStatus status, char *value) {
    StringStackValueResponse response;
    response.status = status;
    response.value = value;
    return response;
}

static char *duplicate_string(const char *src) {
    size_t len;
    char *copy;
    const size_t max_size = (size_t)-1;

    if (src == NULL) {
        return NULL;
    }

    len = strlen(src);
    if (len == max_size) {
        return NULL;
    }

    copy = (char *)malloc(len + 1U);
    if (copy == NULL) {
        return NULL;
    }

    memcpy(copy, src, len + 1U);
    return copy;
}

static StringStackStatus ensure_capacity(StringStack *stack, size_t min_capacity) {
    size_t new_capacity;
    char **new_items;
    const size_t max_size = (size_t)-1;

    if (stack == NULL) {
        return STRING_STACK_INVALID_ARGUMENT;
    }
    if (min_capacity <= stack->capacity) {
        return STRING_STACK_OK;
    }

    new_capacity = stack->capacity;
    while (new_capacity < min_capacity) {
        if (new_capacity > max_size / 2U) {
            return STRING_STACK_CAPACITY_OVERFLOW;
        }
        new_capacity *= 2U;
    }

    if (new_capacity > max_size / sizeof(char *)) {
        return STRING_STACK_CAPACITY_OVERFLOW;
    }

    new_items = (char **)calloc(new_capacity, sizeof(char *));
    if (new_items == NULL) {
        return STRING_STACK_ALLOCATION_FAILED;
    }

    if (stack->size > 0U) {
        memcpy(new_items, stack->items, stack->size * sizeof(char *));
    }

    free(stack->items);
    stack->items = new_items;
    stack->capacity = new_capacity;
    return STRING_STACK_OK;
}

StringStackCreateResponse string_stack_create(size_t initial_capacity) {
    StringStackCreateResponse response;
    StringStack *stack;
    const size_t max_size = (size_t)-1;

    response.status = STRING_STACK_OK;
    response.stack = NULL;

    if (initial_capacity == 0U) {
        response.status = STRING_STACK_INVALID_ARGUMENT;
        return response;
    }
    if (initial_capacity > max_size / sizeof(char *)) {
        response.status = STRING_STACK_CAPACITY_OVERFLOW;
        return response;
    }

    stack = (StringStack *)calloc(1U, sizeof(StringStack));
    if (stack == NULL) {
        response.status = STRING_STACK_ALLOCATION_FAILED;
        return response;
    }

    stack->items = (char **)calloc(initial_capacity, sizeof(char *));
    if (stack->items == NULL) {
        secure_zero(stack, sizeof(StringStack));
        free(stack);
        response.status = STRING_STACK_ALLOCATION_FAILED;
        return response;
    }

    stack->capacity = initial_capacity;
    response.stack = stack;
    return response;
}

StringStackOpResponse string_stack_destroy(StringStack **stack_ref) {
    StringStack *stack;
    size_t i;

    if (stack_ref == NULL || *stack_ref == NULL) {
        return status_response(STRING_STACK_INVALID_ARGUMENT);
    }

    stack = *stack_ref;
    for (i = 0U; i < stack->size; i++) {
        secure_wipe_string_and_free(stack->items[i]);
        stack->items[i] = NULL;
    }

    free(stack->items);
    stack->items = NULL;
    stack->size = 0U;
    stack->capacity = 0U;
    secure_zero(stack, sizeof(StringStack));
    free(stack);

    *stack_ref = NULL;
    return status_response(STRING_STACK_OK);
}

StringStackOpResponse string_stack_push(StringStack *stack, const char *value) {
    StringStackStatus capacity_status;
    char *copied_value;
    const size_t max_size = (size_t)-1;

    if (stack == NULL || value == NULL) {
        return status_response(STRING_STACK_INVALID_ARGUMENT);
    }

    if (stack->size == max_size) {
        return status_response(STRING_STACK_CAPACITY_OVERFLOW);
    }

    capacity_status = ensure_capacity(stack, stack->size + 1U);
    if (capacity_status != STRING_STACK_OK) {
        return status_response(capacity_status);
    }

    copied_value = duplicate_string(value);
    if (copied_value == NULL) {
        return status_response(STRING_STACK_ALLOCATION_FAILED);
    }

    stack->items[stack->size] = copied_value;
    stack->size++;
    return status_response(STRING_STACK_OK);
}

StringStackValueResponse string_stack_pop(StringStack *stack) {
    char *value;
    char *copy;

    if (stack == NULL) {
        return value_response(STRING_STACK_INVALID_ARGUMENT, NULL);
    }
    if (stack->size == 0U) {
        return value_response(STRING_STACK_EMPTY, NULL);
    }

    value = stack->items[stack->size - 1U];
    copy = duplicate_string(value);
    if (copy == NULL) {
        return value_response(STRING_STACK_ALLOCATION_FAILED, NULL);
    }

    stack->size--;
    stack->items[stack->size] = NULL;
    secure_wipe_string_and_free(value);
    return value_response(STRING_STACK_OK, copy);
}

StringStackValueResponse string_stack_peek(const StringStack *stack) {
    char *copy;

    if (stack == NULL) {
        return value_response(STRING_STACK_INVALID_ARGUMENT, NULL);
    }
    if (stack->size == 0U) {
        return value_response(STRING_STACK_EMPTY, NULL);
    }

    copy = duplicate_string(stack->items[stack->size - 1U]);
    if (copy == NULL) {
        return value_response(STRING_STACK_ALLOCATION_FAILED, NULL);
    }

    return value_response(STRING_STACK_OK, copy);
}

StringStackOpResponse string_stack_size(const StringStack *stack, size_t *out_size) {
    if (stack == NULL || out_size == NULL) {
        return status_response(STRING_STACK_INVALID_ARGUMENT);
    }

    *out_size = stack->size;
    return status_response(STRING_STACK_OK);
}

StringStackOpResponse string_stack_is_empty(const StringStack *stack, bool *out_is_empty) {
    if (stack == NULL || out_is_empty == NULL) {
        return status_response(STRING_STACK_INVALID_ARGUMENT);
    }

    *out_is_empty = (stack->size == 0U);
    return status_response(STRING_STACK_OK);
}

#ifdef STRING_STACK_SELF_TEST
static const char *status_to_string(StringStackStatus status) {
    switch (status) {
        case STRING_STACK_OK:
            return "OK";
        case STRING_STACK_INVALID_ARGUMENT:
            return "INVALID_ARGUMENT";
        case STRING_STACK_ALLOCATION_FAILED:
            return "ALLOCATION_FAILED";
        case STRING_STACK_EMPTY:
            return "EMPTY";
        case STRING_STACK_CAPACITY_OVERFLOW:
            return "CAPACITY_OVERFLOW";
        default:
            return "UNKNOWN";
    }
}

int main(void) {
    size_t size_value = 0U;
    bool is_empty = false;
    StringStackCreateResponse created = string_stack_create(2U);
    if (created.status != STRING_STACK_OK) {
        printf("create failed: %s\n", status_to_string(created.status));
        return 1;
    }

    StringStack *stack = created.stack;
    StringStackOpResponse push_alpha = string_stack_push(stack, "alpha");
    StringStackOpResponse push_beta = string_stack_push(stack, "beta");
    StringStackOpResponse push_gamma = string_stack_push(stack, "gamma");
    if (push_alpha.status != STRING_STACK_OK ||
        push_beta.status != STRING_STACK_OK ||
        push_gamma.status != STRING_STACK_OK) {
        printf("push failed: %s, %s\n",
               status_to_string(push_alpha.status),
               status_to_string(push_beta.status));
        (void)string_stack_destroy(&stack);
        return 1;
    }

    StringStackValueResponse peeked = string_stack_peek(stack);
    if (peeked.status != STRING_STACK_OK) {
        printf("peek failed: %s\n", status_to_string(peeked.status));
        (void)string_stack_destroy(&stack);
        return 1;
    }
    if (strcmp(peeked.value, "gamma") != 0) {
        printf("peek value mismatch: expected gamma, got %s\n", peeked.value);
        free(peeked.value);
        (void)string_stack_destroy(&stack);
        return 1;
    }
    printf("peek: %s\n", peeked.value);
    free(peeked.value);

    if (string_stack_size(stack, &size_value).status != STRING_STACK_OK || size_value != 3U) {
        printf("size mismatch after pushes: expected 3, got %zu\n", size_value);
        (void)string_stack_destroy(&stack);
        return 1;
    }

    StringStackValueResponse popped = string_stack_pop(stack);
    if (popped.status != STRING_STACK_OK) {
        printf("pop failed: %s\n", status_to_string(popped.status));
        (void)string_stack_destroy(&stack);
        return 1;
    }
    if (strcmp(popped.value, "gamma") != 0) {
        printf("pop value mismatch: expected gamma, got %s\n", popped.value);
        free(popped.value);
        (void)string_stack_destroy(&stack);
        return 1;
    }
    printf("pop:  %s\n", popped.value);
    free(popped.value);

    peeked = string_stack_peek(stack);
    if (peeked.status != STRING_STACK_OK || strcmp(peeked.value, "beta") != 0) {
        printf("second peek failed or mismatch\n");
        if (peeked.status == STRING_STACK_OK) {
            free(peeked.value);
        }
        (void)string_stack_destroy(&stack);
        return 1;
    }
    printf("peek: %s\n", peeked.value);
    free(peeked.value);

    if (string_stack_push(stack, "delta").status != STRING_STACK_OK) {
        printf("push delta failed\n");
        (void)string_stack_destroy(&stack);
        return 1;
    }

    if (string_stack_size(stack, &size_value).status != STRING_STACK_OK || size_value != 3U) {
        printf("size mismatch after delta push: expected 3, got %zu\n", size_value);
        (void)string_stack_destroy(&stack);
        return 1;
    }

    popped = string_stack_pop(stack);
    if (popped.status != STRING_STACK_OK || strcmp(popped.value, "delta") != 0) {
        printf("pop delta failed or mismatch\n");
        if (popped.status == STRING_STACK_OK) {
            free(popped.value);
        }
        (void)string_stack_destroy(&stack);
        return 1;
    }
    free(popped.value);

    popped = string_stack_pop(stack);
    if (popped.status != STRING_STACK_OK || strcmp(popped.value, "beta") != 0) {
        printf("pop beta failed or mismatch\n");
        if (popped.status == STRING_STACK_OK) {
            free(popped.value);
        }
        (void)string_stack_destroy(&stack);
        return 1;
    }
    free(popped.value);

    popped = string_stack_pop(stack);
    if (popped.status != STRING_STACK_OK || strcmp(popped.value, "alpha") != 0) {
        printf("pop alpha failed or mismatch\n");
        if (popped.status == STRING_STACK_OK) {
            free(popped.value);
        }
        (void)string_stack_destroy(&stack);
        return 1;
    }
    free(popped.value);

    if (string_stack_is_empty(stack, &is_empty).status != STRING_STACK_OK || !is_empty) {
        printf("empty check failed\n");
        (void)string_stack_destroy(&stack);
        return 1;
    }

    StringStackOpResponse destroyed = string_stack_destroy(&stack);
    if (destroyed.status != STRING_STACK_OK) {
        printf("destroy failed: %s\n", status_to_string(destroyed.status));
        return 1;
    }

    printf("C self-test passed\n");
    return 0;
}
#endif
