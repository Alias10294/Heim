# property_map - Heim: Properties
## Synopsis
```cpp
template<typename Get, typename Set>
class property_map;
```

## Overview
`property_map` is a class template implementing the library *property map* concept, that uses a pair of function 
objects as *getter* and *setter* to provide genericity.

## Template Parameters
| Name  | Description                     |
|:------|:--------------------------------|
| `Get` | The getter function object type |
| `Set` | The setter function object type |

## Member Types
| Name          | Value |
|:--------------|:------|
| `getter_type` | `Get` |
| `setter_type` | `Set` |

## Member Functions
| Name                                 | Description                                       |
|:-------------------------------------|:--------------------------------------------------|
| [(constructor)](constructor.md)      | Initializes the `property_map`                    |
| (destructor) (*implicitly declared*) | Destroys the `property_map`                       |
| `operator=` (*implicitly declared*)  | Assigns values to the object                      |
| [`get`](get.md)                      | Returns the property associated with a given key  |
| [`set`](set.md)                      | Modifies the property associated with a given key |

## Non-member Functions
| Name                                                          | Description                                                     |
|:--------------------------------------------------------------|:----------------------------------------------------------------|
| [`make_property_map`](make_property_map.md)                   | Creates a property map object using specific arguments          |
| [`make_readable_property_map`](make_readable_property_map.md) | Creates a readable property map object using specific arguments |
| [`make_writable_property_map`](make_writable_property_map.md) | Creates a writable property map object using specific arguments |

## Non-member Constants
| Name                                                                             | Description                                                                                                                                                           |
|:---------------------------------------------------------------------------------|:----------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| [`no_property_getter`](no_property.md)<br>[`no_property_setter`](no_property.md) | Denote the construction of readable/writable-only `property_map` objects                                                                                              |
| [`identity_arg`](identity_arg.md)                                                | Creates a readable map that returns as property its argument unchanged                                                                                                |
| [`constant_arg`](constant_arg.md)                                                | Creates a readable map that returns the same value no matter the key                                                                                                  |
| [`assocative_container_arg`](associative_container_arg.md)                       | Creates a map that references the given associative container                                                                                                         |
| [`random_access_iterator_arg`](random_access_iterator_arg.md)                    | Creates a map that associates keys with elements of a random-access sequence by using them, directly or through a given index map, as offsets from the given iterator |
| [`random_access_range_arg`](random_access_range_arg.md)                          | Creates a map that associates keys with elements of the given random-access range by using them, directly or through a given index map, as offsets                    |
| [`member_arg`](member_arg.md)                                                    | Creates a map that maps objects to one of their member objects or to the return type of one of their nullary member functions                                         |
| [`pointer_arg`](pointer_arg.md)                                                  | Creates a map that maps pointers to their pointed-to object                                                                                                           |
| [`invocable_arg`](invocable_arg.md)                                              | Creates a map that maps objects to the result of the given function with them as the argument                                                                         |
| [`composable_arg`](composable_arg.md)                                            | Creates a map that composes the two given property maps                                                                                                               |

## See Also
| Name                                                   | Description                                                                             |
|:-------------------------------------------------------|:----------------------------------------------------------------------------------------|
| [`properties::is_map_for`](../../traits/is_map_for.md) | Specifies that a type is both a readable and writable property map type for given types |
