#include <memory>
#include <string>
#include <functional>
#include <type_traits>
#include <vector>
#include <tuple>
#include <unordered_map>
#include <sstream>
#include <ostream>
#include <iostream>

namespace js
{

struct any;

typedef void (*functionPtr)(void);
typedef std::function<void(void)> functionType;
typedef std::unordered_map<std::string, any> objectType;
typedef std::vector<any> arrayType;

enum anyTypeId
{
    undefined,
    null,
    boolean,
    integer,
    integer64,
    real,
    const_string,
    function,
    closure,
    array,
    object,
    string,
};

union anyType {
    bool boolean;
    int integer;
    long integer64; // i think we can use pointer to data to reduce size of anyType
    double real;    // i think we can use pointer to data to reduce size of anyType
    const char *const_string;
    functionPtr function;

    functionType *closure;
    objectType *object;
    arrayType *array;
    std::string *string;
};

template <class T>
struct base_const_iterator
{
  public:
    using iterator_category = std::forward_iterator_tag;

    using self_type = base_const_iterator;
    using value_type = T;
    using pointer = value_type*;
    using reference = value_type&;

    virtual const reference operator*() const {};

    virtual self_type &operator++() {};

    virtual bool operator!=(const self_type &_right) const {};
};

template <class T>
struct base_iterator
{
  public:
    using iterator_category = std::forward_iterator_tag;

    using self_type = base_iterator;
    using value_type = T;
    using pointer = value_type*;
    using reference = value_type&;

    virtual reference operator*() const {};

    virtual self_type &operator++() {};

    virtual bool operator!=(const self_type &_right) const {};
};

template <class T>
struct index_const_iterator : public base_const_iterator<T>
{
  public:
    using iterator_category = std::forward_iterator_tag;

    using self_type = index_const_iterator;
    using value_type = T;
    using pointer = value_type*;
    using reference = value_type&;

    index_const_iterator(T indx, T end) : _indx(indx), _end(end)
    {
    }    

    virtual const reference operator*() const override
    {
        return (reference) _indx;
    }

    virtual self_type &operator++() override
    {
        ++_indx;
        return (*this);
    }

    virtual bool operator!=(const base_const_iterator<T> &right) const override
    {
        // assume right contains end of iteration
        return !(_indx == _end);
    }

    value_type _indx;
    value_type _end;
};

template <class T>
class index_iterator : public base_iterator<T>
{
  public:
    using iterator_category = std::forward_iterator_tag;

    using self_type = index_iterator;
    using value_type = T;
    using pointer = value_type*;
    using reference = value_type&;

    index_iterator(T indx, T end) : _indx(indx), _end(end)
    {
    }    

    virtual reference operator*() const override
    {
        return (reference) _indx;
    }

    virtual self_type &operator++() override
    {
        ++_indx;
        return (*this);
    }

    virtual bool operator!=(const base_iterator<T> &right) const override
    {
        // assume right contains end of iteration
        return !(_indx == _end);
    }

    value_type _indx;
    value_type _end;
};

template <class T>
class object_keys_const_iterator : public base_const_iterator<T>
{
  public:
    using iterator_category = std::forward_iterator_tag;

    using self_type = object_keys_const_iterator;
    using value_type = T;
    using pointer = value_type*;
    using reference = value_type&;

    using beginInstType = decltype(((objectType*)nullptr)->cbegin());
    using endInstType = decltype(((objectType*)nullptr)->cend());

    object_keys_const_iterator(beginInstType beginInst, endInstType endInst) 
        : _indx(), _beginInst(beginInst), _endInst(endInst)
    {
        if (_beginInst != _endInst) 
        {
            _indx = _beginInst->first;
        }
    }    

    virtual const reference operator*() const override
    {
        return (reference) _indx;
    }

    virtual self_type &operator++() override
    {
        ++_beginInst;
        if (_beginInst != _endInst) 
        {
            _indx = _beginInst->first;
        }            

        return (*this);
    }

