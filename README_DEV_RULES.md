# Merge branches

If you want to merge a branch `some-branch` into `some-other-branch`, use the below command:

```
git checkout some-other-branch
git merge --no-ff some-branch    # No fast-forward

# Update it in the remote repository
git push origin some-branch
```

# Naming rules

The most important consistency rules are those that govern naming. The style of a name immediately informs us what sort of thing the named entity is: a type, a variable, a function, a constant, a macro, etc., without requiring us to search for the declaration of that entity. The pattern-matching engine in our brains relies a great deal on these naming rules.

Naming rules are pretty arbitrary, but we feel that consistency is more important than individual preferences in this area, so regardless of whether you find them sensible or not, the rules are the rules.

- [Naming rules](#naming-rules)
  - [Type names](#type-names)
  - [Variable names](#variable-names)
  - [Constant names](#constant-names)
  - [Function names](#function-names)
  - [Macro names](#macro-names)
  - [Exceptions to naming rules](#exceptions-to-naming-rules)


## Type names

Type names start with a capital letter and have a capital letter for each new word, with no underscores: `SomeTypedef`, `SomeClass`, `SomeStruct`, `SomeEnum`, `SomeEnumClass`.

The names of all types — classes, structs, type aliases, enums, and type template parameters — have the same naming convention. Type names should start with a capital letter and have a capital letter for each new word. No underscores. For example:

```
// classes and structs
class SomeClass { ...
struct SomeStruct { ...

// typedefs
typedef std::map<SomeKeyTp, std::string> SomeTypedef;

// using aliases
template <typename SomeValTp>
using SomeTypeAlias = std::map<SomeKeyTp, SomeValTp>;

// enums
enum class SomeEnum { ...
```

## Variable names

The names of variables (including function parameters) and data members are all lowercase, with underscores between words. Data members of classes (but not structs) additionally have trailing underscores. For instance: `local_variable`, `struct_data_member`, `class_data_member_`.

(Use `this->` when using the structure member variable in the member function definition.)

## Constant names

Variables declared `constexpr` or `const`, and whose value is fixed for the duration of the program, are named with a leading `k` followed by all lower case with underscores between words. Underscores can be used as separators in the rare cases where capitalization cannot be used for separation. For example:

```
const int k_bytes_in_block_l1i = 8;
```

## Function names

All functions names are all lowercase, with underscores between words.

```
// regular function
void some_function(...);

// member function
void SomeType::some_member_function(...);
```

When defining the getter and setter, keep the following example:

```
class SomeClass {
private:
    std::int64_t some_member_variable_;
public:
    // bad
    std::int64_t get_some_member_variable() const {
        return some_member_variable_;
    }
    // good
    std::int64_t some_member_variable() const {
        return some_member_variable_;
    }
    // good
    std::int64_t set_some_member_variable(const std::int64_t& val) {
        some_member_variable_ = val;
    }
};
```

## Macro names

Macro names should be named with all capitals and underscores.

```
#define ROUND(x) ...
#define PI_ROUNDED 3.0
```

## Exceptions to naming rules

If you are naming something that is analogous to an existing C or C++ entity then you can follow the existing naming convention scheme.

```
bigopen()         // function name, follows form of open()
uint              // typedef
bigpos            // struct or class, follows form of pos
sparse_hash_map   // STL-like entity; follows STL naming conventions
```