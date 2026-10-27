#!/usr/bin/env python3
"""Apply the targeted Heim source hardening from the 2026-10-27 audit.

Run from the repository root, after extracting the ZIP:
    python scripts/apply_hardening.py

Refuses a partial/unknown implementation version. Re-running is harmless.
"""
from pathlib import Path
import argparse
import sys
import subprocess

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'include/heim/common/sparse.hpp'
TEST_SUPPORT = ROOT / 'test/heim/common/sparse_index/support.hpp'


def apply(original: str) -> tuple[str, list[str]]:
    s = original
    applied = []

    def replace(old: str, new: str, label: str) -> None:
        nonlocal s
        if s.count(old) == 1:
            s = s.replace(old, new)
            applied.append(label)
        elif new in s:
            applied.append(label + ' (already applied)')
        else:
            raise RuntimeError(f'Unexpected sparse.hpp version: cannot locate {label}. No file was written.')

    replace('''  constexpr
  sparse_index &
  operator=(sparse_index const &other)
  {
    if constexpr (is_paged)
    {
      if (this == std::addressof(other))
        return *this;

      if constexpr (page_pointer_alloc_traits::propagate_on_container_copy_assignment::value)
      {
        auto * const ptr{std::addressof(this->m_cont)};

        std::destroy_at  (ptr);
        std::construct_at(ptr, other.m_cont.get_allocator());
      }
      else
        this->m_cont.clear();

      this->m_copy(other.m_cont);
    }
    else
      this->m_cont = other.m_cont;

    return *this;
  }''', '''  constexpr
  sparse_index &
  operator=(sparse_index const &other)
  {
    if (this == std::addressof(other))
      return *this;

    if constexpr (is_paged)
    {
      // Prepare every new page before changing the existing index.
      if constexpr (page_pointer_alloc_traits::propagate_on_container_copy_assignment::value)
      {
        sparse_index replacement{other, other.get_allocator()};
        // A vector move construction cannot allocate: safe after destroy_at.
        static_assert(std::is_nothrow_move_constructible_v<container_type>);
        auto * const ptr{std::addressof(this->m_cont)};
        std::destroy_at(ptr);
        std::construct_at(ptr, std::move(replacement.m_cont));
      }
      else
      {
        sparse_index replacement{other, this->get_allocator()};
        this->m_cont.swap(replacement.m_cont);
      }
    }
    else
      this->m_cont = other.m_cont;

    return *this;
  }''', 'index copy assignment lifetime and exception safety')

    replace('''        other.m_cont.clear();
      }
    }

    return *this;
  }

  constexpr
  void
  swap(sparse_index &other)''', '''        other.m_cont.clear();
      }
    }

    other.clear();
    return *this;
  }

  constexpr
  void
  swap(sparse_index &other)''', 'index source consistency after move assignment')

    replace('''  constexpr
  sparse_index(sparse_index &&other, allocator_type const &alloc)
    : m_cont{std::move(other.m_cont), container_allocator{alloc}}
  { }''', '''  constexpr
  sparse_index(sparse_index &&other, allocator_type const &alloc)
    : m_cont{std::move(other.m_cont), container_allocator{alloc}}
  { other.clear(); }''', 'index allocator-extended move construction')

    replace('''  constexpr
  sparse_map(sparse_map &&other, allocator_type const &alloc)
    : m_conts{
          .keys  {std::make_obj_using_allocator<key_container_type   >(key_allocator  {alloc}, std::move(other.m_conts.keys))},
          .values{std::make_obj_using_allocator<mapped_container_type>(value_allocator{alloc}, std::move(other.m_conts.values))}}
    , m_index{std::move(other.m_index), key_allocator{alloc}}
  { }''', '''  constexpr
  sparse_map(sparse_map &&other, allocator_type const &alloc)
    : m_conts{
          .keys  {std::make_obj_using_allocator<key_container_type   >(key_allocator  {alloc}, std::move(other.m_conts.keys))},
          .values{std::make_obj_using_allocator<mapped_container_type>(value_allocator{alloc}, std::move(other.m_conts.values))}}
    , m_index{std::move(other.m_index), key_allocator{alloc}}
  { other.clear(); }''', 'map allocator-extended move construction')

    replace('''  constexpr
  sparse_set &
  operator=(sparse_set const &other)
  = default;''', '''  constexpr
  sparse_set &
  operator=(sparse_set const &other)
  {
    if (this == std::addressof(other))
      return *this;

    if constexpr (std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment::value)
    {
      try
      {
        this->m_cont = other.m_cont;
        this->m_index = other.m_index;
      }
      catch (...)
      {
        this->clear();
        throw;
      }
    }
    else
    {
      sparse_set replacement{other, this->get_allocator()};
      this->swap(replacement);
    }
    return *this;
  }''', 'set copy assignment')

    replace('''    this->m_cont  = std::move(other.m_cont);
    this->m_index = std::move(other.m_index);
    return *this;
  }

  constexpr
  sparse_set &
  operator=(std::initializer_list<value_type> ilist)''', '''    try
    {
      this->m_cont  = std::move(other.m_cont);
      this->m_index = std::move(other.m_index);
    }
    catch (...)
    {
      this->clear();
      other.clear();
      throw;
    }
    other.clear();
    return *this;
  }

  constexpr
  sparse_set &
  operator=(std::initializer_list<value_type> ilist)''', 'set move assignment')

    replace('''  constexpr
  sparse_map &
  operator=(sparse_map const &)
  = default;''', '''  constexpr
  sparse_map &
  operator=(sparse_map const &other)
  {
    if (this == std::addressof(other))
      return *this;

    if constexpr (alloc_traits::propagate_on_container_copy_assignment::value)
    {
      try
      {
        this->m_conts = other.m_conts;
        this->m_index = other.m_index;
      }
      catch (...)
      {
        this->clear();
        throw;
      }
    }
    else
    {
      sparse_map replacement{other, this->get_allocator()};
      this->swap(replacement);
    }
    return *this;
  }''', 'map copy assignment')

    replace('''    this->m_conts = std::move(other.m_conts);
    this->m_index = std::move(other.m_index);
    return *this;
  }

  constexpr
  sparse_map &
  operator=(std::initializer_list<value_type> ilist)''', '''    try
    {
      this->m_conts = std::move(other.m_conts);
      this->m_index = std::move(other.m_index);
    }
    catch (...)
    {
      this->clear();
      other.clear();
      throw;
    }
    other.clear();
    return *this;
  }

  constexpr
  sparse_map &
  operator=(std::initializer_list<value_type> ilist)''', 'map move assignment')

    replace('''  constexpr
  container_type
  extract() &&
  noexcept
  { return std::move(this->m_cont); }''', '''  constexpr
  container_type
  extract() &&
  noexcept(std::is_nothrow_move_constructible_v<container_type>)
  {
    container_type extracted{std::move(this->m_cont)};
    this->clear();
    return extracted;
  }''', 'set extract invariant')

    replace('''  constexpr
  containers
  extract() &&
  noexcept
  { return std::move(m_conts); }''', '''  constexpr
  containers
  extract() &&
  noexcept(std::is_nothrow_move_constructible_v<containers>)
  {
    containers extracted{std::move(this->m_conts)};
    this->clear();
    return extracted;
  }''', 'map extract invariant')

    # Avoid forcing an allocator-taking constructor on containers that have no allocator_type.
    replace('''  constexpr
  sparse_set(sparse_set const &other, allocator_type const &alloc)
    : m_cont {other.m_cont , alloc}
    , m_index{other.m_index, alloc}
  { }

  constexpr
  sparse_set(sparse_set &&other, allocator_type const &alloc)
    : m_cont {std::move(other.m_cont ), alloc}
    , m_index{std::move(other.m_index), alloc}
  { }''', '''  constexpr
  sparse_set(sparse_set const &other, allocator_type const &alloc)
    : m_cont {std::make_obj_using_allocator<container_type>(alloc, other.m_cont)}
    , m_index{other.m_index, alloc}
  { }

  constexpr
  sparse_set(sparse_set &&other, allocator_type const &alloc)
    : m_cont {std::make_obj_using_allocator<container_type>(alloc, std::move(other.m_cont))}
    , m_index{std::move(other.m_index), alloc}
  { other.clear(); }''', 'set allocator-extended copy/move construction')

    replace('''  constexpr
  sparse_set(container_type &&cont, allocator_type const &alloc)
    : m_cont {std::move(cont), alloc}
    , m_index{alloc}
  { this->m_unique(); }''', '''  constexpr
  sparse_set(container_type &&cont, allocator_type const &alloc)
    : m_cont {std::make_obj_using_allocator<container_type>(alloc, std::move(cont))}
    , m_index{alloc}
  { this->m_unique(); }''', 'set allocator-extended container move construction')

    replace('''  constexpr
  sparse_set(unique_t, container_type const &cont, allocator_type const &alloc)
    : m_cont {cont        , alloc}
    , m_index{this->m_cont, alloc}
  { }

  constexpr
  sparse_set(unique_t, container_type &&cont, allocator_type const &alloc)
    : m_cont {std::move(cont), alloc}
    , m_index{this->m_cont   , alloc}
  { }''', '''  constexpr
  sparse_set(unique_t, container_type const &cont, allocator_type const &alloc)
    : m_cont {std::make_obj_using_allocator<container_type>(alloc, cont)}
    , m_index{this->m_cont, alloc}
  { }

  constexpr
  sparse_set(unique_t, container_type &&cont, allocator_type const &alloc)
    : m_cont {std::make_obj_using_allocator<container_type>(alloc, std::move(cont))}
    , m_index{this->m_cont, alloc}
  { }''', 'set allocator-extended unique input construction')

    replace('''  explicit constexpr
  sparse_set(allocator_type const &alloc)
  noexcept
    : m_cont {alloc}
    , m_index{alloc}
  { }''', '''  explicit constexpr
  sparse_set(allocator_type const &alloc)
    : m_cont {std::make_obj_using_allocator<container_type>(alloc)}
    , m_index{alloc}
  { }''', 'set no-allocator default construction')

    replace('''  constexpr
  sparse_set(unique_t, InputIt first, InputIt last, allocator_type const &alloc)
    : m_cont {first, last , alloc}
    , m_index{this->m_cont, alloc}
  { }''', '''  constexpr
  sparse_set(unique_t, InputIt first, InputIt last, allocator_type const &alloc)
    : m_cont {std::make_obj_using_allocator<container_type>(alloc, first, last)}
    , m_index{this->m_cont, alloc}
  { }''', 'set no-allocator iterator construction')

    return s, applied


