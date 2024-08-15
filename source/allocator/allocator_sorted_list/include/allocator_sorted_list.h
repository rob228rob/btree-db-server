#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_SORTED_LIST_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_SORTED_LIST_H

#include <allocator_guardant.h>
#include <allocator_test_utils.h>
#include <allocator_with_fit_mode.h>
#include <logger_guardant.h>
#include <typename_holder.h>
#include <mutex>

class allocator_sorted_list final:
    private allocator_guardant,
    public allocator_test_utils,
    public allocator_with_fit_mode,
    private logger_guardant,
    private typename_holder
{

private:

  void *_trusted_memory;

public:

  ~allocator_sorted_list() override;

  allocator_sorted_list(
      allocator_sorted_list const &other) = delete;

  allocator_sorted_list &operator=(
      allocator_sorted_list const &other) = delete;

  allocator_sorted_list(
      allocator_sorted_list &&other) noexcept;

  allocator_sorted_list &operator=(
      allocator_sorted_list &&other) noexcept;

public:

  explicit allocator_sorted_list(
      size_t space_size,
      allocator *parent_allocator = nullptr,
      logger *logger = nullptr,
      allocator_with_fit_mode::fit_mode allocate_fit_mode = allocator_with_fit_mode::fit_mode::first_fit);

public:

  [[nodiscard]] void *allocate(
      size_t value_size,
      size_t values_count) override;

  void deallocate(
      void *at) override;

public:

  inline void set_fit_mode(
      allocator_with_fit_mode::fit_mode mode) override;

private:

  inline allocator *get_allocator() const override;

public:

  std::vector<allocator_test_utils::block_info> get_blocks_info() const noexcept override;

private:

  inline logger *get_logger() const override;

private:

  inline std::string get_typename() const noexcept override;

  inline std::mutex *get_mutex() const;

private:

  allocator::block_size_t get_first_aviable_block_size() noexcept;

  static size_t get_ancillary_space_size() noexcept;

  static size_t get_ancillary_aviable_block_size() noexcept;

  static size_t get_ancillary_occupied_block_size() noexcept;

  allocator_with_fit_mode::fit_mode get_fit_mode() const noexcept;

  void *get_first_aviable_block() const noexcept;

private:

  static size_t get_aviable_block_size(
      void *block_address) noexcept;

  static void *get_aviable_block_next_block_address(
      void *block_address) noexcept;

  static block_size_t get_occupied_block_size(
      void *block_address) noexcept;

  allocator* get_parent_allocator(
      void *block_address) noexcept;

  void set_aviable_block_size(
      void *block_address,
      size_t size) noexcept;

  void set_aviable_block_pointer_to_next(
      void *block_adress,
      void *pointer) noexcept;

  void set_first_aviable_block(void *new_first_block) noexcept;

  void *get_memory_begining() const noexcept
      ;
  size_t get_global_size() const noexcept;
};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_SORTED_LIST_H