#ifndef SAFE_PTR_H
#define SAFE_PTR_H

#include <shared_mutex>
#include <atomic>

template<typename T>
// LockedPtr
// ReadPtr   -> shared_lock  / const T*
// WriterPtr  -> unique_lock / T*
class ReadPtr
{
public:
    /*
    ReadPtr(std::shared_mutex& mutex, const T* ptr)
        : _lock(mutex)
        , _ptr(ptr)
    {};*/

    ReadPtr(std::shared_lock<std::shared_mutex>&& lock, const T* ptr) noexcept
        : _lock(std::move(lock))
        , _ptr(ptr)
    {};

    // Move constructor
    ReadPtr(ReadPtr&& Other) noexcept
        : _lock(std::move(Other._lock))
        , _ptr(Other._ptr)
    {
        Other._ptr = nullptr;
    }

    template <class T2, std::enable_if<std::is_convertible<T2*, T*>::value, int>::type = 0>
    ReadPtr(ReadPtr<T2>&& _Other) noexcept
        : _lock(std::move(_Other._lock))
        , _ptr(_Other._ptr)
    {
        _Other._ptr = nullptr;
    }

    ReadPtr() noexcept
        : _ptr(nullptr)
    {}

    ~ReadPtr() {};

    const T* operator&() const
    {
        return _ptr;
    }

    const T* operator->() const
    {
        return _ptr;
    }

    explicit operator bool() const noexcept {
        return _ptr != nullptr;
    }

    // Explicitly forbid to copy the object
    ReadPtr(ReadPtr const&) = delete;
    void operator=(ReadPtr const&) = delete;

    //Move assignement
    ReadPtr& operator=(ReadPtr&& Other) noexcept
    {
        _lock = std::move(Other._lock);
        _ptr = Other._ptr;
        Other._ptr = nullptr;
        return *this;
    }

private:
    std::shared_lock<std::shared_mutex> _lock;
    const T* _ptr;
};

template<typename T>
class WritePtr
{
public:
    // TODO - pass a ref on the shared_mutex
    WritePtr(std::unique_lock<std::shared_mutex>&& lock, T* ptr)
        : _lock(std::move(lock))
        , _ptr(ptr)
    {};

    // Move constructor
    WritePtr(WritePtr&& Other) noexcept
        : _lock(std::move(Other._lock))
        , _ptr(Other._ptr)
    {
        Other._ptr = nullptr;
    }

    WritePtr() noexcept
        : _ptr(nullptr)
    {}

    ~WritePtr() {};

    T* operator&() const
    {
        return _ptr;
    }

    T* operator->() const
    {
        return _ptr;
    }

    explicit operator bool() const noexcept {
        return _ptr != nullptr;
    }

    // Explicitly forbid to copy the object
    WritePtr(WritePtr const&) = delete;
    void operator=(WritePtr const&) = delete;

    //Move assignement
    WritePtr& operator=(WritePtr&& Other) noexcept
    {
        _lock = std::move(Other._lock);
        _ptr = Other._ptr;
        Other._ptr = nullptr;
        return *this;
    }

private:
    std::unique_lock<std::shared_mutex> _lock;
    T* _ptr;
};

// NOTE - "no vtable" can optimize the memory size of the class
class /*__declspec(novtable)*/ Ref_count_safe {
private:
    virtual void destroy() noexcept = 0;     // destroy managed resource
    virtual void delete_this() noexcept = 0; // destroy self

    std::atomic<int> _counter = 1;

protected:
    // Cannot be constexpr because the class contain a mutex, which is not a literal type
    Ref_count_safe() noexcept = default;

    mutable std::shared_mutex _mutex;

public:
    Ref_count_safe(const Ref_count_safe&) = delete;
    Ref_count_safe& operator=(const Ref_count_safe&) = delete;

    virtual ~Ref_count_safe() noexcept {}

    void incref() noexcept {
        _counter.fetch_add(1);
    }

    void decref() noexcept {
        if (_counter.fetch_sub(1) == 1)
        {
            destroy(); // _destroy()
            delete_this(); // _delete_this();
        }
    }

    virtual std::shared_lock<std::shared_mutex> try_lock_read_if_not_null(bool tryLock) noexcept = 0;
    virtual std::unique_lock<std::shared_mutex> try_lock_write_if_not_null(bool tryLock) noexcept = 0;

    virtual std::shared_lock<std::shared_mutex> defer_lock_read() noexcept = 0;
    virtual std::unique_lock<std::shared_mutex> defer_lock_write() noexcept = 0;
    virtual void force_destroy() noexcept = 0;
};

