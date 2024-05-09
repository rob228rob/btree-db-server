#include "allocator_sorted_list.h"
#include <in6addr.h>
#include <mutex>
#include <not_implemented.h>

allocator_sorted_list::~allocator_sorted_list()
{

}

allocator_sorted_list::allocator_sorted_list(
    allocator_sorted_list &&other) noexcept : _trusted_memory(other._trusted_memory)
{
    other._trusted_memory = nullptr;
}

allocator_sorted_list &allocator_sorted_list::operator=(
    allocator_sorted_list &&other) noexcept
{
  if (this != &other)
  {
    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;
  }

  return *this;
}

allocator_sorted_list::allocator_sorted_list(
    size_t space_size,
    allocator *parent_allocator,
    logger *logger,
    allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
  if (space_size < get_ancillary_aviable_block_size())
  {
    throw std::logic_error("can't initialize allocator instance");
  }

  auto common_size = space_size + get_ancillary_space_size() + get_ancillary_aviable_block_size(); // WAS CHANGED;
  try
  {
    _trusted_memory = parent_allocator == nullptr
                          ? ::operator new(common_size)
                          : parent_allocator->allocate(common_size, 1);
  }
  catch (std::bad_alloc const &ex)
  {
    throw ;
  }

  allocator **parent_allocator_space_address = reinterpret_cast<allocator **>(_trusted_memory);
  *parent_allocator_space_address = parent_allocator;

  class logger **logger_space_address = reinterpret_cast<class logger **>(parent_allocator_space_address + 1);
  *logger_space_address = logger;

  size_t *space_size_space_address = reinterpret_cast<size_t *>(logger_space_address + 1);
  *space_size_space_address = space_size;

  allocator_with_fit_mode::fit_mode *fit_mode_space_address = reinterpret_cast<allocator_with_fit_mode::fit_mode *>(space_size_space_address + 1);
  *fit_mode_space_address = allocate_fit_mode;

  void **first_block_address_space_address = reinterpret_cast<void **>(fit_mode_space_address + 1);
  *first_block_address_space_address = reinterpret_cast<u_char *>(_trusted_memory) + sizeof(allocator *) + sizeof(class logger *) + sizeof (size_t) + sizeof(void *) + sizeof (allocator_with_fit_mode::fit_mode) + sizeof(std::mutex);

  std::mutex *_mutex = reinterpret_cast<std::mutex*>(first_block_address_space_address  +1);
  construct(_mutex);
  information_with_guard("Mutex META was defined");

  auto* first_block = reinterpret_cast<u_char *>(_trusted_memory) + sizeof(allocator *) + sizeof(class logger *) + sizeof (size_t) + sizeof(void *) + sizeof (allocator_with_fit_mode::fit_mode) + sizeof(std::mutex);
  set_aviable_block_pointer_to_next(first_block, nullptr);
  set_aviable_block_size(first_block, space_size - get_ancillary_aviable_block_size());
}


