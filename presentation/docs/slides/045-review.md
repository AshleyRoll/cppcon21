## Where to next?

- We've created simple resources of known size/type
  <!-- .element: class="fragment" -->
- What about varying the output size/type based on the input?
  <!-- .element: class="fragment" -->
  - A USB configuration descriptor can have many interfaces,
  <!-- .element: class="fragment" -->
  - and each interface can have many endpoints
  <!-- .element: class="fragment" -->
  - Compressed string data depends on the content of the string, not its length
  <!-- .element: class="fragment" -->
- How do we build easy to use libraries?
  <!-- .element: class="fragment" -->



<!-- down -->
## Return Size & Type

- The size of an array or template parameters of a type
  must be compile-time constants.
  <!-- .element: class="fragment" -->
- We must be able to calculate these values using constexpr
  functions
  <!-- .element: class="fragment" -->
- This means the compiler has to use type information, it can't
  use parameters to functions.
  <!-- .element: class="fragment" -->
- How can we pass user-supplied data into a constexpr function?
  <!-- .element: class="fragment" -->
  - Lambdas!
    <!-- .element: class="fragment" -->



<!-- down -->
## Lambdas for the win!

- A lambda's call operator can be constexpr
  <!-- .element: class="fragment" -->
- Each lambda is a unique type and its return type is known
  at compile-time
  <!-- .element: class="fragment" -->
- Therefore we can write helper methods that take a user-suppiled
  lambda to generate the data needed to render our desired compile-time
  resource!
  <!-- .element: class="fragment" -->
- These are effectively templated functions, but we will use the
  cleaner auto parameter syntax for our helper functions
  <!-- .element: class="fragment" -->



<!-- down -->
## Library sketch

```C++

// library
constexpr auto every_second_item(auto lambda)
{
    constexpr auto data = lambda();
    constexpr auto length = data.size() / 2;
    std::array<int, length> output;

    for(std::size_t i = 0; i < output.size(); ++i)
        output[i] = data[i*2];

    return output;
}

// invoke
constexpr auto test = every_second_item([] {
    return std::to_array<int>({1, 2, 3, 4, 5, 6});
});

int main() { return test.size(); } // == 3

```
<!-- .element class="r-stretch" -->

