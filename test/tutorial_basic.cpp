#include <cassert>
#include <strict_variant/variant.hpp>
#include <string>
#include <vector>

using namespace strict_variant;

void
test_one() {

  //[ strict_variant_tutorial_basic
  /*`
  [note In this example code, we will be implicitly `using namespace strict_variant;`.

        I don't recommend doing this in your actual project, it's just for clarity in the tutorial.]

  A `variant` is a special kind of container that contains ['one] value, which may
  be of several possible types. To create a variant, each of the types is passed
  as a template parameter. Any number of types may be specified, up to the implementation-specific
  limits on template complexity.
  */

  variant<int, std::string> v;

  /*`
  This code example declares a variant over the types `int` and `std::string`.

  At all times, `v` will contain either an `int` or ` std::string`.
  If `v` is default constructed, it will attempt to default construct the first
  type in the list. If that type cannot be default constructed, then `variant`
   is not default constructible either.

  You can also initialize it using one of its values:
  */

  variant<int, std::string> u{"foo"};

  /*`
  Intuitively, you can think of `v` as a type-safe union. It has size and
  layout similar to the class:
  */

  /*=

  class my_tagged_union;
    union {
      std::string s;
      int i;
    };
    int which;
  };
  */

  /*`
  where `which` is `0` or `1` depending on which member of the union is currently
  engaged. A `variant` is better than this because it is smart and
  handles the implementation details of `which`, when to call destructors, etc., for you.

  You can change the value held by a variant simply by assigning to it:
  */

  v = 5;
  v = 6;

  v = "foo";
  v = "bar";

  /*`
  The value can be recovered using the `get` function. `get` has syntax similar to
  a pointer cast -- it takes a pointer to a variant, and as a template parameter,
  the desired type. It then returns a pointer to the type, which is `nullptr` if
  that was not the type of the variant.
  */

  v = 5;
  v = 6;
  assert(get<int>(&v));
  assert(!get<std::string>(&v));
  assert(6 == *get<int>(&v));

  v = "foo";
  v = "bar";
  assert(get<std::string>(&v));
  assert(!get<int>(&v));
  assert("bar" == *get<std::string>(&v));

  /*`
  [note In `boost::variant` docs, you will also see a version of `get` which takes
    a reference rather than a pointer, and returns a reference. E.g.

    ```
       int & foo = get<int>(v);
    ```

    This throws a `bad_variant_access` exception if `v` does not currently have type `int`.

    `strict_variant` doesn't have that built-in, but it's trivial to implement it
    yourself if you want it.]
  */

  //<-
}
//->

/*`
[h4 Visitors]

`get` is fine for small cases, but for larger / more sophisticated cases, a better
way to access a `variant` is to use `apply_visitor`.

A *visitor* is a "callable" C++ object, which can be called with any of the types
contained in the variant as a parameter.

Here's an example of a function object, which maps an `int` or a string to a string.
*/

struct formatter {
  std::string operator()(const std::string & s) const { return s; }
  std::string operator()(int i) const { return "[" + std::to_string(i) + "]"; }
};

//` When calling `apply_visitor`, the visitor comes first, and the variant second:
//` `apply_visitor` returns whatever the visitor returns.
//<-
void
test_two(variant<int, std::string> v = {}) {
  //->
  v = 5;
  assert("[5]" == apply_visitor(formatter{}, v));

  v = "baz";
  assert("baz" == apply_visitor(formatter{}, v));
  //<-
}
//->

/*`
One of the selling points of `apply_visitor` as compared to `get` is that it
allows you to turn a class of runtime errors into compile-time errors.

Suppose that you are writing an application using a `variant`, and you realize that
you need to add another type to the variant, having already written much code.

If your code looks like this:
*/

std::string
format_variant(const variant<std::string, int> & v) {
  if (const auto * i = get<int>(&v)) {
    return "[" + std::to_string(*i) + "]";
  } else if (const auto * str = get<std::string>(&v)) {
    return *str;
  } else {
    assert(false && "Unsupported type!");
  }
}

/*`
then when you add a new type, any functions that you didn't update will fail,
but only at runtime, by throwing exceptions or similar.

When you use `apply_visitor` style, if a new type is added that `formatter`
cannot handle, a compile-time error will result. So the compiler will help you
find any code locations that you didn't update after the new type is added.
*/

/*`
[h4 Recursive Variants]

A very handy use of variants has to do with creating *recursive* data structures.
This crops up commonly in parsing problems, for instance when representing an abstract
syntax tree.

For instance, suppose I want to represent an XML tree. I could start like this:
*/

using xml_attribute = std::pair<std::string, std::string>;

/*`
and try to declare a node like this:

```
  struct xml_node {
    std::string name;
    std::vector<xml_attribute> attributes;
    std::vector<variant<std::string, xml_node>> body; // ERROR
  };
```

The trouble with this is that `xml_node` is incomplete at the time that it is used
in the class body of `xml_node`, so it won't actually work. (`variant` really
needs to know the size of anything that it is supposed to contain.)

The class template `recursive_wrapper<T>` can be used to surmount this difficulty.
The argument to `recursive_wrapper` can be an ['incomplete type] and it can still
be used in a `variant` without a problem. (`recursive_wrapper` works by allocating
the object on the heap instead -- it only actually contains a pointer.)
*/

using xml_attribute = std::pair<std::string, std::string>;
struct xml_node;

using xml_variant = variant<std::string, recursive_wrapper<xml_node>>;

struct xml_node {
  std::string name;
  std::vector<xml_attribute> attributes;
  std::vector<xml_variant> body;
};

/*`
See also `boost::spirit` tutorial for an
[@boost:/libs/spirit/doc/html/spirit/qi/tutorials/mini_xml___asts_.html#spirit.qi.tutorials.mini_xml___asts_.the_structures
extended example] using `boost::variant`.

`recursive_wrapper<T>` is, from the user's point of view, the same in `strict_variant` as it is in
`boost::variant`.

There are several ways to define an xml-tree data structure like this -- you can
have a look also at `boost::property_tree` for instance. But using a `variant` like
this is surely one of the simplest, most lightweight, and extensible solutions.

[note We don't provide any analogue of `boost::make_recursive_variant`.]
*/

/*`
[h4 `emplace`]

Another way to assign a value to a `variant` is to use the `emplace` function.

*/

//<-
void
test_three(variant<int, std::string> v = {}) {
  //->
  v.emplace<int>(5);
  v.emplace<int>(6);
  //<-
}
//->

/*`
There are a few reasons to use `emplace` which are outside the scope of the basic
tutorial. But a few of them are:

# If your type is neither moveable nor copyable, you cannot use assignment but
  you may be able to use `emplace`.
# If assignment would be ambiguous, you
  can use `emplace` to explicitly select the type that you want to put in the
  container.
*/

//]

int
main() {
  test_one();
  test_two();
  test_three();
}