template<typename T>
class CtrlBlock : public Ref_count_safe
{
public:
    template <class... Types>
    explicit CtrlBlock(Types&&... args) : Ref_count_safe() {
        _ptr = new T(std::forward<Types>(args)...);
    }

    ~CtrlBlock()
    {
        // nothing, the managed object is destroyed in destroy()
        //if (_ptr) delete _ptr;
    };

    virtual std::shared_lock<std::shared_mutex> try_lock_read_if_not_null(bool tryLock) noexcept override {
        auto sl = tryLock ? std::shared_lock(_mutex, std::try_to_lock_t()) : std::shared_lock(_mutex);
        if (_ptr == nullptr)
            return std::shared_lock<std::shared_mutex>();
        else
            return sl;
    }

    virtual std::unique_lock<std::shared_mutex> try_lock_write_if_not_null(bool tryLock) noexcept override {
        auto ul = tryLock ? std::unique_lock(_mutex, std::try_to_lock_t()) : std::unique_lock(_mutex);
        if (_ptr == nullptr)
            return std::unique_lock<std::shared_mutex>();
        else
            return ul;
    }


    virtual std::shared_lock<std::shared_mutex> defer_lock_read() noexcept override
    {
        auto ul = std::shared_lock(_mutex, std::defer_lock_t());
        if (_ptr == nullptr)
            return std::shared_lock<std::shared_mutex>();
        else
            return ul;

    }

    virtual std::unique_lock<std::shared_mutex> defer_lock_write() noexcept override
    {
        auto ul = std::unique_lock(_mutex, std::defer_lock_t());
        if (_ptr == nullptr)
            return std::unique_lock<std::shared_mutex>();
        else
            return ul;
    }

    virtual void force_destroy() noexcept override {
        auto ul = std::unique_lock(_mutex);
        delete _ptr;
        _ptr = nullptr;
    }

    T* _ptr;

private:
    virtual void destroy() noexcept override {
        delete _ptr;
    }

    virtual void delete_this() noexcept override {
        delete this;
    }
};

template <typename T>
class SafePtr
{
    template <class U>
    friend class SafePtr;

public:
    constexpr SafePtr() noexcept = default;
    SafePtr(const SafePtr& _Other) noexcept {
        copy_construct_from(_Other);
    };

    template <class T2, std::enable_if<std::is_convertible<T2*, T*>::value, int>::type = 0>
    SafePtr(const SafePtr<T2>& _Other) noexcept {
        copy_construct_from(_Other);
    };

    SafePtr(SafePtr&& _Other) noexcept {
        move_construct_from(std::move(_Other));
    }

    template <class T2, std::enable_if<std::is_convertible<T2*, T*>::value, int>::type = 0>
    SafePtr(SafePtr<T2>&& _Other) noexcept {
        move_construct_from(std::move(_Other));
    }

    template <class T2>
    SafePtr(const SafePtr<T2>& _Right, T* _px) noexcept {
        // construct SafePtr object that aliases _Right
        this->alias_construct_from(_Right, _px);
    }

    // decrease the ref count by 1
    ~SafePtr() noexcept {
        this->decref();
    };

    SafePtr& operator=(const SafePtr& _Right) noexcept {
        // NOTE - le shared_ptr utilise {copy_ctor + swap()} pour prendre en charge automatiquement les différentes
        SafePtr(_Right).swap(*this);
        return *this;
    };

    template<class T2>
    SafePtr& operator=(const SafePtr<T2>& _Right) noexcept {
        SafePtr(_Right).swap(*this);
        return *this;
    };

    template<class T2>
    [[nodiscard]] constexpr bool operator==(const SafePtr<T2>& _Right) const noexcept {
        return this->_ref == _Right._ref;
    };

#if _HAS_CXX20
    template<class T2>
    [[nodiscard]] std::strong_ordering operator<=>(const SafePtr<T2>& _Right) const noexcept {
        return this->_ref <=> _Right._ref;
    };
#endif

    // TODO - On peut aussi ajouter les operator=(const SafePtr&&) pour faire des move(),
    //        mais je ne vois dans quel cas c'est utile.

    void reset() {
        SafePtr().swap(*this);
    }

    void destroy() { // delete the object pointed by _ptr AND reset the control block.
        if (_ref) {
            _ref->force_destroy();
            reset();
        }
    }

