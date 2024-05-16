
#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_BINARY_SEARCH_TREE_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_BINARY_SEARCH_TREE_H

#include <list>
#include <optional>
#include <stack>
#include <vector>
#include <logger.h>
#include <logger_guardant.h>
#include <allocator.h>
#include <allocator_guardant.h>
#include <not_implemented.h>
#include <search_tree.h>
#include <cstddef>

template<
	typename tkey,
	typename tvalue>
class binary_search_tree :
	public search_tree<tkey, tvalue>
{

protected:

    struct node
    {

    public:

	tkey key;

	tvalue value;

	node *left_subtree;

	node *right_subtree;


    public:

	explicit node(
		tkey const &key,
		tvalue const &value);

	explicit node(
		tkey const &key,
		tvalue &&value);

    public:

	virtual ~node() noexcept = default;

    };

public:
    explicit binary_search_tree<tkey, tvalue>(
	    std::function<int(tkey const &, tkey const &)> keys_comparer = std::less<tkey>(),
	    logger *logger = nullptr,
	    allocator *allocator = nullptr) : search_tree<tkey, tvalue>(keys_comparer, logger, allocator)
    {

    }

public:

    // region iterators definition

    class prefix_iterator;

    class prefix_const_iterator;

    class prefix_reverse_iterator;

    class prefix_reverse_const_iterator;

    class infix_iterator;

    class infix_const_iterator;

    class infix_reverse_iterator;

    class infix_reverse_const_iterator;

    class postfix_iterator;

    class postfix_const_iterator;

    class postfix_reverse_iterator;

    class postfix_reverse_const_iterator;

    struct iterator_data
    {

	friend class prefix_iterator;

	friend class prefix_const_iterator;

	friend class prefix_reverse_iterator;

	friend class prefix_reverse_const_iterator;

	friend class infix_iterator;

	friend class infix_const_iterator;

	friend class infix_reverse_iterator;

	friend class infix_reverse_const_iterator;

	friend class postfix_iterator;

	friend class postfix_const_iterator;

	friend class postfix_reverse_iterator;

	friend class postfix_reverse_const_iterator;

    public:

	unsigned int depth;

    private:

	tkey *_key;

	tvalue *_value;

    private:

	bool _is_state_initialized;

    public:

	inline tkey const &get_key() const noexcept
	{
	    if (is_state_initialized())
	    {
		return *_key;
	    }
	    throw std::logic_error("State is uninitialised");
	}

	inline tvalue const &get_value() const noexcept
	{
	    if (this->is_state_initialized())
	    {
		return *_value;
	    }

	    throw std::logic_error("State is uninitialised");
	}

	inline bool is_state_initialized() const noexcept
	{
	    return _is_state_initialized;
	}

    public:

	iterator_data() :
		_key(reinterpret_cast<tkey *>(::operator new(sizeof(tkey)))),
		_value(reinterpret_cast<tvalue *>(::operator new(sizeof(tvalue)))),
		_is_state_initialized(false)
	{

	}

	iterator_data(
		unsigned int depth,
		tkey const &key,
		tvalue const &value)
		: depth(depth),
		  _key(reinterpret_cast<tkey *>(::operator new(sizeof(tkey)))),
		  _value(reinterpret_cast<tvalue *>(::operator new(sizeof(tvalue)))),
		  _is_state_initialized(true)
	{
	    allocator::construct(_key, key);
	    allocator::construct(_value, value);
	}

    public:

	iterator_data(iterator_data const &other) noexcept:
		_key(reinterpret_cast<tkey *>(::operator new(sizeof(tkey)))),
		_value(reinterpret_cast<tvalue *>(operator new(sizeof(tvalue)))),
		_is_state_initialized(other._is_state_initialized),
		depth(other.depth)
	{
	    allocator::construct(this->_key, *(other._key));
	    allocator::construct(this->_value, *(other._value));
	}

	iterator_data &operator=(iterator_data const &other) noexcept
	{
	    if (this != &other)
	    {
		this->_key = other._key;
		this->_value = other._value;
		this->_is_state_initialized = other._is_state_initialized;
		this->depth = other.depth;
	    }

	    return *this;
	}

	iterator_data(iterator_data &&other) noexcept: _key(other._key), _value(other._value),
						       _is_state_initialized(other._is_state_initialized),
						       depth(other.depth)
	{
	    other._value = nullptr;
	    other._key = nullptr;
	    other.depth = 0;
	    other._is_state_initialized = false;
	}

	iterator_data &operator=(iterator_data &&other) noexcept
	{
	    if (this != &other)
	    {
		this->_key = other._key;
		this->_value = other._value;
		this->_is_state_initialized = other._is_state_initialized;
		this->depth = other.depth;

		other._value = nullptr;
		other._key = nullptr;
		other.depth = 0;
		other._is_state_initialized = false;
	    }

	    return *this;
	}


	virtual ~iterator_data() noexcept
	{
	    if (_is_state_initialized)
	    {
		allocator::destruct(_key);
		allocator::destruct(_value);

	    }

	    ::operator delete(_key);
	    _key = nullptr;
	    ::operator delete(_value);
	    _value = nullptr;

	    _is_state_initialized = false;
	};
    };

    class prefix_iterator final
    {

	friend class binary_search_tree<tkey, tvalue>;

    private:
	iterator_data *_data;
	std::stack<node *> _state;
	binary_search_tree<tkey, tvalue> *_holder;

    private:

	explicit prefix_iterator(
		binary_search_tree<tkey, tvalue> *holder,
		typename binary_search_tree<tkey, tvalue>::node *subtree_root,
		iterator_data *data);

	explicit prefix_iterator(
		binary_search_tree<tkey, tvalue> *holder,
		std::stack<node *> state,
		iterator_data *data
	) : _holder(holder), _state(state), _data(data)
	{
	    if (_state.empty())
	    {
		return;
	    }

	    allocator::construct(_data->_key, _state.top()->key);
	    allocator::construct(_data->_value, _state.top()->value);
	}

    private:

	tkey *_key;

	tvalue *_value;

    public:

	bool operator==(
		prefix_iterator const &other) const noexcept;

	bool operator!=(
		prefix_iterator const &other) const noexcept;

	prefix_iterator &operator++();

	prefix_iterator const operator++(
		int not_used);

	iterator_data *operator*() const;

    private:
	void assign_data()
	{
	    if (_data->is_state_initialized())
	    {
		*(_data->_key) = _state.top()->key;
		*(_data->_value) = _state.top()->value;
	    }
	    else
	    {
		allocator::construct(_data->_key, _state.top()->key);
		allocator::construct(_data->_value, _state.top()->value);
		_data->_is_state_initialized = true;
	    }
	    _data->depth = _state.size() - 1;
	    _holder->inject_additional_data(_data, _state.top());
	}

    public:

	~prefix_iterator()
	{
	    delete _data;
	}

	prefix_iterator(prefix_iterator const &other) : _holder(other.holder), _data(other._data), _state(other._state)
	{

	}

	prefix_iterator &operator=(prefix_iterator const &other)
	{
	    if (this != &other)
	    {
		this->_holder = other._holder;
		this->_data = other._data;
		this->_state = other._state;
	    }
	    return *this;
	}

	prefix_iterator(prefix_iterator &&other)
	{
	    this->_state = std::move(other._state);
	    this->_data = other._data;
	    this->_holder = other._holder;

	    other._holder = nullptr;
	    other._data = nullptr;
	}

	prefix_iterator &operator=(prefix_iterator &&other)
	{
	    if (this != &other)
	    {
		this->_state = std::move(other._state);
		this->_data = other._data;
		this->_holder = other._holder;

		other._holder = nullptr;
		other._data = nullptr;
	    }
	    return *this;
	}
    };

    class prefix_const_iterator final
    {

	friend class binary_search_tree<tkey, tvalue>;

    private:
	binary_search_tree<tkey, tvalue> *_holder;
	std::stack<node *> _state;
	iterator_data *_data;

    private:

	explicit prefix_const_iterator(
		binary_search_tree<tkey, tvalue> *holder,
		typename binary_search_tree<tkey, tvalue>::node *subtree_root,
		iterator_data *data);

	explicit prefix_const_iterator(
		binary_search_tree<tkey, tvalue> const *holder,
		std::stack<node *> state,
		iterator_data const *data
	) : _holder(holder), _state(state), _data(data)
	{
	    if (_state.empty())
	    {
		return;
	    }

	    allocator::construct(_data->_key, _state.top()->key);
	    allocator::construct(_data->_value, _state.top()->value);
	}

    public:

	bool operator==(
		prefix_const_iterator const &other) const noexcept;

	bool operator!=(
		prefix_const_iterator const &other) const noexcept;

	prefix_const_iterator &operator++();

	prefix_const_iterator const operator++(
		int not_used);

	iterator_data const *operator*() const;


    private:
	void assign_data()
	{
	    if (_data->is_state_initialized())
	    {
		*(_data->_key) = _state.top()->key;
		*(_data->_value) = _state.top()->value;
	    }
	    else
	    {
		allocator::construct(_data->_key, _state.top()->key);
		allocator::construct(_data->_value, _state.top()->value);
		_data->_is_state_initialized = true;
	    }
	    _holder->inject_additional_data(_data, _state.top());
	}

    public:
	prefix_const_iterator(prefix_const_iterator const &other) : _holder(other._holder), _data(other._data),
								    _state(other._state)
	{

	}

	prefix_const_iterator &operator=(prefix_const_iterator const &other) noexcept
	{
	    if (this != &other)
	    {
		this->_holder = other._holder;
		this->_data = other._data;
		this->_state = other._state;
	    }
	    return *this;
	}

	prefix_const_iterator(prefix_const_iterator &&other)
	{
	    this->_state = std::move(other._state);
	    this->_data = other._data;
	    this->_holder = other._holder;

	    other._holder = nullptr;
	    other._data = nullptr;
	}

	prefix_const_iterator &operator=(prefix_const_iterator &&other) noexcept
	{
	    if (this != &other)
	    {
		this->_state = std::move(other._state);
		this->_data = other._data;
		this->_holder = other._holder;

		other._holder = nullptr;
		other._data = nullptr;
	    }
	    return *this;
	}

	~prefix_const_iterator()
	{
	    delete _data;
	}
    };

    class prefix_reverse_iterator final
    {

	friend class binary_search_tree<tkey, tvalue>;

    private:
	binary_search_tree<tkey, tvalue> *_holder;
	std::stack<node *> _state;
	iterator_data *_data;

    private:

	explicit prefix_reverse_iterator(
		binary_search_tree<tkey, tvalue> const *holder,
		typename binary_search_tree<tkey, tvalue>::node *subtree_root,
		iterator_data *data);

	explicit prefix_reverse_iterator(
		binary_search_tree<tkey, tvalue> const *holder,
		std::stack<node *> state,
		iterator_data *data
	) : _holder(holder), _state(state), _data(data)
	{
	    if (_state.empty())
	    {
		return;
	    }

	    allocator::construct(_data->_key, _state.top()->key);
	    allocator::construct(_data->_value, _state.top()->value);
	}

    public:

	bool operator==(
		prefix_reverse_iterator const &other) const noexcept;

	bool operator!=(
		prefix_reverse_iterator const &other) const noexcept;

	prefix_reverse_iterator &operator++();

	prefix_reverse_iterator const operator++(
		int not_used);

	iterator_data *operator*() const;

    private:
	void assign_data()
	{
	    if (_data->is_state_initialized())
	    {
		*(_data->_key) = _state.top()->key;
		*(_data->_value) = _state.top()->value;
	    }
	    else
	    {
		allocator::construct(_data->_key, _state.top()->key);
		allocator::construct(_data->_value, _state.top()->value);
		_data->_is_state_initialized = true;
	    }
	    _data->depth = _state.size() - 1;
	    _holder->inject_additional_data(_data, _state.top());
	}

    public:
	prefix_reverse_iterator(prefix_reverse_iterator const &other) : _holder(other._holder), _state(other._state),
									_data(other._data)
	{

	}

	prefix_reverse_iterator(prefix_reverse_iterator &&other)
	{
	    this->_state = std::move(other._state);
	    this->_data = other._data;
	    this->_holder = other._holder;

	    other._holder = nullptr;
	    other._data = nullptr;
	}

	prefix_reverse_iterator &operator=(prefix_reverse_iterator const &other) noexcept
	{
	    if (this != &other)
	    {
		this->_state = std::move(other._state);
		this->_data = other._data;
		this->_holder = other._holder;

	    }
	    return *this;
	}

	prefix_reverse_iterator &operator=(prefix_reverse_iterator &&other) noexcept
	{
	    if (this != &other)
	    {
		this->_state = std::move(other._state);
		this->_data = other._data;
		this->_holder = other._holder;

		other._holder = nullptr;
		other._data = nullptr;
	    }
	    return *this;
	}
    };

    class prefix_const_reverse_iterator final
    {

	friend class binary_search_tree<tkey, tvalue>;

    private:
	binary_search_tree<tkey, tvalue> *_holder;
	std::stack<node *> _state;
	iterator_data *_data;

    public:
	~prefix_const_reverse_iterator()
	{
	    delete _data;
	}

	prefix_const_reverse_iterator(prefix_const_reverse_iterator const &other) : _holder(other._holder),
										    _state(other._state),
										    _data(other._data)
	{

	}

	prefix_const_reverse_iterator &operator=(prefix_const_reverse_iterator const &other) noexcept
	{
	    if (this != &other)
	    {
		this->_state = other._state;
		this->_data = other._data;
		this->_holder = other._holder;

	    }
	    return *this;
	}

	prefix_const_reverse_iterator &operator=(prefix_const_reverse_iterator &&other) noexcept
	{
	    if (this != &other)
	    {
		this->_state = std::move(other._state);
		this->_data = other._data;
		this->_holder = other._holder;

		other._holder = nullptr;
		other._data = nullptr;
	    }
	    return *this;
	}

	prefix_const_reverse_iterator(prefix_const_reverse_iterator &&other)
	{
	    this->_holder = other._holder;
	    this->_data = other._data;
	    this->_state = std::move(other._state);

	    other._holder = nullptr;
	    other._data = nullptr;
	}


    private:

	explicit prefix_const_reverse_iterator(
		binary_search_tree<tkey, tvalue> const *holder,
		typename binary_search_tree<tkey, tvalue>::node *subtree_root,
		iterator_data const *data);

	explicit prefix_const_reverse_iterator(
		binary_search_tree<tkey, tvalue> const *holder,
		std::stack<node *> state,
		iterator_data *data
	)
	{
	    if (_state.empty())
	    {
		return;
	    }

	    allocator::construct(_data->_key, _state.top()->key);
	    allocator::construct(_data->_value, _state.top()->value);
	}

    public:

	bool operator==(
		prefix_const_reverse_iterator const &other) const noexcept;

	bool operator!=(
		prefix_const_reverse_iterator const &other) const noexcept;

	prefix_const_reverse_iterator &operator++();

	prefix_const_reverse_iterator const operator++(
		int not_used);

	iterator_data const *operator*() const;

    private:

	void assign_data()
	{
	    if (_data->is_state_initialized())
	    {
		*(_data->_key) = _state.top()->key;
		*(_data->_value) = _state.top()->value;
	    }
	    else
	    {
		allocator::construct(_data->_key, _state.top()->key);
		allocator::construct(_data->_value, _state.top()->value);
		_data->_is_state_initialized = true;
	    }
	    _data->depth = _state.size() - 1;
	    _holder->inject_additional_data(_data, _state.top());
	}


    };

    class infix_iterator final
    {

	friend class binary_search_tree<tkey, tvalue>;

    private:

	iterator_data *_data;
	std::stack<node *> _state;
	binary_search_tree<tkey, tvalue> *_holder;


    private:

	explicit infix_iterator(
		binary_search_tree<tkey, tvalue> *_holder,
		typename binary_search_tree<tkey, tvalue>::node *subtree_root,
		iterator_data *_data);

	explicit infix_iterator(
		binary_search_tree<tkey, tvalue> *_holder,
		std::stack<node *> _state,
		iterator_data *_data
	) : _data(_data), _state(_state), _holder(_holder)
	{
	    if (_state.empty())
	    {
		return;
	    }

	    allocator::construct(_data->_key, _state.top()->key);
	    allocator::construct(_data->_value, _state.top()->value);
	    _data->_is_state_initialized = true;
	    _data->depth = _state.size() - 1;
	}

    public:

	infix_iterator(infix_iterator const &other) : _data(other._data), _holder(other._holder), _state(other._state)
	{

	}

	infix_iterator(infix_iterator &&other)
	{
	    this->_data = other._data;
	    this->_state = std::move(other._state);
	    this->_holder = other._holder;

	    other._data = nullptr;
	    other._holder = nullptr;
	}

	infix_iterator &operator=(infix_iterator const &other)
	{
	    if (this != &other)
	    {
		this->_state = other._state;
		this->_holder = other._holder;
		this->_data = other._data;
	    }

	    return *this;
	}

	infix_iterator &operator=(infix_iterator &&other)
	{
	    if (this != &other)
	    {
		this->_state = other._state;
		this->_holder = other._holder;
		this->_data = other._data;

		other._holder = nullptr;
		other._data = nullptr;

	    }

	    return *this;
	}

	~infix_iterator()
	{
	    delete _data;
	}

    public:

	bool operator==(
		infix_iterator const &other) const noexcept;

	bool operator!=(
		infix_iterator const &other) const noexcept;

	infix_iterator &operator++();

	infix_iterator const operator++(
		int not_used);

	iterator_data *operator*() const;

	void assign_data()
	{
	    if (_data->is_state_initialized())
	    {
		*(_data->_key) = _state.top()->key;
		*(_data->_value) = _state.top()->value;
	    }
	    else
	    {
		allocator::construct(_data->_key, _state.top()->key);
		allocator::construct(_data->_value, _state.top()->value);
		_data->_is_state_initialized = true;
	    }
	    _data->depth = _state.size() - 1;
	    _holder->inject_additional_data(_data, _state.top());
	}

    };

    class infix_const_iterator final
    {

	friend class binary_search_tree<tkey, tvalue>;

    private:

	iterator_data *_data;
	std::stack<node *> _state;
	binary_search_tree<tkey, tvalue> *_holder;


    private:

	explicit infix_const_iterator(
		binary_search_tree<tkey, tvalue> *_holder,
		typename binary_search_tree<tkey, tvalue>::node *subtree_root,
		iterator_data *_data);

	explicit infix_const_iterator(
		binary_search_tree<tkey, tvalue> *_holder,
		std::stack<node *> _state,
		iterator_data *_data
	) : _data(_data), _state(_state), _holder(_holder)
	{
	    if (_state.empty())
	    {
		return;
	    }

	    allocator::construct(_data->_key, _state.top()->key);
	    allocator::construct(_data->_value, _state.top()->value);
	    _data->_is_state_initialized = true;
	    _data->depth = _state.size() - 1;
	}

    public:

	infix_const_iterator(infix_const_iterator const &other) : _data(other._data), _holder(other._holder),
								  _state(other._state)
	{

	}

	infix_const_iterator(infix_const_iterator &&other)
	{
	    this->_data = other._data;
	    this->_state = std::move(other._state);
	    this->_holder = other._holder;

	    other._data = nullptr;
	    other._holder = nullptr;
	}

	infix_const_iterator &operator=(infix_const_iterator const &other)
	{
	    if (this != &other)
	    {
		this->_state = other._state;
		this->_holder = other._holder;
		this->_data = other._data;
	    }

	    return *this;
	}

	infix_const_iterator &operator=(infix_const_iterator &&other)
	{
	    if (this != &other)
	    {
		this->_state = other._state;
		this->_holder = other._holder;
		this->_data = other._data;

		other._holder = nullptr;
		other._data = nullptr;
	    }

	    return *this;
	}

	~infix_const_iterator()
	{
	    delete _data;
	}

    public:

	bool operator==(
		infix_const_iterator const &other) const noexcept;

	bool operator!=(
		infix_const_iterator const &other) const noexcept;

	infix_const_iterator &operator++();

	infix_const_iterator const operator++(
		int not_used);

	iterator_data const *operator*() const;

	void assign_data()
	{
	    if (_data->is_state_initialized())
	    {
		*(_data->_key) = _state.top()->key;
		*(_data->_value) = _state.top()->value;
	    }
	    else
	    {
		allocator::construct(_data->_key, _state.top()->key);
		allocator::construct(_data->_value, _state.top()->value);
		_data->_is_state_initialized = true;
	    }
	    _data->depth = _state.size() - 1;
	    _holder->inject_additional_data(_data, _state.top());
	}

    };

    class infix_reverse_iterator final
    {

	friend class binary_search_tree<tkey, tvalue>;

    private:
	binary_search_tree<tkey, tvalue> *_holder;
	std::stack<node *> _state;
	iterator_data *_data;

    private:

	explicit infix_reverse_iterator(
		binary_search_tree<tkey, tvalue> const *holder,
		typename binary_search_tree<tkey, tvalue>::node *subtree_root,
		iterator_data *_data);

	explicit infix_reverse_iterator(
		binary_search_tree<tkey, tvalue> const *holder,
		std::stack<node *> state,
		iterator_data *data
	) : _holder(holder), _state(state), _data(data)
	{
	    if (_state.empty())
	    {
		return; 
	    }

	    allocator::construct(_data->_key, _state.top()->key);
	    allocator::construct(_data->_value, _state.top()->value);
	}

    public:
	infix_reverse_iterator(infix_reverse_iterator const &other) : _holder(other._holder), _state(other._state)
	{

	}

	infix_reverse_iterator &operator=(infix_reverse_iterator const &other) noexcept
	{
	    if (this != &other)
	    {
		this->_state = other._state;
		this->_holder = other._holder;
		this->_data = other._data;
	    }
	    return *this;
	}

	infix_reverse_iterator(infix_reverse_iterator &&other)
	{

	    this->_data = other._data;
	    this->_holder = other._holder;
	    this->_state = std::move(other._state);

	    other._holder = nullptr;
	    other._data = nullptr;
	}

	infix_reverse_iterator &operator=(infix_reverse_iterator &&other) noexcept
	{
	    if (this != &other)
	    {
		this->_data = other._data;
		this->_holder = other._holder;
		this->_state = std::move(other._state);

		other._holder = nullptr;
		other._data = nullptr;
	    }
	    return *this;
	}

	~infix_reverse_iterator()
	{
	    delete _data;
	}

    public:

	bool operator==(
		infix_reverse_iterator const &other) const noexcept;

	bool operator!=(
		infix_reverse_iterator const &other) const noexcept;

	infix_reverse_iterator &operator++();

	infix_reverse_iterator const operator++(
		int not_used);

	iterator_data *operator*() const;

    private:

	void assign_data()
	{
	    if (_data->is_state_initialized())
	    {
		*(_data->_value) = _state.top()->value;
		*(_data->_key) = _state.top()->key;
	    }
	    else
	    {
		allocator::construct(_data->_key, _state.top()->key);
		allocator::construct(_data->_value, _state.top()->value);
		_data->_is_state_initialized = true;
	    }
	    _data->depth = _state.size() - 1;
	    _holder->inject_additional_data(_data, _state.top());
	}

    };

    class infix_const_reverse_iterator final
    {

	friend class binary_search_tree<tkey, tvalue>;

    private:
	binary_search_tree<tkey, tvalue> *_holder;
	std::stack<node *> _state;
	iterator_data *_data;
    public:

	infix_const_reverse_iterator(infix_reverse_iterator const &other) : _holder(other._holder), _state(other._state)
	{

	}

	infix_const_reverse_iterator &operator=(infix_const_reverse_iterator const &other) noexcept
	{
	    if (this != &other)
	    {
		this->_state = other._state;
		this->_holder = other._holder;
		this->_data = other._data;
	    }
	    return *this;
	}

	infix_const_reverse_iterator(infix_const_reverse_iterator &&other)
	{

	    this->_data = other._data;
	    this->_holder = other._holder;
	    this->_state = std::move(other._state);

	    other._holder = nullptr;
	    other._data = nullptr;
	}

	infix_const_reverse_iterator &operator=(infix_const_reverse_iterator &&other) noexcept
	{
	    if (this != &other)
	    {
		this->_data = other._data;
		this->_holder = other._holder;
		this->_state = std::move(other._state);

		other._holder = nullptr;
		other._data = nullptr;
	    }
	    return *this;
	}

	~infix_const_reverse_iterator()
	{
	    delete _data;
	}

    private:

	explicit infix_const_reverse_iterator(
		binary_search_tree<tkey, tvalue> *holder,
		typename binary_search_tree<tkey, tvalue>::node *subtree_root,
		iterator_data *data);

	explicit infix_const_reverse_iterator(
		binary_search_tree<tkey, tvalue> const *holder,
		std::stack<node *> state,
		iterator_data *data
	) : _holder(holder), _state(state), _data(data)
	{
	    if (_state.empty())
	    {
		return; 
	    }

	    allocator::construct(_data->_key, _state.top()->key);
	    allocator::construct(_data->_value, _state.top()->value);
	}

    public:

	bool operator==(
		infix_const_reverse_iterator const &other) const noexcept;

	bool operator!=(
		infix_const_reverse_iterator const &other) const noexcept;

	infix_const_reverse_iterator &operator++();

	infix_const_reverse_iterator const operator++(
		int not_used);

	iterator_data const *operator*() const;

    private:
	void assign_data()
	{
	    if (_data->is_state_initialized())
	    {
		*(_data->_key) = _state.top()->key;
		*(_data->_value) = _state.top()->value;
	    }
	    else
	    {
		allocator::construct(_data->_key, _state.top()->key);
		allocator::construct(_data->_value, _state.top()->value);
		_data->_is_state_initialized = true;
	    }
	    _data->depth = _state.size() - 1;
	    _holder->inject_additional_data(_data, _state.top());
	}
    };

    class postfix_iterator final
    {

	friend class binary_search_tree<tkey, tvalue>;

    private:
	binary_search_tree<tkey, tvalue> *_holder;
	std::stack<node *> _state;
	iterator_data *_data;
    public:
	postfix_iterator(postfix_iterator const &other) : _holder(other._holder), _state(other._state)
	{

	}

	postfix_iterator &operator=(postfix_iterator const &other) noexcept
	{
	    if (this != &other)
	    {
		this->_state = other._state;
		this->_holder = other._holder;
		this->_data = other._data;
	    }
	    return *this;
	}

	postfix_iterator(postfix_iterator &&other)
	{

	    this->_data = other._data;
	    this->_holder = other._holder;
	    this->_state = std::move(other._state);

	    other._holder = nullptr;
	    other._data = nullptr;
	}

	postfix_iterator &operator=(postfix_iterator &&other) noexcept
	{
	    if (this != &other)
	    {
		this->_data = other._data;
		this->_holder = other._holder;
		this->_state = std::move(other._state);

		other._holder = nullptr;
		other._data = nullptr;
	    }
	    return *this;
	}

	~postfix_iterator()
	{
	    delete _data;
	}


    private:

	explicit postfix_iterator(
		binary_search_tree<tkey, tvalue> *holder,
		typename binary_search_tree<tkey, tvalue>::node *subtree_root,
		iterator_data *data);

	explicit postfix_iterator(
		binary_search_tree<tkey, tvalue> *holder,
		std::stack<std::pair<node *, bool>> state,
		iterator_data *data
	) : _holder(holder), _state(state), _data(data)
	{
	    if (_state.empty())
	    {
		return; 
	    }

	    allocator::construct(_data->_key, _state.top().first->key);
	    allocator::construct(_data->value, _state.top().first->value);
	}


    public:

	bool operator==(
		postfix_iterator const &other) const noexcept;

	bool operator!=(
		postfix_iterator const &other) const noexcept;

	postfix_iterator &operator++();

	postfix_iterator const operator++(
		int not_used);

	iterator_data *operator*() const;

    private:
	void assign_data()
	{
	    if (_data->is_state_initialized())
	    {
		*(_data->_key) = _state.top()->key;
		*(_data->_value) = _state.top()->value;
	    }
	    else
	    {
		allocator::construct(_data->_key, _state.top()->key);
		allocator::construct(_data->_value, _state.top()->value);
		_data->_is_state_initialized = true;
	    }
	    _holder->inject_additional_data(_data, _state.top());
	}

    };

    class postfix_const_iterator final
    {

	friend class binary_search_tree<tkey, tvalue>;

    private:
	binary_search_tree<tkey, tvalue> *_holder;
	std::stack<std::pair<node *, bool>> _state;
	iterator_data *_data;

    private:

	explicit postfix_const_iterator(
		binary_search_tree<tkey, tvalue> *holder,
		typename binary_search_tree<tkey, tvalue>::node *subtree_root,
		iterator_data *data);

	explicit postfix_const_iterator(
		binary_search_tree<tkey, tvalue> *holder,
		std::stack<std::pair<node *, bool>> state,
		iterator_data const *data
	) : _holder(holder), _state(state), _data(data)
	{
	    if (_state.empty())
	    {
		return; 
	    }
	    
	    allocator::construct(_data->_key, _state.top().first->key);
	    allocator::construct(_data->_value, _state.top().first->value);
	}

    public:
	postfix_const_iterator(postfix_const_iterator const &other) : _holder(other._holder), _state(other._state)
	{

	}

	postfix_const_iterator &operator=(postfix_const_iterator const &other) noexcept
	{
	    if (this != &other)
	    {
		this->_state = other._state;
		this->_holder = other._holder;
		this->_data = other._data;
	    }
	    return *this;
	}

	postfix_const_iterator(postfix_const_iterator &&other)
	{

	    this->_data = other._data;
	    this->_holder = other._holder;
	    this->_state = std::move(other._state);

	    other._holder = nullptr;
	    other._data = nullptr;
	}

	postfix_const_iterator &operator=(postfix_const_iterator &&other) noexcept
	{
	    if (this != &other)
	    {
		this->_data = other._data;
		this->_holder = other._holder;
		this->_state = std::move(other._state);

		other._holder = nullptr;
		other._data = nullptr;
	    }
	    return *this;
	}

	~postfix_const_iterator()
	{
	    delete _data;
	}

    public:

	bool operator==(
		postfix_const_iterator const &other) const noexcept;

	bool operator!=(
		postfix_const_iterator const &other) const noexcept;

	postfix_const_iterator &operator++();

	postfix_const_iterator const operator++(
		int not_used);

	iterator_data const *operator*() const;

    private:
	void assign_data()
	{
	    if (_data->is_state_initialized())
	    {
		*(_data->_key) = _state.top().first->key;
		*(_data->_value) = _state.top().first->value;
	    }
	    else
	    {
		allocator::construct(_data->_key, _state.top().first->key);
		allocator::construct(_data->_value, _state.top().first->value);
		_data->_is_state_initialized = true;
	    }
	    _holder->inject_additional_data(_data, _state.top().first);

	}
    };

    class postfix_reverse_iterator final
    {

	friend class binary_search_tree<tkey, tvalue>;

    private:
	std::stack<node *> _state;
	binary_search_tree<tkey, tvalue> *_holder;
	iterator_data *_data;

    private:

	explicit postfix_reverse_iterator(
		binary_search_tree<tkey, tvalue> *holder,
		typename binary_search_tree<tkey, tvalue>::node *subtree_root,
		iterator_data *data);

	explicit postfix_reverse_iterator(
		binary_search_tree<tkey, tvalue> *holder,
		std::stack<node *> state,
		iterator_data *data
	) : _holder(holder), _state(state), _data(data)
	{
	    if (_state.empty())
	    {
		return; 
	    }

	    allocator::construct(_data->_key, _state.top()->key);
	    allocator::construct(_data->_value, _state.top()->value);
	    _data->_is_state_initialized = true;
	}

    public:
	postfix_reverse_iterator(postfix_reverse_iterator const &other) : _data(other._data),
									  _holder(other._holder), _state(other._state)
	{

	}

	postfix_reverse_iterator &operator=(postfix_reverse_iterator const &other)
	{
	    if (this != &other)
	    {
		this->_state = other._state;
		this->_data = other._data;
		this->_holder = other._holder;
	    }
	    return *this;
	}

	postfix_reverse_iterator(postfix_reverse_iterator &&other) : _holder(other._holder),
								     _data(other._data)
	{
	    _state = std::move(other._state);
	    other._holder = nullptr;
	    other._data = nullptr;
	}

	postfix_reverse_iterator &operator=(postfix_reverse_iterator &&other)
	{
	    if (this != &other)
	    {
		this->_holder = other._holder;
		this->_data = other._data;
		this->_state = std::move(other._state);

		other._data = nullptr;
		other._holder = nullptr;
	    }
	    return *this;
	}

	~postfix_reverse_iterator()
	{
	    delete _data;
	}

    public:

	bool operator==(
		postfix_reverse_iterator const &other) const noexcept;

	bool operator!=(
		postfix_reverse_iterator const &other) const noexcept;

	postfix_reverse_iterator &operator++();

	postfix_reverse_iterator const operator++(
		int not_used);

	iterator_data *operator*() const;

    private:

	void assign_data()
	{
	    if (_data->is_state_initialized())
	    {
		*(_data->_key) = _state.top().first->key;
		*(_data->_value) = _state.top().first->value;
	    }
	    else
	    {
		allocator::construct(_data->_key, _state.top().first->key);
		allocator::construct(_data->_value, _state.top().first->value);
		_data->_is_state_initialized = true;
	    }
	    _holder->inject_additional_data(_data, _state.top().first);

	}
    };

    class postfix_const_reverse_iterator final
    {

	friend class binary_search_tree<tkey, tvalue>;

    private:
	std::stack<node *> _state;
	binary_search_tree<tkey, tvalue> *_holder;
	iterator_data *_data;

    private:

	explicit postfix_const_reverse_iterator(
		binary_search_tree<tkey, tvalue> *holder,
		typename binary_search_tree<tkey, tvalue>::node *subtree_root,
		iterator_data *data);

	explicit postfix_const_reverse_iterator(
		binary_search_tree<tkey, tvalue> *holder,
		std::stack<node *> state,
		iterator_data *data
	) : _holder(holder), _state(state), _data(data)
	{
	    if (_state.empty())
	    {
		return; 
	    }

	    allocator::construct(_data->_key, _state.top()->key);
	    allocator::construct(_data->_value, _state.top()->value);
	    _data->_is_state_initialized = true;
	}

    public:

	bool operator==(
		postfix_const_reverse_iterator const &other) const noexcept;

	bool operator!=(
		postfix_const_reverse_iterator const &other) const noexcept;

	postfix_const_reverse_iterator &operator++();

	postfix_const_reverse_iterator const operator++(
		int not_used);

	iterator_data const *operator*() const;

    public:

	postfix_const_reverse_iterator(postfix_const_reverse_iterator const &other) : _holder(other._holder),
										      _state(other._state)
	{

	}

	postfix_const_reverse_iterator &operator=(postfix_const_reverse_iterator const &other) noexcept
	{
	    if (this != &other)
	    {
		this->_state = other._state;
		this->_holder = other._holder;
		this->_data = other._data;
	    }
	    return *this;
	}

	postfix_const_reverse_iterator(postfix_const_reverse_iterator &&other)
	{

	    this->_data = other._data;
	    this->_holder = other._holder;
	    this->_state = std::move(other._state);

	    other._holder = nullptr;
	    other._data = nullptr;
	}

	postfix_const_reverse_iterator &operator=(postfix_const_reverse_iterator &&other) noexcept
	{
	    if (this != &other)
	    {
		this->_data = other._data;
		this->_holder = other._holder;
		this->_state = std::move(other._state);

		other._holder = nullptr;
		other._data = nullptr;
	    }
	    return *this;
	}

	~postfix_const_reverse_iterator()
	{
	    delete _data;
	}

    private:
	void assign_data()
	{
	    if (_data->is_state_initialized())
	    {
		*(_data->_key) = _state.top().first->key;
		*(_data->_value) = _state.top().first->value;
	    }
	    else
	    {
		allocator::construct(_data->_key, _state.top().first->key);
		allocator::construct(_data->_value, _state.top().first->value);
		_data->_is_state_initialized = true;
	    }
	    _holder->inject_additional_data(_data, _state.top().first);

	}
    };

    // endregion iterators definition

public:

    // region target operations strategies definition

    enum class insertion_of_existent_key_attempt_strategy
    {
	update_value,
	throw_an_exception
    };

    enum class disposal_of_nonexistent_key_attempt_strategy
    {
	do_nothing,
	throw_an_exception
    };

    // endregion target operations strategies definition

    // region target operations associated exception types

    class insertion_of_existent_key_attempt_exception final :
	    public std::logic_error
    {

    private:

	tkey _key;

    public:

	explicit insertion_of_existent_key_attempt_exception(
		tkey const &key);

    public:

	tkey const &get_key() const noexcept;

    };

    class obtaining_of_nonexistent_key_attempt_exception final :
	    public std::logic_error
    {

    private:

	tkey _key;

    public:

	explicit obtaining_of_nonexistent_key_attempt_exception(
		tkey const &key);

    public:

	tkey const &get_key() const noexcept;

    };

    class disposal_of_nonexistent_key_attempt_exception final :
	    public std::logic_error
    {

    private:

	tkey _key;

    public:

	explicit disposal_of_nonexistent_key_attempt_exception(
		tkey const &key);

    public:

	tkey const &get_key() const noexcept;

    };

    // endregion target operations associated exception types

    // region template methods definition

protected:

    class template_method_basics :
	    public logger_guardant
    {

    protected:

	binary_search_tree<tkey, tvalue> *_tree;

    public:

	explicit template_method_basics(
		binary_search_tree<tkey, tvalue> *tree);

    protected:

	std::stack<node **> find_path(
		tkey const &key) const
	{
	    std::stack<node **> result_path;

	    node **path_finder = &(_tree->_root);
	    auto const &comparer = _tree->_keys_comparer;

	    while (true)
	    {
		result_path.push(path_finder);

		if (*path_finder == nullptr)
		{
		    break;
		}

		auto comparison_result = comparer(key, (*path_finder)->key);
		if (comparison_result == 0)
		{
		    break;
		}

		path_finder = comparison_result < 0
			      ? &((*path_finder)->left_subtree)
			      : &((*path_finder)->right_subtree);
	    }

	    return result_path;
	}

	virtual void balance(
		std::stack<node **> &path)
	{

	}

    protected:

	[[nodiscard]] logger *get_logger() const noexcept final;

    };

    class insertion_template_method :
	    public template_method_basics,
	    public allocator_guardant
    {

    private:

	binary_search_tree<tkey, tvalue>::insertion_of_existent_key_attempt_strategy _insertion_strategy;

	allocator *_allocator;

	logger *logger;

    public:

	explicit insertion_template_method(
		binary_search_tree<tkey, tvalue> *tree,
		typename binary_search_tree<tkey, tvalue>::insertion_of_existent_key_attempt_strategy insertion_strategy);

    public:

	void insert(
		tkey const &key,
		tvalue const &value);

	void insert(
		tkey const &key,
		tvalue &&value);

	void set_insertion_strategy(
		typename binary_search_tree<tkey, tvalue>::insertion_of_existent_key_attempt_strategy insertion_strategy) noexcept;

    private:

	[[nodiscard]] allocator *get_allocator() const noexcept final;

    };

    class obtaining_template_method :
	    public template_method_basics
    {

    public:

	explicit obtaining_template_method(
		binary_search_tree<tkey, tvalue> *tree);

    public:

	tvalue const &obtain(
		tkey const &key);

	std::vector<typename associative_container<tkey, tvalue>::key_value_pair> obtain_between(
		tkey const &lower_bound,
		tkey const &upper_bound,
		bool lower_bound_inclusive,
		bool upper_bound_inclusive)
	{
	    std::vector<typename associative_container<tkey, tvalue>::key_value_pair> range;

	    std::stack<node *> path;
	    node *current = this->_tree->_root;
	    while (true)
	    {
		if (current == nullptr)
		{
		    break;
		}

		auto comparison_result = this->_tree->_keys_comparer(lower_bound, current->key);
		path.push(current);

		if (comparison_result == 0)
		{
		    if (lower_bound_inclusive)
		    {
			break;
		    }

		    current = current->right_subtree;
		}
		else if (comparison_result < 0)
		{
		    current = current->left_subtree;
		}
		else
		{
		    current = current->right_subtree;
		}

		if (current == nullptr &&
		    this->_tree->_keys_comparer(path.top()->key, lower_bound) < (lower_bound_inclusive
										 ? 0
										 : 1))
		{
		    path = std::move(std::stack<node *>());
		}
	    }

	    auto it = infix_iterator(this->_tree, path, this->_tree->create_iterator_data());
	    while ((it != this->_tree->end_infix()) &&
		   (this->_tree->_keys_comparer(upper_bound, (*it)->get_key()) > (upper_bound_inclusive
										  ? -1
										  : 0)))
	    {
		range.push_back(std::move(typename associative_container<tkey, tvalue>::key_value_pair((*it)->get_key(),
												       (*it)->get_value())));
		++it;
	    }

	    return range;
	}

    };

    class disposal_template_method :
	    public template_method_basics,
	    public allocator_guardant
    {

    private:

	binary_search_tree<tkey, tvalue>::disposal_of_nonexistent_key_attempt_strategy _disposal_strategy;

    public:

	explicit disposal_template_method(
		binary_search_tree<tkey, tvalue> *tree,
		typename binary_search_tree<tkey, tvalue>::disposal_of_nonexistent_key_attempt_strategy disposal_strategy);

    public:

	virtual tvalue dispose(
		tkey const &key);

	void set_disposal_strategy(
		typename binary_search_tree<tkey, tvalue>::disposal_of_nonexistent_key_attempt_strategy disposal_strategy) noexcept;

    private:

	template<
		typename T>
	inline void swap(
		T &&one,
		T &&another)
	{
	    T temp = std::move(one);
	    one = std::move(another);
	    another = std::move(temp);
	}

    private:

	[[nodiscard]] allocator *get_allocator() const noexcept final;

    };

    // endregion template methods definition

private:

    node *_root;

    insertion_template_method *_insertion_template;

    obtaining_template_method *_obtaining_template;

    disposal_template_method *_disposal_template;

protected:

    explicit binary_search_tree(
	    typename binary_search_tree<tkey, tvalue>::insertion_template_method *insertion_template,
	    typename binary_search_tree<tkey, tvalue>::obtaining_template_method *obtaining_template,
	    typename binary_search_tree<tkey, tvalue>::disposal_template_method *disposal_template,
	    std::function<int(tkey const &, tkey const &)>,
	    allocator *allocator,
	    logger *logger);

public:

    explicit binary_search_tree(
	    std::function<int(tkey const &, tkey const &)> comparer = std::less<tkey>(),
	    allocator *allocator = nullptr,
	    logger *logger = nullptr,
	    typename binary_search_tree<tkey, tvalue>::insertion_of_existent_key_attempt_strategy insertion_strategy = binary_search_tree<tkey, tvalue>::insertion_of_existent_key_attempt_strategy::throw_an_exception,
	    typename binary_search_tree<tkey, tvalue>::disposal_of_nonexistent_key_attempt_strategy disposal_strategy = binary_search_tree<tkey, tvalue>::disposal_of_nonexistent_key_attempt_strategy::throw_an_exception);

public:

    binary_search_tree(
	    binary_search_tree<tkey, tvalue> const &other);

    binary_search_tree(
	    binary_search_tree<tkey, tvalue> &&other) noexcept;

    binary_search_tree<tkey, tvalue> &operator=(
	    binary_search_tree<tkey, tvalue> const &other);

    binary_search_tree<tkey, tvalue> &operator=(
	    binary_search_tree<tkey, tvalue> &&other) noexcept;

    ~binary_search_tree() override;

protected:

    virtual void clear(
	    node *&subtree_root)
    {
	if (subtree_root == nullptr)
	{
	    return;
	}

	clear(subtree_root->left_subtree);
	clear(subtree_root->right_subtree);
	subtree_root->~node();
	this->deallocate_with_guard(subtree_root);

	subtree_root = nullptr;
    }

    virtual node *copy(
	    node const *subtree_root)
    {
	if (subtree_root == nullptr)
	{
	    return nullptr;
	}

	node *subtree_root_copied = reinterpret_cast<node *>(this->allocate_with_guard(get_node_size(), 1));
	call_node_constructor(subtree_root_copied, subtree_root->key, subtree_root->value);
	subtree_root_copied->left_subtree = copy(subtree_root->left_subtree);
	subtree_root_copied->right_subtree = copy(subtree_root->right_subtree);

	return subtree_root_copied;
    }

    virtual size_t get_node_size() const noexcept
    {
	return sizeof(typename binary_search_tree<tkey, tvalue>::node);
    }

    virtual void call_node_constructor(
	    node *raw_space,
	    tkey const &key,
	    tvalue const &value)
    {
	allocator::construct(raw_space, key, value);
    }

    // TODO: should it be here or inside insertion template method?!
    virtual void call_node_constructor(
	    node *raw_space,
	    tkey const &key,
	    tvalue &&value)
    {
	allocator::construct(raw_space, key, std::move(value));
    }

    virtual void inject_additional_data(
	    iterator_data *destination,
	    node *source)
    {

    }

    virtual iterator_data *create_iterator_data() const
    {
	return new iterator_data;
    }

public:

    void insert(
	    tkey const &key,
	    tvalue const &value);

    void insert(
	    tkey const &key,
	    tvalue &&value);

    tvalue const &obtain(
	    tkey const &key);

    std::vector<typename associative_container<tkey, tvalue>::key_value_pair> obtain_between(
	    tkey const &lower_bound,
	    tkey const &upper_bound,
	    bool lower_bound_inclusive,
	    bool upper_bound_inclusive);

    tvalue dispose(
	    tkey const &key);

public:

    void set_insertion_strategy(
	    typename binary_search_tree<tkey, tvalue>::insertion_of_existent_key_attempt_strategy insertion_strategy) noexcept;

    void set_removal_strategy(
	    typename binary_search_tree<tkey, tvalue>::disposal_of_nonexistent_key_attempt_strategy disposal_strategy) noexcept;

public:

    // region iterators requests definition

    prefix_iterator begin_prefix() const noexcept;

    prefix_iterator end_prefix() const noexcept;

    prefix_const_iterator cbegin_prefix() const noexcept;

    prefix_const_iterator cend_prefix() const noexcept;

    prefix_reverse_iterator rbegin_prefix() const noexcept;

    prefix_reverse_iterator rend_prefix() const noexcept;

    prefix_const_reverse_iterator crbegin_prefix() const noexcept;

    prefix_const_reverse_iterator crend_prefix() const noexcept;

    infix_iterator begin_infix() const noexcept;

    infix_iterator end_infix() const noexcept;

    infix_const_iterator cbegin_infix() const noexcept;

    infix_const_iterator cend_infix() const noexcept;

    infix_reverse_iterator rbegin_infix() const noexcept;

    infix_reverse_iterator rend_infix() const noexcept;

    infix_const_reverse_iterator crbegin_infix() const noexcept;

    infix_const_reverse_iterator crend_infix() const noexcept;

    postfix_iterator begin_postfix() const noexcept;

    postfix_iterator end_postfix() const noexcept;

    postfix_const_iterator cbegin_postfix() const noexcept;

    postfix_const_iterator cend_postfix() const noexcept;

    postfix_reverse_iterator rbegin_postfix() const noexcept;

    postfix_reverse_iterator rend_postfix() const noexcept;

    postfix_const_reverse_iterator crbegin_postfix() const noexcept;

    postfix_const_reverse_iterator crend_postfix() const noexcept;

    // endregion iterators requests definition

protected:

    // region subtree rotations definition

    void small_left_rotation(
	    typename binary_search_tree<tkey, tvalue>::node *&subtree_root,
	    bool validate = true) const;

    void small_right_rotation(
	    typename binary_search_tree<tkey, tvalue>::node *&subtree_root,
	    bool validate = true) const;

    void big_left_rotation(
	    typename binary_search_tree<tkey, tvalue>::node *&subtree_root,
	    bool validate = true) const;

    void big_right_rotation(
	    typename binary_search_tree<tkey, tvalue>::node *&subtree_root,
	    bool validate = true) const;

    void double_left_rotation(
	    typename binary_search_tree<tkey, tvalue>::node *&subtree_root,
	    bool at_grandparent_first,
	    bool validate = true) const;

    void double_right_rotation(
	    typename binary_search_tree<tkey, tvalue>::node *&subtree_root,
	    bool at_grandparent_first,
	    bool validate = true) const;

    // endregion subtree rotations definition

};

