#pragma once
#include <cassert>
#include <cstdlib>
#include <new>
#include <utility>
#include <memory>
#include <algorithm>

template <typename T>
class RawMemory {
public:
    RawMemory() = default;

    explicit RawMemory(size_t capacity);

    RawMemory(const RawMemory&) = delete;
    RawMemory& operator=(const RawMemory& rhs) = delete;

    RawMemory(RawMemory&& other) noexcept;
    RawMemory& operator=(RawMemory&& rhs) noexcept;

    ~RawMemory();

    T* operator+(size_t offset) noexcept;
    const T* operator+(size_t offset) const noexcept;

    const T& operator[](size_t index) const noexcept;
    T& operator[](size_t index) noexcept;

    void Swap(RawMemory& other) noexcept;

    const T* GetAddress() const noexcept;

    T* GetAddress() noexcept;

    size_t Capacity() const;

private:
    // Выделяет сырую память под n элементов и возвращает указатель на неё
    static T* Allocate(size_t n);

    // Освобождает сырую память, выделенную ранее по адресу buf при помощи Allocate
    static void Deallocate(T* buf) noexcept;

    T* buffer_ = nullptr;
    size_t capacity_ = 0;
};

template <typename T>
class Vector {
public:
    using iterator = T*;
    using const_iterator = const T*;

    iterator begin() noexcept;
    iterator end() noexcept;
    const_iterator begin() const noexcept;
    const_iterator end() const noexcept;
    const_iterator cbegin() const noexcept;
    const_iterator cend() const noexcept;

    Vector() = default;
    explicit Vector(size_t size);

    Vector(const Vector& other);
    Vector& operator=(const Vector& rhs);

    Vector(Vector&& other) noexcept;
    Vector& operator=(Vector&& rhs) noexcept;

    void Swap(Vector& other) noexcept;

    size_t Size() const noexcept;
    size_t Capacity() const noexcept;

    void Reserve(size_t new_capacity);
    void Resize(size_t new_size);
    void PushBack(const T& value);
    void PushBack(T&& value);
    void PopBack() /* noexcept */;
    iterator Insert(const_iterator pos, const T& value);
    iterator Insert(const_iterator pos, T&& value);
    iterator Erase(const_iterator pos);

    template <typename... Args>
    T& EmplaceBack(Args&&... args);
    template <typename... Args>
    iterator Emplace(const_iterator pos, Args&&... args);

    const T& operator[](size_t index) const noexcept;
    T& operator[](size_t index) noexcept;

    ~Vector();

private:
    // Вызывает деструкторы n объектов массива по адресу buf
    static void DestroyN(T* buf, size_t n) noexcept;

    // Создаёт копию объекта elem в сырой памяти по адресу buf
    static void CopyConstruct(T* buf, const T& elem);

    static void MoveConstruct(T* buf, T&& elem);

    // Вызывает деструктор объекта по адресу buf
    static void Destroy(T* buf) noexcept;

private:
    RawMemory<T> data_;
    size_t size_ = 0;
};

template<typename T>
inline RawMemory<T>::RawMemory(size_t capacity)
    : buffer_(Allocate(capacity))
    , capacity_(capacity) {
}

template<typename T>
inline RawMemory<T>::RawMemory(RawMemory&& other) noexcept
    : buffer_(std::move(other.buffer_)), capacity_(std::move(other.capacity_))
{
    other.buffer_ = nullptr;
    other.capacity_ = 0;
}

template<typename T>
inline RawMemory<T>& RawMemory<T>::operator=(RawMemory&& rhs) noexcept
{
    buffer_ = std::move(rhs.buffer_);
    capacity_ = std::move(rhs.capacity_);
    rhs.buffer_ = nullptr;
    rhs.capacity_ = 0;
    return *this;
}

template<typename T>
inline RawMemory<T>::~RawMemory()
{
    Deallocate(buffer_);
}

template<typename T>
inline T* RawMemory<T>::operator+(size_t offset) noexcept
{
    // Разрешается получать адрес ячейки памяти, следующей за последним элементом массива
    assert(offset <= capacity_);
    return buffer_ + offset;
}

template<typename T>
inline const T* RawMemory<T>::operator+(size_t offset) const noexcept
{
    return const_cast<RawMemory&>(*this) + offset;
}

template<typename T>
inline const T& RawMemory<T>::operator[](size_t index) const noexcept
{
    return const_cast<RawMemory&>(*this)[index];
}

template<typename T>
inline T& RawMemory<T>::operator[](size_t index) noexcept
{
    assert(index < capacity_);
    return buffer_[index];
}

template<typename T>
inline void RawMemory<T>::Swap(RawMemory& other) noexcept
{
    std::swap(buffer_, other.buffer_);
    std::swap(capacity_, other.capacity_);
}