    WritePtr<T> get(bool tryLock = false) const noexcept {
        // test si _cb
        //   * OUI -> lock, continue
        //   * NON -> return 0
        // test si _cb._ptr
        //   * OUI -> continue
        //   * NON -> reset, return 0
        std::unique_lock lock = lock_write(tryLock);  // try_lock_write(), non bloquant
        if (lock.owns_lock()) {
            return WritePtr<T>(std::move(lock), _ptr);
        }
        else
        {
            return WritePtr<T>();
        }
    }

    // rvalue && ?
    // [[nodiscard]] ?
    ReadPtr<T> cget(bool tryLock = false) const noexcept {
        //return ReadPtr<T>(lock_read(), _ptr);
        
        // On veut vérifier que le control block possède toujours le pointer géré.
        //  * Si oui, on lock le mutex et on renvoie le pointer stocké
        //  * Si non, on se sépare du bloc de contrôle et on renvoie nullptr
        if (!_ref)
            return ReadPtr<T>();

        std::shared_lock<std::shared_mutex> lock = lock_read(tryLock);
        if (lock.owns_lock()) {
            return ReadPtr<T>(std::move(lock), _ptr);
        }
        else {
            return ReadPtr<T>();
        }
    }

    explicit operator bool() const noexcept {
        return _ptr != nullptr;
    }

protected:
    template <class T2>
    void copy_construct_from(const SafePtr<T2>& _Other) noexcept {
        _Other.incref();
        _ref = _Other._ref;
        _ptr = _Other._ptr;
    }

    template <class T2>
    void move_construct_from(SafePtr<T2>&& _Other) noexcept {
        _ptr = _Other._ptr;
        _ref = _Other._ref;

        _Other._ptr = nullptr;
        _Other._ref = nullptr;
    }

    template <class T2>
    void alias_construct_from(const SafePtr<T2>& _Other, T* _px) noexcept {
        _Other.incref();
        _ptr = _px;
        _ref = _Other._ref;
    }

    void incref() const noexcept {
        if (_ref) {
            _ref->incref();
        }
    }

    void decref() const noexcept {
        if (_ref) {
            _ref->decref();
        }
    }

    void swap(SafePtr& _Right) noexcept {
        std::swap(_ptr, _Right._ptr);
        std::swap(_ref, _Right._ref);
    };

    std::unique_lock<std::shared_mutex> lock_write(bool tryLock) const noexcept {
        if (_ref) {
            return _ref->try_lock_write_if_not_null(tryLock);
        }
        else {
            return std::unique_lock<std::shared_mutex>(); // empty lock
        }
    }

    std::shared_lock<std::shared_mutex> lock_read(bool tryLock) const noexcept {
        if (_ref) {
            return _ref->try_lock_read_if_not_null(tryLock);
        }
        else {
            return std::shared_lock<std::shared_mutex>(); // empty lock
        }
    }

    void set_ptr_and_ref(T* const in_ptr, CtrlBlock<T>* const in_ref)
    {
        _ptr = in_ptr;
        _ref = in_ref;
    }

    T* _ptr = nullptr;           // Stored pointer
    Ref_count_safe* _ref = nullptr; // implement a CtrlBlock of a independent type

    template<class Ty, class... Types>
    friend SafePtr<Ty> make_safe(Types&&... args);

    friend struct std::hash<SafePtr<T>>;

    template <class Ty1, class Ty2>
    friend SafePtr<Ty1> static_pointer_cast(const SafePtr<Ty2>& _Other) noexcept;


    template <class Ty1, class Ty2>
    friend void multi_cget(const SafePtr<Ty1>& sptr1, const SafePtr<Ty2>& sptr2, ReadPtr<Ty1>& rptr1, ReadPtr<Ty2>& rptr2) noexcept;

    template <class Ty1, class Ty2>
    friend void multi_get(const SafePtr<Ty1>& sptr1, const SafePtr<Ty2>& sptr2, WritePtr<Ty1>& rptr1, WritePtr<Ty2>& rptr2) noexcept;

    template <class Ty1, class Ty2, class Ty3> //NOTE(Quentin) Comment généraliser avec des Types différents ?
    friend void multi_get(const SafePtr<Ty1>& sptr1, const SafePtr<Ty2>& sptr2, const SafePtr<Ty3>& sptr3, WritePtr<Ty1>& rptr1, WritePtr<Ty2>& rptr2, WritePtr<Ty3>& rptr3) noexcept;
};

template <typename Ty, class... Types>
SafePtr<Ty> make_safe(Types&&... args)
{
    CtrlBlock<Ty>* const _new_cb = new CtrlBlock<Ty>(std::forward<Types>(args)...);
    SafePtr<Ty> _Ret;
    _Ret.set_ptr_and_ref(_new_cb->_ptr, _new_cb);
    return _Ret;
}