    virtual bool operator!=(const base_const_iterator<T> &right) const override
    {
        // assume right contains end of iteration
        return _beginInst != _endInst;
    }

    value_type _indx;
    beginInstType _beginInst;    
    endInstType _endInst;    
};

template <class T>
class object_keys_iterator : public base_iterator<T>
{
  public:
    using iterator_category = std::forward_iterator_tag;

    using self_type = object_keys_iterator;
    using value_type = T;
    using pointer = value_type*;
    using reference = value_type&;

    using beginInstType = decltype(((objectType*)nullptr)->begin());
    using endInstType = decltype(((objectType*)nullptr)->end());

    object_keys_iterator(beginInstType beginInst, endInstType endInst) 
        : _indx(), _beginInst(beginInst), _endInst(endInst)
    {
        if (_beginInst != _endInst) 
        {
            _indx = _beginInst->first;
        }
    }    

    virtual reference operator*() const override
    {
        return (reference) _indx;
    }

    virtual self_type &operator++() override
    {
        ++_beginInst;
        if (_beginInst != _endInst) 
        {
            _indx = _beginInst->first;
        }            

        return (*this);
    }

    virtual bool operator!=(const base_iterator<T> &right) const override
    {
        // assume right contains end of iteration
        return _beginInst != _endInst;
    }

    value_type _indx;
    beginInstType _beginInst;    
    endInstType _endInst;    
};

struct any
{
    anyTypeId _type;
    anyType _value;

    struct iterator
    {
        iterator(int initial, int size) : _initial(initial), _size(size), _obj(nullptr)
        {            
        }

        iterator(objectType *obj) : _initial(0), _size(0), _obj(obj)
        {            
        }

        base_const_iterator<any> cbegin()
        {            
            return _obj 
                ? (base_const_iterator<any>)object_keys_const_iterator<any>(_obj->cbegin(), _obj->cend()) 
                : (base_const_iterator<any>)index_const_iterator<any>(_initial, _size);
        }

        base_iterator<any> begin()
        {
            return _obj 
                ? (base_iterator<any>)object_keys_iterator<any>(_obj->begin(), _obj->end()) 
                : (base_iterator<any>)index_iterator<any>(_initial, _size);
        }

        base_const_iterator<any> cend()
        {
            return _obj 
                ? (base_const_iterator<any>)object_keys_const_iterator<any>(_obj->cend(), _obj->cend()) 
                : (base_const_iterator<any>)index_const_iterator<any>(_size, _size);
        }        

        base_iterator<any> end()
        {
            return _obj 
                ? (base_iterator<any>)object_keys_iterator<any>(_obj->end(), _obj->end())
                : (base_iterator<any>)index_iterator<any>(_size, _size);
        }    

        int _initial;
        int _size;
        objectType* _obj;
    };

    any()
    {
        _type = anyTypeId::undefined;
    }

    any(const any &other)
    {
        _type = other._type;
        _value = other._value;
    }

    any(any &&other)
    {
        _type = other._type;
        _value = other._value;
        other._type = undefined;
    }

    any(bool value)
    {
        _type = anyTypeId::boolean;
        _value.boolean = value;
    }

    any(int value)
    {
        _type = anyTypeId::integer;
        _value.integer = value;
    }

    any(long value)
    {
        _type = anyTypeId::integer64;
        _value.integer64 = value;
    }

    any(double value)
    {
        _type = anyTypeId::real;
        _value.real = value;
    }

    any(std::nullptr_t value)
    {
        _type = anyTypeId::null;
        _value.string = value;
    }

    any(const char *value)
    {
        _type = anyTypeId::const_string;
        _value.const_string = value;
    }

    any(const std::string &value)
    {
        _type = anyTypeId::string;
        _value.string = new std::string(value);
    }

    template <class R, class... Args>
    any(R (*value)(Args &&...))
    {
        _type = anyTypeId::function;
        _value.function = (functionPtr)value;
    }