// region binary_search_tree<tkey, tvalue>::node methods implementation

template<
	typename tkey,
	typename tvalue>
binary_search_tree<tkey, tvalue>::node::node(
	tkey const &key,
	tvalue const &value):
	key(key),
	value(value),
	left_subtree(nullptr),
	right_subtree(nullptr)
{

}

template<
	typename tkey,
	typename tvalue>
binary_search_tree<tkey, tvalue>::node::node(
	tkey const &key,
	tvalue &&value):
	key(key),
	value(std::move(value)),
	left_subtree(nullptr),
	right_subtree(nullptr)
{

}

// endregion binary_search_tree<tkey, tvalue>::node methods implementation

// region iterators implementation

// region iterator data implementation



// endregion iterator data implementation

// region prefix_iterator implementation

template<
	typename tkey,
	typename tvalue>
binary_search_tree<tkey, tvalue>::prefix_iterator::prefix_iterator(
	binary_search_tree<tkey, tvalue> *holder,
	typename binary_search_tree<tkey, tvalue>::node *subtree_root,
	iterator_data *data) : _holder(holder), _data(data)
{
    if (subtree_root)
    {
	_state.push(subtree_root);
	allocator::construct(_data->_key, _state.top()->key);
	allocator::construct(_data->_value, _state.top()->value);
	_data->depth = 0;
	_data->_is_state_initialized = true;
    }

}