[[nodiscard]] void *allocator_sorted_list::allocate(
    size_t value_size,
    size_t values_count)
{
  debug_with_guard("void * allocate STARTED");
  std::mutex* _mutex = get_mutex();
  std::lock_guard<std::mutex> lock(*_mutex);
  debug_with_guard("void * allocate CONTINUED AFTER mutex");
  auto requested_size = value_size * values_count;

  if (requested_size < sizeof(block_pointer_t))
  {
    requested_size = sizeof(block_pointer_t);
    warning_with_guard("requested space size was changed");
  }

  allocator_with_fit_mode::fit_mode fit_mode = get_fit_mode();

  void *target_block = nullptr;
  void *previous_to_target_block = nullptr;
  void *next_to_target_block = nullptr;


    void *previous_block = nullptr;
    void *current_block =  get_first_aviable_block();
    size_t min_max_size = get_aviable_block_size(current_block);
    size_t current_block_size = get_aviable_block_size(current_block);

  while (current_block != nullptr)
  {

    current_block_size = get_aviable_block_size(current_block);

    if (fit_mode == allocator_with_fit_mode::fit_mode::first_fit && current_block_size >= requested_size + get_ancillary_aviable_block_size())
    {
      min_max_size = current_block_size;
      previous_to_target_block = previous_block;
      target_block = current_block;
      next_to_target_block = get_aviable_block_next_block_address(current_block);
      break;
    }
    else if (fit_mode == allocator_with_fit_mode::fit_mode::the_best_fit && current_block_size >= requested_size + get_ancillary_aviable_block_size())
    {

      if (min_max_size <= current_block_size)
      {
        previous_to_target_block = previous_block;
        target_block = current_block;
        next_to_target_block = get_aviable_block_next_block_address(target_block);
        min_max_size = current_block_size;
      }
    }
    else if (fit_mode == allocator_with_fit_mode::fit_mode::the_worst_fit && current_block_size >= requested_size + get_ancillary_aviable_block_size())
    {
      if (min_max_size >= current_block_size)
      {
        previous_to_target_block = previous_block;
        target_block = current_block;
        next_to_target_block = get_aviable_block_next_block_address(target_block);
        min_max_size = current_block_size;
      }
    }

    previous_block = current_block;
    current_block = get_aviable_block_next_block_address(current_block);

  }

  if (target_block == nullptr)
  {
    error_with_guard("can't allocate block!");
    throw std::bad_alloc();
  }

  auto blocks_sizes_difference = get_aviable_block_size(target_block) - requested_size;
  if (blocks_sizes_difference > 0 && blocks_sizes_difference < (sizeof(void *) + sizeof(size_t)))
  {
    warning_with_guard("requested space size was changed");
    requested_size = get_aviable_block_size(target_block);
  }

  *reinterpret_cast<size_t *>(target_block) = requested_size;

  *reinterpret_cast<allocator **>(reinterpret_cast<unsigned char *>(target_block) + sizeof(size_t)) = this;

  void* new_target = reinterpret_cast<unsigned char *>(target_block) + requested_size;

  if (next_to_target_block == nullptr)
  {
    size_t new_size = min_max_size - requested_size - get_ancillary_aviable_block_size() ;

    set_aviable_block_pointer_to_next(new_target, nullptr);

    set_aviable_block_size(new_target, new_size);

    if (previous_to_target_block == nullptr)
    {
        set_first_aviable_block(new_target);
    }
    else
    {
        set_aviable_block_pointer_to_next(previous_to_target_block, new_target);
    }
  }
  else
  {
    if (previous_to_target_block == nullptr)
    {
        set_first_aviable_block(new_target);
        set_aviable_block_pointer_to_next(new_target, next_to_target_block);
    }
    else
    {
        set_aviable_block_pointer_to_next(previous_to_target_block, new_target);
        set_aviable_block_pointer_to_next(new_target, next_to_target_block);
    }
  }

  debug_with_guard("block was allocated");
  return reinterpret_cast<u_char*>(target_block) + get_ancillary_aviable_block_size() ;
}