    template <class R, class... Args>
    any(std::function<R(Args &&...)> func)
    {
        _type = anyTypeId::closure;
        _value.closure = new functionType(func);
    }

    any(const std::initializer_list<any> &values)
    {
        _type = anyTypeId::array;

        std::vector<any> vals;
        vals.reserve(values.size());
        for (auto &item : values)
        {
            vals.push_back(item);
        }

        _value.array = new arrayType(vals);
    }

    any(const std::initializer_list<std::tuple<any, any>> &values)
    {
        _type = anyTypeId::object;

        objectType obj;
        for (auto &item : values)
        {
            std::string stringIndex;
            auto index = std::get<0>(item);
            switch (index._type)
            {
            case anyTypeId::integer:
                stringIndex = std::to_string(index._value.integer);
                break;
            case anyTypeId::integer64:
                stringIndex = std::to_string(index._value.integer64);
                break;
            case anyTypeId::const_string:
                stringIndex = index._value.const_string;
                break;
            case anyTypeId::string:
                stringIndex = *index._value.string;
                break;
            }

            obj[stringIndex] = std::get<1>(item);
        }

        _value.object = new objectType(obj);
    }

    any(anyTypeId initType)
    {
        _type = initType;

        switch (_type)
        {
        case anyTypeId::array:
            _value.array = new arrayType();
            return;

        case anyTypeId::object:
            _value.object = new objectType();
            return;
        }

        throw "wrong type";
    }

    operator bool()
    {
        switch (_type)
        {
        case anyTypeId::undefined:
        case anyTypeId::null:
            return false;

        case anyTypeId::boolean:
            return _value.boolean;

        case anyTypeId::integer:
            return (bool)_value.integer;

        case anyTypeId::integer64:
            return (bool)_value.integer64;

        case anyTypeId::const_string:
            return (bool)_value.const_string[0];

        case anyTypeId::string:
            return _value.string->size() > 0;
        }

        return true;
    }

    operator int()
    {
        switch (_type)
        {
        case anyTypeId::undefined:
        case anyTypeId::null:
            return 0;

        case anyTypeId::boolean:
            return (int)_value.boolean;

        case anyTypeId::integer:
            return _value.integer;

        case anyTypeId::integer64:
            return (int)_value.integer64;

        case anyTypeId::real:
            return (int)_value.real;

        case anyTypeId::const_string:
            return std::stoi(_value.const_string);

        case anyTypeId::string:
            return std::stoi(*_value.string);
        }

        return true;
    }

    void operator()()
    {
        switch (_type)
        {
        case anyTypeId::function:
            _value.function();
            return;

        case anyTypeId::closure:
            (*(_value.closure))();
            return;

        default:
            break;
        }

        throw "not function or closure";
    }

    template <class T, class = std::enable_if<std::is_integral_v<T>>>
    const any operator[](T index) const
    {
        try
        {
            switch (_type)
            {
            case anyTypeId::array:
                return (*(_value.array))[index];
            case anyTypeId::object:
                return (*(_value.object))[std::to_string(index)];
            }
        }
        catch (const std::out_of_range &)
        {
            return any();
        }

        throw "not an array or an object";
    }

    const any operator[](const char *field) const
    {
        try
        {
            switch (_type)
            {
            case anyTypeId::object:
                return (*(_value.object))[field];
            }
        }
        catch (const std::out_of_range &)
        {
            return any();
        }

        throw "not an array or an object";
    }

