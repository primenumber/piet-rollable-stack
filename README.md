PietRollableStack
====

An Efficient Data Structure for the Piet Programming Language

## Overview

`PietStack` supports these ordinal stack operations(see: [Stack (abstract data type) - Wikipedia](https://en.wikipedia.org/wiki/Stack\_\(abstract\_data\_type\)))

- `push(element)`, which adds an element to the collection
- `pop`, which removes the most recently added element that was not yet removed

In addition to that, it supports special operation for Piet programming language:

- `roll(depth, count)`, which repeats the following operaion `count` times: burying the top value on the stack `depth` deep and bringing all values above it up by 1 place
    - Note: when `count` is negative, which repeats the *reverse* operation `abs(count)` times.

## Computational Cost

N: length of the stack

| operation | cost |
|----|----|
|push|amortized O(1)|
|pop|amortized O(1)|
|roll|O(log N)|
