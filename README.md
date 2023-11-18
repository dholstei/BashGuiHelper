# BashQtHelper
Set of minimalist GUI `(Qt)` tools written in **`C++`** to be used by `bash` to:
- Browse files
- Enter qualified data, such as IP, numeric, strings, etcetera
- Make list selections
- Make tree selections
## `LibXML2` Class Library
In order to allow tree browsing, I use `XML/XPath` as the tree DB.

`xmllint` is already a great XML tool for `bash`
In addition to the DOM document and node classes, the most useful component of the class library is the `XPathOBJ` class:
- Provide a simple means to resolve XPath queries
- Handles errors for both XML Parsing and XPath queries
- Has type-specific methods (*with specific error handling*) for:
  - Integers
  - Floating point
  - Boolean
  - String
  - Node sets
## Building
Use the Makefile

Or, open the folder with VS Code and hit `F5`
## Status
Not yet usable, development emphasis is good architecture
