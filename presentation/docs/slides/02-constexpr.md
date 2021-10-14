## constexpr in Brief

- Specifies a variable or function CAN appear in a constant expression
  <!-- .element: class="fragment" data-fragment-index="1" -->
- Constant expressions can be evaluated at compile-time
  <!-- .element: class="fragment" data-fragment-index="2" -->
- eg: <!-- .element: class="fragment" data-fragment-index="3" -->
  - Array size
    <!-- .element: class="fragment" data-fragment-index="3" -->
  - non-type template parameter
    <!-- .element: class="fragment" data-fragment-index="4" -->

Note:
- Before C++20 there are many more restrictions
- Refer to Jason Turner's Talk earlier today and  C++ Weekly Youtube channel


<!-- down -->
## constexpr Variable

- Must be a Literal Type <!-- .element: class="fragment" -->
  - scalar (int, char, etc) <!-- .element: class="fragment" -->
  - array of literals <!-- .element: class="fragment" -->
  - struct/class/union (with some constraints) <!-- .element: class="fragment" -->
  - a closure type (lambda) <!-- .element: class="fragment" -->
- Must be immediately initialised <!-- .element: class="fragment" -->

```C++
constexpr int BASE{0b0010'1000};
constexpr std::array<int, 3> VALUES{1, 2, 3};

int main()
{
    return BASE + VALUES[1];
}
```
<!-- .element: class="fragment" -->



<!-- down -->
## constexpr Function

- Must return a Literal Type (or void) <!-- .element: class="fragment" -->
  - must ensure all members of return value are initialised <!-- .element: class="fragment" -->
- Can be a constructor <!-- .element: class="fragment" -->
  - must initialise all members of object <!-- .element: class="fragment" -->
- parameters must be a literal type (if any) <!-- .element: class="fragment" -->

```C++
constexpr int make_bigger(int v)
{
    return v * 100;
}
```
<!-- .element: class="fragment" -->

Note:
- as of C++20, can contain most code
- can even allocate memory as long as it is freed before return

---

## consteval

- Specifies that a function MUST produce a compile-time constant <!-- .element: class="fragment" -->
- Implies constexpr <!-- .element: class="fragment" -->

---

## constinit

- Specifies a variable has static initialization <!-- .element: class="fragment" -->
  - zero initialised (not that useful for us) <!-- .element: class="fragment" -->
  - constant initialised (from a constant expression) <!-- .element: class="fragment" -->
- computed at compile-time, and stored in program binary <!-- .element: class="fragment" -->
  - can be mutable or const <!-- .element: class="fragment" -->

```C++
constinit int foo = 10;
constinit const bar = 1;

int main()
{
    foo += 1;
    // bar += 1;  // ERROR
}

```
 <!-- .element: class="fragment" -->