template<typename T>
inline const T* RawMemory<T>::GetAddress() const noexcept
{
    return buffer_;
}

template<typename T>
inline T* RawMemory<T>::GetAddress() noexcept
{
    return buffer_;
}

template<typename T>
inline size_t RawMemory<T>::Capacity() const
{
    return capacity_;
}

template<typename T>
inline T* RawMemory<T>::Allocate(size_t n)
{
    return n != 0 ? static_cast<T*>(operator new(n * sizeof(T))) : nullptr;
}

template<typename T>
inline void RawMemory<T>::Deallocate(T* buf) noexcept
{
    operator delete(buf);
}

template<typename T>
inline typename Vector<T>::iterator Vector<T>::begin() noexcept
{
    return data_.GetAddress();
}

template<typename T>
inline typename Vector<T>::iterator Vector<T>::end() noexcept
{
    return data_ + size_;
}

template<typename T>
inline typename Vector<T>::const_iterator Vector<T>::begin() const noexcept
{
    return data_.GetAddress();
}

template<typename T>
inline typename Vector<T>::const_iterator Vector<T>::end() const noexcept
{
    return data_ + size_;
}

template<typename T>
inline typename Vector<T>::const_iterator Vector<T>::cbegin() const noexcept
{
    return data_.GetAddress();
}

template<typename T>
inline typename Vector<T>::const_iterator Vector<T>::cend() const noexcept
{
    return data_ + size_;
}

template<typename T>
inline Vector<T>::Vector(size_t size)
    : data_(size)
    , size_(size)
{
    std::uninitialized_value_construct_n(data_.GetAddress(), size);
}

template<typename T>
inline Vector<T>::Vector(const Vector& other)
    : data_(other.size_)
    , size_(other.size_)
{
    std::uninitialized_copy_n(other.data_.GetAddress(), other.size_, data_.GetAddress());
}

template<typename T>
inline Vector<T>& Vector<T>::operator=(const Vector& rhs)
{
    if (this != &rhs)
    {
        if (data_.Capacity() >= rhs.size_)
        {
            if (rhs.size_ < size_)
            {
                for (size_t i = 0; i < rhs.size_; i++) {
                    data_[i] = rhs.data_[i];
                }
                for (size_t i = rhs.size_; i < size_; i++)
                {
                    Destroy(data_.GetAddress() + i);
                }
                size_ = rhs.size_;
            }
            else
            {
                for (size_t i = 0; i < size_; i++)
                {
                    data_[i] = rhs.data_[i];
                }
                std::uninitialized_copy(rhs.data_.GetAddress() + size_, rhs.data_.GetAddress() + rhs.size_, data_.GetAddress() + size_);
                size_ = rhs.size_;
            }
        }
        else
        {
            Vector rhs_copy(rhs);
            Swap(rhs_copy);
        }
    }
    return *this;
}

template<typename T>
inline Vector<T>::Vector(Vector&& other) noexcept
    : data_(std::move(other.data_))
    , size_(std::move(other.size_))
{
    other.size_ = 0;
}

template<typename T>
inline Vector<T>& Vector<T>::operator=(Vector&& rhs) noexcept
{
    Swap(rhs);
    return *this;
}

template<typename T>
inline void Vector<T>::Swap(Vector& other) noexcept
{
    data_.Swap(other.data_);
    std::swap(size_, other.size_);
}

template<typename T>
inline size_t Vector<T>::Size() const noexcept
{
    return size_;
}

template<typename T>
inline size_t Vector<T>::Capacity() const noexcept
{
    return data_.Capacity();
}

template<typename T>
inline void Vector<T>::Reserve(size_t new_capacity)
{
    if (new_capacity <= data_.Capacity()) {
        return;
    }
    RawMemory<T> new_data(new_capacity);
    if constexpr (!std::is_copy_constructible_v<T> || std::is_nothrow_move_constructible_v<T>) {
        std::uninitialized_move_n(data_.GetAddress(), size_, new_data.GetAddress());
    }
    else
    {
        std::uninitialized_copy_n(data_.GetAddress(), size_, new_data.GetAddress());
    }
    std::destroy_n(data_.GetAddress(), size_);
    data_.Swap(new_data);
}

template<typename T>
inline void Vector<T>::Resize(size_t new_size)
{
    if (new_size <= size_)
    {
        for (size_t i = new_size; i < size_; i++)
        {
            Destroy(data_.GetAddress() + i);
        }
    }
    else
    {
        Reserve(new_size);
        std::uninitialized_value_construct(data_.GetAddress() + size_, data_.GetAddress() + new_size);
    }
	size_ = new_size;
}

template<typename T>
inline void Vector<T>::PushBack(const T& value)
{
    EmplaceBack(value);
}

template<typename T>
inline void Vector<T>::PushBack(T&& value)
{
    EmplaceBack(std::move(value));
}