    template <class T, class = std::enable_if<std::is_integral_v<T>>>
    any &operator[](T index)
    {
        int tries = 2;
        while (tries-- > 0)
        {
            try
            {
                switch (_type)
                {
                case anyTypeId::array:
                    return (_value.array)->at(index);
                case anyTypeId::object:
                    return (_value.object)->at(std::to_string(index));
                }
            }
            catch (const std::out_of_range &)
            {
                if (tries < 1)
                {
                    throw;
                }

                // create new element
                any newUndefined;
                switch (_type)
                {
                case anyTypeId::array:
                {
                    auto &arrayInst = (*(_value.array));
                    while (arrayInst.size() <= index)
                    {
                        arrayInst.push_back(newUndefined);
                    }
                }

                break;
                case anyTypeId::object:
                    (*(_value.object))[std::to_string(index)] = newUndefined;
                    break;
                }

                continue;
            }
        }

        throw "not an object";
    }

    any &operator[](const char *field)
    {
        int tries = 2;
        while (tries-- > 0)
        {
            try
            {
                switch (_type)
                {
                case anyTypeId::object:
                    return (_value.object)->at(field);
                }
            }
            catch (const std::out_of_range &)
            {
                if (tries < 1)
                {
                    throw;
                }

                // create new element
                any newUndefined;
                switch (_type)
                {
                case anyTypeId::object:
                    (*(_value.object))[field] = newUndefined;
                    break;
                }

                continue;
            }
        }

        throw "not an object";
    }

    any &operator[](const any &index)
    {
        int tries = 2;
        while (tries-- > 0)
        {
            try
            {
                switch (_type)
                {
                case anyTypeId::array:
                    switch (index._type)
                    {
                    case anyTypeId::integer:
                        return (_value.array)->at(index._value.integer);
                    case anyTypeId::integer64:
                        return (_value.array)->at(index._value.integer64);
                    }

                    throw "not allowed index type";
                case anyTypeId::object:
                    switch (index._type)
                    {
                    case anyTypeId::integer:
                        return (_value.object)->at(std::to_string(index._value.integer));
                    case anyTypeId::integer64:
                        return (_value.object)->at(std::to_string(index._value.integer64));
                    case anyTypeId::const_string:
                        return (_value.object)->at(index._value.const_string);
                    case anyTypeId::string:
                        return (_value.object)->at(*(index._value.string));
                    }

                    throw "not allowed index type";
                }
            }
            catch (const std::out_of_range &)
            {
                if (tries < 1)
                {
                    throw;
                }

                // create new element
                any newUndefined;
                switch (_type)
                {
                case anyTypeId::array:
                    switch (index._type)
                    {
                    case anyTypeId::integer:
                    {
                        auto &arrayInst = (*(_value.array));
                        while (arrayInst.size() <= index._value.integer)
                        {
                            arrayInst.push_back(newUndefined);
                        }
                    }

                    break;
                    case anyTypeId::integer64:
                    {
                        auto &arrayInst = (*(_value.array));
                        while (arrayInst.size() <= index._value.integer64)
                        {
                            arrayInst.push_back(newUndefined);
                        }
                    }

                    break;
                    default:
                        throw "not allowed index type";
                        break;
                    }

                    continue;

                case anyTypeId::object:
                    switch (index._type)
                    {
                    case anyTypeId::integer:
                        (*(_value.object))[std::to_string(index._value.integer)] = newUndefined;
                        break;
                    case anyTypeId::integer64:
                        (*(_value.object))[std::to_string(index._value.integer64)] = newUndefined;
                        break;
                    case anyTypeId::const_string:
                        (*(_value.object))[index._value.const_string] = newUndefined;
                        break;
                    case anyTypeId::string:
                        (*(_value.object))[*(index._value.string)] = newUndefined;
                        break;
                    default:
                        throw "not allowed index type";
                        break;
                    }

                    continue;
                }
            }
        }

        throw "not an array or an object";
    }

