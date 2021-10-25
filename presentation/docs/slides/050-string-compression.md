## String Compression

Lets make a compressed string table

- https://github.com/AshleyRoll/squeeze
- map from enum Key to Compressed String
  <!-- .element: class="fragment" -->
- Huffman Coding for compression
  <!-- .element: class="fragment" -->
- Output struct:
  <!-- .element: class="fragment" -->
  - Mapping of Key -> bit stream location/length
  <!-- .element: class="fragment" -->
  - Compressed bitstream
  <!-- .element: class="fragment" -->
  - Huffman tree encoded into array
  <!-- .element: class="fragment" -->
  - Lookup -> iterator over compressed data
  <!-- .element: class="fragment" -->

Note:
- Will only touch on the code as it is quite complex
- Suggest looking at repo is interested



<!-- down -->
```C++
enum class Key { String_1, String_2, String_3 };

static constexpr auto buildMapStrings = [] {
    return std::to_array<squeeze::KeyedStringView<Key>>({
      // out of order and missing a key
      { Key::String_3, "There is little point to using short strings in a compressed string table." },
      { Key::String_1, "We will include some long strings in the table to test it." },
    });
};

constexpr auto map = squeeze::StringMap<Key>(buildMapStrings);

int main()
{
    static constexpr auto str1 = map.get(Key::String_1);
    for (auto const &c : str1) {
        std::putchar(c);
    }

    std::putchar('\n');
    return 0;
}

```
<!-- .element: class="r-stretch" -->



<!-- down -->
### Overview

- Under the StringMap is a StringTable - simple indexed access to compressed string
  <!-- .element: class="fragment" -->
- Wrap that with mapping Key -> index
  <!-- .element: class="fragment" -->
- Both StringMap and StringTable are usable
  <!-- .element: class="fragment" -->
- Strings (and keys) are passed to library using lambda providing
  std::string_view
  <!-- .element: class="fragment" -->
- Iterator interface to avoid need to decompress full string
  <!-- .element: class="fragment" -->
- Useful for long strings - minimum metadata size
  <!-- .element: class="fragment" -->

Note:
- Currently std::size_t, but could allow user defined index type
  which is smaller
- Can optimise storage size a little more



<!-- down -->
```text
$ objdump -s --section=.compressedmap example

example:     file format elf64-x86-64

Contents of section .compressedmap:
 20c0 00000000 00000000 01000000 00000000  ................
 20d0 02000000 00000000 00000000 00000000  ................
 20e0 00000000 00000000 4a000000 00000000  ........J.......
 20f0 24010000 00000000 3a000000 00000000  $.......:.......
 2100 85fbe507 2e60c7ef 50b4675d 4e741d6f  .....`..P.g]Nt.o
 2110 158f1674 e5d03b97 687ac893 5f8e1674  ...t..;.hz.._..t
 2120 9dcf2cfe 5afc5d10 31741279 f9a3a6bf  ..,.Z.].1t.y....
 2130 d0aea305 5d39f47c ff7c66f1 b3ce4f0f  ....]9.|.|f...O.
 2140 be020100 02000100 03000400 01000500  ................
 2150 06000100 07000800 01000900 0a000100  ................
 2160 0b000c00 01000d00 0e000100 69000000  ............i...
 2170 00007300 00000000 0f001000 01007400  ..s...........t.
 2180 00000000 11001200 01001300 14000100  ................
 2190 20000000 00001500 16000100 72000000   ...........r...
 21a0 00006f00 00000000 6c000000 00001700  ..o.....l.......
 21b0 18000100 19001a00 01006e00 00000000  ..........n.....
 21c0 1b001c00 01006500 00000000 1d001e00  ......e.........
 21d0 01001f00 20000100 21002200 01006700  .... ...!."...g.
 21e0 00000000 23002400 01002500 26000100  ....#.$...%.&...
 21f0 63000000 00006d00 00000000 62000000  c.....m.....b...
 2200 00007500 00000000 27002800 01006400  ..u.....'.(...d.
 2210 00000000 70000000 00006100 00000000  ....p.....a.....
 2220 29002a00 01006800 00000000 54000000  ).*...h.....T...
 2230 00005700 00000000 77000000 00002e00  ..W.....w.......
 2240 00000000 00000000                    ........
```
<!-- .element: class="r-stretch" -->



<!-- down -->
### Huffman coding (overview)

- Optimal encoding given the list of symbols and their frequency
  <!-- .element: class="fragment" -->
- symbols are all characters in all strings
  <!-- .element: class="fragment" -->
- Uses a min-heap priority_queue to build a tree bottom up
  from least used to most used
  <!-- .element: class="fragment" -->
- Bit pattern assigned by walking down the tree
  Most common symbols have shortest bit pattern
  <!-- .element: class="fragment" -->



<!-- down -->
![from Wikipedia](https://upload.wikimedia.org/wikipedia/commons/8/82/Huffman_tree_2.svg)
<!-- .element: class="r-stretch" -->
"this is an example of a huffman tree"
https://en.wikipedia.org/wiki/Huffman_coding



<!-- down -->
### Library Process

- Build character frequency table
  <!-- .element: class="fragment" -->
- Build Huffman tree
  <!-- .element: class="fragment" -->
- Build char -> bitstream cache
  <!-- .element: class="fragment" -->
- Build encoded bit stream
  <!-- .element: class="fragment" -->
- Build index -> bit position / string length lookup
  <!-- .element: class="fragment" -->
- Package Huffman tree to array
  <!-- .element: class="fragment" -->
- Combine Huffman tree, bit stream and index lookup
  into result
  <!-- .element: class="fragment" -->

Note:
- Suggest looking at code for algorithm
- Highlights to follow



<!-- down -->
### Constrain Lambdas

```C++
template<typename TKey>
struct KeyedStringView
{
    TKey Key;
    std::string_view Value;
};

