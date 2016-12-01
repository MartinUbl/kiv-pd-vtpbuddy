#ifndef VTPBUDDY_SINGLETON_H
#define VTPBUDDY_SINGLETON_H

template<class T>
class Singleton
{
    public:
        static T* getInstance()
        {
            if (!instance)
                instance = new T();

            return instance;
        }

    protected:
        static T* instance;
};

template<class T>
T* Singleton<T>::instance = nullptr;

#endif