    const any operator[](const any &index) const
    {
        try
        {
            switch (_type)
            {
            case anyTypeId::array:
                switch (index._type)
                {
                case anyTypeId::integer:
                    return (*(_value.array))[index._value.integer];
                case anyTypeId::integer64:
                    return (*(_value.array))[index._value.integer64];
                }

                throw "not allowed index type";
            case anyTypeId::object:
                switch (index._type)
                {
                case anyTypeId::integer:
                    return (*(_value.object))[std::to_string(index._value.integer)];
                case anyTypeId::integer64:
                    return (*(_value.object))[std::to_string(index._value.integer64)];
                case anyTypeId::const_string:
                    return (*(_value.object))[index._value.const_string];
                case anyTypeId::string:
                    return (*(_value.object))[*(index._value.string)];
                }

                throw "not allowed index type";
            }
        }
        catch (const std::out_of_range &)
        {
            return any();
        }

        throw "not an array or an object";
    }

    iterator keys() 
    {
        if (_type == anyTypeId::array)
        {
            return iterator(0, _value.array->size());
        }

        if (_type == anyTypeId::object)
        {
            return iterator(_value.object);
        }

        throw "can't iterate";
    }

    std::vector<js::any>::iterator begin()
    {
        if (_type == anyTypeId::array)
        {
            return _value.array->begin();
        }

        throw "not an array or anobject";
    }

    std::vector<js::any>::const_iterator cbegin() const
    {
        if (_type == anyTypeId::array)
        {
            return _value.array->cbegin();
        }

        throw "not an array or anobject";
    }

    std::vector<js::any>::iterator end()
    {
        if (_type == anyTypeId::array)
        {
            return _value.array->end();
        }

        throw "not an array or anobject";
    }

    std::vector<js::any>::const_iterator cend() const
    {
        if (_type == anyTypeId::array)
        {
            return _value.array->cend();
        }

        throw "not an array or anobject";
    }

    any &operator=(const any &other)
    {
        _type = other._type;
        _value = other._value;
        return *this;
    }

    template <class T, class = std::enable_if<std::is_integral_v<T>>>
    any operator+(T other)
    {
        switch (_type)
        {
        case anyTypeId::integer:
            return any(_value.integer + other);

        case anyTypeId::integer64:
            return any(_value.integer64 + other);
            break;

        case anyTypeId::real:
            return any(_value.real + other);
            break;

        case anyTypeId::const_string:
        case anyTypeId::string:
        {
            std::stringstream stream;
            stream << *this;
            stream << other;
            return any(stream.str());
        }

        default:
            throw "wrong type";
        }

        throw "not implemented";
    }

    any operator+(const char *other)
    {
        switch (_type)
        {
        case anyTypeId::integer:
            return any(_value.integer + std::stoi(other));

        case anyTypeId::integer64:
            return any(_value.integer64 + std::stol(other));
            break;

        case anyTypeId::const_string:
        case anyTypeId::string:
        {
            std::stringstream stream;
            stream << *this;
            stream << other;
            return any(stream.str());
        }

        default:
            throw "wrong type";
        }

        throw "not implemented";
    }

    any operator+(const any &other)
    {
        switch (other._type)
        {
        case anyTypeId::const_string:
        case anyTypeId::string:
        {
            std::stringstream stream;
            stream << *this;
            stream << other;
            return any(stream.str());
        }
        }

        switch (_type)
        {
        case anyTypeId::const_string:
        case anyTypeId::string:
        {
            std::stringstream stream;
            stream << *this;
            stream << other;
            return any(stream.str());
        }

        default:
            throw "wrong type";
        }

        throw "not implemented";
    }

    any& operator+=(const any &other)
    {
        (*this) = (*this) + other;
        return *this;
    }    

    template <class T, class = std::enable_if<std::is_integral_v<T>>>
    auto operator>(T other)
    {
        switch (_type)
        {
        case anyTypeId::integer:
            return _value.integer > other;

        case anyTypeId::integer64:
            return _value.integer64 > other;

        case anyTypeId::real:
            return _value.real > other;

        case anyTypeId::const_string:
            return std::stoi(_value.const_string) > other;
        case anyTypeId::string:
            return std::stoi(*(_value.string)) > other;

        default:
            throw "wrong type";
        }

        throw "not implemented";
    }

