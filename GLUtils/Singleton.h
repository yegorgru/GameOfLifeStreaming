#pragma once

#include <memory>
#include <stdexcept>
#include <utility>

template <typename T>
class Singleton {
public:
    Singleton(const Singleton& other) = delete;
    Singleton(Singleton&& other) = delete;
    Singleton& operator=(const Singleton& other) = delete;
    Singleton& operator=(Singleton&& other) = delete;
    virtual ~Singleton() = default;
public:
    template <typename... Args>
    static void Init(Args&&... args);
    static void Destroy();
    static T& Get();
protected:
    using Instance = std::unique_ptr<T>;
protected:
    Singleton() = default;
protected:
    inline static Instance mInstance = nullptr;
};

template <typename T>
template <typename... Args>
void Singleton<T>::Init(Args&&... args) {
    mInstance = std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <typename T>
T& Singleton<T>::Get() {
    if (!mInstance) {
        throw std::runtime_error("Singleton is not initialized");
    }
    return *mInstance;
}

template <typename T>
void Singleton<T>::Destroy() {
    if (mInstance) {
        mInstance.reset();
    }
}