void allocator_sorted_list::deallocate(
    void *at) {
  std::mutex *_mutex = get_mutex();
  std::lock_guard<std::mutex> lock(*_mutex);

  void *to_dealloc =
      reinterpret_cast<u_char *>(at) - get_ancillary_aviable_block_size();
  auto size_at = get_occupied_block_size(to_dealloc);
  auto *parent_allocator = get_parent_allocator(to_dealloc);

  if (parent_allocator != this) {
    error_with_guard("can't deallocate, paren't allocator was not matched");
    throw std::logic_error(
        "can't deallocate, paren't allocator was not matched");
  }

  void *target_block = get_first_aviable_block();
  void *prev_block = nullptr;

  if (target_block > get_memory_begining() && to_dealloc < target_block) {

    if (reinterpret_cast<u_char *>(to_dealloc) +
            get_ancillary_occupied_block_size() +
            get_occupied_block_size(to_dealloc) <
        target_block) {
        set_aviable_block_pointer_to_next(to_dealloc, target_block);
        set_first_aviable_block(to_dealloc);
        set_aviable_block_size(to_dealloc, get_occupied_block_size(to_dealloc));
    } else {
        size_t updated_block_size = get_occupied_block_size(to_dealloc) +
                                    get_aviable_block_size(target_block) +
                                    get_ancillary_aviable_block_size();
        void *pointer_next_to_target =
            get_aviable_block_next_block_address(target_block);
        set_aviable_block_size(to_dealloc, updated_block_size);
        set_aviable_block_pointer_to_next(to_dealloc, pointer_next_to_target);
        set_first_aviable_block(to_dealloc);
    }

    debug_with_guard("block was deallocated");
    return;
  }
  int iteration_count = 0;
  while (true)
  {
    ++iteration_count;
    if (prev_block < to_dealloc && to_dealloc < target_block)
    {
        break;
    }

    prev_block = target_block;

    target_block = get_aviable_block_next_block_address(target_block);

    if (target_block == nullptr)
    {
        if (reinterpret_cast<u_char *>(prev_block) +
                get_aviable_block_size(prev_block) +
                get_ancillary_aviable_block_size() ==
            to_dealloc)
        {
        size_t new_block_size = get_aviable_block_size(prev_block) +
                                  get_ancillary_occupied_block_size() +
                                  get_occupied_block_size(to_dealloc);
          set_aviable_block_size(prev_block, new_block_size);
          set_aviable_block_pointer_to_next(prev_block, nullptr);
        }
        else
        {
          set_aviable_block_pointer_to_next(prev_block, to_dealloc);
          set_aviable_block_pointer_to_next(to_dealloc, nullptr);
          set_aviable_block_size(to_dealloc, size_at);
        }

        return;
    }
  }
  size_t prev_and_at_block_diff;

  //if (prev_block != nullptr)
  {
    // TODO: prev_block < to_dealloc, but prev_block + meta > to_dealloc(How is it possibe????)
    prev_and_at_block_diff =
        reinterpret_cast<u_char *>(prev_block) +
                    get_ancillary_aviable_block_size() +
                    get_aviable_block_size(prev_block) ==
                to_dealloc
            ? 0
            : reinterpret_cast<u_char *>(to_dealloc) -
                  reinterpret_cast<u_char *>(prev_block) -
                  get_ancillary_aviable_block_size() -
                  get_aviable_block_size(prev_block);
  }

  size_t at_and_target_block_diff;
  //if (target_block != nullptr)
  {
    //TODO: fix overloading
    at_and_target_block_diff =
        reinterpret_cast<u_char *>(to_dealloc) +
                    get_ancillary_occupied_block_size() +
                    get_occupied_block_size(to_dealloc) ==
                target_block
            ? 0
            : reinterpret_cast<u_char *>(target_block) -
                  reinterpret_cast<u_char *>(to_dealloc) -
                  get_ancillary_occupied_block_size() -
                  get_occupied_block_size(to_dealloc);
  }



  if (prev_and_at_block_diff == 0 && at_and_target_block_diff == 0) {
    //merge 3 available
    size_t new_block_size = get_aviable_block_size(prev_block) +
                            get_aviable_block_size(target_block) +
                            get_ancillary_aviable_block_size() +
                            get_occupied_block_size(to_dealloc) +
                            get_ancillary_occupied_block_size();
    void *new_block_pointer_to_next =
        get_aviable_block_next_block_address(target_block);
    set_aviable_block_pointer_to_next(prev_block, new_block_pointer_to_next);
    set_aviable_block_size(prev_block, new_block_size);

    return;
  }
  else if (prev_and_at_block_diff == 0 && at_and_target_block_diff > 0) {
    // merge 2 available;
    size_t new_block_size = get_aviable_block_size(prev_block) + get_occupied_block_size(to_dealloc) + get_ancillary_occupied_block_size();
    set_aviable_block_size(prev_block, new_block_size);
    set_aviable_block_pointer_to_next(prev_block, target_block);
  }
  else if (prev_and_at_block_diff > 0 && at_and_target_block_diff == 0)
  {
    // merge 2 available;
    size_t new_block_size = get_occupied_block_size(to_dealloc) + get_ancillary_aviable_block_size() + get_aviable_block_size(target_block);
    set_aviable_block_size(to_dealloc, new_block_size);
    set_aviable_block_pointer_to_next(to_dealloc, get_aviable_block_next_block_address(target_block));
    set_aviable_block_pointer_to_next(prev_block, to_dealloc);
  }
  else if (prev_and_at_block_diff > 0 && at_and_target_block_diff > 0)
  {
    set_aviable_block_pointer_to_next(prev_block, to_dealloc);
    set_aviable_block_pointer_to_next(to_dealloc, target_block);
  }

  debug_with_guard("memory was deallocated");
}