    template <class T, class = std::enable_if<std::is_integral_v<T>>>
    auto operator<(T other)
    {
        switch (_type)
        {
        case anyTypeId::integer:
            return _value.integer < other;

        case anyTypeId::integer64:
            return _value.integer64 < other;

        case anyTypeId::real:
            return _value.real < other;

        case anyTypeId::const_string:
            return std::stoi(_value.const_string) < other;
        case anyTypeId::string:
            return std::stoi(*(_value.string)) < other;

        default:
            throw "wrong type";
        }

        throw "not implemented";
    }

    friend bool operator==(const any& lhs, const any& rhs)
    {
        if (lhs._type != rhs._type) 
        {
            return false;
        }

        switch (lhs._type)
        {
        case anyTypeId::integer:
            return lhs._value.integer == rhs._value.integer;
        case anyTypeId::integer64:
            return lhs._value.integer64 == rhs._value.integer64;
        case anyTypeId::real:
            return lhs._value.real == rhs._value.real;

        case anyTypeId::const_string:
            return std::strcmp(lhs._value.const_string, rhs._value.const_string) == 0;
        case anyTypeId::string:
            return *(lhs._value.string) == *(rhs._value.string);            
        }        

        throw "not implemented";
    }

    bool operator==(const any& other)
    {
        return *this == other;
    }    

    template <class T, class = std::enable_if<std::is_integral_v<T>>>
    auto operator==(T other)
    {
        switch (_type)
        {
        case anyTypeId::integer:
            return _value.integer == other;

        case anyTypeId::integer64:
            return _value.integer64 == other;

        case anyTypeId::real:
            return _value.real == other;

        case anyTypeId::const_string:
            return std::stoi(_value.const_string) == other;
        case anyTypeId::string:
            return std::stoi(*(_value.string)) == other;

        default:
            throw "wrong type";
        }

        throw "not implemented";
    }

    template <class T, class = std::enable_if<std::is_integral_v<T>>>
    any operator-(T other)
    {
        switch (_type)
        {
        case anyTypeId::integer:
            return any(_value.integer - other);

        case anyTypeId::integer64:
            return any(_value.integer64 - other);

        case anyTypeId::real:
            return any(_value.real - other);

        default:
            throw "wrong type";
        }

        throw "not implemented";
    }

    any &operator++()
    {
        switch (_type)
        {
        case anyTypeId::integer:
            _value.integer++;
            break;

        case anyTypeId::integer64:
            _value.integer64++;
            break;

        case anyTypeId::real:
            _value.real++;
            break;

        default:
            throw "wrong type";
        }

        return *this;
    }

    any operator++(int)
    {
        any res(*this);
        ++(*this);
        return res;
    }

    any &operator--()
    {
        switch (_type)
        {
        case anyTypeId::integer:
            _value.integer--;
            break;

        case anyTypeId::integer64:
            _value.integer64--;
            break;

        case anyTypeId::real:
            _value.real--;
            break;

        default:
            throw "wrong type";
        }

        return *this;
    }

    any operator--(int)
    {
        any res(*this);
        --(*this);
        return res;
    }

    friend any operator+(int value, const any &rhs);
    friend any operator+(const char *value, const any &rhs);

    friend std::ostream &operator<<(std::ostream &os, const any &other);

    void Delete(const char* field)
    {
        switch (_type)
        {
        case anyTypeId::object:
            _value.object->erase(field);
            break;

        default:
            throw "wrong type";
        }
    }    
};

static struct Helper
{
    any Void(any value)
    {
        return any();
    }
} helper;

static struct Console : any
{
    Console() : any(anyTypeId::object)
    {
        //(*this)["log"] = static_cast<std::function<void(void)>>(std::bind(&Console::__log, this));
    }

    void log(any value)
    {
        std::cout << value << std::endl;
    }

    /*
    void __log()
    {
        // experiment to call method by expression
        std::cout << "I'm working..." << std::endl;
    }
    */
} console;

} // namespace js
