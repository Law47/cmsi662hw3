#ifndef SECURE_STRING_STACK_HPP
#define SECURE_STRING_STACK_HPP

#include <cstddef>
#include <limits>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

class SecureStringStack final {
public:
    explicit SecureStringStack(std::size_t initialCapacity = 8)
        : data_(nullptr), size_(0), capacity_(0) {
        if (initialCapacity == 0) {
            throw std::invalid_argument("initialCapacity must be > 0");
        }
        allocateEmpty(initialCapacity);
    }

    SecureStringStack(const SecureStringStack &other)
        : data_(nullptr), size_(0), capacity_(0) {
        allocateEmpty(other.capacity_);
        for (std::size_t i = 0; i < other.size_; ++i) {
            data_[i] = std::make_unique<std::string>(*other.data_[i]);
        }
        size_ = other.size_;
    }

    SecureStringStack &operator=(const SecureStringStack &other) {
        if (this == &other) {
            return *this;
        }
        SecureStringStack copy(other);
        swap(copy);
        return *this;
    }

    SecureStringStack(SecureStringStack &&other) noexcept
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    SecureStringStack &operator=(SecureStringStack &&other) noexcept {
        if (this == &other) {
            return *this;
        }
        delete[] data_;
        data_ = other.data_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
        return *this;
    }

    ~SecureStringStack() {
        clear();
        delete[] data_;
    }

    void push(const std::string &value) {
        if (size_ == maxSize()) {
            throw std::overflow_error("stack size overflow");
        }
        ensureCapacity(size_ + 1);
        data_[size_] = std::make_unique<std::string>(value);
        ++size_;
    }

    void push(const char *value) {
        if (value == nullptr) {
            throw std::invalid_argument("value must not be null");
        }
        push(std::string(value));
    }

    std::string pop() {
        if (size_ == 0) {
            throw std::underflow_error("cannot pop from empty stack");
        }
        const std::string valueCopy = *data_[size_ - 1];
        data_[size_ - 1].reset();
        --size_;
        return valueCopy;
    }

    std::string peek() const {
        if (size_ == 0) {
            throw std::underflow_error("cannot peek empty stack");
        }
        return *data_[size_ - 1];
    }

    std::size_t size() const noexcept {
        return size_;
    }

    bool empty() const noexcept {
        return size_ == 0;
    }

    void clear() noexcept {
        for (std::size_t i = 0; i < size_; ++i) {
            data_[i].reset();
        }
        size_ = 0;
    }

private:
    std::unique_ptr<std::string> *data_;
    std::size_t size_;
    std::size_t capacity_;

    static constexpr std::size_t maxSize() noexcept {
        return std::numeric_limits<std::size_t>::max();
    }

    static constexpr std::size_t maxCapacity() noexcept {
        return std::numeric_limits<std::size_t>::max() /
               sizeof(std::unique_ptr<std::string>);
    }

    void allocateEmpty(std::size_t capacity) {
        if (capacity == 0 || capacity > maxCapacity()) {
            throw std::overflow_error("capacity overflow");
        }
        data_ = new std::unique_ptr<std::string>[capacity]();
        capacity_ = capacity;
        size_ = 0;
    }

    void ensureCapacity(std::size_t minCapacity) {
        if (minCapacity <= capacity_) {
            return;
        }
        std::size_t newCapacity = capacity_;
        while (newCapacity < minCapacity) {
            if (newCapacity > maxCapacity() / 2) {
                throw std::overflow_error("capacity overflow");
            }
            newCapacity *= 2;
        }
        resize(newCapacity);
    }

    void resize(std::size_t newCapacity) {
        if (newCapacity < size_) {
            throw std::invalid_argument("new capacity cannot be less than size");
        }
        if (newCapacity == capacity_) {
            return;
        }
        if (newCapacity > maxCapacity()) {
            throw std::overflow_error("capacity overflow");
        }

        auto *newData = new std::unique_ptr<std::string>[newCapacity]();
        for (std::size_t i = 0; i < size_; ++i) {
            newData[i] = std::move(data_[i]);
        }

        delete[] data_;
        data_ = newData;
        capacity_ = newCapacity;
    }

    void swap(SecureStringStack &other) noexcept {
        using std::swap;
        swap(data_, other.data_);
        swap(size_, other.size_);
        swap(capacity_, other.capacity_);
    }
};

#ifdef SECURE_STRING_STACK_SELF_TEST
int main() {
    try {
        SecureStringStack stack(2);
        stack.push("alpha");
        stack.push(std::string("beta"));
        stack.push("gamma");

        if (stack.peek() != "gamma") {
            throw std::runtime_error("peek mismatch after initial pushes");
        }
        std::cout << "peek: " << stack.peek() << "\n";

        if (stack.size() != 3) {
            throw std::runtime_error("size mismatch after initial pushes");
        }
        std::cout << "size: " << stack.size() << "\n";

        if (stack.pop() != "gamma") {
            throw std::runtime_error("first pop mismatch");
        }
        std::cout << "pop:  gamma\n";

        if (stack.peek() != "beta") {
            throw std::runtime_error("peek mismatch after pop");
        }
        std::cout << "peek: beta\n";

        stack.push("delta");
        if (stack.size() != 3) {
            throw std::runtime_error("size mismatch after delta push");
        }
        std::cout << "size: " << stack.size() << "\n";

        if (stack.pop() != "delta") {
            throw std::runtime_error("delta pop mismatch");
        }
        if (stack.pop() != "beta") {
            throw std::runtime_error("beta pop mismatch");
        }
        if (stack.pop() != "alpha") {
            throw std::runtime_error("alpha pop mismatch");
        }
        std::cout << "pop sequence: delta, beta, alpha\n";

        if (!stack.empty()) {
            throw std::runtime_error("stack should be empty at end of test");
        }
        std::cout << "C++ self-test passed\n";
    } catch (const std::exception &ex) {
        std::cerr << "error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
#endif

#endif
