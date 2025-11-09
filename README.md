# CStreams

A simple, Java-inspired Streams API for C.

This library provides a basic framework for processing sequences of data using a pipeline of operations, similar to Java's `Stream` API.

## Features

* **Extensible**: Use any data source by providing two simple functions.
* **Filter**: Filter elements based on a predicate.
* **Map**: Transform elements from one value to another.
* **For Each**: Terminal operation to execute an handler for each element in the stream.
* **To Array**: Terminal operation to create a new array with the elements in the stream.

## How to Build
This is a library, not a standalone executable; to use it, you just need to compile your main.c with cstreams.

## How to Use
Take a look at the `examples` directory