template<typename T>
concept CallableGivesIterableStringViews = requires(T t) {
    t();
    std::input_iterator<decltype(t().begin())>;
    std::is_same_v<decltype(t().begin()), std::string_view>;
};

template<typename T, typename K>
concept CallableGivesIterableKeyedStringViews = requires(T t, K k)
{
    t();
    std::input_iterator<decltype(t().begin())>;
    std::is_same_v<decltype(t().begin()), KeyedStringView<K>>;
};
```
<!-- .element: class="r-stretch" -->

Note:
- provides library user guidance on what to pass into auto property



<!-- down -->
### Map to Table

```C++
template<typename TKey>
static constexpr auto MapToStrings(
    CallableGivesIterableKeyedStringViews<TKey> auto f
    ) -> CallableGivesIterableStringViews auto
{
    return [=]() {
        constexpr auto stringmap = f();
        constexpr auto NumStrings =
            std::distance(stringmap.begin(), stringmap.end());

        std::array<std::string_view, NumStrings> result;
        std::size_t idx{0};
        for(auto const &v : stringmap) {
            result.at(idx++) = v.Value;
        }
        return result;
    };
}
```
<!-- .element: class="r-stretch" -->

Note:
- copy capture for `f` input lambda



<!-- down -->
### Modularizing Code

- Need to calculate compile-time constants from user data
  <!-- .element: class="fragment" -->
- Can't pass from one function to another as parameters - not constant
  <!-- .element: class="fragment" -->
- The user supplied lambda needs re-evaluation in each context
  <!-- .element: class="fragment" -->
- Can define local lambdas to help but results in long methods
  <!-- .element: class="fragment" -->
- Still results in large chunks of code, can't see a better way
  <!-- .element: class="fragment" -->



<!-- down -->
```C++

static constexpr auto BuildHuffmanTree(
    CallableGivesIterableStringViews auto makeStringsLambda)
{
    constexpr auto st = makeStringsLambda();
    // ...
    return tree;
}

static constexpr auto MakeEncodedBitStream(
    CallableGivesIterableStringViews auto makeStringsLambda)
{
    constexpr auto st = makeStringsLambda();
    constexpr auto NumStrings = std::distance(st.begin(), st.end());

    constexpr auto tree = BuildHuffmanTree(makeStringsLambda);

    ResultType<NumStrings> stream;
    // fill output stream...
    return stream
}
```
<!-- .element: class="r-stretch" -->

Note:
- call makeStringLambda in each context



<!-- down -->
```C++
constexpr auto charLookup = MakeCharacterLookupTable();

constexpr auto CalculateStringLength =
    [=](std::string_view s) -> std::size_t {
    std::size_t len{0};
    for(char const c : s) {
        len += charLookup.at(static_cast<std::size_t>(c)).BitLength;
    }
    return len;
};

constexpr auto CalculateEncodedStringBitLengths = [=]() {
    std::array<std::size_t, NumStrings> result;
    std::size_t i{0};
    for(auto const &s : st) {
        result.at(i) = CalculateStringLength(s);
        ++i;
    }
    return result;
};

constexpr auto stringLengths = CalculateEncodedStringBitLengths();
constexpr auto totalEncodedLength =
    std::accumulate(stringLengths.begin(), stringLengths.end(), 0);
```
<!-- .element: class="r-stretch" -->

Note:
- function local lambda "modularize" code
- can access previous data (but captured by copy)
- can use std::accumulate and other algorithms to compute
  constexpr values



<!-- down -->
### Using &lt;algorithm&gt;s

- Able to use most algorithms, eg:
  - std::sort
  <!-- .element: class="fragment" -->
  - std::push_heap / std::pop_heap
  <!-- .element: class="fragment" -->
  - std::count_if
  <!-- .element: class="fragment" -->
  - std::difference
  <!-- .element: class="fragment" -->
  - std::accumulate
  <!-- .element: class="fragment" -->



<!-- down -->
### But not containers

- except std::array
- std::vector waiting for compiler support
  <!-- .element: class="fragment" -->
- had to build my own
  <!-- .element: class="fragment" -->
  - priority_queue
  <!-- .element: class="fragment" -->
  - list
  <!-- .element: class="fragment" -->
  - bit_stream (std::bitset?)
  <!-- .element: class="fragment" -->
- More constexpr containers please!
  <!-- .element: class="fragment" -->



<!-- down -->
### Heap Allocations

- Can allocate heap as long as it is freed before leaving context
- Means you can't return it, even to another constexpr context
  <!-- .element: class="fragment" -->
- Used list implementation as a queue for breadth first tree traversal
  when laying out nodes in output array
  <!-- .element: class="fragment" -->
- Was easier to count required nodes and allocate std::array
  to hold all nodes making the tree (std::vector please!)
  <!-- .element: class="fragment" -->
- More container support would make this much easier
  <!-- .element: class="fragment" -->



<!-- down -->
### Limits

- The compilers choose an arbitrary amount of work
  they will allow in constexpr context
- Complex processing like compression will hit the limits
  <!-- .element: class="fragment" -->
- Had to make more complex implementation to cache bit streams
  rather than walk tree for each character
  <!-- .element: class="fragment" -->
- Still possible to hit limits on large strings
  <!-- .element: class="fragment" -->
- -fconstexpr-ops-limit=VERY_BIG_NUMBER
  <!-- .element: class="fragment" -->