inline void allocator_sorted_list::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
  std::mutex* _mutex = get_mutex();
  std::lock_guard<std::mutex> lock(*_mutex);

  *reinterpret_cast<allocator_with_fit_mode::fit_mode *>(reinterpret_cast<unsigned char *>(_trusted_memory)
                                                         + sizeof(allocator *) + sizeof(logger *) + sizeof(size_t)) = mode;
}

inline allocator *allocator_sorted_list::get_allocator() const
{
  return *reinterpret_cast<allocator**>(_trusted_memory);
}

inline std::mutex *allocator_sorted_list::get_mutex() const
{
  return reinterpret_cast<std::mutex *>(reinterpret_cast<unsigned char *>(_trusted_memory) + sizeof(allocator *) + sizeof(logger *) + sizeof(size_t) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(void *));
}

std::vector<allocator_test_utils::block_info> allocator_sorted_list::get_blocks_info() const noexcept {
  // TODO: fix incorrect algorithm
  std::mutex *_mutex = get_mutex();
  std::lock_guard<std::mutex> lock(*_mutex);

  std::vector<allocator_test_utils::block_info> block_inf;

  size_t global_size = get_global_size();

  void *target_block = get_first_aviable_block();

  void *space_begining = get_memory_begining();

  void *next_to_target = get_aviable_block_next_block_address(target_block);

  if (target_block > space_begining)
  {
    while (true) {

        if (space_begining >= target_block)
        {
          break;
        }

        size_t occupied_block_size = get_occupied_block_size(space_begining);
        block_inf.push_back(
            allocator_test_utils::block_info{occupied_block_size, true});

        space_begining = reinterpret_cast<u_char *>(space_begining) +
                         get_ancillary_occupied_block_size() +
                         get_occupied_block_size(space_begining);
    }
  }

  while (target_block != nullptr)
  {
    block_inf.push_back(allocator_test_utils::block_info{get_aviable_block_size(target_block),false});

    auto occupied_target = target_block;

    while (true)
    {

        if (occupied_target >= target_block)
        {
          break;
        }

        size_t occupied_block_size = get_occupied_block_size(occupied_target);
        block_inf.push_back(
            allocator_test_utils::block_info{occupied_block_size, true});

        occupied_target = reinterpret_cast<u_char *>(occupied_target) +
                         get_ancillary_occupied_block_size() +
                         get_occupied_block_size(occupied_target);
    }

    target_block = next_to_target;
    next_to_target = get_aviable_block_next_block_address(target_block);

    auto end_of_target = reinterpret_cast<u_char*>(target_block) + get_aviable_block_size(target_block) + get_ancillary_aviable_block_size();
    auto end_of_space = reinterpret_cast<u_char*>(get_memory_begining()) + global_size;
    if (next_to_target == nullptr &&  end_of_target <  end_of_space)
    {
        block_inf.push_back(allocator_test_utils::block_info{get_aviable_block_size(target_block), false});
        while (true)
        {
          if (end_of_target >= end_of_space)
          {
            break;
          }

          block_inf.push_back(allocator_test_utils::block_info{get_occupied_block_size(end_of_target), true});

          end_of_target = reinterpret_cast<u_char*>(end_of_target) + get_ancillary_occupied_block_size() + get_occupied_block_size(end_of_target);
        }

    }
    else if (next_to_target == nullptr && end_of_target == end_of_space)
    {
        break;
        //merge 2
       // auto updated_block_size = get_aviable_block_size(target_block) + get_ancillary_occupied_block_size() + get_occupied_block_size(end_of_target);
    }

  }


  void* current_block = reinterpret_cast<void *>(reinterpret_cast<u_char*>(_trusted_memory) + get_ancillary_space_size());

  while (current_block != nullptr)
  {
      size_t size;
      size = get_ancillary_aviable_block_size() + get_aviable_block_size(current_block);

      block_inf.push_back(allocator_test_utils::block_info{size, 0});
      bool has_next_occupied;
      size_t delta = get_ancillary_aviable_block_size() + get_aviable_block_size(current_block);
      void* next_occupied = reinterpret_cast<unsigned char *>(current_block) + delta;
      while (has_next_occupied)
      {
        size_t size_occupied = get_occupied_block_size(next_occupied);
        if (reinterpret_cast<unsigned char *>(next_occupied) + get_ancillary_occupied_block_size() + get_occupied_block_size(next_occupied) == get_aviable_block_next_block_address(current_block))
        {
          has_next_occupied = false;
        }
    }

    current_block = get_aviable_block_next_block_address(current_block);

    //TODO: info about occupied blocks;
  }

  return block_inf;
}