template<typename T>
inline void Vector<T>::PopBack()
{
    Destroy(data_ + size_ - 1);
    --size_;
}

template<typename T>
inline typename Vector<T>::iterator Vector<T>::Insert(const_iterator pos, const T& value)
{
    return Emplace(pos, value);
}

template<typename T>
inline typename Vector<T>::iterator Vector<T>::Insert(const_iterator pos, T&& value)
{
    return Emplace(pos, std::move(value));
}

template<typename T>
inline typename Vector<T>::iterator Vector<T>::Erase(const_iterator pos)
{
    size_t id = pos - begin();
    Destroy(std::move(begin() + id + 1, end(), begin() + id));
    --size_;
    return data_ + (pos - begin());
}

template<typename T>
inline const T& Vector<T>::operator[](size_t index) const noexcept
{
    return const_cast<Vector&>(*this)[index];
}

template<typename T>
inline T& Vector<T>::operator[](size_t index) noexcept
{
    assert(index < size_);
    return data_[index];
}

template<typename T>
inline Vector<T>::~Vector()
{
    DestroyN(data_.GetAddress(), size_);
}

template<typename T>
inline void Vector<T>::DestroyN(T* buf, size_t n) noexcept
{
    for (size_t i = 0; i != n; ++i) {
        Destroy(buf + i);
    }
}

template<typename T>
inline void Vector<T>::CopyConstruct(T* buf, const T& elem)
{
    new (buf) T(elem);
}

template<typename T>
inline void Vector<T>::MoveConstruct(T* buf, T&& elem)
{
    new (buf) T(std::move(elem));
}

template<typename T>
inline void Vector<T>::Destroy(T* buf) noexcept
{
    buf->~T();
}

template<typename T>
template<typename ...Args>
inline T& Vector<T>::EmplaceBack(Args&&... args)
{
    if (size_ < Capacity())
    {
        new (data_ + size_) T(std::forward<Args>(args)...);
    }
    else
    {
        RawMemory<T> new_data(size_ == 0 ? 1 : size_ * 2);
        new (new_data + size_) T(std::forward<Args>(args)...);
        if constexpr (!std::is_copy_constructible_v<T> || std::is_nothrow_move_constructible_v<T>) {
            for (size_t i = 0; i < size_; i++)
            {
                MoveConstruct(new_data + i, std::move(data_[i]));
            }
            DestroyN(data_.GetAddress(), size_);
        }
        else
        {
            for (size_t i = 0; i < size_; i++)
            {
                CopyConstruct(new_data + i, data_[i]);
            }
            DestroyN(data_.GetAddress(), size_);
        }
        data_.Swap(new_data);
    }
    ++size_;
    return data_[size_ - 1];
}

template<typename T>
template<typename... Args>
inline typename Vector<T>::iterator Vector<T>::Emplace(const_iterator pos, Args&&... args)
{
    iterator result_it;
    size_t id = pos - begin();
    if (size_ != Capacity())
    {
        if (size_ > 0)
        {
            T obj(std::forward<Args>(args)...);
            MoveConstruct(end(), std::move(*(end() - 1)));
            if (id + 1 < size_)
            {
                std::move_backward(begin() + id, end() - 1, end());
                data_[id] = std::move(obj);
            }
        }
        else
        {
			new (data_ + id) T(std::forward<Args>(args)...);
        }
        
        ++size_;
        return begin() + id;
    }
    else
    {
        RawMemory<T> new_data(size_ == 0 ? 1 : size_ * 2);
        new (new_data + id) T(std::forward<Args>(args)...);
        result_it = new_data + id;
        try
        {
            if constexpr (!std::is_copy_constructible_v<T> || std::is_nothrow_move_constructible_v<T>) {
                for (size_t i = 0; i < id; i++)
                {
					MoveConstruct(new_data + i, std::move(data_[i]));
                }
            }
            else
            {
                for (size_t i = 0; i < id; i++)
                {
                    CopyConstruct(new_data + i, data_[i]);
                }
            }
        }
        catch (...)
        {
            Destroy(new_data + id);
        }

        try
        {
            if constexpr (!std::is_copy_constructible_v<T> || std::is_nothrow_move_constructible_v<T>) {
                for (size_t i = id; i < size_; i++)
                {
                    MoveConstruct(new_data + i + 1, std::move(data_[i]));
                }
            }
            else
            {
                for (size_t i = id; i < size_; i++)
                {
                    CopyConstruct(new_data + i + 1, data_[i]);
                }
            }
        }
        catch (...)
        {
            DestroyN(new_data.GetAddress(), id + 1);
        }

        DestroyN(begin(), size_);
        data_.Swap(new_data);
        ++size_;
        return result_it;
    }

    return iterator();
}
