#pragma once

#pragma once

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

template <typename T>
class RawMemory {
public:
    // constructors
    RawMemory() = default;

    explicit RawMemory(size_t capacity)
        : buffer_(Allocate(capacity))
        , capacity_(capacity) {
    }

    RawMemory(const RawMemory&) = delete;
    RawMemory& operator=(const RawMemory& rhs) = delete;
    RawMemory(RawMemory&& other) noexcept
    {
        Swap(other);
    }
    RawMemory& operator=(RawMemory&& rhs) noexcept
    {
        Swap(rhs);
        return *this;
    }

    ~RawMemory() {
        Deallocate(buffer_);
    }

    // operators
    T* operator+(size_t offset) noexcept {
        assert(offset <= capacity_);
        return buffer_ + offset;
    }

    const T* operator+(size_t offset) const noexcept {
        return const_cast<RawMemory&>(*this) + offset;
    }

    const T& operator[](size_t index) const noexcept {
        return const_cast<RawMemory&>(*this)[index];
    }

    T& operator[](size_t index) noexcept {
        assert(index < capacity_);
        return buffer_[index];
    }

    // methods
    void Swap(RawMemory& other) noexcept {
        std::swap(buffer_, other.buffer_);
        std::swap(capacity_, other.capacity_);
    }

    const T* GetAddress() const noexcept {
        return buffer_;
    }

    T* GetAddress() noexcept {
        return buffer_;
    }

    size_t Capacity() const {
        return capacity_;
    }

private:
    // Выделяет сырую память под n элементов и возвращает указатель на неё
    static T* Allocate(size_t n) {
        return n != 0 ? static_cast<T*>(operator new(n * sizeof(T))) : nullptr;
    }

    // Освобождает сырую память, выделенную ранее по адресу buf при помощи Allocate
    static void Deallocate(T* buf) noexcept {
        operator delete(buf);
    }

    T* buffer_ = nullptr;
    size_t capacity_ = 0;
};

template <typename T>
class Vector {
public:
    //
    using iterator = T*;
    using const_iterator = const T*;

    // constructors and operators =
    Vector() = default;

    explicit Vector(size_t size)
        : data_(size)
        , size_(size)  //
    {
        std::uninitialized_value_construct_n(data_.GetAddress(), size_);
    }

    Vector(const Vector& other)
        : data_(other.size_)
        , size_(other.size_)  //
    {
        std::uninitialized_copy_n(other.data_.GetAddress(), size_, data_.GetAddress());

    }

    Vector& operator=(const Vector& rhs)
    {
        if (this != &rhs)
        {
            if (rhs.size_ > data_.Capacity())
            {
                Vector rhs_copy(rhs);
                Swap(rhs_copy);
            }
            else
            {
                std::copy(rhs.data_.GetAddress(), rhs.data_.GetAddress() + std::min(rhs.size_, size_), data_.GetAddress());
                if (size_ > rhs.size_)
                {
                    std::destroy_n(data_.GetAddress() + rhs.size_, size_ - rhs.size_);
                }
                else //if (size_ < rhs.size_)
                {
                    std::uninitialized_copy_n(rhs.data_.GetAddress() + size_, rhs.size_ - size_, data_.GetAddress() + size_);
                }
               size_ = rhs.size_;
            }
        }

        return *this;
    }

    Vector(Vector&& other) noexcept
        : data_(std::move(other.data_)), size_(std::move(other.size_))
    {
    }

    Vector& operator=(Vector&& rhs) noexcept
    {
        if (this != &rhs)
        {
            data_ = std::move(rhs.data_);
            size_ = std::move(rhs.size_);
        }
        return *this;
    }

    ~Vector() {
        if (data_.GetAddress() != nullptr)
        {
            std::destroy_n(data_.GetAddress(), size_);
        }
    }

    // iterators
    iterator begin() noexcept
    {
        return data_.GetAddress();
    }

    iterator end() noexcept
    {
        return data_.GetAddress() + size_;
    }

    const_iterator begin() const noexcept
    {
        return data_.GetAddress();
    }

    const_iterator end() const noexcept
    {
        return data_.GetAddress() + size_;
    }

    const_iterator cbegin() const noexcept
    {
        return data_.GetAddress();
    }

    const_iterator cend() const noexcept
    {
        return data_.GetAddress() + size_;
    }

    // operator[]
    const T& operator[](size_t index) const noexcept {
        return const_cast<Vector&>(*this)[index];
    }

    T& operator[](size_t index) noexcept {
        assert(index < size_);
        return data_[index];
    }