inline logger *allocator_sorted_list::get_logger() const
{
  return *reinterpret_cast<logger **>(reinterpret_cast<unsigned char *>(_trusted_memory) + sizeof(allocator *));
}

inline std::string allocator_sorted_list::get_typename() const noexcept
{
  return "allocator_sorted_list";
}

size_t allocator_sorted_list::get_ancillary_aviable_block_size() noexcept
{
  return sizeof(void*) + sizeof(size_t);
}

size_t allocator_sorted_list::get_ancillary_occupied_block_size() noexcept
{
  return sizeof(size_t) + sizeof(allocator *);
}

size_t allocator_sorted_list::get_ancillary_space_size() noexcept
{
  return sizeof(logger *) + sizeof(allocator *) + sizeof(size_t) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(std::mutex) + sizeof(void *);
}

allocator_with_fit_mode::fit_mode allocator_sorted_list::get_fit_mode() const noexcept
{
  return *reinterpret_cast<allocator_with_fit_mode::fit_mode *>(reinterpret_cast<unsigned char *>(_trusted_memory) + sizeof(allocator *) + sizeof(logger *) + sizeof(size_t));
}

size_t allocator_sorted_list::get_aviable_block_size(
    void *block_address) noexcept
{
  return *reinterpret_cast<size_t *>(reinterpret_cast<u_char *>(block_address) + sizeof(void *));
}

void *allocator_sorted_list::get_first_aviable_block() const noexcept
{
  auto memory_begining = reinterpret_cast<u_char *>(_trusted_memory) + sizeof(logger *) + sizeof(allocator *)
                     + sizeof(size_t) + sizeof(allocator_with_fit_mode::fit_mode);
  return *reinterpret_cast<void**>(memory_begining);
}

size_t allocator_sorted_list::get_global_size() const noexcept
{
  auto memory_begining = reinterpret_cast<u_char *>(_trusted_memory) + sizeof(logger *) + sizeof(allocator *);
    return *reinterpret_cast<size_t *>(memory_begining);
}

void allocator_sorted_list::set_first_aviable_block(void* new_first_block) noexcept
{
  auto memory_begining = reinterpret_cast<u_char *>(_trusted_memory) + sizeof(logger *) + sizeof(allocator *)
                         + sizeof(size_t) + sizeof(allocator_with_fit_mode::fit_mode);
  *reinterpret_cast<void**>(memory_begining) = new_first_block;
}

allocator::block_size_t allocator_sorted_list::get_first_aviable_block_size() noexcept
{
  return get_aviable_block_size(get_first_aviable_block());
}

void allocator_sorted_list::set_aviable_block_size(
    void *block_address, size_t size) noexcept
{
  *reinterpret_cast<size_t *>(reinterpret_cast<u_char *>(block_address) + sizeof(void *)) = size;
}

void allocator_sorted_list::set_aviable_block_pointer_to_next(
    void *block_adress, void* pointer) noexcept
{
    *reinterpret_cast<void**>(block_adress) = pointer;
}

void *allocator_sorted_list::get_aviable_block_next_block_address(
    void *block_address) noexcept
{
  return *reinterpret_cast<void **>(block_address);
}

allocator::block_size_t allocator_sorted_list::get_occupied_block_size(
    void *block_address) noexcept
{
  return *reinterpret_cast<allocator::block_size_t *>(block_address);
}

allocator* allocator_sorted_list::get_parent_allocator(
    void *block_address) noexcept
{
  return *reinterpret_cast<allocator **>(reinterpret_cast<allocator::block_size_t *>(block_address) + 1);
}

void* allocator_sorted_list::get_memory_begining() const noexcept
{
  return reinterpret_cast<void *>(reinterpret_cast<u_char*>(_trusted_memory)
      + sizeof(allocator *) + sizeof(logger *)
      + sizeof(size_t) + sizeof(fit_mode)
      + sizeof(void*) + sizeof(std::mutex));
}