namespace std
{
    template<class Ty1>
    struct hash<SafePtr<Ty1>>
    {
        std::size_t operator()(const SafePtr<Ty1>& sptr) const
        {
            return std::hash<Ty1*>()(sptr._ptr);
        }
    };
}

template <class Ty1, class Ty2>
[[nodiscard]] SafePtr<Ty1> static_pointer_cast(const SafePtr<Ty2>& _Other) noexcept {
    // static_cast for SafePtr that check on compile time that the static_cast on the ptr works.
    const auto cast_ptr = static_cast<Ty1*>(_Other._ptr);
    return SafePtr<Ty1>(_Other, cast_ptr);
}

template <class Ty1, class Ty2>
[[nodiscard]] ReadPtr<Ty1> static_read_cast(const SafePtr<Ty2>& _Other) noexcept {
    SafePtr<Ty1> sp(static_pointer_cast<Ty1>(_Other));
    return sp.cget();
}

template <class Ty1, class Ty2>
[[nodiscard]] WritePtr<Ty1> static_write_cast(const SafePtr<Ty2>& _Other) noexcept {
    SafePtr<Ty1> sp(static_pointer_cast<Ty1>(_Other));
    return sp.get();
}

template <class Ty1, class Ty2>
void multi_cget(const SafePtr<Ty1>& sptr1, const SafePtr<Ty2>& sptr2, ReadPtr<Ty1>& rptr1, ReadPtr<Ty2>& rptr2) noexcept
{
    if (sptr1._ref == nullptr || sptr2._ref == nullptr)
        return;

    std::shared_lock<std::shared_mutex> slock1 = sptr1._ref->defer_lock_read();
    std::shared_lock<std::shared_mutex> slock2 = sptr2._ref->defer_lock_read();
    try
    {
        std::lock(slock1, slock2);
    }
    catch (...)
    {
        return;
    }

    if(slock1.owns_lock())
        rptr1 = std::move(ReadPtr<Ty1>(std::move(slock1), sptr1._ptr));

    if (slock2.owns_lock())
        rptr2 = std::move(ReadPtr<Ty2>(std::move(slock2), sptr2._ptr));

}

template<class Ty1, class Ty2>
void multi_get(const SafePtr<Ty1>& sptr1, const SafePtr<Ty2>& sptr2, WritePtr<Ty1>& rptr1, WritePtr<Ty2>& rptr2) noexcept
{
    if (sptr1._ref == nullptr || sptr2._ref == nullptr)
        return;

    std::unique_lock<std::shared_mutex> slock1 = sptr1._ref->defer_lock_write();
    std::unique_lock<std::shared_mutex> slock2 = sptr2._ref->defer_lock_write();
    try
    {
        std::lock(slock1, slock2);
    }
    catch (...)
    {
        return;
    }

    if (slock1.owns_lock())
        rptr1 = std::move(WritePtr<Ty1>(std::move(slock1), sptr1._ptr));

    if (slock2.owns_lock())
        rptr2 = std::move(WritePtr<Ty2>(std::move(slock2), sptr2._ptr));
}

template<class Ty1, class Ty2, class Ty3>
inline void multi_get(const SafePtr<Ty1>& sptr1, const SafePtr<Ty2>& sptr2, const SafePtr<Ty3>& sptr3, WritePtr<Ty1>& rptr1, WritePtr<Ty2>& rptr2, WritePtr<Ty3>& rptr3) noexcept
{
    if (sptr1._ref == nullptr || sptr2._ref == nullptr || sptr3._ref == nullptr)
        return;

    std::unique_lock<std::shared_mutex> slock1 = sptr1._ref->defer_lock_write();
    std::unique_lock<std::shared_mutex> slock2 = sptr2._ref->defer_lock_write();
    std::unique_lock<std::shared_mutex> slock3 = sptr3._ref->defer_lock_write();

    try
    {
        std::lock(slock1, slock2, slock3);
    }
    catch (...)
    {
        return;
    }

    if (slock1.owns_lock())
        rptr1 = std::move(WritePtr<Ty1>(std::move(slock1), sptr1._ptr));

    if (slock2.owns_lock())
        rptr2 = std::move(WritePtr<Ty2>(std::move(slock2), sptr2._ptr));

    if (slock3.owns_lock())
        rptr3 = std::move(WritePtr<Ty3>(std::move(slock3), sptr3._ptr));
}


#endif // SAFE_PTR_H