def apply_test_support(original: str) -> tuple[str, list[str]]:
    """Add bounded allocation fault injection used by advanced sparse_index tests."""
    edits = [
        (
            '  bool fail_page_allocation{};\n  bool fail_all_allocation{};',
            '  bool fail_page_allocation{};\n  bool fail_all_allocation{};\n'
            '  std::size_t remaining = std::numeric_limits<std::size_t>::max();',
            'sparse_index allocation budget field',
        ),
        (
            '    if (log->fail_all_allocation || (log->fail_page_allocation && is_page<T>::value))\n'
            '      throw std::bad_alloc{};',
            '    if (log->fail_all_allocation || (log->fail_page_allocation && is_page<T>::value)\n'
            '        || log->remaining == 0)\n'
            '      throw std::bad_alloc{};',
            'sparse_index allocation budget rejection',
        ),
        (
            '    ++log->allocations;\n    if constexpr (is_page<T>::value)',
            '    if (log->remaining != std::numeric_limits<std::size_t>::max())\n'
            '      --log->remaining;\n'
            '    ++log->allocations;\n    if constexpr (is_page<T>::value)',
            'sparse_index allocation budget decrement',
        ),
    ]
    s = original
    applied = []
    for old, new, label in edits:
        if new in s:
            applied.append(label + ' (already applied)')
        elif s.count(old) == 1:
            s = s.replace(old, new)
            applied.append(label)
        else:
            raise RuntimeError('Unexpected sparse_index/support.hpp version: ' + label)
    return s, applied


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true', help='validate patch anchors without modifying files')
    options = parser.parse_args()
    if not SOURCE.is_file():
        sys.exit(f'Cannot locate {SOURCE}; run from a Heim checkout root.')
    before = SOURCE.read_text(encoding='utf-8')
    after, applied = apply(before)
    support_before = TEST_SUPPORT.read_text(encoding='utf-8')
    support_after, support_edits = apply_test_support(support_before)
    if not options.check:
        if after != before:
            SOURCE.write_text(after, encoding='utf-8', newline='')
        if support_after != support_before:
            TEST_SUPPORT.write_text(support_after, encoding='utf-8', newline='')
    print('\n'.join(' - ' + item for item in applied + support_edits))
    print('Checked' if options.check else 'Updated', SOURCE.relative_to(ROOT), 'and', TEST_SUPPORT.relative_to(ROOT))
    if not options.check and after != before:
        generator = ROOT / 'scripts/generate_single_include.py'
        if generator.is_file():
            print('Regenerating single_include distribution headers...')
            subprocess.run([sys.executable, str(generator)], cwd=ROOT, check=True)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