    // methods
    void Swap(Vector& other) noexcept
    {
        if (data_.GetAddress() != other.data_.GetAddress())
        {
            data_.Swap(other.data_);
            std::swap(size_, other.size_);
        }
    }

    size_t Size() const noexcept {
        return size_;
    }

    void Resize(size_t new_size)
    {
        if (new_size < size_)
        {
            std::destroy_n(data_.GetAddress() + new_size, size_ - new_size);
            size_ = new_size;
        }
        else if (new_size > size_)
        {
            Reserve(new_size);
            std::uninitialized_value_construct_n(data_.GetAddress() + size_, new_size - size_);
            size_ = new_size;
        }
    }

    void PushBack(const T& value)
    {
        EmplaceBack(value); 
    }

    void PushBack(T&& value)
    {
        EmplaceBack(std::move(value));
    }//*/

    template <typename... Args>
    T& EmplaceBack(Args&&... args)
    {
        return *(Emplace(begin() + size_, std::forward<Args>(args)...));
    }

    template <typename... Args>
    iterator Emplace(const_iterator pos, Args&&... args)
    {
        if (size_ == Capacity())
        {
            return EmplaceWithRelocation(pos, std::forward<Args>(args)...);
        }
        else
        {
            return EmplaceWithoutRelocation(pos, std::forward<Args>(args)...);

        }//*/
    }

    iterator Erase(const_iterator pos) /*noexcept(std::is_nothrow_move_assignable_v<T>)*/
    {
        size_t position = pos - begin();
        std::move(begin() + position + 1, end(), begin() + position);
        PopBack();
        return data_ + position;;
    }

    iterator Insert(const_iterator pos, const T& value)
    {
        return Emplace(pos, value);
    }

    iterator Insert(const_iterator pos, T&& value)
    {
        return Emplace(pos, std::move(value));
    }

    void PopBack() /* noexcept */
    {
        if (size_)
        {
            std::destroy_n(data_.GetAddress() + size_ - 1, 1);
            --size_;
        }
    }

    size_t Capacity() const noexcept {
        return data_.Capacity();
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity <= data_.Capacity()) {
            return;
        }
        RawMemory<T> new_data(new_capacity);
        // constexpr оператор if будет вычислен во время компиляции
        if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>)
        {
            std::uninitialized_move_n(data_.GetAddress(), size_, new_data.GetAddress());
        }
        else
        {
            std::uninitialized_copy_n(data_.GetAddress(), size_, new_data.GetAddress());
        }
        std::destroy_n(data_.GetAddress(), size_);
        data_.Swap(new_data);
    }

private:
    // Вызывает деструкторы n объектов массива по адресу buf
    static void DestroyN(T* buf, size_t n) noexcept {
        for (size_t i = 0; i != n; ++i) {
            Destroy(buf + i);
        }
    }

    // Создаёт копию объекта elem в сырой памяти по адресу buf
    static void CopyConstruct(T* buf, const T& elem) {
        new (buf) T(elem);
    }

    // Вызывает деструктор объекта по адресу buf
    static void Destroy(T* buf) noexcept {
        buf->~T();
    }

    template <typename... Args>
    iterator EmplaceWithRelocation(const_iterator pos, Args&&... args)
    {
        size_t position = pos - begin(); 
        RawMemory<T> new_data(size_ == 0 ? 1 : size_ * 2);
        new (new_data + position) T(std::forward<Args>(args)...);
        if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>)
        {
            std::uninitialized_move_n(data_.GetAddress(), position, new_data.GetAddress());
            std::uninitialized_move_n(data_.GetAddress() + position, size_ - position, new_data.GetAddress() + position + 1);
        }
        else
        {
            std::uninitialized_copy_n(data_.GetAddress(), position, new_data.GetAddress());
            std::uninitialized_copy_n(data_.GetAddress() + position, size_ - position, new_data.GetAddress() + position + 1);
        }
        std::destroy_n(data_.GetAddress(), size_);
        data_.Swap(new_data);
        ++size_;

        return data_ + position;
    }

    template <typename... Args>
    iterator EmplaceWithoutRelocation(const_iterator pos, Args&&... args)
    {
        size_t position = pos - begin(); 
        if (pos != end())
        {
            T current(std::forward<Args>(args)...);
            std::uninitialized_move_n(end() - 1, 1, end());
            if (pos != end() - 1)
            {
                std::move_backward(begin() + position, end() - 1, end());
            }
            *(data_ + position) = std::move(current);
        }
        else
        {
            new (data_ + position) T(std::forward<Args>(args)...);
        }
        ++size_;

        return data_ + position;
    }

private:
    RawMemory<T> data_;
    size_t size_ = 0;
};