template<
	typename tkey,
	typename tvalue>
bool binary_search_tree<tkey, tvalue>::prefix_iterator::operator==(
	typename binary_search_tree<tkey, tvalue>::prefix_iterator const &other) const noexcept
{
    if (this->_state.empty() && other._state.empty())
    {
	return true;
    }

    if (this->_state.empty() ^ other._state.empty())
    {
	return false;
    }

    return this->_state.top() == other._state.top();
}

template<
	typename tkey,
	typename tvalue>
bool binary_search_tree<tkey, tvalue>::prefix_iterator::operator!=(
	typename binary_search_tree<tkey, tvalue>::prefix_iterator const &other) const noexcept
{
    return !(other == this);
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::prefix_iterator &
binary_search_tree<tkey, tvalue>::prefix_iterator::operator++()
{
    if (_state.empty())
    {
	throw std::logic_error("iterator already ended!!!!)!)010!)!!");
    }

    node *current = _state.top();
    _state.pop();
    if (current->right_subtree)
    {
	_state.push(current->right_subtree);
    }

    if (current->left_subtree)
    {
	_state.push(current->left_subtree);
    }
    _data->depth++;

    if (!_state.empty())
    {
	assign_data();
    }

    return *this;
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::prefix_iterator const
binary_search_tree<tkey, tvalue>::prefix_iterator::operator++(
	int not_used)
{
    auto it = this;
    this++;
    return it;
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::iterator_data *
binary_search_tree<tkey, tvalue>::prefix_iterator::operator*() const
{
    if (_state.empty())
    {
	throw std::logic_error("iterator already ended!!!!)!)010!)!!");
    }
    
    return _data;
}

// endregion prefix_iterator implementation

// region prefix_const_iterator implementation

template<
	typename tkey,
	typename tvalue>
binary_search_tree<tkey, tvalue>::prefix_const_iterator::prefix_const_iterator(
	binary_search_tree<tkey, tvalue> *holder,
	typename binary_search_tree<tkey, tvalue>::node *subtree_root,
	iterator_data *data): _holder(holder), _data(data)
{
    if (subtree_root)
    {
	_state.push(subtree_root);
	allocator::construct(_data->_key, _state.top()->key);
	allocator::construct(_data->_value, _state.top()->value);
	_data->depth = 0;
	_data->_is_state_initialized = true;
    }

}

template<
	typename tkey,
	typename tvalue>
bool binary_search_tree<tkey, tvalue>::prefix_const_iterator::operator==(
	typename binary_search_tree<tkey, tvalue>::prefix_const_iterator const &other) const noexcept
{
    if (this->_state.empty() && other._state.empty())
    {
	return true;
    }

    if (this->_state.empty() ^ other._state.empty())
    {
	return false;
    }

    return this->_state.top() == other._state.top();
}


template<
	typename tkey,
	typename tvalue>
bool binary_search_tree<tkey, tvalue>::prefix_const_iterator::operator!=(
	typename binary_search_tree<tkey, tvalue>::prefix_const_iterator const &other) const noexcept
{
    return !(*this == other);
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::prefix_const_iterator &
binary_search_tree<tkey, tvalue>::prefix_const_iterator::operator++()
{
    if (_state.empty())
    {
	throw std::logic_error("iterator already ended!!!!)!)010!)!!");
    }
    
    node *current = _state.top();
    _state.pop();
    if (current->right_subtree)
    {
	_state.push(current->right_subtree);
    }

    if (current->left_subtree)
    {
	_state.push(current->left_subtree);
    }
    _data->depth++;

    if (!_state.empty())
    {
	assign_data();
    }

    return *this;
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::prefix_const_iterator const
binary_search_tree<tkey, tvalue>::prefix_const_iterator::operator++(
	int not_used)
{
    auto it = *this;
    this++;
    return it;
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::iterator_data const *
binary_search_tree<tkey, tvalue>::prefix_const_iterator::operator*() const
{
    if (_state.empty())
    {
	throw std::logic_error("iterator already ended!!!!)!)010!)!!");
    }
    
    return _data;
}

// endregion prefix_const_iterator implementation

// region prefix_reverse_iterator implementation

template<
	typename tkey,
	typename tvalue>
binary_search_tree<tkey, tvalue>::prefix_reverse_iterator::prefix_reverse_iterator(
	binary_search_tree<tkey, tvalue> const *holder,
	typename binary_search_tree<tkey, tvalue>::node *subtree_root,
	iterator_data *data): _holder(holder), _data(data)
{
    if (subtree_root)
    {
	_state.push(subtree_root);
	allocator::construct(_data->_key, _state.top()->key);
	allocator::construct(_data->_value, _state.top()->value);
	_data->depth = 0;
	_data->_is_state_initialized = true;
    }
}

template<
	typename tkey,
	typename tvalue>
bool binary_search_tree<tkey, tvalue>::prefix_reverse_iterator::operator==(
	typename binary_search_tree<tkey, tvalue>::prefix_reverse_iterator const &other) const noexcept
{
    if (this->_state.empty() && other._state.empty())
    {
	return true;
    }

    if (this->_state.empty() ^ other._state.empty())
    {
	return false;
    }

    return this->_state.top() == other._state.top();
}


template<
	typename tkey,
	typename tvalue>
bool binary_search_tree<tkey, tvalue>::prefix_reverse_iterator::operator!=(
	typename binary_search_tree<tkey, tvalue>::prefix_reverse_iterator const &other) const noexcept
{
    return !(*this == other);
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::prefix_reverse_iterator &
binary_search_tree<tkey, tvalue>::prefix_reverse_iterator::operator++()
{
    if (_state.empty())
    {
	throw std::logic_error("iterator already ended!!!!)!)010!)!!");
    }

    node *current = _state.top();
    _state.pop();

    if (current->left_subtree)
    {
	_state.push(current->left_subtree);
    }

    if (current->right_subtree)
    {
	_state.push(current->right_subtree);
    }

    _data->depth++;

    if (!_state.empty())
    {
	assign_data();
    }

    return *this;
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::prefix_reverse_iterator const
binary_search_tree<tkey, tvalue>::prefix_reverse_iterator::operator++(
	int not_used)
{
    auto it = *this;
    this++;
    return it;
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::iterator_data *
binary_search_tree<tkey, tvalue>::prefix_reverse_iterator::operator*() const
{
    if (_state.empty())
    {
	throw std::logic_error("iterator already ended!!");
    }

    return _data;
}

// endregion prefix_reverse_iterator implementation

// region prefix_const_reverse_iterator implementation

template<
	typename tkey,
	typename tvalue>
binary_search_tree<tkey, tvalue>::prefix_const_reverse_iterator::prefix_const_reverse_iterator(
	binary_search_tree<tkey, tvalue> const *holder,
	typename binary_search_tree<tkey, tvalue>::node *subtree_root,
	iterator_data const *data): _holder(holder), _data(data)
{
    if (subtree_root)
    {
	_state.push(subtree_root);
	allocator::construct(_data->_key, _state.top()->key);
	allocator::construct(_data->_value, _state.top()->value);
	_data->depth = 0;
	_data->_is_state_initialized = true;
    }
}

template<
	typename tkey,
	typename tvalue>
bool binary_search_tree<tkey, tvalue>::prefix_const_reverse_iterator::operator==(
	typename binary_search_tree<tkey, tvalue>::prefix_const_reverse_iterator const &other) const noexcept
{
    if (_state.empty() && other._state.empty())
    {
	return true;
    }

    if (_state.empty() ^ other._state.empty())
    {
	return false;
    }

    return _state.top() == other._state.top();
}

template<
	typename tkey,
	typename tvalue>
bool binary_search_tree<tkey, tvalue>::prefix_const_reverse_iterator::operator!=(
	typename binary_search_tree<tkey, tvalue>::prefix_const_reverse_iterator const &other) const noexcept
{
    return !(*this == other);
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::prefix_const_reverse_iterator &
binary_search_tree<tkey, tvalue>::prefix_const_reverse_iterator::operator++()
{
    if (_state.empty())
    {
	throw std::logic_error("iterator already ended!!!!)!)010!)!!");
    }

    node *current = _state.top();
    _state.pop();

    if (current->left_subtree)
    {
	_state.push(current->left_subtree);
    }

    if (current->right_subtree)
    {
	_state.push(current->right_subtree);
    }

    _data->depth++;

    if (!_state.empty())
    {
	assign_data();
    }

    return *this;
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::prefix_const_reverse_iterator const
binary_search_tree<tkey, tvalue>::prefix_const_reverse_iterator::operator++(
	int not_used)
{
    auto it = *this;
    this++;
    return it;
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::iterator_data const *
binary_search_tree<tkey, tvalue>::prefix_const_reverse_iterator::operator*() const
{
    if (_state.empty())
    {
	throw std::logic_error("iterator already ended!!!!)!)010!)!!");
    }
    
    return _data;
}

// endregion prefix_const_reverse_iterator implementation

// region infix_iterator implementation

template<
	typename tkey,
	typename tvalue>
binary_search_tree<tkey, tvalue>::infix_iterator::infix_iterator(
	binary_search_tree<tkey, tvalue> *_holder,
	typename binary_search_tree<tkey, tvalue>::node *subtree_root,
	iterator_data *_data): _holder(_holder), _data(_data)
{
    while (subtree_root != nullptr)
    {
	_state.push(subtree_root);
	subtree_root = subtree_root->left_subtree;
    }

    if (_state.empty())
    {
	return;
    }

    allocator::construct(_data->_key, _state.top()->key);
    allocator::construct(_data->_value, _state.top()->value);
    _data->depth = _state.size() - 1;
    _data->_is_state_initialized = true;
}

template<
	typename tkey,
	typename tvalue>
bool binary_search_tree<tkey, tvalue>::infix_iterator::operator==(
	typename binary_search_tree<tkey, tvalue>::infix_iterator const &other) const noexcept
{
    if (this->_state.empty() && other._state.empty())
    {
	return true;
    }

    if (this->_state.empty() ^ other._state.empty())
    {
	return false;
    }

    return this->_state.top() == other._state.top();
}


template<
	typename tkey,
	typename tvalue>
bool binary_search_tree<tkey, tvalue>::infix_iterator::operator!=(
	typename binary_search_tree<tkey, tvalue>::infix_iterator const &other) const noexcept
{
    return !(*this == other);
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::infix_iterator &
binary_search_tree<tkey, tvalue>::infix_iterator::operator++()
{
    if (_state.empty())
    {
	throw std::logic_error("iterator already ended!!!!)!)010!)!!");
    }

    if (_state.top()->right_subtree)
    {
	_state.push(_state.top()->right_subtree);
	while (_state.top()->left_subtree != nullptr)
	{
	    _state.push(_state.top()->left_subtree);
	}

	assign_data();
    }
    else
    {
	node *current = nullptr;
	do
	{
	    current = _state.top();
	    _state.pop();
	    if (_state.empty())
	    {
		return *this;
	    }

	    if (_state.top()->left_subtree == current)
	    {
		assign_data();
		return *this;
	    }
	} while (true);
    }

    return *this;
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::infix_iterator const
binary_search_tree<tkey, tvalue>::infix_iterator::operator++(
	int not_used)
{
    auto it = *this;
    ++*this;
    return it;
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::iterator_data *
binary_search_tree<tkey, tvalue>::infix_iterator::operator*() const
{
    if (_state.empty())
    {
	throw std::logic_error("iterator already ended!!!!)!)010!)!!");
    }

    return _data;
}

// endregion infix_iterator implementation

// region infix_const_iterator implementation

template<
	typename tkey,
	typename tvalue>
binary_search_tree<tkey, tvalue>::infix_const_iterator::infix_const_iterator(
	binary_search_tree<tkey, tvalue> *_holder,
	typename binary_search_tree<tkey, tvalue>::node *subtree_root,
	iterator_data *_data): _holder(_holder), _data(_data)
{
    while (subtree_root != nullptr)
    {
	_state.push(subtree_root);
	subtree_root = subtree_root->left_subtree;
    }

    if (_state.empty())
    {
	return;
    }

    allocator::construct(_data->_key, _state.top()->key);
    allocator::construct(_data->_value, _state.top()->value);
    _data->depth = _state.size() - 1;
    _data->_is_state_initialized = true;
}

template<
	typename tkey,
	typename tvalue>
bool binary_search_tree<tkey, tvalue>::infix_const_iterator::operator==(
	typename binary_search_tree<tkey, tvalue>::infix_const_iterator const &other) const noexcept
{
    if (this->_state.empty() && other._state.empty())
    {
	return true;
    }

    if (this->_state.empty() ^ other._state.empty())
    {
	return false;
    }

    return this->_state.top() == other._state.top();
}


template<
	typename tkey,
	typename tvalue>
bool binary_search_tree<tkey, tvalue>::infix_const_iterator::operator!=(
	typename binary_search_tree<tkey, tvalue>::infix_const_iterator const &other) const noexcept
{
    return !(*this == other);
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::infix_const_iterator &
binary_search_tree<tkey, tvalue>::infix_const_iterator::operator++()
{
    if (_state.empty())
    {
	throw std::logic_error("iterator already ended!!!!)!)010!)!!");
    }

    if (_state.top()->right_subtree)
    {
	_state.push(_state.top()->right_subtree);
	while (_state.top()->left_subtree != nullptr)
	{
	    _state.push(_state.top()->left_subtree);
	}

	assign_data();
    }
    else
    {
	node *current = nullptr;
	do
	{
	    current = _state.top();
	    _state.pop();
	    if (_state.empty())
	    {
		return *this;
	    }

	    if (_state.top()->left_subtree == current)
	    {
		assign_data();
		return *this;
	    }
	} while (true);
    }

    return *this;
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::infix_const_iterator const
binary_search_tree<tkey, tvalue>::infix_const_iterator::operator++(
	int not_used)
{
    auto it = *this;
    this++;
    return it;
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::iterator_data const *
binary_search_tree<tkey, tvalue>::infix_const_iterator::operator*() const
{
    if (_state.empty())
    {
	throw std::logic_error("iterator already ended!!!!)!)010!)!!");
    }
    
    return _data;
}

// endregion infix_const_iterator implementation

// region infix_reverse_iterator implementation

template<
	typename tkey,
	typename tvalue>
binary_search_tree<tkey, tvalue>::infix_reverse_iterator::infix_reverse_iterator(
	binary_search_tree<tkey, tvalue> const *holder,
	typename binary_search_tree<tkey, tvalue>::node *subtree_root,
	iterator_data *_data): _holder(holder), _data(_data)
{
    while (subtree_root != nullptr)
    {
	_state.push(subtree_root);
	subtree_root = subtree_root->right_subtree;
    }

    if (_state.empty())
    {
	return;
    }

    allocator::construct(_data->_key, _state.top()->key);
    allocator::construct(_data->_value, _state.top()->value);
    _data->depth = _state.size() - 1;
    _data->_is_state_initialized = true;
}

template<
	typename tkey,
	typename tvalue>
bool binary_search_tree<tkey, tvalue>::infix_reverse_iterator::operator==(
	typename binary_search_tree<tkey, tvalue>::infix_reverse_iterator const &other) const noexcept
{
    if (this->_state.empty() && other._state.empty())
    {
	return true;
    }

    if (this->_state.empty() ^ other._state.empty())
    {
	return false;
    }

    return this->_state.top() == other._state.top();
}


template<
	typename tkey,
	typename tvalue>
bool binary_search_tree<tkey, tvalue>::infix_reverse_iterator::operator!=(
	typename binary_search_tree<tkey, tvalue>::infix_reverse_iterator const &other) const noexcept
{
    return !(*this == other);
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::infix_reverse_iterator &
binary_search_tree<tkey, tvalue>::infix_reverse_iterator::operator++()
{
    if (_state.empty())
    {
	throw std::logic_error("iterator already ended!!!!)!)010!)!!");
    }

    if (_state.top()->right_subtree)
    {
	_state.push(_state.top()->right_subtree);
	while (_state.top()->left_subtree != nullptr)
	{
	    _state.push(_state.top()->left_subtree);
	}

	assign_data();
    }
    else
    {
	node *current = nullptr;
	do
	{
	    current = _state.top();
	    _state.pop();
	    if (_state.empty())
	    {
		return *this;
	    }

	    if (_state.top()->right_subtree == current)
	    {
		assign_data();
		return *this;
	    }
	} while (true);
    }
    return *this;
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::infix_reverse_iterator const
binary_search_tree<tkey, tvalue>::infix_reverse_iterator::operator++(
	int not_used)
{
    auto it = *this;
    this++;
    return it;
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::iterator_data *
binary_search_tree<tkey, tvalue>::infix_reverse_iterator::operator*() const
{
    if (_state.empty())
    {
	throw std::logic_error("iterator already ended!!!!)!)010!)!!");
    }

    return _data;
}

// endregion infix_reverse_iterator implementation

// region infix_const_reverse_iterator implementation

template<
	typename tkey,
	typename tvalue>
binary_search_tree<tkey, tvalue>::infix_const_reverse_iterator::infix_const_reverse_iterator(
	binary_search_tree<tkey, tvalue> *holder,
	typename binary_search_tree<tkey, tvalue>::node *subtree_root,
	iterator_data *data): _holder(holder), _data(data)
{
    while (subtree_root)
    {
	_state.push(subtree_root);
	subtree_root = subtree_root->right_subtree;
    }
    if (_state.empty())
    {
	return; 
    }

    allocator::construct(_data->_key, _state.top()->key);
    allocator::construct(_data->_value, _state.top()->value);
    _data->depth = _state.size() - 1;
    _data->_is_state_initialized = true;
}

template<
	typename tkey,
	typename tvalue>
bool binary_search_tree<tkey, tvalue>::infix_const_reverse_iterator::operator==(
	typename binary_search_tree<tkey, tvalue>::infix_const_reverse_iterator const &other) const noexcept
{
    if (this->_state.empty() && other._state.empty())
    {
	return true;
    }

    if (this->_state.empty() ^ other._state.empty())
    {
	return false;
    }

    return this->_state.top() == other._state.top();
}


template<
	typename tkey,
	typename tvalue>
bool binary_search_tree<tkey, tvalue>::infix_const_reverse_iterator::operator!=(
	typename binary_search_tree<tkey, tvalue>::infix_const_reverse_iterator const &other) const noexcept
{
    return !(*this == other);
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::infix_const_reverse_iterator &
binary_search_tree<tkey, tvalue>::infix_const_reverse_iterator::operator++()
{
    if (_state.empty())
    {
	throw std::logic_error("Incrementing empty iterator");
    }

    if (_state.top()->right_subtree)
    {
	_state.push(_state.top()->right_subtree);
	while (_state.top()->left_subtree != nullptr)
	{
	    _state.push(_state.top()->left_subtree);
	}

	assign_data();
    }
    else
    {
	node *current = nullptr;
	do
	{
	    current = _state.top();
	    _state.pop();
	    if (_state.empty())
	    {
		return *this;
	    }

	    if (_state.top()->right_subtree == current)
	    {
		assign_data();
		return *this;
	    }
	} while (true);
    }
    return *this;
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::infix_const_reverse_iterator const
binary_search_tree<tkey, tvalue>::infix_const_reverse_iterator::operator++(
	int not_used)
{
    auto it = *this;
    this++;
    return it;
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::iterator_data const *
binary_search_tree<tkey, tvalue>::infix_const_reverse_iterator::operator*() const
{
    if (_state.empty())
    {
	throw std::logic_error("iterator already ended!!!!)!)010!)!!");
    }
    
    return _data;
}

// endregion infix_const_reverse_iterator implementation

// region postfix_iterator implementation

template<
	typename tkey,
	typename tvalue>
binary_search_tree<tkey, tvalue>::postfix_iterator::postfix_iterator(
	binary_search_tree<tkey, tvalue> *holder,
	typename binary_search_tree<tkey, tvalue>::node *subtree_root,
	iterator_data *data): _holder(holder), _data(data)
{
    if (subtree_root)
    {
	_state.push(subtree_root);
	_data->depth = 0;
	while (true)
	{
	    node *current = _state.top();
	    if (!current->right_subtree && !current->left_subtree)
	    {
		break;
	    }

	    if (current->right_subtree)
	    {
		_state.push(current->right_subtree);
		++_data->depth;
	    }

	    if (current->left_subtree)
	    {
		_state.push(current->right_subtree);
		++_data->depth;
	    }
	}

	allocator::construct(_data->_key, _state.top()->key);
	allocator::construct(_data->_value, _state.top()->value);
	_data->_is_state_initialized = true;
    }
    else
    {
	return;
    }

}

template<
	typename tkey,
	typename tvalue>
bool binary_search_tree<tkey, tvalue>::postfix_iterator::operator==(
	typename binary_search_tree<tkey, tvalue>::postfix_iterator const &other) const noexcept
{
    if (this->_state.empty() && other._state.empty())
    {
	return true;
    }

    if (this->_state.empty() ^ other._state.empty())
    {
	return false;
    }

    return this->_state.top() == other._state.top();
}


template<
	typename tkey,
	typename tvalue>
bool binary_search_tree<tkey, tvalue>::postfix_iterator::operator!=(
	typename binary_search_tree<tkey, tvalue>::postfix_iterator const &other) const noexcept
{
    return !(*this == other);
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::postfix_iterator &
binary_search_tree<tkey, tvalue>::postfix_iterator::operator++()
{
    if (_state.empty())
    {
	throw std::logic_error("iterator already ended!!!!)!)010!)!!");
    }
    
    while (true)
    {
	auto current = _state.top();
	_state.pop();

	if (_state.empty())
	{
	    return *this; 
	}

	if (current == _state.top()->left_subtree)
	{
	    node *temp = _state.top()->right_subtree;
	    --_data->depth;
	    while (temp != nullptr)
	    {
		_state.push(temp);
		temp = temp->left_subtree;
		++_data->depth;
	    }
	}
	else if (current == _state.top()->right_subtree)
	{
	    --_data->depth;
	}

	assign_data();
	return *this;
    }
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::postfix_iterator const
binary_search_tree<tkey, tvalue>::postfix_iterator::operator++(
	int not_used)
{
    auto it = *this;
    this++;
    return it;
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::iterator_data *
binary_search_tree<tkey, tvalue>::postfix_iterator::operator*() const
{
    if (_state.empty())
    {
	throw std::logic_error("iterator already ended!!!!)!)010!)!!");
    }
    
    return _data;
}

// endregion postfix_iterator implementation

// region postfix_const_iterator implementation

template<
	typename tkey,
	typename tvalue>
binary_search_tree<tkey, tvalue>::postfix_const_iterator::postfix_const_iterator(
	binary_search_tree<tkey, tvalue> *holder,
	typename binary_search_tree<tkey, tvalue>::node *subtree_root,
	iterator_data *data): _holder(holder), _data(data)
{
    if (subtree_root)
    {
	_state.push(subtree_root);
	_data->depth = 0;
	while (true)
	{
	    node *current = _state.top();
	    if (!current->right_subtree && !current->left_subtree)
	    { break; }

	    if (current->right_subtree)
	    {
		_state.push(current->right_subtree);
		++_data->depth;
	    }

	    if (current->left_subtree)
	    {
		_state.push(current->right_subtree);
		++_data->depth;
	    }
	}

	allocator::construct(_data->_key, _state.top()->key);
	allocator::construct(_data->_value, _state.top()->value);
	_data->_is_state_initialized = true;
    }
}

template<
	typename tkey,
	typename tvalue>
bool binary_search_tree<tkey, tvalue>::postfix_const_iterator::operator==(
	typename binary_search_tree<tkey, tvalue>::postfix_const_iterator const &other) const noexcept
{
    if (this->_state.empty() && other._state.empty())
    {
	return true;
    }

    if (this->_state.empty() ^ other._state.empty())
    {
	return false;
    }

    return this->_state.top() == other._state.top();
}


template<
	typename tkey,
	typename tvalue>
bool binary_search_tree<tkey, tvalue>::postfix_const_iterator::operator!=(
	typename binary_search_tree<tkey, tvalue>::postfix_const_iterator const &other) const noexcept
{
    return !(*this == other);
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::postfix_const_iterator &
binary_search_tree<tkey, tvalue>::postfix_const_iterator::operator++()
{
    if (_state.empty())
    {
	throw std::logic_error("iterator already empty!!!!");
    }

    while (true)
    {
	auto current = _state.top();
	_state.pop();

	if (_state.empty())
	{ 
	    return *this; 
	}

	if (current == _state.top()->left_subtree)
	{
	    node *tmp = _state.top()->right_subtree;
	    --_data->depth;
	    while (tmp)
	    {
		_state.push(tmp);
		tmp = tmp->left_subtree;
		++_data->depth;
	    }
	}
	else if (current == _state.top()->right_subtree)
	{
	    --_data->depth;
	}
	assign_data();
	return *this;
    }
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::postfix_const_iterator const
binary_search_tree<tkey, tvalue>::postfix_const_iterator::operator++(
	int not_used)
{
    auto it = *this;
    this++;
    return it;
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::iterator_data const *
binary_search_tree<tkey, tvalue>::postfix_const_iterator::operator*() const
{
    if (_state.empty())
    {
	throw std::logic_error("iterator already ended!!!!)!)010!)!!");
    }
    
    return _data;
}

// endregion postfix_const_iterator implementation

// region postfix_reverse_iterator implementation

template<
	typename tkey,
	typename tvalue>
binary_search_tree<tkey, tvalue>::postfix_reverse_iterator::postfix_reverse_iterator(
	binary_search_tree<tkey, tvalue> *holder,
	typename binary_search_tree<tkey, tvalue>::node *subtree_root,
	iterator_data *data) : _holder(holder), _data(data)
{
    if (subtree_root)
    {
	_state.push(subtree_root);
	_data->depth = 0;
	while (true)
	{
	    node *current = _state.top();
	    if (!current->right_subtree && !current->left_subtree)
	    { break; }

	    if (current->left_subtree)
	    {
		_state.push(current->left_subtree);
		++_data->depth;
	    }

	    if (current->right_subtree)
	    {
		_state.push(current->left_subtree);
		++_data->depth;
	    }
	}

	allocator::construct(_data->_key, _state.top()->key);
	allocator::construct(_data->_value, _state.top()->value);
	_data->_is_state_initialized = true;
    }
}

template<
	typename tkey,
	typename tvalue>
bool binary_search_tree<tkey, tvalue>::postfix_reverse_iterator::operator==(
	typename binary_search_tree<tkey, tvalue>::postfix_reverse_iterator const &other) const noexcept
{
    if (this->_state.empty() && other._state.empty())
    {
	return true;
    }

    if (this->_state.empty() ^ other._state.empty())
    {
	return false;
    }

    return this->_state.top() == other._state.top();
}


template<
	typename tkey,
	typename tvalue>
bool binary_search_tree<tkey, tvalue>::postfix_reverse_iterator::operator!=(
	typename binary_search_tree<tkey, tvalue>::postfix_reverse_iterator const &other) const noexcept
{
    return !(*this == other);
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::postfix_reverse_iterator &
binary_search_tree<tkey, tvalue>::postfix_reverse_iterator::operator++()
{
    if (_state.empty())
    {
	throw std::logic_error("iterator already ended!!!!)!)010!)!!");
    }
    
    while (true)
    {
	auto current = _state.top();
	_state.pop();

	if (_state.empty())
	{ 
	    return *this; 
	}

	if (current == _state.top()->right_subtree)
	{
	    node *tmp = _state.top()->left_subtree;
	    --_data->depth;
	    while (tmp)
	    {
		_state.push(tmp);
		tmp = tmp->right_subtree;
		++_data->depth;
	    }
	}
	else if (current == _state.top()->left_subtree)
	{
	    --_data->depth;
	}

	assign_data();
	return *this;
    }
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::postfix_reverse_iterator const
binary_search_tree<tkey, tvalue>::postfix_reverse_iterator::operator++(
	int not_used)
{
    auto it = *this;
    this++;
    return it;
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::iterator_data *
binary_search_tree<tkey, tvalue>::postfix_reverse_iterator::operator*() const
{
    if (_state.empty())
    {
	throw std::logic_error("iterator already ended!!!!)!)010!)!!");
    }
    
    return _data;
}

// endregion postfix_reverse_iterator implementation

// region postfix_const_reverse_iterator implementation

template<
	typename tkey,
	typename tvalue>
binary_search_tree<tkey, tvalue>::postfix_const_reverse_iterator::postfix_const_reverse_iterator(
	binary_search_tree<tkey, tvalue> *holder,
	typename binary_search_tree<tkey, tvalue>::node *subtree_root,
	iterator_data *data)
{
    if (subtree_root)
    {
	_state.push(subtree_root);
	_data->depth = 0;
	while (true)
	{
	    node *current = _state.top();
	    if (!current->right_subtree && !current->left_subtree)
	    { break; }

	    if (current->right_subtree)
	    {
		_state.push(current->right_subtree);
		++_data->depth;
	    }

	    if (current->left_subtree)
	    {
		_state.push(current->right_subtree);
		++_data->depth;
	    }
	}

	allocator::construct(_data->_key, _state.top()->key);
	allocator::construct(_data->_value, _state.top()->value);
	_data->_is_state_initialized = true;
    }

}

template<
	typename tkey,
	typename tvalue>
bool binary_search_tree<tkey, tvalue>::postfix_const_reverse_iterator::operator==(
	typename binary_search_tree<tkey, tvalue>::postfix_const_reverse_iterator const &other) const noexcept
{
    if (this->_state.empty() && other._state.empty())
    {
	return true;
    }

    if (this->_state.empty() ^ other._state.empty())
    {
	return false;
    }

    return this->_state.top() == other._state.top();
}


template<
	typename tkey,
	typename tvalue>
bool binary_search_tree<tkey, tvalue>::postfix_const_reverse_iterator::operator!=(
	typename binary_search_tree<tkey, tvalue>::postfix_const_reverse_iterator const &other) const noexcept
{
    return !(*this == other);
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::postfix_const_reverse_iterator &
binary_search_tree<tkey, tvalue>::postfix_const_reverse_iterator::operator++()
{
    if (_state.empty())
    {
	throw std::logic_error("iterator already ended!!!!)!)010!)!!"); 
    }

    while (true)
    {
	auto current = _state.top();
	
	_state.pop();

	if (_state.empty())
	{
	    return *this;
	}

	if (current == _state.top()->right_subtree)
	{
	    node *tmp = _state.top()->left_subtree;
	    --_data->depth;
	    while (tmp)
	    {
		_state.push(tmp);
		tmp = tmp->right_subtree;
		++_data->depth;
	    }
	}
	else if (current == _state.top()->left_subtree)
	{
	    --_data->depth;
	}
	
	assign_data();
	return *this;
    }
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::postfix_const_reverse_iterator const
binary_search_tree<tkey, tvalue>::postfix_const_reverse_iterator::operator++(
	int not_used)
{
    auto it = *this;
    this++;
    return it;
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::iterator_data const *
binary_search_tree<tkey, tvalue>::postfix_const_reverse_iterator::operator*() const
{
    if (_state.empty())
    {
	throw std::logic_error("iterator already ended!!!!)!)010!)!!");
    }
    
    return _data;
}

// endregion postfix_const_reverse_iterator implementation

// endregion iterators implementation

// region target operations associated exception types implementation

template<
	typename tkey,
	typename tvalue>
binary_search_tree<tkey, tvalue>::insertion_of_existent_key_attempt_exception::insertion_of_existent_key_attempt_exception(
	tkey const &key):
	std::logic_error("Attempt to insert already existing key inside the tree.")
{

}

template<
	typename tkey,
	typename tvalue>
tkey const &binary_search_tree<tkey, tvalue>::insertion_of_existent_key_attempt_exception::get_key() const noexcept
{
    return _key;
}

template<
	typename tkey,
	typename tvalue>
binary_search_tree<tkey, tvalue>::obtaining_of_nonexistent_key_attempt_exception::obtaining_of_nonexistent_key_attempt_exception(
	tkey const &key):
	std::logic_error("Attempt to obtain a value by non-existing key from the tree.")
{

}

template<
	typename tkey,
	typename tvalue>
tkey const &binary_search_tree<tkey, tvalue>::obtaining_of_nonexistent_key_attempt_exception::get_key() const noexcept
{
    return _key;
}

template<
	typename tkey,
	typename tvalue>
binary_search_tree<tkey, tvalue>::disposal_of_nonexistent_key_attempt_exception::disposal_of_nonexistent_key_attempt_exception(
	tkey const &key):
	std::logic_error("Attempt to dispose a value by non-existing key from the tree.")
{

}

template<
	typename tkey,
	typename tvalue>
tkey const &binary_search_tree<tkey, tvalue>::disposal_of_nonexistent_key_attempt_exception::get_key() const noexcept
{
    return _key;
}

// endregion target operations associated exception types implementation

// region template methods implementation

// region binary_search_tree<tkey, tvalue>::template_method_basics implementation

template<
	typename tkey,
	typename tvalue>
binary_search_tree<tkey, tvalue>::template_method_basics::template_method_basics(
	binary_search_tree<tkey, tvalue> *tree)
	: _tree(tree)
{

}

template<
	typename tkey,
	typename tvalue>
[[nodiscard]] inline logger *binary_search_tree<tkey, tvalue>::template_method_basics::get_logger() const noexcept
{
    return this->_tree->_logger;
}

// endregion binary_search_tree<tkey, tvalue>::template_method_basics implementation

// region search_tree<tkey, tvalue>::insertion_template_method implementation

template<
	typename tkey,
	typename tvalue>
binary_search_tree<tkey, tvalue>::insertion_template_method::insertion_template_method(
	binary_search_tree<tkey, tvalue> *tree,
	typename binary_search_tree<tkey, tvalue>::insertion_of_existent_key_attempt_strategy insertion_strategy):
	binary_search_tree<tkey, tvalue>::template_method_basics::template_method_basics(tree),
	_insertion_strategy(insertion_strategy)
{

}

template<
	typename tkey,
	typename tvalue>
void binary_search_tree<tkey, tvalue>::insertion_template_method::insert(
	tkey const &key,
	tvalue const &value)
{
    auto path = this->find_path(key);
    if (*(path.top()) != nullptr)
    {
	switch (_insertion_strategy)
	{
	    case insertion_of_existent_key_attempt_strategy::throw_an_exception:
		throw insertion_of_existent_key_attempt_exception(key);
	    case insertion_of_existent_key_attempt_strategy::update_value:
		(*(path.top()))->value = value;
		break;
	}

	return;
    }

    this->_tree->call_node_constructor(
	    *(path.top()) = reinterpret_cast<node *>(allocate_with_guard(this->_tree->get_node_size(), 1)), key, value);

    this->balance(path);
}

template<
	typename tkey,
	typename tvalue>
void binary_search_tree<tkey, tvalue>::insertion_template_method::insert(
	tkey const &key,
	tvalue &&value)
{
    auto path = this->find_path(key);
    if (*(path.top()) != nullptr)
    {
	switch (_insertion_strategy)
	{
	    case insertion_of_existent_key_attempt_strategy::throw_an_exception:
		throw insertion_of_existent_key_attempt_exception(key);
	    case insertion_of_existent_key_attempt_strategy::update_value:
		(*(path.top()))->value = std::move(value);
		break;
	}

	return;
    }

    this->_tree->call_node_constructor(
	    *(path.top()) = reinterpret_cast<node *>(allocate_with_guard(this->_tree->get_node_size(), 1)), key,
	    std::move(value));

    this->balance(path);
}

template<
	typename tkey,
	typename tvalue>
void binary_search_tree<tkey, tvalue>::insertion_template_method::set_insertion_strategy(
	typename binary_search_tree<tkey, tvalue>::insertion_of_existent_key_attempt_strategy insertion_strategy) noexcept
{
    _insertion_strategy = insertion_strategy;
}

template<
	typename tkey,
	typename tvalue>
allocator *binary_search_tree<tkey, tvalue>::insertion_template_method::get_allocator() const noexcept
{
    return this->_tree->get_allocator();
}

// endregion search_tree<tkey, tvalue>::insertion_template_method implementation

// region search_tree<tkey, tvalue>::obtaining_template_method implementation

template<
	typename tkey,
	typename tvalue>
binary_search_tree<tkey, tvalue>::obtaining_template_method::obtaining_template_method(
	binary_search_tree<tkey, tvalue> *tree):
	binary_search_tree<tkey, tvalue>::template_method_basics::template_method_basics(tree)
{

}

template<
	typename tkey,
	typename tvalue>
tvalue const &binary_search_tree<tkey, tvalue>::obtaining_template_method::obtain(
	tkey const &key)
{
    auto path = this->find_path(key);
    if (*(path.top()) == nullptr)
    {
	throw obtaining_of_nonexistent_key_attempt_exception(key);
    }

    tvalue const &got_value = (*(path.top()))->value;

    this->balance(path);

    return got_value;
}

// endregion search_tree<tkey, tvalue>::obtaining_template_method implementation

// region search_tree<tkey, tvalue>::disposal_template_method implementation

template<
	typename tkey,
	typename tvalue>
binary_search_tree<tkey, tvalue>::disposal_template_method::disposal_template_method(
	binary_search_tree<tkey, tvalue> *tree,
	typename binary_search_tree<tkey, tvalue>::disposal_of_nonexistent_key_attempt_strategy disposal_strategy):
	binary_search_tree<tkey, tvalue>::template_method_basics(tree)
{

}

template<
	typename tkey,
	typename tvalue>
tvalue binary_search_tree<tkey, tvalue>::disposal_template_method::dispose(
	tkey const &key)
{
    auto path = this->find_path(key);
    if (*(path.top()) == nullptr)
    {
	switch (_disposal_strategy)
	{
	    case disposal_of_nonexistent_key_attempt_strategy::throw_an_exception:
		throw disposal_of_nonexistent_key_attempt_exception(key);
	    case disposal_of_nonexistent_key_attempt_strategy::do_nothing:
		return tvalue();
	}
    }

    if ((*(path.top()))->left_subtree != nullptr && (*(path.top()))->right_subtree != nullptr)
    {
	auto *target_to_swap = *(path.top());
	auto **current = &((*(path.top()))->left_subtree);

	while (*current != nullptr)
	{
	    path.push(current);
	    current = &((*current)->right_subtree);
	}

	swap(std::move(target_to_swap->key), std::move((*(path.top()))->key));
	swap(std::move(target_to_swap->value), std::move((*(path.top()))->value));
    }

    tvalue value = std::move((*(path.top()))->value);
    node *leftover_subtree = (*(path.top()))->left_subtree == nullptr
			     ? (*(path.top()))->right_subtree
			     : (*(path.top()))->left_subtree;
    allocator::destruct(*(path.top()));
    deallocate_with_guard(*(path.top()));
    *(path.top()) = leftover_subtree;
    this->balance(path);
    return value;
}

template<
	typename tkey,
	typename tvalue>
void binary_search_tree<tkey, tvalue>::disposal_template_method::set_disposal_strategy(
	typename binary_search_tree<tkey, tvalue>::disposal_of_nonexistent_key_attempt_strategy disposal_strategy) noexcept
{
    _disposal_strategy = disposal_strategy;
}

template<
	typename tkey,
	typename tvalue>
[[nodiscard]] inline allocator *
binary_search_tree<tkey, tvalue>::disposal_template_method::get_allocator() const noexcept
{
    return this->_tree->get_allocator();
}

// endregion search_tree<tkey, tvalue>::disposal_template_method implementation

// endregion template methods

// region construction, assignment, destruction implementation

template<
	typename tkey,
	typename tvalue>
binary_search_tree<tkey, tvalue>::binary_search_tree(
	typename binary_search_tree<tkey, tvalue>::insertion_template_method *insertion_template,
	typename binary_search_tree<tkey, tvalue>::obtaining_template_method *obtaining_template,
	typename binary_search_tree<tkey, tvalue>::disposal_template_method *disposal_template,
	std::function<int(tkey const &, tkey const &)> comparer,
	allocator *allocator,
	logger *logger):
	search_tree<tkey, tvalue>(comparer, logger, allocator),
	_root(nullptr),
	_insertion_template(insertion_template),
	_obtaining_template(obtaining_template),
	_disposal_template(disposal_template)
{

}

template<
	typename tkey,
	typename tvalue>
binary_search_tree<tkey, tvalue>::binary_search_tree(
	std::function<int(tkey const &, tkey const &)> keys_comparer,
	allocator *allocator,
	logger *logger,
	typename binary_search_tree<tkey, tvalue>::insertion_of_existent_key_attempt_strategy insertion_strategy,
	typename binary_search_tree<tkey, tvalue>::disposal_of_nonexistent_key_attempt_strategy disposal_strategy):
	binary_search_tree(
		new binary_search_tree<tkey, tvalue>::insertion_template_method(this, insertion_strategy),
		new binary_search_tree<tkey, tvalue>::obtaining_template_method(this),
		new binary_search_tree<tkey, tvalue>::disposal_template_method(this, disposal_strategy),
		keys_comparer,
		allocator,
		logger)
{

}

template<
	typename tkey,
	typename tvalue>
binary_search_tree<tkey, tvalue>::binary_search_tree(
	binary_search_tree<tkey, tvalue> const &other):
	search_tree<tkey, tvalue>(other._keys_comparer, other.get_logger(), other.get_allocator()),
	_insertion_template(new binary_search_tree<tkey, tvalue>::insertion_template_method(this,
											    binary_search_tree<tkey, tvalue>::insertion_of_existent_key_attempt_strategy::throw_an_exception)),
	_obtaining_template(new binary_search_tree<tkey, tvalue>::obtaining_template_method(this)),
	_disposal_template(new binary_search_tree<tkey, tvalue>::disposal_template_method(this,
											  binary_search_tree<tkey, tvalue>::disposal_of_nonexistent_key_attempt_strategy::throw_an_exception))
{
    _root = copy(other._root);

}

template<
	typename tkey,
	typename tvalue>
binary_search_tree<tkey, tvalue>::binary_search_tree(
	binary_search_tree<tkey, tvalue> &&other) noexcept:
	search_tree<tkey, tvalue>(other._keys_comparer, other.get_logger(), other.get_allocator())
{
    other._logger = nullptr;
    other._allocator = nullptr;

    _insertion_template = other._insertion_template;
    other._insertion_template = nullptr;

    _obtaining_template = other._obtaining_template;
    other._obtaining_template = nullptr;

    _disposal_template = other._disposal_template;
    other._disposal_template = nullptr;

    _root = other._root;
    other._root = nullptr;
}

template<
	typename tkey,
	typename tvalue>
binary_search_tree<tkey, tvalue> &binary_search_tree<tkey, tvalue>::operator=(
	binary_search_tree<tkey, tvalue> const &other)
{
    if (this != &other)
    {
	// TODO: assign template methods or not?!
	clear(_root);
	this->_allocator = other._allocator;
	this->_logger = other._logger;
	this->_keys_comparer = other._keys_comparer;
	_root = copy(other._root);
    }

    return *this;
}

template<
	typename tkey,
	typename tvalue>
binary_search_tree<tkey, tvalue> &binary_search_tree<tkey, tvalue>::operator=(
	binary_search_tree<tkey, tvalue> &&other) noexcept
{
    if (this != &other)
    {
	clear(_root);
	this->_keys_comparer = other._keys_comparer;

	this->_logger = other._logger;
	other._logger = nullptr;

	this->_allocator = other._allocator;
	other._allocator = nullptr;

	_insertion_template = other._insertion_template;
	other._insertion_template = nullptr;

	_obtaining_template = other._obtaining_template;
	other._obtaining_template = nullptr;

	_disposal_template = other._disposal_template;
	other._disposal_template = nullptr;

	_root = other._root;
	other._root = nullptr;
    }

    return *this;
}

template<
	typename tkey,
	typename tvalue>
binary_search_tree<tkey, tvalue>::~binary_search_tree()
{
    delete _insertion_template;
    delete _obtaining_template;
    delete _disposal_template;
    clear(_root);
}

// endregion construction, assignment, destruction implementation

// region associative_container<tkey, tvalue> contract implementation

template<
	typename tkey,
	typename tvalue>
void binary_search_tree<tkey, tvalue>::insert(
	tkey const &key,
	tvalue const &value)
{
    _insertion_template->insert(key, value);
}

template<
	typename tkey,
	typename tvalue>
void binary_search_tree<tkey, tvalue>::insert(
	tkey const &key,
	tvalue &&value)
{
    _insertion_template->insert(key, std::move(value));
}

template<
	typename tkey,
	typename tvalue>
tvalue const &binary_search_tree<tkey, tvalue>::obtain(
	tkey const &key)
{
    return _obtaining_template->obtain(key);
}

template<
	typename tkey,
	typename tvalue>
std::vector<typename associative_container<tkey, tvalue>::key_value_pair>
binary_search_tree<tkey, tvalue>::obtain_between(
	tkey const &lower_bound,
	tkey const &upper_bound,
	bool lower_bound_inclusive,
	bool upper_bound_inclusive)
{
    return _obtaining_template->obtain_between(lower_bound, upper_bound, lower_bound_inclusive, upper_bound_inclusive);
}

template<
	typename tkey,
	typename tvalue>
tvalue binary_search_tree<tkey, tvalue>::dispose(
	tkey const &key)
{
    return _disposal_template->dispose(key);
}

// endregion associative_containers contract implementations

template<
	typename tkey,
	typename tvalue>
void binary_search_tree<tkey, tvalue>::set_insertion_strategy(
	typename binary_search_tree<tkey, tvalue>::insertion_of_existent_key_attempt_strategy insertion_strategy) noexcept
{
    this->_insertion_template->set_insertion_strategy(insertion_strategy);
}

template<
	typename tkey,
	typename tvalue>
void binary_search_tree<tkey, tvalue>::set_removal_strategy(
	typename binary_search_tree<tkey, tvalue>::disposal_of_nonexistent_key_attempt_strategy disposal_strategy) noexcept
{
    this->_disposal_template->set_disposal_strategy(disposal_strategy);
}

// region iterators requesting implementation

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::prefix_iterator
binary_search_tree<tkey, tvalue>::begin_prefix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::prefix_iterator(const_cast<binary_search_tree<tkey, tvalue> *>(this),
							     _root, create_iterator_data());
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::prefix_iterator binary_search_tree<tkey, tvalue>::end_prefix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::prefix_iterator(const_cast<binary_search_tree<tkey, tvalue> *>(this),
							     nullptr, create_iterator_data());
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::prefix_const_iterator
binary_search_tree<tkey, tvalue>::cbegin_prefix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::prefix_const_iterator(const_cast<binary_search_tree<tkey, tvalue> *>(this),
								   _root, create_iterator_data());
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::prefix_const_iterator
binary_search_tree<tkey, tvalue>::cend_prefix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::prefix_const_iterator(const_cast<binary_search_tree<tkey, tvalue> *>(this),
								   nullptr, create_iterator_data());
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::prefix_reverse_iterator
binary_search_tree<tkey, tvalue>::rbegin_prefix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::prefix_reverse_iterator(
	    const_cast<binary_search_tree<tkey, tvalue> *>(this), _root, create_iterator_data());
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::prefix_reverse_iterator
binary_search_tree<tkey, tvalue>::rend_prefix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::prefix_reverse_iterator(
	    const_cast<binary_search_tree<tkey, tvalue> *>(this), nullptr, create_iterator_data());
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::prefix_const_reverse_iterator
binary_search_tree<tkey, tvalue>::crbegin_prefix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::prefix_const_reverse_iterator(
	    const_cast<binary_search_tree<tkey, tvalue> *>(this), _root, create_iterator_data());
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::prefix_const_reverse_iterator
binary_search_tree<tkey, tvalue>::crend_prefix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::prefix_const_reverse_iterator(
	    const_cast<binary_search_tree<tkey, tvalue> *>(this), nullptr, create_iterator_data());
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::infix_iterator binary_search_tree<tkey, tvalue>::begin_infix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::infix_iterator(const_cast<binary_search_tree<tkey, tvalue> *>(this), _root,
							    create_iterator_data());
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::infix_iterator binary_search_tree<tkey, tvalue>::end_infix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::infix_iterator(const_cast<binary_search_tree<tkey, tvalue> *>(this),
							    nullptr, create_iterator_data());
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::infix_const_iterator
binary_search_tree<tkey, tvalue>::cbegin_infix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::infix_const_iterator(const_cast<binary_search_tree<tkey, tvalue> *>(this),
								  _root, create_iterator_data());
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::infix_const_iterator
binary_search_tree<tkey, tvalue>::cend_infix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::infix_const_iterator(const_cast<binary_search_tree<tkey, tvalue> *>(this),
								  nullptr, create_iterator_data());
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::infix_reverse_iterator
binary_search_tree<tkey, tvalue>::rbegin_infix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::infix_reverse_iterator(
	    const_cast<binary_search_tree<tkey, tvalue> *>(this), _root, create_iterator_data());
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::infix_reverse_iterator
binary_search_tree<tkey, tvalue>::rend_infix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::infix_reverse_iterator(
	    const_cast<binary_search_tree<tkey, tvalue> *>(this), nullptr, create_iterator_data());
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::infix_const_reverse_iterator
binary_search_tree<tkey, tvalue>::crbegin_infix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::infix_const_reverse_iterator(
	    const_cast<binary_search_tree<tkey, tvalue> *>(this), _root, create_iterator_data());
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::infix_const_reverse_iterator
binary_search_tree<tkey, tvalue>::crend_infix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::infix_const_reverse_iterator(
	    const_cast<binary_search_tree<tkey, tvalue> *>(this), nullptr, create_iterator_data());
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::postfix_iterator
binary_search_tree<tkey, tvalue>::begin_postfix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::postfix_iterator(const_cast<binary_search_tree<tkey, tvalue> *>(this),
							      _root, create_iterator_data());
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::postfix_iterator
binary_search_tree<tkey, tvalue>::end_postfix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::postfix_iterator(const_cast<binary_search_tree<tkey, tvalue> *>(this),
							      nullptr, create_iterator_data());
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::postfix_const_iterator
binary_search_tree<tkey, tvalue>::cbegin_postfix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::postfix_const_iterator(
	    const_cast<binary_search_tree<tkey, tvalue> *>(this), _root, create_iterator_data());
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::postfix_const_iterator
binary_search_tree<tkey, tvalue>::cend_postfix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::postfix_const_iterator(
	    const_cast<binary_search_tree<tkey, tvalue> *>(this), nullptr, create_iterator_data());
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::postfix_reverse_iterator
binary_search_tree<tkey, tvalue>::rbegin_postfix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::postfix_reverse_iterator(
	    const_cast<binary_search_tree<tkey, tvalue> *>(this), _root, create_iterator_data());
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::postfix_reverse_iterator
binary_search_tree<tkey, tvalue>::rend_postfix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::postfix_reverse_iterator(
	    const_cast<binary_search_tree<tkey, tvalue> *>(this), nullptr, create_iterator_data());
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::postfix_const_reverse_iterator
binary_search_tree<tkey, tvalue>::crbegin_postfix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::postfix_const_reverse_iterator(
	    const_cast<binary_search_tree<tkey, tvalue> *>(this), _root, create_iterator_data());
}

template<
	typename tkey,
	typename tvalue>
typename binary_search_tree<tkey, tvalue>::postfix_const_reverse_iterator
binary_search_tree<tkey, tvalue>::crend_postfix() const noexcept
{
    return binary_search_tree<tkey, tvalue>::postfix_const_reverse_iterator(
	    const_cast<binary_search_tree<tkey, tvalue> *>(this), nullptr, create_iterator_data());
}

// endregion iterators request implementation

// region subtree rotations implementation

template<
	typename tkey,
	typename tvalue>
void binary_search_tree<tkey, tvalue>::small_left_rotation(
	binary_search_tree<tkey, tvalue>::node *&subtree_root,
	bool validate) const
{
    if (validate && (!subtree_root || !subtree_root->right_subtree))
    {
	throw std::logic_error("Invalid tree");
    }

    node *tmp = subtree_root->right_subtree;
    tmp->right_subtree = subtree_root->left_subtree;
    tmp->left_subtree = subtree_root;

}

template<
	typename tkey,
	typename tvalue>
void binary_search_tree<tkey, tvalue>::small_right_rotation(
	binary_search_tree<tkey, tvalue>::node *&subtree_root,
	bool validate) const
{
    if (validate && (!subtree_root || !subtree_root->left_subtree))
    {
	throw std::logic_error("Tree is not valid :((9(((");
    }

    node *tmp = subtree_root->left_subtree;
    tmp->left_subtree = subtree_root->right_subtree;
    tmp->left_subtree = subtree_root;
}

template<
	typename tkey,
	typename tvalue>
void binary_search_tree<tkey, tvalue>::big_left_rotation(
	binary_search_tree<tkey, tvalue>::node *&subtree_root,
	bool validate) const
{

    if (validate && (!subtree_root || !subtree_root->left_subtree || !subtree_root->left_subtree->right_subtree))
    {
	throw std::logic_error("Tree is not valid :((9(((");
    }

    node *child = subtree_root->left_subtree;
    node *grand_child = child->right_subtree;

    subtree_root->left_subtree = grand_child;
    grand_child->left_subtree = child;

    small_left_rotation(subtree_root, true);
}

template<
	typename tkey,
	typename tvalue>
void binary_search_tree<tkey, tvalue>::big_right_rotation(
	binary_search_tree<tkey, tvalue>::node *&subtree_root,
	bool validate) const
{
    if (validate && (!subtree_root || !subtree_root->right_subtree || !subtree_root->right_subtree->left_subtree))
    {
	throw std::logic_error("Tree is not valid :((9(((");
    }

    node *child = subtree_root->right_subtree;
    node *grand_child = child->left_subtree;

    subtree_root->right_subtree = grand_child;
    grand_child->right_subtree = child;

    small_right_rotation(subtree_root, true);
}

template<
	typename tkey,
	typename tvalue>
void binary_search_tree<tkey, tvalue>::double_left_rotation(
	binary_search_tree<tkey, tvalue>::node *&subtree_root,
	bool at_grandparent_first,
	bool validate) const
{
    if (validate && (!subtree_root || !subtree_root->right_subtree || !subtree_root->right_subtree->right_subtree))
    {
	throw std::logic_error("Tree is not valid :((9(((");
    }

    small_left_rotation(subtree_root, true);
    small_left_rotation(subtree_root, true);
}

template<
	typename tkey,
	typename tvalue>
void binary_search_tree<tkey, tvalue>::double_right_rotation(
	binary_search_tree<tkey, tvalue>::node *&subtree_root,
	bool at_grandparent_first,
	bool validate) const
{
    if (validate && (!subtree_root || !subtree_root->left_subtree || !subtree_root->left_subtree->left_subtree))
    {
	throw std::logic_error("Tree is not valid :((9(((");
    }

    small_right_rotation(subtree_root, true);
    small_right_rotation( subtree_root, true);
}

// endregion subtree rotations implementation

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_BINARY_SEARCH_TREE_H